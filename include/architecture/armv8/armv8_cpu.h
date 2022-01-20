// EPOS ARMv8 CPU Mediator Declarations

#ifndef __armv8_h
#define __armv8_h

#include <architecture/cpu.h>

__BEGIN_SYS

class ARMv8: protected CPU_Common
{
    friend class Init_System; // for CPU::init()

protected:
    static const bool multicore = Traits<System>::multicore;
    static const bool multitask = Traits<System>::multitask;

public:
    // CPU Native Data Types
    using CPU_Common::Reg8;
    using CPU_Common::Reg16;
    using CPU_Common::Reg32;
    using CPU_Common::Reg64;
    using CPU_Common::Reg;
    using CPU_Common::Log_Addr;
    using CPU_Common::Phy_Addr;

    class Context
    {
    public:
        Context() {}
        Context(Log_Addr usp, Log_Addr ulr, Reg flags, Log_Addr  lr, Log_Addr pc): _usp(usp), _ulr(ulr), _flags(flags), _lr(lr), _pc(pc) {
            if(Traits<Build>::hysterically_debugged || Traits<Thread>::trace_idle) {
                _x0 = 0; _x1 = 1; _x2 = 2; _x3 = 3; _x4 = 4; _x5 = 5; _x6 = 6; _x7 = 7; _x8 = 8; _x9 = 9; _x10 = 10;
                _x11 = 11; _x12 = 12; _x13 = 13; _x14 = 14; _x15 = 15; _x16 = 16; _x17 = 17; _x18 = 18; _x19 = 19; _x20 = 20;
                _x21 = 21; _x22 = 22; _x23 = 23; _x24 = 24; _x25 = 25; _x26 = 26; _x27 = 27; _x28 = 28; _x29 = 29; _x30 = 30;
            }
        }
    public:
        Reg _usp;     // usp (only used in multitasking)
        Reg _ulr;     // ulr (only used in multitasking)
        Reg _flags;
        Reg _x0;
        Reg _x1;
        Reg _x2;
        Reg _x3;
        Reg _x4;
        Reg _x5;
        Reg _x6;
        Reg _x7;
        Reg _x8;
        Reg _x9;
        Reg _x10;
        Reg _x11;
        Reg _x12;
        Reg _x13;
        Reg _x14;
        Reg _x15;
        Reg _x16;
        Reg _x17;
        Reg _x18;
        Reg _x19;
        Reg _x20;
        Reg _x21;
        Reg _x22;
        Reg _x23;
        Reg _x24;
        Reg _x25;
        Reg _x26;
        Reg _x27;
        Reg _x28;
        Reg _x29;
        Reg _x30;
        Reg _lr;
        Reg _pc;
    };

    // Interrupt Service Routines
    typedef void (* ISR)();

    // Fault Service Routines (exception handlers)
    typedef void (* FSR)();

protected:
    ARMv8() {};

public:
    // TODO
    //static Log_Addr pc() { Reg r; ASM("mov %0, pc" : "=r"(r) :); return r; } // due to RISC pipelining, PC is read with a +8 (4 for thumb) offset
    static Log_Addr pc() { return 0; } // due to RISC pipelining, PC is read with a +8 (4 for thumb) offset

    static void sp(Log_Addr sp) {}
    static Log_Addr sp() {return 0;}

    // TODO ????
    static void fr(Reg r) {}
    static Reg fr() {return 0;}

    static Log_Addr ra() {return 0;}

    static void halt() {}

    template<typename T>
    static T tsl(volatile T & lock) {
        return lock;
    }

    template<typename T>
    static T finc(volatile T & value) {
        return value;
    }

    template<typename T>
    static T fdec(volatile T & value) {
        return value;
    }

    template <typename T>
    static T cas(volatile T & value, T compare, T replacement) {
        return value;
    }

    // ARMv8 specifics
    // TODO
    static Reg r0() { return 0; }
    static void r0(Reg r) {  }

    static Reg r1() { return 0; }
    static void r1(Reg r) {}

    static Reg actlr() { return 0; }
    static void actlr(Reg r) { }

    static void dsb() { ASM("dsb sy"); }
    static void isb() { ASM("isb"); }

    static void svc() {}

    static void vbar_el1(Reg r) {}
};

