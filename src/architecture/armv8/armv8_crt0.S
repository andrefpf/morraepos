// EPOS ARMv8 Program Starter

        .file "armv8_crt0.S"

        .align  2
        .global _start
        .type   _start, function
_start:

        bl      _init

        .align  2
        .globl  __epos_app_entry
        .type   __epos_app_entry, function
__epos_app_entry:
        // Main's stack was allocated by Thread::init()
        bl      main
        //push    {r0}         // save main's return value to be used by exit()
        bl      _fini
        bl      _exit
