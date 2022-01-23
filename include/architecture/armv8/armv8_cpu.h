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

    // SPSR bits
    typedef Reg Flags;
    enum {
        FLAG_N          = 1 << 31,       // Negative
        FLAG_Z          = 1 << 30,       // Zero
        FLAG_C          = 1 << 29,       // Carry
        FLAG_V          = 1 << 28,       // Overflow
        
        FLAG_SS         = 1 << 21,       // Software Step
        FLAG_IL         = 1 << 20,       // Ilegal
        
        FLAG_D          = 1 << 9,        // Debug disable
        FLAG_A          = 1 << 8,        // SError disable
        FLAG_I          = 1 << 7,        // IRQ disable
        FLAG_F          = 1 << 6,        // FIQ disable

        // FLAG_M values
        MODE_USR        = 0x10,
        MODE_FIQ        = 0x11,
        MODE_IRQ        = 0x12,
        MODE_SVC        = 0x13,
        MODE_ABORT      = 0x17,
        MODE_UNDEFINED  = 0x1b,
        MODE_SYS        = 0x1f
    };

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

    // SCTLR bits - TODO
    enum {
        MMU_ENABLE  = 1 << 0,  // MMU enable
        DCACHE      = 1 << 2,  // Data cache enable
        BRANCH_PRED = 1 << 11, // Z bit, branch prediction enable
        ICACHE      = 1 << 12, // Instruction cache enable
        // AFE         = 1 << 29  // Access Flag enable //TODO
    };

    // ACTLR bits - TODO
    enum {
        DCACHE_PREFE = 1 << 2, // DCache prefetch Enabled
        SMP          = 1 << 6 // SMP bit
    };