class ARMv8_A: public ARMv8
{
public:
    static const bool thumb = false;


    typedef Reg Flags;
    enum {
        // DAIF
        FLAG_F          = 1    << 6,       // FIQ disable
        FLAG_I          = 1    << 7,       // IRQ disable
        FLAG_A          = 1    << 8,       // SError disable
        FLAG_D          = 1    << 9,       // Debug disable
        
        FLAG_IL         = 1    << 20,
        FLAG_SS         = 1    << 21,

        // NZCV
        FLAG_V          = 1    << 28,
        FLAG_C          = 1    << 29,
        FLAG_Z          = 1    << 30,
        FLAG_N          = 1    << 31,


        // FLAG_M values
        MODE_USR        = 0x10,
        MODE_FIQ        = 0x11,
        MODE_IRQ        = 0x12,
        MODE_SVC        = 0x13,
        MODE_ABORT      = 0x17,
        MODE_UNDEFINED  = 0x1b,
        MODE_SYS        = 0x1f
    };


    // // CPU Flags
    // typedef Reg Flags;
    // enum {
    //     FLAG_F          = 1    << 6,       // FIQ disable
    //     FLAG_I          = 1    << 7,       // IRQ disable

    //     //TODO
    //     FLAG_M          = 0x1f << 0,       // Processor Mode (5 bits)
    //     FLAG_T          = 1    << 5,       // Thumb state
    //     FLAG_A          = 1    << 8,       // Imprecise Abort disable
    //     FLAG_E          = 1    << 9,       // Endianess (0 ->> little, 1 -> big)
    //     FLAG_GE         = 0xf  << 16,      // SIMD Greater than or Equal (4 bits)
    //     FLAG_J          = 1    << 24,      // Jazelle state
    //     FLAG_Q          = 1    << 27,      // Underflow and/or DSP saturation
    //     FLAG_V          = 1    << 28,      // Overflow
    //     FLAG_C          = 1    << 29,      // Carry
    //     FLAG_Z          = 1    << 30,      // Zero
    //     FLAG_N          = 1    << 31,      // Negative

    //     // FLAG_M values
    //     MODE_USR        = 0x10,
    //     MODE_FIQ        = 0x11,
    //     MODE_IRQ        = 0x12,
    //     MODE_SVC        = 0x13,
    //     MODE_ABORT      = 0x17,
    //     MODE_UNDEFINED  = 0x1b,
    //     MODE_SYS        = 0x1f
    // };

    // Exceptions
    typedef Reg Exception_Id;
    enum {
        EXC_START                   = 1,
        EXC_UNDEFINED_INSTRUCTION   = 2,
        EXC_SWI                     = 3,
        EXC_PREFETCH_ABORT          = 4,
        EXC_DATA_ABORT              = 5,
        EXC_RESERVED                = 6,
        EXC_IRQ                     = 7,
        EXC_FIQ                     = 8
    };

    enum {
        CLI_DOMAIN = 0x55555555, // 0b01 - Client, all memory domains check for memory access permission
        MNG_DOMAIN = 0xFFFFFFFF  // 0b11 - Manager, memory access permissions are not checked
    };

    // SCTLR bits
    enum {
        MMU_ENABLE  = 1 << 0,  // MMU enable
        DCACHE      = 1 << 2,  // Data cache enable
        BRANCH_PRED = 1 << 11, // Z bit, branch prediction enable
        ICACHE      = 1 << 12, // Instruction cache enable
        // AFE         = 1 << 29  // Access Flag enable //TODO
    };

    // ACTLR bits
    enum {
        DCACHE_PREFE = 1 << 2, // DCache prefetch Enabled
        SMP          = 1 << 6 // SMP bit
    };

    // EL
    enum {
        EL0 = 0,
        EL1 = 1,
        EL2 = 2,
        EL3 = 3
    };

    // HCR_EL2
    enum {
        HCR_EL2_RW = 1 << 31,
        HCR_EL2_SWIO = 1 << 1,
    };

    // TCR_EL1
    enum {
        TCR_VADDR_BITS = 42l,
        TCR_PADDR_BITS = 42l,

        TCR_T0SZ = ((64 - (TCR_VADDR_BITS)) << 0),
        TCR_T1SZ = ((64 - (TCR_VADDR_BITS)) << 16),
        TCR_TxSZ = (TCR_T0SZ | TCR_T1SZ),

