// EPOS ARMv8 PMU Events Declaration

#include <architecture/pmu.h>

#if defined(__mmod_zynq__) | defined(__mmod_raspberry_pi3__) | defined(__mmod_realview_pbx__)

__BEGIN_SYS

const ARMv8_A_PMU::Event ARMv8_A_PMU::_events[ARMv8_A_PMU::EVENTS] = {
    ARMv8_A_PMU::INSTRUCTIONS_ARCHITECTURALLY_EXECUTED,  // 0
    ARMv8_A_PMU::IMMEDIATE_BRANCH,                       // 1
    ARMv8_A_PMU::CYCLE,                                  // 2
    ARMv8_A_PMU::BRANCHES_ARCHITECTURALLY_EXECUTED,      // 3
    ARMv8_A_PMU::MISPREDICTED_BRANCH,                    // 4
    ARMv8_A_PMU::L1D_ACCESS,                             // 5
    ARMv8_A_PMU::L2D_ACCESS,                             // 6
    ARMv8_A_PMU::L1D_REFILL,                             // 7
    ARMv8_A_PMU::DATA_MEMORY_ACCESS,                     // 8 (LLC MISS)
    ARMv8_A_PMU::L1I_REFILL,                             // 9
    ARMv8_A_PMU::L1I_TLB_REFILL,                         // 10
    ARMv8_A_PMU::PREDICTABLE_BRANCH_EXECUTED,            // 11
    ARMv8_A_PMU::L1D_WRITEBACK,                          // 12
    ARMv8_A_PMU::L2D_WRITEBACK,                          // 13
    ARMv8_A_PMU::L2D_REFILL,                             // 14
    ARMv8_A_PMU::UNALIGNED_LOAD_STORE,                   // 15
    ARMv8_A_PMU::L1I_ACCESS,                             // 16
    ARMv8_A_PMU::L1D_TLB_REFILL,                         // 17
    ARMv8_A_PMU::EXCEPTION_TAKEN,                        // 18
    ARMv8_A_PMU::BUS_ACCESS,                             // 19
    ARMv8_A_PMU::LOCAL_MEMORY_ERROR,                     // 20
    ARMv8_A_PMU::INSTRUCTION_SPECULATIVELY_EXECUTED,     // 21
    ARMv8_A_PMU::BUS_CYCLE,                              // 22
    ARMv8_A_PMU::CHAIN,                                  // 23
    ARMv8_A_PMU::// Cortex-A9 specific events
    ARMv8_A_PMU::JAVA_BYTECODE_EXECUTE,                  // 24
    ARMv8_A_PMU::SOFTWARE_JAVA_BYTECODE_EXECUTED,        // 25
    ARMv8_A_PMU::JAZELLE_BACKWARDS_BRANCHES_EXECUTED,    // 26
    ARMv8_A_PMU::COHERENT_LINEFILL_MISS,                 // 27
    ARMv8_A_PMU::COHERENT_LINEFILL_HIT,                  // 28
    ARMv8_A_PMU::ICACHE_DEPENDENT_STALL_CYCLES,          // 29
    ARMv8_A_PMU::DCACHE_DEPENDENT_STALL_CYCLES,          // 30
    ARMv8_A_PMU::MAIN_TLB_MISS_STALL_CYCLES,             // 31
    ARMv8_A_PMU::STREX_PASSED,                           // 32
    ARMv8_A_PMU::STREX_FAILED,                           // 33
    ARMv8_A_PMU::DATA_EVICTION,                          // 34
    ARMv8_A_PMU::ISSUE_DOESNT_DISPATCH,                  // 35
    ARMv8_A_PMU::ISSUE_EMPTY,                            // 36
    ARMv8_A_PMU::ISSUE_CORE_RENAMING,                    // 37
    ARMv8_A_PMU::PREDICTABLE_FUNCTION_RETURNS,           // 38
    ARMv8_A_PMU::MAIN_EXECUTION_UNIT_RETURNS,            // 39
    ARMv8_A_PMU::SECOND_EXECUTION_UNIT_RETURNS,          // 40
    ARMv8_A_PMU::LOAD_STORE_INSTRUCTIONS,                // 41
    ARMv8_A_PMU::FLOATING_POINT_INSTRUCTIONS,            // 42
    ARMv8_A_PMU::NEON_INSTRUCTIONS,                      // 43
    ARMv8_A_PMU::PROCESSOR_STALL_PLD,                    // 44
    ARMv8_A_PMU::PROCESSOR_STALL_WRITE_MEMORY,           // 45
    ARMv8_A_PMU::PROCESSOR_STALL_ITLB_MISS,              // 46
    ARMv8_A_PMU::PROCESSOR_STALL_DTLB_MISS,              // 47
    ARMv8_A_PMU::PROCESSOR_STALL_IUTLB_MISS,             // 48
    ARMv8_A_PMU::PROCESSOR_STALL_DUTLB_MISS,             // 49
    ARMv8_A_PMU::PROCESSOR_STALL_DMB,                    // 50
    ARMv8_A_PMU::INTEGER_CLOCK_ENABLED,                  // 51
    ARMv8_A_PMU::DATA_ENGINE_CLOCK_ENABLED,              // 52
    ARMv8_A_PMU::ISB_INSTRUCTIONS,                       // 53
    ARMv8_A_PMU::DSB_INSTRUCTIONS,                       // 54
    ARMv8_A_PMU::DMB_INSTRUCTIONS,                       // 55
    ARMv8_A_PMU::EXTERNAL_INTERRUPTS,                    // 56
    ARMv8_A_PMU::PLE_CACHE_LINE_REQUEST_COMPLETED,       // 57
    ARMv8_A_PMU::PLE_CACHE_LINE_REQUEST_SKIPPED,         // 58
    ARMv8_A_PMU::PLE_FIFO_FLUSH,                         // 59
    ARMv8_A_PMU::PLE_REQUEST_COMPLETED,                  // 60
    ARMv8_A_PMU::PLE_FIFO_OVERFLOW,                      // 61
    ARMv8_A_PMU::PLE_REQUEST_PROGRAMMED,                 // 62
    ARMv8_A_PMU::// ARM Cortex-A53 specific events
    ARMv8_A_PMU::BUS_ACCESS_LD,                          // 63
    ARMv8_A_PMU::BUS_ACCESS_ST,                          // 64
    ARMv8_A_PMU::BR_INDIRECT_SPEC,                       // 65
    ARMv8_A_PMU::EXC_IRQ,                                // 66
    ARMv8_A_PMU::EXC_FIQ,                                // 67
    ARMv8_A_PMU::EXTERNAL_MEM_REQUEST,                   // 68
    ARMv8_A_PMU::EXTERNAL_MEM_REQUEST_NON_CACHEABLE,     // 69
    ARMv8_A_PMU::PREFETCH_LINEFILL,                      // 70
    ARMv8_A_PMU::ICACHE_THROTTLE,                        // 71
    ARMv8_A_PMU::ENTER_READ_ALLOC_MODE,                  // 72
    ARMv8_A_PMU::READ_ALLOC_MODE,                        // 73
    ARMv8_A_PMU::PRE_DECODE_ERROR,                       // 74
    ARMv8_A_PMU::DATA_WRITE_STALL_ST_BUFFER_FULL,        // 75
    ARMv8_A_PMU::SCU_SNOOPED_DATA_FROM_OTHER_CPU,        // 76
    ARMv8_A_PMU::CONDITIONAL_BRANCH_EXECUTED,            // 77
    ARMv8_A_PMU::IND_BR_MISP,                            // 78
    ARMv8_A_PMU::IND_BR_MISP_ADDRESS_MISCOMPARE,         // 79
    ARMv8_A_PMU::CONDITIONAL_BRANCH_MISP,                // 80
    ARMv8_A_PMU::L1_ICACHE_MEM_ERROR,                    // 81
    ARMv8_A_PMU::L1_DCACHE_MEM_ERROR,                    // 82
    ARMv8_A_PMU::TLB_MEM_ERROR,                          // 83
    ARMv8_A_PMU::EMPTY_DPU_IQ_NOT_GUILTY,                // 84
    ARMv8_A_PMU::EMPTY_DPU_IQ_ICACHE_MISS,               // 85
    ARMv8_A_PMU::EMPTY_DPU_IQ_IMICRO_TLB_MISS,           // 86
    ARMv8_A_PMU::EMPTY_DPU_IQ_PRE_DECODE_ERROR,          // 87
    ARMv8_A_PMU::INTERLOCK_CYCLE_NOT_GUILTY,             // 88
    ARMv8_A_PMU::INTERLOCK_CYCLE_LD_ST_WAIT_AGU_ADDRESS, // 89
    ARMv8_A_PMU::INTERLOCK_CYCLE_ADV_SIMD_FP_INST,       // 90
    ARMv8_A_PMU::INTERLOCK_CYCLE_WR_STAGE_STALL_BC_MISS, // 91
    ARMv8_A_PMU::INTERLOCK_CYCLE_WR_STAGE_STALL_BC_STR   // 92
};

__END_SYS

#endif