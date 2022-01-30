#!/bin/bash

tmux kill-session -t foo
tmux new -s foo 'qemu-system-aarch64 -M raspi3b -cpu cortex-a53 -smp 4 -gdb tcp::1236 -S -m 1G -serial null -serial mon:stdio -nographic -no-reboot -device loader,file=img/test.img,addr=0x800000,force-raw=on -kernel img/test.bin | tee img/test.out &
konsole -e aarch64-linux-gnu-gdb -ex "target remote:1236" -ex "set confirm off" \
-ex "add-symbol-file ./app/test/test" \
-ex "add-symbol-file ./img/setup_raspberry_pi3" \
-ex "add-symbol-file ./img/system_raspberry_pi3" \
-ex "add-symbol-file ./img/init_raspberry_pi3" \
-ex "b _entry()" \
-ex "b EPOS::S::ARMv8_MMU::free()" \
-ex "layout split"'


# -ex "c" \
#-ex "b *0x0080000" \
#EPOS::S::Setup::Setup