        TCR_IRGN0_WBWA = 1ul << 8,
        TCR_ORGN0_WBWA = 1ul << 10,
        TCR_IRGN1_WBWA = 1ul << 24,
        TCR_ORGN1_WBWA = 1ul << 26,

        TCR_IRGN_WBWA = (TCR_IRGN0_WBWA | TCR_IRGN1_WBWA),
        TCR_ORGN_WBWA = (TCR_ORGN0_WBWA | TCR_ORGN1_WBWA),
        TCR_CACHE_FLAGS = (TCR_IRGN_WBWA | TCR_ORGN_WBWA),

        TCR_SH0_INNER = 3ul << 12,
        TCR_SH1_INNER = 3ul << 28,

        TCR_SHARED = (TCR_SH0_INNER | TCR_SH1_INNER),

        TCR_TG0_64K = (1UL << 14),
        TCR_TG1_64K = (3UL << 30),
        TCR_TG_FLAGS = (TCR_TG0_64K | TCR_TG1_64K),

        TCR_ASID16 = (0UL << 36),
        TCR_TBI0 = (1UL << 37),
        TCR_EPD1 = (1 << 23),
        TCR_IPS = (3UL << 32),

        TCR_VALUE = (TCR_TxSZ | TCR_CACHE_FLAGS | TCR_SHARED | TCR_TG_FLAGS | TCR_ASID16 | TCR_TBI0 | TCR_EPD1 | TCR_IPS),

    };

    enum {
        //MAIR
        MAIR_DEVICE_NGNRNE	= 0,
	    MAIR_DEVICE_NGNRE	= 1,
        MAIR_DEVICE_GRE		= 2,
	    MAIR_NORMAL_NC		= 3,
	    MAIR_NORMAL			= 4,

        MAIR_VALUE = ((0x0 << (MAIR_DEVICE_NGNRNE * 8)) | (0x04 << (MAIR_DEVICE_NGNRE * 8)) | (0x0c << (MAIR_DEVICE_GRE * 8)) |	(0x44 << (MAIR_NORMAL_NC * 8)) |(0xffLL << (MAIR_NORMAL * 8)))
    };

    // CPU Context
    class Context: public ARMv8::Context
    {
    public:
        Context() {}
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp): ARMv8::Context(usp, exit | thumb, multitask ? (usp ? MODE_USR : MODE_SVC) : MODE_SVC, exit | thumb, entry | thumb) {}
    };

protected:
    ARMv8_A() {};

