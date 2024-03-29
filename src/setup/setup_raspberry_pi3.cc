// EPOS Raspberry Pi3 (Cortex-A53) SETUP

#include <architecture.h>
#include <machine.h>
#include <utility/elf.h>
#include <utility/string.h>

extern "C" {
    void _start();
    void _bss_clear();

    // SETUP entry point is the Vector Table and resides in the .init section (not in .text), so it will be linked first and will be the first function after the ELF header in the image.
    void _entry() __attribute__ ((used, section(".init")));
    void _vector_table() __attribute__ ((used)); // 
    void _reset(); // so it can be safely reached from the vector table
    void _setup(); // just to create a Setup object

    // LD eliminates this variable while performing garbage collection, so --undefined=__boot_time_system_info must be present while linking
    char __boot_time_system_info[sizeof(EPOS::S::System_Info)] = "System_Info placeholder. Actual System_Info will be added by mkbi!";
}

__BEGIN_SYS

extern OStream kout, kerr;

class Setup
{
private:
    // Physical memory map
    static const unsigned int RAM_BASE          = Memory_Map::RAM_BASE;
    static const unsigned int RAM_TOP           = Memory_Map::RAM_TOP;
    static const unsigned int IMAGE             = Memory_Map::IMAGE;
    static const unsigned int SETUP             = Memory_Map::SETUP;
    static const unsigned int PAGE_TABLES       = (Memory_Map::RAM_TOP - 4 * 4096) & ~(0x3FFF); // 16KB for 4K entries of 4B each. Moreover, we need 16K aligned TTBR entry
    static const unsigned int VECTOR_TABLE      = Traits<Build>::EXPECTED_SIMULATION_TIME ? 0x00010000 : 0x00008000;   // defined by uboot@QEMU

    // Logical memory map
    static const unsigned int APP_LOW           = Memory_Map::APP_LOW;
    static const unsigned int PHY_MEM           = Memory_Map::PHY_MEM;
    static const unsigned int IO                = Memory_Map::IO;
    static const unsigned int SYS               = Memory_Map::SYS;
    static const unsigned int SYS_INFO          = Memory_Map::SYS_INFO;
    static const unsigned int SYS_PT            = Memory_Map::SYS_PT;
    static const unsigned int SYS_PD            = Memory_Map::SYS_PD;
    static const unsigned int SYS_CODE          = Memory_Map::SYS_CODE;
    static const unsigned int SYS_DATA          = Memory_Map::SYS_DATA;
    static const unsigned int SYS_STACK         = Memory_Map::SYS_STACK;
    static const unsigned int APP_CODE          = Memory_Map::APP_CODE;
    static const unsigned int APP_DATA          = Memory_Map::APP_DATA;

    // Architecture Imports
    typedef CPU::Reg Reg;
    typedef CPU::Phy_Addr Phy_Addr;
    typedef CPU::Log_Addr Log_Addr;
    typedef MMU::Page Page;
    typedef MMU::Page_Flags Flags;
    typedef MMU::Page_Table Page_Table;
    typedef MMU::Page_Directory Page_Directory;
    typedef MMU::PT_Entry PT_Entry;
    typedef MMU::PD_Entry PD_Entry;

    // System_Info Imports
    typedef System_Info::Boot_Map BM;
    typedef System_Info::Physical_Memory_Map PMM;
    typedef System_Info::Load_Map LM;

private:
    // TTBR0 Page Table Entry Descriptor for Sections configuration --> One level translation for Flat Mapping
    enum {
        TTB_MEMORY_DESCRIPTOR           = 0x90c0e,
        TTB_DEVICE_DESCRIPTOR           = 0x90c0a,
        TTB_PERIPHERAL_DESCRIPTOR       = 0x90c16
    };

public:
    Setup();

private:
    void build_lm();
    void build_pmm();

    void say_hi();

    void setup_pt(PT_Entry * pts, Phy_Addr base, unsigned int size, unsigned int n_pts, Flags flag, bool print = false);
    void setup_sys_pt();
    void setup_app_pt();
    void setup_sys_pd();

    void setup_flat_paging();

    void enable_paging();

    void load_parts();
    void adjust_perms();
    void call_next();

    void panic() { Machine::panic(); }

private:
    char * bi;
    System_Info * si;

    static volatile bool paging_ready;
};

volatile bool Setup::paging_ready = false;

Setup::Setup()
{
    CPU::int_disable(); // interrupts will be re-enabled at init_end
    Display::init();

    bi = reinterpret_cast<char *>(IMAGE);
    si = reinterpret_cast<System_Info *>(&__boot_time_system_info);

    db<Setup>(TRC) << "Setup(bi=" << reinterpret_cast<void *>(bi) << ",sp=" << CPU::sp() << ")" << endl;
    db<Setup>(INF) << "Setup:si=" << *si << endl;

    if(si->bm.n_cpus > Traits<Machine>::CPUS)
        si->bm.n_cpus = Traits<Machine>::CPUS;

    if(CPU::id() == 0) { // bootstrap CPU (BSP)

        if(Traits<System>::multitask) {

            // Build the memory model
            build_lm();
            build_pmm();

            // Print basic facts about this EPOS instance
            // say_hi();

            // Configure the memory model defined above
            setup_sys_pt();
            setup_app_pt();
            setup_sys_pd();

            // Enable paging
            enable_paging();

            // Load EPOS parts (e.g. INIT, SYSTEM, APPLICATION)
            load_parts();

        } else { // library mode

            // Configure a flat memory model for the single task in the system
            setup_flat_paging();

            // Enable paging
            enable_paging();
        }

        // Signalizes other CPUs that paging is up
        paging_ready = true;

    } else { // additional CPUs (APs)

        // Wait for the Boot CPU to setup page tables
        while(!paging_ready);
        enable_paging();
    }

    // SETUP ends here, so let's transfer control to next stage (INIT or APP)
    call_next();
}

void Setup::setup_flat_paging()
{
    volatile CPU::Reg * pt = reinterpret_cast<volatile CPU::Reg *>(PAGE_TABLES);
    for(int i = 0; i <= 1006; i++)
        pt[i] = TTB_MEMORY_DESCRIPTOR | (i << 20);
    pt[1007] = TTB_DEVICE_DESCRIPTOR | (1007 << 20);
    for(int i = 1008; i <= 4095; i++)
        pt[i] = TTB_PERIPHERAL_DESCRIPTOR | (i << 20);
}