public: // SPECIAL REGISTERS

    static Reg zr() {return 0;}

    static Log_Addr pc() {Reg r; ASM("mrs %0, pc" : "=r"(r)); return r; }   // due to RISC pipelining, PC is read with a +8 (4 for thumb) offset

    // Stack Pointer Registers
    static Log_Addr sp() { Reg r; ASM("mov %0, sp" : "=r"(r) :); return r; }
    static void sp(Log_Addr sp) { ASM("mov sp, %0" : : "r"(Reg(sp))); ASM("isb"); }

    static Reg sp_el0() {Reg r; ASM("mrs %0, sp_el0" : "=r"(r)); return r; }
    static void sp_el0(Reg r) { ASM("msr sp_el0, %0" : : "r"(r) :);}

    static Reg sp_el1() {Reg r; ASM("mrs %0, sp_el1" : "=r"(r)); return r; }
    static void sp_el1(Reg r) { ASM("msr sp_el1, %0" : : "r"(r) :);}

    static Reg sp_el2() {Reg r; ASM("mrs %0, sp_el2" : "=r"(r)); return r; }
    static void sp_el2(Reg r) { ASM("msr sp_el2, %0" : : "r"(r) :);}

    static Reg sp_el3() {Reg r; ASM("mrs %0, sp_el3" : "=r"(r)); return r; }
    static void sp_el3(Reg r) { ASM("msr sp_el3, %0" : : "r"(r) :);}
    
    // Saved Processor State Registers
    static Reg spsr_el1() {Reg r; ASM("mrs %0, spsr_el1" : "=r"(r)); return r; }
    static void spsr_el1(Reg r) { ASM("msr spsr_el1, %0" : : "r"(r) :);}

    static Reg spsr_el2() {Reg r; ASM("mrs %0, spsr_el2" : "=r"(r)); return r; }
    static void spsr_el2(Reg r) { ASM("msr spsr_el2, %0" : : "r"(r) :);}

    static Reg spsr_el3() {Reg r; ASM("mrs %0, spsr_el3" : "=r"(r)); return r; }
    static void spsr_el3(Reg r) { ASM("msr spsr_el3, %0" : : "r"(r) :);}

    static Reg daif() {Reg r; ASM("mrs %0, daif" : "=r"(r)); return r; }
    static void daif(Reg r) {}

    static Reg nzcv() {Reg r; ASM("mrs %0, nzcv" : "=r"(r)); return r; }
    static void nzcv(Reg r) { ASM("msr nzcv, %0" : : "r"(r) :);}

    // Exception Link Registers
    static Reg elr_el1() {Reg r; ASM("mrs %0, elr_el1" : "=r"(r)); return r; }
    static void elr_el1(Reg r) { ASM("msr elr_el1, %0" : : "r"(r) :);}

    static Reg elr_el2() {Reg r; ASM("mrs %0, elr_el2" : "=r"(r)); return r; }
    static void elr_el2(Reg r) { ASM("msr elr_el2, %0" : : "r"(r) :);}

    static Reg elr_el3() {Reg r; ASM("mrs %0, elr_el3" : "=r"(r)); return r; }
    static void elr_el3(Reg r) { ASM("msr elr_el3, %0" : : "r"(r) :);}

    // Translation Table Base Registers 0 
    static Reg ttbr0_el1() {Reg r; ASM("mrs %0, ttbr0_el1" : "=r"(r)); return r; }
    static void ttbr0_el1(Reg r) { ASM("msr ttbr0_el1, %0" : : "r"(r) :);}

    static Reg ttbr0_el2() {Reg r; ASM("mrs %0, ttbr0_el2" : "=r"(r)); return r; }
    static void ttbr0_el2(Reg r) { ASM("msr ttbr0_el2, %0" : : "r"(r) :);}

    static Reg ttbr0_el3() {Reg r; ASM("mrs %0, ttbr0_el3" : "=r"(r)); return r; }
    static void ttbr0_el3(Reg r) { ASM("msr ttbr0_el3, %0" : : "r"(r) :);}

    // Translation Table Base Registers 1
    static Reg ttbr1_el1() {Reg r; ASM("mrs %0, ttbr1_el1" : "=r"(r)); return r; }
    static void ttbr1_el1(Reg r) { ASM("msr ttbr1_el1, %0" : : "r"(r) :);}

    static Reg ttbr1_el2() {Reg r; ASM("mrs %0, ttbr1_el2" : "=r"(r)); return r; }
    static void ttbr1_el2(Reg r) { ASM("msr ttbr1_el2, %0" : : "r"(r) :);}

    static Reg ttbr1_el3() {Reg r; ASM("mrs %0, ttbr1_el3" : "=r"(r)); return r; }
    static void ttbr1_el3(Reg r) { ASM("msr ttbr1_el3, %0" : : "r"(r) :);}

    // Current Cache Size Id Registers
    static Reg ccsidr_el1() {Reg r; ASM("mrs %0, ccsidr_el1" : "=r"(r)); return r; }
    static void ccsidr_el1(Reg r) { ASM("msr ccsidr_el1, %0" : : "r"(r) :);}

    // Cache Size Selection Registers
    static Reg csselr_el1() {Reg r; ASM("mrs %0, csselr_el1" : "=r"(r)); return r; }
    static void csselr_el1(Reg r) { ASM("msr csselr_el1, %0" : : "r"(r) :);}

    // Vector Based Address Registers
    static Reg vbar_el1() {Reg r; ASM("mrs %0, vbar_el1" : "=r"(r)); return r; }
    static void vbar_el1(Reg r) { ASM("msr vbar_el1, %0" : : "r"(r) :);}

    static Reg vbar_el2() {Reg r; ASM("mrs %0, vbar_el2" : "=r"(r)); return r; }
    static void vbar_el2(Reg r) { ASM("msr vbar_el2, %0" : : "r"(r) :);}

    static Reg vbar_el3() {Reg r; ASM("mrs %0, vbar_el3" : "=r"(r)); return r; }
    static void vbar_el3(Reg r) { ASM("msr vbar_el3, %0" : : "r"(r) :);}

    // Auxiliary Control Registers
    static Reg actlr_el1() {Reg r; ASM("mrs %0, actlr_el1" : "=r"(r)); return r; }
    static void actlr_el1(Reg r) { ASM("msr actlr_el1, %0" : : "r"(r) :);}

    static Reg actlr_el2() {Reg r; ASM("mrs %0, actlr_el2" : "=r"(r)); return r; }
    static void actlr_el2(Reg r) { ASM("msr actlr_el2, %0" : : "r"(r) :);}

    static Reg actlr_el3() {Reg r; ASM("mrs %0, actlr_el3" : "=r"(r)); return r; }
    static void actlr_el3(Reg r) { ASM("msr actlr_el3, %0" : : "r"(r) :);}

    // System Control Registers
    static Reg sctlr_el0() {Reg r; ASM("mrs %0, sctlr_el0" : "=r"(r)); return r; }
    static void sctlr_el0(Reg r) { ASM("msr sctlr_el0, %0" : : "r"(r) :);}

    static Reg sctlr_el1() {Reg r; ASM("mrs %0, sctlr_el1" : "=r"(r)); return r; }
    static void sctlr_el1(Reg r) { ASM("msr sctlr_el1, %0" : : "r"(r) :);}

    static Reg sctlr_el2() {Reg r; ASM("mrs %0, sctlr_el2" : "=r"(r)); return r; }
    static void sctlr_el2(Reg r) { ASM("msr sctlr_el2, %0" : : "r"(r) :);}

    static Reg sctlr_el3() {Reg r; ASM("mrs %0, sctlr_el3" : "=r"(r)); return r; }
    static void sctlr_el3(Reg r) { ASM("msr sctlr_el3, %0" : : "r"(r) :);}

    // Hypervisor Configuration Registers
    static Reg hcr_el2() {Reg r; ASM("mrs %0, hcr_el2" : "=r"(r)); return r; }
    static void hcr_el2(Reg r) { ASM("msr hcr_el2, %0" : : "r"(r) :);}


public:
    static void dsb() { ASM("dsb sy"); }

    static void isb() { ASM("isb"); }

    static void eret() { ASM("eret"); }

    static Reg pd() { return ttbr0_el1(); }
    static void pd(Reg r) {  ttbr0_el1(r); }

    static unsigned int id() {
        Reg id;
        ASM("mrs %0, mpidr_el1" : "=r"(id) : : );
        return id & 0x3;
    }

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
            return 0;
        }
    }

    static void int_enable() { }
    static void int_disable() { }

    static bool int_enabled() { return 0; }
    static bool int_disabled() { return 0; }

    static void smp_barrier(unsigned long cores = Traits<Build>::CPUS) { CPU_Common::smp_barrier<&finc>(cores, id()); }

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

    static void fpu_save() { }
    static void fpu_restore() { }
    static void enable_fpu() { }

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

};


class CPU: public ARMv8
{
    friend class Init_System;

private:
    typedef ARMv8 Base;

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
        Context(Log_Addr entry, Log_Addr exit, Log_Addr usp) {
            Base::Context(usp, exit, multitask ? (usp ? MODE_USR : MODE_SVC) : MODE_SVC, exit, entry);
        }

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

    static Reg sctlr() { return Base::sctlr_el1(); }
    static void sctlr(Reg r) { Base::sctlr_el1(r); }

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