public:
    // TODO
    static Flags flags() { return cpsr(); }
    static void flags(Flags flags) { cpsr(flags); }

    //DONE
    static Reg daif() { Reg r; ASM("mrs %0, daif" : "=r"(r)); return r; }
    static void daif(Reg r) { ASM("msr daif, %0" : : "r"(r) :);}

    // DONE
    static Reg nzcv() {Reg r; ASM("mrs %0, nzcv" : "=r"(r)); return r;}
    static void nzcv(Reg r) { ASM("msr nzcv, %0" : : "r"(r) :);}

    // DONE
    static Reg elr_el1() {Reg r; ASM("mrs %0, elr_el1" : "=r"(r)); return r;}
    static void elr_el1(Reg r) { ASM("msr elr_el1, %0" : : "r"(r) :);}

    static Reg elr_el2() {Reg r; ASM("mrs %0, elr_el2" : "=r"(r)); return r;}
    static void elr_el2(Reg r) { ASM("msr elr_el2, %0" : : "r"(r) :);}

    static Reg elr_el3() {Reg r; ASM("mrs %0, elr_el3" : "=r"(r)); return r;}
    static void elr_el3(Reg r) { ASM("msr elr_el3, %0" : : "r"(r) :);}

    // DONE
    // esse é igual o sctlr_el1 mas não tirei pra não quebrar nada 
    static Reg sctlr() { Reg r; ASM("mrs %0, sctlr_el1" : "=r"(r)); return r; }
    static void sctlr(Reg r) { ASM("msr sctlr_el1, %0" : : "r"(r) :); }

    // DONE
    static Reg sctlr_el1() { Reg r; ASM("mrs %0, sctlr_el1" : "=r"(r)); return r; }
    static void sctlr_el1(Reg r) { ASM("msr sctlr_el1, %0" : : "r"(r) :); }

    static Reg sctlr_el2() { Reg r; ASM("mrs %0, sctlr_el2" : "=r"(r)); return r; }
    static void sctlr_el2(Reg r) { ASM("msr sctlr_el2, %0" : : "r"(r) :); }

    static Reg sctlr_el3() { Reg r; ASM("mrs %0, sctlr_el3" : "=r"(r)); return r; }
    static void sctlr_el3(Reg r) { ASM("msr sctlr_el3, %0" : : "r"(r) :); }


    //DONE
    static unsigned int id() {
        Reg id;
        ASM("mrs %0, mpidr_el1" : "=r"(id) : : );
        return id & 0x3;
    }

    //DONE
    static unsigned int el() {
        Reg el;
        ASM("mrs %0, CurrentEL" : "=r"(el) : : );
        ASM("and %0, %0, #12" : "=r"(el) : : );
        ASM("lsr %0, %0, #2" : "=r"(el) : : );
        return el;
    }

    static unsigned int cores() {
        if(Traits<Build>::MODEL == Traits<Build>::Raspberry_Pi3) {
            return Traits<Build>::CPUS;
        } else {
            //TODO
            return 0;
        }
    }

    //DONE
    static void int_enable() {  flags(flags() & ~(FLAG_F | FLAG_I)); }
    static void int_disable() { flags(flags() | (FLAG_F | FLAG_I)); }

    //DONE
    static bool int_enabled() { return !int_disabled(); }
    static bool int_disabled() { return flags() & (FLAG_F | FLAG_I); }

    static void smp_barrier(unsigned long cores = Traits<Build>::CPUS) { CPU_Common::smp_barrier<&finc>(cores, id()); }

    static void fpu_save() {  }
    static void fpu_restore() { }

    // ARMv8-A specifics
    // TODO: REMOVE ARMV7 REGISTERS
    static Reg cpsr() { return 0; }
    static void cpsr(Reg r) {  }

    static Reg cpsrc() { return 0; }
    static void cpsrc(Reg r) {}

    static void psr_to_r12() {}
    static void r12_to_psr() { }

    static void save_regs(bool ret = false) {
        // if(ret)
        //     ASM("stmfd sp!, {r0-r3, r12, lr, pc}");
        // else
        //     ASM("stmfd sp!, {r0-r3, r12, lr}");
    }

    static void restore_regs(bool ret = false) {
    }

    static void mode(unsigned int m) { }

    static void svc_enter(unsigned int from, bool ret = true) {
    }

    static void svc_leave() {
    }

    static void svc_stay() { restore_regs(false); }

    static Reg elr_hyp() { return 0; }
    static void elr_hyp(Reg r) { }

    static void ldmia() {}
    static void stmia() { }

    // CP15 operations
    //TODO
    static Reg ttbr0() {return 0;}
    static void ttbr0(Reg r) {}

    static Reg ttbr1() {return 0;}
    static void ttbr1(Reg r) {}

    static Reg ttbcr() { return 0; }
    static void ttbcr(Reg r) { }

    static Reg dacr() { return 0; }
    static void dacr(Reg r) { }

    static Reg tcr() {return 0;}
    static void tcr(Reg r) {}

    static Reg mair() {return 0;}
    static void mair(Reg r) {}

    static Reg pd() { return ttbr0(); }
    static void pd(Reg r) {  ttbr0(r); }

    static void flush_tlb() {  } // TLBIALL - invalidate entire unifed TLB
    static void flush_tlb(Reg r) { }

    static void flush_branch_predictors() { }

    static void flush_caches() {
        /*
        ASM("MRS X0, CLIDR_EL1                                                                                              \t\n\
             AND W3, W0, #0x07000000                         // Get 2 x Level of Coherence                                  \t\n\
             LSR W3, W3, #23                                                                                                \t\n\
             CBZ W3, Finished                                                                                               \t\n\
             MOV W10, #0                                     // W10 = 2 x cache level                                       \t\n\
             MOV W8, #1                                      // W8 = constant 0b1                                           \t\n\
         Loop1:                                                                                                             \t\n\
             ADD W2, W10, W10, LSR #1                        // Calculate 3 x cache level                                   \t\n\
             LSR W1, W0, W2                                  // extract 3-bit cache type for this level                     \t\n\
             AND W1, W1, #0x7                                                                                               \t\n\
             CMP W1, #2                                                                                                     \t\n\
             B.LT Skip                                       // No data or unified cache at this level                      \t\n\
             MSR CSSELR_EL1, X10                             // Select this cache level                                     \t\n\
             ISB                                             // Synchronize change of CSSELR                                \t\n\
             MRS X1, CCSIDR_EL1                              // Read CCSIDR                                                 \t\n\
             AND W2, W1, #7                                  // W2 = log2(linelen)-4                                        \t\n\
             ADD W2, W2, #4                                  // W2 = log2(linelen)                                          \t\n\
             UBFX W4, W1, #3, #10                            // W4 = max way number, right aligned                          \t\n\
             CLZ W5, W4                                      // W5 = 32-log2(ways), bit position of way in DC operand       \t\n\
             LSL W9, W4, W5                                  // W9 = max way number, aligned to position in DC operand      \t\n\
             LSL W16, W8, W5                                 // W16 = amount to decrement way number per iteration          \t\n\
         Loop2:                                                                                                             \t\n\
             UBFX W7, W1, #13, #15                           // W7 = max set number, right aligned                          \t\n\
             LSL W7, W7, W2                                  // W7 = max set number, aligned to position in DC operand      \t\n\
             LSL W17, W8, W2                                 // W17 = amount to decrement set number per iteration          \t\n\
         Loop3:                                                                                                             \t\n\
             ORR W11, W10, W9                                // W11 = combine way number and cache number...                \t\n\
             ORR W11, W11, W7                                // ... and set number for DC operand                           \t\n\
             DC CSW, X11                                     // Do data cache clean by set and way                          \t\n\
             SUBS W7, W7, W17                                // Decrement set number B.GE Loop3                             \t\n\
             SUBS X9, X9, X16                                // Decrement way number B.GE Loop2                             \t\n\
         Skip: ADD W10, W10, #2                              // Increment 2 x cache level                                   \t\n\
             CMP W3, W10                                                                                                    \t\n\
             DSB SY                                          // Ensure completion of previous cache maintenance operation        ");
        */
    }

    static void enable_fpu() {
    }

};