void Setup::build_lm()
{
    db<Setup>(TRC) << "Setup::build_lm()" << endl;

    // Get boot image structure
    si->lm.has_stp = (si->bm.setup_offset != -1u);
    si->lm.has_ini = (si->bm.init_offset != -1u);
    si->lm.has_sys = (si->bm.system_offset != -1u);
    si->lm.has_app = (si->bm.application_offset != -1u);
    si->lm.has_ext = (si->bm.extras_offset != -1u);

    // Check SETUP integrity and get the size of its segments
    si->lm.stp_entry = 0;
    si->lm.stp_segments = 0;
    si->lm.stp_code = ~0U;
    si->lm.stp_code_size = 0;
    si->lm.stp_data = ~0U;
    si->lm.stp_data_size = 0;
    if(si->lm.has_stp) {
        ELF * stp_elf = reinterpret_cast<ELF *>(&bi[si->bm.setup_offset]);
        if(!stp_elf->valid()) {
            db<Setup>(ERR) << "SETUP ELF image is corrupted!" << endl;
            panic();
        }

        si->lm.stp_entry = stp_elf->entry();
        si->lm.stp_segments = stp_elf->segments();
        si->lm.stp_code = stp_elf->segment_address(0);
        si->lm.stp_code_size = stp_elf->segment_size(0);
        if(stp_elf->segments() > 1) {
            for(int i = 1; i < stp_elf->segments(); i++) {
                if(stp_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(stp_elf->segment_address(i) < si->lm.stp_data)
                    si->lm.stp_data = stp_elf->segment_address(i);
                si->lm.stp_data_size += stp_elf->segment_size(i);
            }
        }
    }

    // Check INIT integrity and get the size of its segments
    si->lm.ini_entry = 0;
    si->lm.ini_segments = 0;
    si->lm.ini_code = ~0U;
    si->lm.ini_code_size = 0;
    si->lm.ini_data = ~0U;
    si->lm.ini_data_size = 0;
    if(si->lm.has_ini) {
        ELF * ini_elf = reinterpret_cast<ELF *>(&bi[si->bm.init_offset]);
        if(!ini_elf->valid()) {
            db<Setup>(ERR) << "INIT ELF image is corrupted!" << endl;
            panic();
        }

        si->lm.ini_entry = ini_elf->entry();
        si->lm.ini_segments = ini_elf->segments();
        si->lm.ini_code = ini_elf->segment_address(0);
        si->lm.ini_code_size = ini_elf->segment_size(0);
        if(ini_elf->segments() > 1) {
            for(int i = 1; i < ini_elf->segments(); i++) {
                if(ini_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(ini_elf->segment_address(i) < si->lm.ini_data)
                    si->lm.ini_data = ini_elf->segment_address(i);
                si->lm.ini_data_size += ini_elf->segment_size(i);
            }
        }
    }

    // Check SYSTEM integrity and get the size of its segments
    si->lm.sys_entry = 0;
    si->lm.sys_segments = 0;
    si->lm.sys_code = ~0U;
    si->lm.sys_code_size = 0;
    si->lm.sys_data = ~0U;
    si->lm.sys_data_size = 0;
    si->lm.sys_stack = SYS_STACK;
    si->lm.sys_stack_size = Traits<System>::STACK_SIZE * si->bm.n_cpus;
    if(si->lm.has_sys) {
        ELF * sys_elf = reinterpret_cast<ELF *>(&bi[si->bm.system_offset]);
        if(!sys_elf->valid()) {
            db<Setup>(ERR) << "OS ELF image is corrupted!" << endl;
            panic();
        }

        si->lm.sys_entry = sys_elf->entry();
        si->lm.sys_segments = sys_elf->segments();
        si->lm.sys_code = sys_elf->segment_address(0);
        si->lm.sys_code_size = sys_elf->segment_size(0);
        if (sys_elf->segments() > 1) {
            if (sys_elf->segment_address(1) < si->lm.sys_code) {
                si->lm.sys_code = sys_elf->segment_address(1);
                si->lm.sys_code_size += sys_elf->segment_size(1);
            } else {
                si->lm.sys_data = sys_elf->segment_address(1);
                si->lm.sys_data_size = sys_elf->segment_size(1);
            }
        }

        db<Setup>(INF) << "SYS Segments: " << sys_elf->segments()<< endl;

        if(sys_elf->segments() > 2) {
            for(int i = 2; i < sys_elf->segments(); i++) {
                if(sys_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(sys_elf->segment_address(i) < si->lm.sys_data)
                    si->lm.sys_data = sys_elf->segment_address(i);
                si->lm.sys_data_size += sys_elf->segment_size(i);
            }
            db<Setup>(INF) << "SYS Segments[0]: " << reinterpret_cast<void *>(sys_elf->segment_address(0)) << ", size="<< sys_elf->segment_size(0) << endl;
            db<Setup>(INF) << "SYS Segments[1]: " << reinterpret_cast<void *>(sys_elf->segment_address(1)) << ", size="<< sys_elf->segment_size(1) << endl;
            db<Setup>(INF) << "SYS Segments[2]: " << reinterpret_cast<void *>(sys_elf->segment_address(2)) << ", size="<< sys_elf->segment_size(2) << endl;
        }

        // CODE and DATA Segments are concatenated, only code seg is available...
        if(si->lm.sys_code != SYS_CODE) {
            db<Setup>(ERR) << "OS code segment address (" << reinterpret_cast<void *>(si->lm.sys_code) << ") does not match the machine's memory map (" << reinterpret_cast<void *>(SYS_CODE) << ")!" << endl;
            panic();
        }

        if(si->lm.sys_code + si->lm.sys_code_size > si->lm.sys_data) {
            db<Setup>(ERR) << "OS code segment is too large!" << endl;
            panic();
        }

        if(si->lm.sys_data != SYS_DATA) {
            db<Setup>(ERR) << "OS data segment address (" << reinterpret_cast<void *>(si->lm.sys_data) << ") does not match the machine's memory map (" << reinterpret_cast<void *>(SYS_DATA) << ")!" << endl;
            panic();
        }

        if(si->lm.sys_data + si->lm.sys_data_size > si->lm.sys_stack) {
            db<Setup>(ERR) << "OS data segment is too large!" << endl;
            panic();
        }

        if(MMU::page_tables(MMU::pages(si->lm.sys_stack_size)) > 1) {
            db<Setup>(ERR) << "OS stack segment is too large!" << endl;
            panic();
        }
    }

    // Check APPLICATION integrity and get the size of its segments
    si->lm.app_entry = 0;
    si->lm.app_segments = 0;
    si->lm.app_code = ~0U;
    si->lm.app_code_size = 0;
    si->lm.app_data = ~0U;
    si->lm.app_data_size = 0;
    si->lm.app_extra = ~0U;
    si->lm.app_extra_size = 0;
    if(si->lm.has_app) {
        ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
        if(!app_elf->valid()) {
            db<Setup>(ERR) << "APP ELF image is corrupted!" << endl;
            panic();
        }
        si->lm.app_entry = app_elf->entry();
        si->lm.app_segments = app_elf->segments();
        si->lm.app_code = app_elf->segment_address(0);
        if(si->lm.app_code != MMU::align_directory(si->lm.app_code)) {
            db<Setup>(ERR) << "Unaligned APP CODE image:" << hex << si->lm.app_code << endl;
            panic();
        }
        si->lm.app_code_size = app_elf->segment_size(0);
        if(app_elf->segments() > 1) {
            for(int i = 1; i < app_elf->segments(); i++) {
                if(app_elf->segment_type(i) != PT_LOAD)
                    continue;
                if(app_elf->segment_address(i) < si->lm.app_data)
                    si->lm.app_data = app_elf->segment_address(i);
                si->lm.app_data_size += app_elf->segment_size(i);
            }
        }
        if(si->lm.app_data == ~0U) {
            db<Setup>(WRN) << "APP ELF image has no data segment!" << endl;
            si->lm.app_data = MMU::align_page(APP_DATA);
        }
        if(Traits<System>::multiheap) { // Application heap in data segment
            si->lm.app_data_size = MMU::align_page(si->lm.app_data_size);
            si->lm.app_stack = si->lm.app_data + si->lm.app_data_size;
            si->lm.app_data_size += MMU::align_page(Traits<Application>::STACK_SIZE);
            si->lm.app_heap = si->lm.app_data + si->lm.app_data_size;
            si->lm.app_data_size += MMU::align_page(Traits<Application>::HEAP_SIZE);
        }
        if(si->lm.has_ext) { // Check for EXTRA data in the boot image
            si->lm.app_extra = si->lm.app_data + si->lm.app_data_size;
            si->lm.app_extra_size = si->bm.img_size - si->bm.extras_offset;
            if(Traits<System>::multiheap)
                si->lm.app_extra_size = MMU::align_page(si->lm.app_extra_size);
            si->lm.app_data_size += si->lm.app_extra_size;
        }
    }
}

void Setup::build_pmm()
{
    db<Setup>(TRC) << "Setup::build_pmm() => top_page=" << MMU::pages(si->bm.mem_top) << endl;

    // Allocate (reserve) memory for all entities we have to setup.
    // We'll start at the highest address to make possible a memory model
    // on which the application's logical and physical address spaces match.
    Phy_Addr top_page = MMU::pages(si->bm.mem_top);

    // System Page Directory (4 x sizeof(Page)) -- up to 4k PD entries of 32 bits, moreover we need a page aligned to 16 KB 
    top_page -= 4;
    si->pmm.sys_pd = top_page * sizeof(Page);

    // Page tables to map the System address space
    top_page -= 1;
    si->pmm.sys_pt = top_page * sizeof(Page);

    // Page tables to map the whole physical memory
    // = NP/NPTE_PT * sizeof(Page)
    //   NP = size of physical memory in pages
    //   NPTE_PT = number of page table entries per page table, e.g., 256 in this config (12, 8, 12)
    db<Setup>(INF) << "Setup::build_pmm(): mmu::pages for phy_mem_pts;" << MMU::page_tables(MMU::pages(si->bm.mem_top - si->bm.mem_base)) << ", and curr top_page=" << top_page << endl;
    top_page -= MMU::page_tables(MMU::pages(si->bm.mem_top - si->bm.mem_base));
    si->pmm.phy_mem_pts = top_page * sizeof(Page);

    // Page tables to map the IO address space
    // = NP/NPTE_PT * sizeof(Page)
    // NP = size of I/O address space in pages
    // NPTE_PT = number of page table entries per page table
    top_page -= MMU::page_tables(MMU::pages(si->bm.mio_top - si->bm.mio_base));
    si->pmm.io_pts = top_page * sizeof(Page);

    // Page tables to map the first APPLICATION code segment
    top_page -= MMU::page_tables(MMU::pages(si->lm.app_code_size));
    si->pmm.app_code_pts = top_page * sizeof(Page);

    // Page tables to map the first APPLICATION data segment (which contains heap, stack and extra)
    top_page -= MMU::page_tables(MMU::pages(si->lm.app_data_size));
    si->pmm.app_data_pts = top_page * sizeof(Page);

    // System Info (1 x sizeof(Page))
    if (SYS_INFO != Traits<Machine>::NOT_USED)
        top_page -= 1;
    si->pmm.sys_info = top_page * sizeof(Page);

    // SYSTEM code segment -- For this test, everything will be in physical memory 
    top_page -= MMU::pages(si->lm.sys_code_size);
    si->pmm.sys_code = top_page * sizeof(Page);

    // SYSTEM data segment
    top_page -= MMU::pages(si->lm.sys_data_size);
    si->pmm.sys_data = top_page * sizeof(Page);

    // The memory allocated so far will "disappear" from the system as we set mem_top as follows:
    si->pmm.usr_mem_base = si->bm.mem_base;
    si->pmm.usr_mem_top = top_page * sizeof(Page);

    // APPLICATION code segment
    top_page -= MMU::pages(si->lm.app_code_size);
    si->pmm.app_code = top_page * sizeof(Page);

    // APPLICATION data segment (contains stack, heap and extra)
    top_page -= MMU::pages(si->lm.app_data_size);
    si->pmm.app_data = top_page * sizeof(Page);

    // SYSTEM stack segment -- We use boot stack right after sys_pt
    top_page -= MMU::pages(si->lm.sys_stack_size);
    si->pmm.sys_stack = top_page * sizeof(Page);

    // Free chunks (passed to MMU::init)
    si->pmm.free1_base = si->lm.stp_code + si->lm.stp_code_size + si->lm.stp_data_size; // vector table should not be deleted!
    si->pmm.free1_top = top_page * sizeof(Page); // we will free the stack here
    db<Setup>(TRC) << "Top page = " << top_page << endl;

    // Test if we didn't overlap SETUP and the boot image
    if(si->pmm.usr_mem_top <= si->lm.stp_code + si->lm.stp_code_size + si->lm.stp_data_size) {
        db<Setup>(ERR) << "SETUP would have been overwritten!" << endl;
        panic();
    }
}

void Setup::say_hi()
{
    db<Setup>(TRC) << "Setup::say_hi()" << endl;
    db<Setup>(INF) << "System_Info=" << *si << endl;

    kout << endl;

    if(!si->lm.has_app) {
        db<Setup>(ERR) << "No APPLICATION in boot image, you don't need EPOS!" << endl;
        panic();
    }
    if(!si->lm.has_sys)
        db<Setup>(INF) << "No SYSTEM in boot image, assuming EPOS is a library!" << endl;

    kout << "Setting up this machine as follows: " << endl;
    kout << "  Mode:         " << ((Traits<Build>::MODE == Traits<Build>::LIBRARY) ? "library" : (Traits<Build>::MODE == Traits<Build>::BUILTIN) ? "built-in" : "kernel") << endl;
    kout << "  Processor:    " << Traits<Machine>::CPUS << " x Cortex A53 at " << Traits<CPU>::CLOCK / 1000000 << " MHz (BUS clock = " << Traits<CPU>::CLOCK / 1000000 << " MHz)" << endl;
    kout << "  Memory:       " << (si->bm.mem_top - si->bm.mem_base) / 1024 << " KB [" << (void *)si->bm.mem_base << ":" << (void *)si->bm.mem_top << "]" << endl;
    kout << "  User memory:  " << (si->pmm.usr_mem_top - si->pmm.usr_mem_base) / 1024 << " KB [" << (void *)si->pmm.usr_mem_base << ":" << (void *)si->pmm.usr_mem_top << "]" << endl;
    kout << "  I/O space:    " << (si->bm.mio_top - si->bm.mio_base) / 1024 << " KB [" << (void *)si->bm.mio_base << ":" << (void *)si->bm.mio_top << "]" << endl;
    kout << "  Node Id:      ";
    if(si->bm.node_id != -1)
        kout << si->bm.node_id << " (" << Traits<Build>::NODES << ")" << endl;
    else
        kout << "will get from the network!" << endl;
    kout << "  Position:     ";
    if(si->bm.space_x != -1)
        kout << "(" << si->bm.space_x << "," << si->bm.space_y << "," << si->bm.space_z << ")" << endl;
    else
        kout << "will get from the network!" << endl;
    if(si->lm.has_stp)
        kout << "  Setup:        " << si->lm.stp_code_size + si->lm.stp_data_size << " bytes" << endl;
    if(si->lm.has_ini)
        kout << "  Init:         " << si->lm.ini_code_size + si->lm.ini_data_size << " bytes" << endl;
    if(si->lm.has_sys)
        kout << "  OS code:      " << si->lm.sys_code_size << " bytes" << "\tdata: " << si->lm.sys_data_size << " bytes" << "    stack: " << si->lm.sys_stack_size << " bytes" << endl;
    if(si->lm.has_app)
        kout << "  APP code:     " << si->lm.app_code_size << " bytes" << "\tdata: " << si->lm.app_data_size << " bytes" << endl;
    if(si->lm.has_ext)
        kout << "  Extras:       " << si->lm.app_extra_size << " bytes" << endl;

    kout << endl;
}

void Setup::setup_pt(PT_Entry * pts, Phy_Addr base, unsigned int size, unsigned int n_pts, Flags flag, bool print)
{
    // n_pts equal to the number of PDs necessary to map the requested PTEs (given by size) from the memory base 
    // Each PTE maps one Page (4k), 
    // Each Page can have 4 pages with 256 ptes each
    // Thus, for each PD, map 256 pte until all requested ptes are mapped
    db<Setup>(INF) << "Size of Page: " << sizeof(Page) << endl;
    for(unsigned int i = 0; i < size; i++) {
        pts[i] = MMU::phy2pte((base + i * sizeof(Page)), flag);
        if(Traits<Setup>::hysterically_debugged && print)
            db<Setup>(INF) << "pts[" << i << "]=" << pts[i] << ",addr="<< &pts[i] << endl;
    }
}

void Setup::setup_sys_pt()
{
    db<Setup>(TRC) << "Setup::setup_sys_pt(pmm="
                   << "{si="      << (void *)si->pmm.sys_info
                   << ",pt="      << (void *)si->pmm.sys_pt
                   << ",pd="      << (void *)si->pmm.sys_pd
                   << ",sysc={b=" << (void *)si->pmm.sys_code << ",s=" << MMU::pages(si->lm.sys_code_size) << "}"
                   << ",sysd={b=" << (void *)si->pmm.sys_data << ",s=" << MMU::pages(si->lm.sys_data_size) << "}"
                   << ",syss={b=" << (void *)si->pmm.sys_stack << ",s=" << MMU::pages(si->lm.sys_stack_size) << "}"
                   << "})" << endl;

    // Get the physical address for the SYSTEM Page Table
    PT_Entry * sys_pt = reinterpret_cast<PT_Entry *>(si->pmm.sys_pt);

    // Clear the System Page Table
    memset(reinterpret_cast<void *>(sys_pt), 0, sizeof(Page));

    // System Info
    sys_pt[MMU::directory(SYS_INFO - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_INFO)] = MMU::phy2pte(si->pmm.sys_info, Flags::SYS);
    db<Setup>(INF) << "SYS_INFO PT = " << MMU::directory(SYS_INFO - SYS) * (MMU::PT_ENTRIES-1) + MMU::page(SYS_INFO) << endl;

    // Set an entry to this page table, so the system can access it later -- 1 page
    sys_pt[MMU::directory(SYS_PT - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_PT)]     = MMU::phy2pte(si->pmm.sys_pt, Flags::SYS);
    db<Setup>(INF) << "SYS_PT PT = " << MMU::directory(SYS_PT - SYS) * (MMU::PT_ENTRIES-1) + MMU::page(SYS_PT) << endl;

    // System Page Directory -- 4 Pages for directory
    sys_pt[MMU::directory(SYS_PD - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_PD)]   = MMU::phy2pte(si->pmm.sys_pd + 0 * sizeof(Page), Flags::SYS);
    sys_pt[MMU::directory(SYS_PD - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_PD)+1] = MMU::phy2pte(si->pmm.sys_pd + 1 * sizeof(Page), Flags::SYS);
    sys_pt[MMU::directory(SYS_PD - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_PD)+2] = MMU::phy2pte(si->pmm.sys_pd + 2 * sizeof(Page), Flags::SYS);
    sys_pt[MMU::directory(SYS_PD - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_PD)+3] = MMU::phy2pte(si->pmm.sys_pd + 3 * sizeof(Page), Flags::SYS);
    db<Setup>(INF) << "SYS_PD PT = " << MMU::directory(SYS_PD - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_PD) << endl;

    // SYSTEM code
    setup_pt(reinterpret_cast<PT_Entry *>(&sys_pt[MMU::directory(SYS_CODE - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_CODE)]), si->pmm.sys_code, MMU::pages(si->lm.sys_code_size), MMU::page_tables(MMU::pages(si->lm.sys_code_size)), Flags::SYS, true);
    db<Setup>(INF) << "SYS_CODE PT = " << MMU::directory(SYS_CODE - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_CODE) << ",size=" << si->lm.sys_code_size << endl;
    
    // SYSTEM data
    setup_pt(reinterpret_cast<PT_Entry *>(&sys_pt[MMU::directory(SYS_DATA - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_DATA)]), si->pmm.sys_data, MMU::pages(si->lm.sys_data_size), MMU::page_tables(MMU::pages(si->lm.sys_data_size)), Flags::SYS, true);
    db<Setup>(INF) << "SYS_DATA PT = " << MMU::directory(SYS_DATA - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_DATA) << ",size=" << si->lm.sys_data_size << endl;

    // SYSTEM stack (used only during init and for the ukernel model)
    setup_pt(reinterpret_cast<PT_Entry *>(&sys_pt[MMU::directory(SYS_STACK - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_STACK)]), si->pmm.sys_stack, MMU::pages(si->lm.sys_stack_size), MMU::page_tables(MMU::pages(si->lm.sys_stack_size)), Flags::SYS, true);
    db<Setup>(INF) << "SYS_STACK PT = " << MMU::directory(SYS_STACK - SYS) * (MMU::PT_ENTRIES) + MMU::page(SYS_STACK) << ",size=" << si->lm.sys_stack_size << endl;
    
    db<Setup>(TRC) << "SYS_PT=" << *reinterpret_cast<Page_Table *>(sys_pt) << endl;
}

void Setup::setup_app_pt()
{
    db<Setup>(TRC) << "Setup::setup_app_pt(appc={b=" << (void *)si->pmm.app_code << ",s=" << MMU::pages(si->lm.app_code_size) << "}"
                   << ",appd={b=" << (void *)si->pmm.app_data << ",s=" << MMU::pages(si->lm.app_data_size) << "}"
                   << ",appe={b=" << (void *)si->pmm.app_extra << ",s=" << MMU::pages(si->lm.app_extra_size) << "}"
                   << "})" << endl;

    // Get the physical address for the first APPLICATION Page Tables
    PT_Entry * app_code_pt = reinterpret_cast<PT_Entry *>(si->pmm.app_code_pts);
    PT_Entry * app_data_pt = reinterpret_cast<PT_Entry *>(si->pmm.app_data_pts);

    // Clear the first APPLICATION Page Tables
    memset(reinterpret_cast<void*>(app_code_pt), 0, MMU::page_tables(MMU::pages(si->lm.app_code_size)) * sizeof(Page_Table));
    memset(reinterpret_cast<void*>(app_data_pt), 0, MMU::page_tables(MMU::pages(si->lm.app_data_size)) * sizeof(Page_Table));

    // APPLICATION code
    setup_pt(reinterpret_cast<PT_Entry *>(&app_code_pt[MMU::page(si->lm.app_code)]), si->pmm.app_code, MMU::pages(si->lm.app_code_size), MMU::page_tables(MMU::pages(si->lm.app_code_size)), Flags::APP);

    // APPLICATION data (contains stack, heap and extra)
    setup_pt(reinterpret_cast<PT_Entry *>(&app_data_pt[MMU::page(si->lm.app_data)]), si->pmm.app_data, MMU::pages(si->lm.app_data_size), MMU::page_tables(MMU::pages(si->lm.app_data_size)), Flags::APP);

    db<Setup>(INF) << "APPC_PT=" << *reinterpret_cast<Page_Table *>(app_code_pt) << endl;
    db<Setup>(INF) << "APPD_PT=" << *reinterpret_cast<Page_Table *>(app_data_pt) << endl;
}

void Setup::setup_sys_pd()
{
    db<Setup>(TRC) << "setup_sys_pd(bm="
                   << "{memb="  << (void *)si->bm.mem_base
                   << ",memt="  << (void *)si->bm.mem_top
                   << ",miob="  << (void *)si->bm.mio_base
                   << ",miot="  << (void *)si->bm.mio_top
                   << "{si="    << (void *)si->pmm.sys_info
                   << ",spt="   << (void *)si->pmm.sys_pt
                   << ",spd="   << (void *)si->pmm.sys_pd
                   << ",mem="   << (void *)si->pmm.phy_mem_pts
                   << ",io="    << (void *)si->pmm.io_pts
                   << ",umemb=" << (void *)si->pmm.usr_mem_base
                   << ",umemt=" << (void *)si->pmm.usr_mem_top
                   << ",sysc="  << (void *)si->pmm.sys_code
                   << ",sysd="  << (void *)si->pmm.sys_data
                   << ",syss="  << (void *)si->pmm.sys_stack
                   << ",apct="  << (void *)si->pmm.app_code_pts
                   << ",apdt="  << (void *)si->pmm.app_data_pts
                   << ",fr1b="  << (void *)si->pmm.free1_base
                   << ",fr1t="  << (void *)si->pmm.free1_top
                   << ",fr2b="  << (void *)si->pmm.free2_base
                   << ",fr2t="  << (void *)si->pmm.free2_top
                   << "})" << endl;

    // Get the physical address for the System Page Directory
    PT_Entry * sys_pd = reinterpret_cast<PT_Entry *>(si->pmm.sys_pd);

    // Clear the System Page Directory
    memset(reinterpret_cast<void*>(sys_pd), 0, sizeof(Page));

    // Calculate the number of page tables needed to map the physical memory
    unsigned int mem_size = MMU::pages(si->bm.mem_top - si->bm.mem_base);
    unsigned int n_pts = MMU::page_tables(mem_size);
    db<Setup>(INF) << "mem_size="<< mem_size << ",n_pts=" << n_pts << (void *) si->pmm.phy_mem_pts << ",syspd=" << (void *) si->pmm.sys_pd << "Size pte=" << sizeof(PT_Entry) << endl;
    // Map the whole physical memory into the page tables pointed by phy_mem_pts
    PT_Entry * pts = reinterpret_cast<PT_Entry *>(si->pmm.phy_mem_pts);

    db<Setup>(INF) << &pts[0] <<  " to "<< &pts[(n_pts-1) * sizeof(Page_Table) + MMU::PT_ENTRIES] << endl;

    setup_pt(pts, si->bm.mem_base, mem_size, n_pts, Flags::SYS);

    db<Setup>(INF) << "PHY_MEM_PT("<< (void *) pts <<")=" << *reinterpret_cast<Page_Table *>(pts) << endl; // if this print is removed, the config goes wrong
    db<Setup>(INF) << "mem_size="<< mem_size <<",n_pts=" << n_pts << endl;

    db<Setup>(INF) << "pts done" << endl;
    // Attach all physical memory starting at PHY_MEM
    if (PHY_MEM != RAM_BASE) {
        assert((MMU::directory(MMU::align_directory(PHY_MEM)) + n_pts) < (MMU::PD_ENTRIES - 3)); // check if it would overwrite the OS
        for(unsigned int i = MMU::directory(MMU::align_directory(PHY_MEM)), j = 0; i < MMU::directory(MMU::align_directory(PHY_MEM)) + n_pts; i++, j++)
            sys_pd[i] = MMU::phy2pde(si->pmm.phy_mem_pts + j * sizeof(Page_Table));
        db<Setup>(INF) << "sys pd PHY_MEM  done" << endl;
    }

    // Attach the portion of the physical memory used by Setup at SETUP
    sys_pd[MMU::directory(SETUP)] =  MMU::phy2pde(si->pmm.phy_mem_pts);
    db<Setup>(INF) << "sys pd on SETUP directory = " << MMU::directory(SETUP) << endl;

    // Attach all physical memory starting at RAM_BASE
    assert((MMU::directory(MMU::align_directory(RAM_BASE)) + n_pts) < (MMU::PD_ENTRIES - 2)); // check if it would overwrite the OS
    for(unsigned int i = MMU::directory(MMU::align_directory(RAM_BASE)), j = 0; i < MMU::directory(MMU::align_directory(RAM_BASE)) + n_pts; i++, j++)
        sys_pd[i] = MMU::phy2pde(si->pmm.phy_mem_pts + j * sizeof(Page_Table));
    db<Setup>(INF) << "sys pd RAM_BASE done, dir= " << MMU::directory(RAM_BASE) << endl;

    // Calculate the number of page tables needed to map the IO address space
    unsigned int io_size = MMU::pages(si->bm.mio_top - si->bm.mio_base);
    n_pts = MMU::page_tables(io_size);

    db<Setup>(INF) << "starting io pts, npts=" << n_pts << endl;
    // Map IO address space into the page tables pointed by io_pts
    pts = reinterpret_cast<PT_Entry *>(si->pmm.io_pts);
    setup_pt(pts, si->bm.mio_base, io_size, n_pts, Flags::IO);

    db<Setup>(INF) << "IO_PTS("<< (void *) pts <<")=" << *reinterpret_cast<Page_Table *>(pts) << endl; // if this print is removed, the config g
    db<Setup>(INF) << "io pts done" << endl;

    // Attach devices' memory at Memory_Map::IO
    assert((MMU::directory(MMU::align_directory(IO)) + n_pts) < (MMU::PD_ENTRIES - 1)); // check if it would overwrite the OS
    for(unsigned int i = MMU::directory(MMU::align_directory(IO)), j = 0; i < MMU::directory(MMU::align_directory(IO)) + n_pts; i++, j++)
        sys_pd[i] = MMU::phy2pde((si->pmm.io_pts + j * sizeof(Page_Table)));
    db<Setup>(INF) << "sys pd for io pts done" << endl;

    db<Setup>(INF) << "attach SYS pt on sys pd[sys]:" << MMU::directory(SYS) 
                    << ", with sys_pt[0] = " <<  hex << *((int *) si->pmm.sys_pt) << endl;
    db<Setup>(INF) << "sys_pd[sys+1];" << MMU::directory(SYS)+1 
                    << ", with sys_pt[1] = " << hex << *((int *) (si->pmm.sys_pt+sizeof(Page_Table))) << endl;
    // Attach the OS (i.e. sys_pt)
    // One sys_pt for code
    sys_pd[MMU::directory(SYS)] = MMU::phy2pde(si->pmm.sys_pt);
    db<Setup>(INF) << "attach SYS on sys pd done" << endl;

    // // Two sys_pt for data
    // // NO NO NO, One is enougth 
    // sys_pd[MMU::directory(SYS) + 1] = MMU::phy2pde(si->pmm.sys_pt + sizeof(Page_Table));
    // sys_pd[MMU::directory(SYS) + 2] = MMU::phy2pde(si->pmm.sys_pt + 2 * sizeof(Page_Table));
    // db<Setup>(INF) << "attach SYS on sys pd done" << endl;

    // Attach the first APPLICATION CODE (i.e. app_code_pt)
    n_pts = MMU::page_tables(MMU::pages(si->lm.app_code_size));
    for(unsigned int i = MMU::directory(MMU::align_directory(si->lm.app_code)), j = 0; i < MMU::directory(MMU::align_directory(si->lm.app_code)) + n_pts; i++, j++)
        sys_pd[i] = MMU::phy2pde(si->pmm.app_code_pts + j * sizeof(Page_Table));

    // Attach the first APPLICATION DATA (i.e. app_data_pt, containing heap, stack and extra)
    n_pts = MMU::page_tables(MMU::pages(si->lm.app_data_size));
    for(unsigned int i = MMU::directory(MMU::align_directory(si->lm.app_data)), j = 0; i < MMU::directory(MMU::align_directory(si->lm.app_data)) + n_pts; i++, j++)
        sys_pd[i] = MMU::phy2pde(si->pmm.app_data_pts + j * sizeof(Page_Table));

    db<Setup>(INF) << "SYS_PD=" << *reinterpret_cast<Page_Table *>(sys_pd) << endl;
}

void Setup::enable_paging()
{
    db<Setup>(TRC) << "Setup::enable_paging()" << endl;
    if(Traits<Setup>::hysterically_debugged) {
        db<Setup>(INF) << "pc=" << CPU::pc() << endl;
        db<Setup>(INF) << "sp=" << CPU::sp() << endl;
    }

    // SET FLAGS
    long tcr_flags;
    long mair_flags;

    tcr_flags  = CPU::IRGN0_WB_WA | CPU::IRGN1_WB_WA | CPU::ORGN0_WB_WA | CPU::ORGN1_WB_WA;
    tcr_flags |= CPU::SH0_INNER | CPU::SH1_INNER;
    tcr_flags |= CPU::TG0_64KB | CPU::TG1_64KB;
    tcr_flags |= CPU::TBI0 | CPU::EPD1 | CPU::IPS;
    tcr_flags |= 0x160016;  // T0SZ | T1SZ

    mair_flags  = (0x0      << (8 * CPU::MAIR_DEVICE_NGNRNE));
    mair_flags |= (0x4      << (8 * CPU::MAIR_DEVICE_NGNRE));
    mair_flags |= (0xc      << (8 * CPU::MAIR_DEVICE_GRE));
    mair_flags |= (0x44     << (8 * CPU::MAIR_NORMAL_NC));
    mair_flags |= (0xffll   << (8 * CPU::MAIR_NORMAL));

    CPU::tcr_el1(tcr_flags);
    CPU::mair_el1(mair_flags);

    CPU::dsb();
    CPU::isb();

    // TODO: Change this to separate kernel from application
    // Set the page tables that MMU will use 
    CPU::ttbr0_el1((Traits<System>::multitask) ? si->pmm.sys_pd : PAGE_TABLES);
    CPU::ttbr1_el1((Traits<System>::multitask) ? si->pmm.sys_pd : PAGE_TABLES);

    // Enable MMU
    CPU::sctlr_el1((CPU::sctlr_el1() | CPU::DCACHE | CPU::ICACHE | CPU::MMU_ENABLE));

    CPU::dsb();
    CPU::isb();

    MMU::flush_tlb();

    // Adjust pointers that will still be used to their logical addresses
    Display::init(); // adjust the pointers in Display by calling init 

    if(Traits<Setup>::hysterically_debugged) {
        db<Setup>(INF) << "pc=" << CPU::pc() << endl;
        db<Setup>(INF) << "sp=" << CPU::sp() << endl;
    }
}

void Setup::load_parts()
{
    db<Setup>(TRC) << "Setup::load_parts()" << endl;

    // Relocate System_Info
    if(sizeof(System_Info) > sizeof(Page))
        db<Setup>(WRN) << "System_Info is bigger than a page (" << sizeof(System_Info) << ")!" << endl;

    if(Traits<Setup>::hysterically_debugged)
        db<Setup>(INF) << "Setup:SYS_INFO: " << MMU::Translation(SYS_INFO) << endl;

    memcpy(reinterpret_cast<void *>(SYS_INFO), si, sizeof(System_Info));
    si = reinterpret_cast<System_Info *>(SYS_INFO);

    // Load INIT
    if(si->lm.has_ini) {
        db<Setup>(TRC) << "Setup::load_init()" << endl;
        ELF * ini_elf = reinterpret_cast<ELF *>(&bi[si->bm.init_offset]);
        if(Traits<Setup>::hysterically_debugged) {
            db<Setup>(INF) << "Setup:ini_elf: " << MMU::Translation(ini_elf) << endl;
            db<Setup>(INF) << "Setup:ini_elf[0]: " << MMU::Translation(ini_elf->segment_address(0)) << endl;
            db<Setup>(INF) << "Setup:ini_elf[0].size: " << ini_elf->segment_size(0) << endl;
        }
        if(ini_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "INIT code segment was corrupted during SETUP!" << endl;
            panic();
        }
        for(int i = 1; i < ini_elf->segments(); i++)
            if(ini_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "INIT data segment was corrupted during SETUP!" << endl;
                panic();
            }
    }

    // Load SYSTEM
    if(si->lm.has_sys) {
        db<Setup>(TRC) << "Setup::load_os()" << endl;
        ELF * sys_elf = reinterpret_cast<ELF *>(&bi[si->bm.system_offset]);
        if(Traits<Setup>::hysterically_debugged) {
            db<Setup>(INF) << "Setup:sys_elf: " << MMU::Translation(sys_elf) << endl;
            db<Setup>(INF) << "Setup:sys_elf[0]: " << MMU::Translation(sys_elf->segment_address(0)) << endl;
            db<Setup>(INF) << "Setup:sys_elf[0].size: " << sys_elf->segment_size(0) << endl;
        }
        if(sys_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "OS code segment was corrupted during SETUP!" << endl;
            panic();
        }
        if(sys_elf->load_segment(1) < 0) {
            db<Setup>(ERR) << "OS code segment was corrupted during SETUP!" << endl;
            panic();
        }
        for(int i = 2; i < sys_elf->segments(); i++) {
            if(sys_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "OS data segment was corrupted during SETUP!" << endl;
                panic();
            }
        }
    }

    // Load APP
    if(si->lm.has_app) {
        db<Setup>(TRC) << "Setup::load_app()" << endl;
        ELF * app_elf = reinterpret_cast<ELF *>(&bi[si->bm.application_offset]);
        if(Traits<Setup>::hysterically_debugged) {
            db<Setup>(INF) << "Setup:app_elf: " << (void*)app_elf << endl;
            db<Setup>(INF) << "Setup:app_elf: " << MMU::Translation(app_elf) << endl;
            db<Setup>(INF) << "Setup:app_elf[0]: " << MMU::Translation(app_elf->segment_address(0)) << endl;
            db<Setup>(INF) << "Setup:app_elf[0].size: " << app_elf->segment_size(0) << endl;
        }
        if(app_elf->load_segment(0) < 0) {
            db<Setup>(ERR) << "Application code segment was corrupted during SETUP!" << endl;
            panic();
        }
        for(int i = 1; i < app_elf->segments(); i++) {
            if(app_elf->load_segment(i) < 0) {
                db<Setup>(ERR) << "Application data segment was corrupted during SETUP!" << endl;
                panic();
            }
        }
    }

    // Load EXTRA
    if(si->lm.has_ext) {
        db<Setup>(TRC) << "Setup::load_extra()" << endl;
        if(Traits<Setup>::hysterically_debugged)
            db<Setup>(INF) << "Setup:APP_EXTRA:" << MMU::Translation(si->lm.app_extra) << endl;
        memcpy(reinterpret_cast<void *>(si->lm.app_extra), &bi[si->bm.extras_offset], si->lm.app_extra_size);
    }
}

void Setup::adjust_perms()
{
    db<Setup>(TRC) << "Setup::adjust_perms(appc={b=" << (void *)si->pmm.app_code << ",s=" << MMU::pages(si->lm.app_code_size) << "}"
                   << ",appd={b=" << (void *)si->pmm.app_data << ",s=" << MMU::pages(si->lm.app_data_size) << "}"
                   << ",appe={b=" << (void *)si->pmm.app_extra << ",s=" << MMU::pages(si->lm.app_extra_size) << "}"
                   << "})" << endl;


    // Get the logical address of the first APPLICATION Page Tables
    PT_Entry * app_code_pt = MMU::phy2log(reinterpret_cast<PT_Entry *>(si->pmm.app_code_pts));
    PT_Entry * app_data_pt = MMU::phy2log(reinterpret_cast<PT_Entry *>(si->pmm.app_data_pts));

    unsigned int i;
    PT_Entry aux;

    // APPLICATION code
    for(i = 0, aux = si->pmm.app_code; i < MMU::pages(si->lm.app_code_size); i++, aux = aux + sizeof(Page))
        app_code_pt[MMU::page(APP_CODE) + i] = MMU::phy2pte(aux, Flags::APPC);

    // APPLICATION data (contains stack, heap and extra)
    for(i = 0, aux = si->pmm.app_data; i < MMU::pages(si->lm.app_data_size); i++, aux = aux + sizeof(Page))
        app_data_pt[MMU::page(APP_DATA) + i] = MMU::phy2pte(aux, Flags::APPD);
}

void Setup::call_next()
{
    // Check for next stage and obtain the entry point
    Log_Addr ip;
    Reg cpu_id = CPU::id();

    if(Traits<System>::multitask) {
        int cpu_id = CPU::id();
        if(si->lm.has_ini) {
            if(cpu_id == 0) {
                db<Setup>(TRC) << "Executing system's global constructors ..." << endl;
                reinterpret_cast<void (*)()>((void *)si->lm.sys_entry)();
            }
            ip = si->lm.ini_entry;
        } 
        else if(si->lm.has_sys)
            ip = si->lm.sys_entry;
        else
            ip = si->lm.app_entry;

        // Arrange a stack for each CPU to support stage transition
        Log_Addr sp = SYS_STACK + Traits<Machine>::STACK_SIZE * (cpu_id + 1);

        db<Setup>(TRC) << "Setup::call_next(ip=" << ip << ",sp=" << sp << ") => ";
        if(si->lm.has_ini)
            db<Setup>(TRC) << "INIT" << endl;
        else if(si->lm.has_sys)
            db<Setup>(TRC) << "SYSTEM" << endl;
        else
            db<Setup>(TRC) << "APPLICATION" << endl;

        CPU::sp(sp);
    } else
        ip = &_start;

    db<Setup>(INF) << "SETUP ends here!" << endl;

    // Set SP and call next stage
    static_cast<void (*)()>(ip)();

    if(Traits<System>::multitask && (cpu_id == 0)) {
        // This will only happen when INIT was called and Thread was disabled
        // Note we don't have the original stack here anymore!
        reinterpret_cast<void (*)()>(si->lm.app_entry)();
    }

    // SETUP is now part of the free memory and this point should never be
    // reached, but, just in case ... :-)
    panic();
}

__END_SYS

using namespace EPOS::S;

void _entry()
{
    // Configure a stack for the current level and goto _reset
    CPU::sp(Traits<Machine>::BOOT_STACK + Traits<Machine>::STACK_SIZE * CPU::id());
    ASM("b _reset");
}

void _reset()
{
    // ARMV8 starts in Exception level 2
    if (CPU::el() == 2) {
        CPU::hcr_el2((1l << 31) | (1l << 1));   // configure the hypervisor

        // go to _entry as EL1
        CPU::spsr_el2(0x3c4);
        CPU::elr_el2(reinterpret_cast<CPU::Reg> (&_entry));
        CPU::eret();
    }

    if(CPU::id() == 0) {
        CPU::vbar_el1(reinterpret_cast<CPU::Reg>(&_vector_table));  // setup the vector table
        _bss_clear();
    }  else {
        BCM_Mailbox * mbox = reinterpret_cast<BCM_Mailbox *>(Memory_Map::MBOX_CTRL_BASE);
        mbox->eoi(0);
        mbox->enable();
    }

    _setup();
}

void _setup()
{
    // CPU::int_disable(); // interrupts will be re-enabled at init_end

    // CPU::enable_fpu();
    // CPU::flush_caches();
    // CPU::flush_branch_predictors();
    // CPU::flush_tlb();
    // CPU::actlr_el1(CPU::actlr_el1() | CPU::DCACHE_PREFE); // enable Dside prefetch
    
    Setup setup;
}

void _vector_table() {
    ASM(
        "_vector_table:         \n"
        ".balign 0x80           \n"
        "mov x0, x0             \n"
        ".balign 0x80           \n"
        "mov x1, x0             \n"
        ".balign 0x80           \n"
        "mov x2, x0             \n"
        ".balign 0x80           \n"
        "mov x3, x0             \n"

        ".balign 0x80           \n"
        "mov x4, x0             \n"
        ".balign 0x80           \n"
        "mov x5, x0             \n"
        ".balign 0x80           \n"
        "mov x6, x0             \n"
        ".balign 0x80           \n"
        "mov x7, x0             \n"

        ".balign 0x80           \n"
        "mov x8, x0             \n"
        ".balign 0x80           \n"
        "mov x9, x0             \n"
        ".balign 0x80           \n"
        "mov x10, x0            \n"
        ".balign 0x80           \n"
        "mov x11, x0            \n"

        ".balign 0x80           \n"
        "mov x12, x0            \n"
        ".balign 0x80           \n"
        "mov x13, x0            \n"
        ".balign 0x80           \n"
        "mov x14, x0            \n"
        ".balign 0x80           \n"
        "mov x15, x0            \n");
}