class CPU: public ARMv8_A
{
    friend class Init_System;

private:
    typedef ARMv8_A Base;

public:
    // CPU Native Data Types
    using ARMv8::Reg8;
    using ARMv8::Reg16;
    using ARMv8::Reg32;
    using ARMv8::Reg64;
    using ARMv8::Reg;
    using ARMv8::Log_Addr;
    using ARMv8::Phy_Addr;

    // CPU Context
    class Context: public Base::Context
    {
    public:
        Context() {}
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp): Base::Context(entry, exit, usp) {}

        void save() volatile ;
        void load() const volatile;

        friend Debug & operator<<(Debug & db, const Context & c) {
            db << hex
               << "{x0="  << c._x0
               << ",x1="  << c._x1
               << ",x2="  << c._x2
               << ",x3="  << c._x3
               << ",x4="  << c._x4
               << ",x5="  << c._x5
               << ",x6="  << c._x6
               << ",x7="  << c._x7
               << ",x8="  << c._x8
               << ",x9="  << c._x9
               << ",x10=" << c._x10
               << ",x11=" << c._x11
               << ",x12=" << c._x12
               << ",x13="  << c._x13
               << ",x14="  << c._x14
               << ",x15="  << c._x15
               << ",x16="  << c._x16
               << ",x17="  << c._x17
               << ",x18="  << c._x18
               << ",x19="  << c._x19
               << ",x20="  << c._x20
               << ",x21="  << c._x21
               << ",x22="  << c._x22
               << ",x23=" << c._x23
               << ",x24=" << c._x24
               << ",x25=" << c._x25
               << ",x26="  << c._x26
               << ",x27="  << c._x27
               << ",x28="  << c._x28
               << ",x29="  << c._x29
               << ",x30="  << c._x30
               << ",sp="  << &c
               << ",lr="  << c._lr
               << ",pc="  << c._pc
               << ",psr=" << c._flags
               << ",usp=" << c._usp
               << ",ulr=" << c._ulr
               << "}" << dec;
            return db;
        }
    };

public:
    CPU() {}

    using ARMv8::pc;
    using ARMv8::ra;
    using ARMv8::sp;
    using ARMv8::fr;

    using Base::id;
    using Base::cores;

    static Hertz clock() { return _cpu_clock; }
    static void clock(const Hertz & frequency); // defined along with each machine's IOCtrl
    static Hertz max_clock();
    static Hertz min_clock();
    static Hertz bus_clock() { return _bus_clock; }

    using Base::int_enable;
    using Base::int_disable;
    using Base::int_enabled;
    using Base::int_disabled;

    using ARMv8::halt;

    using Base::fpu_save;
    using Base::fpu_restore;

    using ARMv8::tsl;
    using ARMv8::finc;
    using ARMv8::fdec;
    using ARMv8::cas;

    static void switch_context(Context ** o, Context * n);

    template<typename ... Tn>
    static Context * init_stack(Log_Addr usp, Log_Addr sp, void (* exit)(), int (* entry)(Tn ...), Tn ... an) {
        sp -= sizeof(Context);
        Context * ctx = new(sp) Context(entry, exit, usp); // init_stack is called with usp = 0 for kernel threads
        init_stack_helper(&ctx->_x0, an ...);
        return ctx;
    }

    // In ARMv8, the main thread of each task gets parameters over registers, not the stack, and they are initialized by init_stack.
    template<typename ... Tn>
    static Log_Addr init_user_stack(Log_Addr usp, void (* exit)(), Tn ... an) { return usp; }

    static void syscall(void * message);
    static void syscalled();

    using CPU_Common::htole64;
    using CPU_Common::htole32;
    using CPU_Common::htole16;
    using CPU_Common::letoh64;
    using CPU_Common::letoh32;
    using CPU_Common::letoh16;

    using CPU_Common::htobe64;
    using CPU_Common::htobe32;
    using CPU_Common::htobe16;
    using CPU_Common::betoh64;
    using CPU_Common::betoh32;
    using CPU_Common::betoh16;

    using CPU_Common::htonl;
    using CPU_Common::htons;
    using CPU_Common::ntohl;
    using CPU_Common::ntohs;

private:
    template<typename Head, typename ... Tail>
    static void init_stack_helper(Log_Addr sp, Head head, Tail ... tail) {
        *static_cast<Head *>(sp) = head;
        init_stack_helper(sp + sizeof(Head), tail ...);
    }
    static void init_stack_helper(Log_Addr sp) {}

    static void context_load_helper();

    static void init();

private:
    static unsigned int _cpu_clock;
    static unsigned int _bus_clock;
};

inline CPU::Reg64 htole64(CPU::Reg64 v) { return CPU::htole64(v); }
inline CPU::Reg32 htole32(CPU::Reg32 v) { return CPU::htole32(v); }
inline CPU::Reg16 htole16(CPU::Reg16 v) { return CPU::htole16(v); }
inline CPU::Reg64 letoh64(CPU::Reg64 v) { return CPU::letoh64(v); }
inline CPU::Reg32 letoh32(CPU::Reg32 v) { return CPU::letoh32(v); }
inline CPU::Reg16 letoh16(CPU::Reg16 v) { return CPU::letoh16(v); }

inline CPU::Reg64 htobe64(CPU::Reg64 v) { return CPU::htobe64(v); }
inline CPU::Reg32 htobe32(CPU::Reg32 v) { return CPU::htobe32(v); }
inline CPU::Reg16 htobe16(CPU::Reg16 v) { return CPU::htobe16(v); }
inline CPU::Reg64 betoh64(CPU::Reg64 v) { return CPU::betoh64(v); }
inline CPU::Reg32 betoh32(CPU::Reg32 v) { return CPU::betoh32(v); }
inline CPU::Reg16 betoh16(CPU::Reg16 v) { return CPU::betoh16(v); }

inline CPU::Reg32 htonl(CPU::Reg32 v)   { return CPU::htonl(v); }
inline CPU::Reg16 htons(CPU::Reg16 v)   { return CPU::htons(v); }
inline CPU::Reg32 ntohl(CPU::Reg32 v)   { return CPU::ntohl(v); }
inline CPU::Reg16 ntohs(CPU::Reg16 v)   { return CPU::ntohs(v); }

__END_SYS

#endif
