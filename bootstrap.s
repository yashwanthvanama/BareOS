# 0 "kernel/system/bootstrap.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/nfs/nfs9-insecure/home/insecure-ro/software/ubuntu-20.04/riscv-gnu-toolchain/sysroot/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "kernel/system/bootstrap.S"
# 12 "kernel/system/bootstrap.S"
 .file "bootstrap.s"
 .option arch, +zicsr
 .equ _mstatus_init, 0x80a

.section .text.entry
_start:
 csrr t0, mhartid # -.
 bne t0, x0, idle # -'    Skip setup on all harts execpt hart 0

 la t1, _mstatus_init # --
 or t0, t0, t1 # | Enable interrupts and set running state to Supervisor mode
 csrw mstatus, t0 # --

 la t0, __traps # --
 addi t0, t0, 0x1 # | Set exception and interrupt vector to the '__traps' label
 csrw mtvec, t0 # --

 la gp, _mmap_global_ptr # --
 la sp, _mmap_kstack_top # | Set initial stack pointer, global pointer,
 la t0, initialize # | and system entry function
 csrw mepc, t0 # --

 li t0, 0x0f0f # --
 li t1, 0x20000000 # |
 li t2, 0x22000000 # | Set up memory protection so that Supervisor mode
 csrw pmpcfg0, t0 # | can acccess all regions of memory
 csrw pmpaddr0, t1 # |
 csrw pmpaddr1, t2 # --

 call clk_init # -- Initialize clock interrupts
 call plic_init # -- Initialize external interrupts

 la ra, idle # -. Set the return point for the kernel to idle
 mret # -'    Return to Supervisor mode at 'initialize'

idle: # --
 wfi # | Loop forever if not hart0
 j idle # --







.globl __noop
__noop: mret
__exception:
 sd t0, -8(sp)
 csrr t0, mepc
 addi t0, t0, 0x4
 csrw mepc, t0
 ld t0, -8(sp)
 j handle_exception
.align 8
__traps: # Interrupt table index | Cause
.org __traps + 0*4 #-----------------------+---------------------------------------
  j __exception # 0 | SOFTWARE interrupt [User] or Exception
.org __traps + 1*4 #-----------------------+---------------------------------------
 j __noop # 1 | SOFTWARE interrupt [Supervisor]
.org __traps + 2*4 #-----------------------+---------------------------------------
 j __noop # 2 | ------ /reserved/
.org __traps + 3*4 #-----------------------+---------------------------------------
 j __noop # 3 | SOFTWARE interrupt [Machine]
.org __traps + 4*4 #-----------------------+---------------------------------------
 j __noop # 4 | TIMER interrupt [User]
.org __traps + 5*4 #-----------------------+---------------------------------------
 j __noop # 5 | TIMER interrupt [Supervisor]
.org __traps + 6*4 #-----------------------+---------------------------------------
 j __noop # 6 | ------ /reserved/
.org __traps + 7*4 #-----------------------+---------------------------------------
 j handle_clk # 7 | TIMER interrupt [Machine]
.org __traps + 8*4 #-----------------------+---------------------------------------
 j __noop # 8 | EXTERNAL interrupt [User]
.org __traps + 9*4 #-----------------------+---------------------------------------
 j __noop # 9 | EXTERNAL interrupt [Supervisor]
.org __traps + 10*4 #-----------------------+---------------------------------------
 j __noop # 10 | ----- /reserved/
.org __traps + 11*4 #-----------------------+---------------------------------------
 j handle_plic # 11 | EXTERNAL interrupt [Machine]
                            #-----------------------+---------------------------------------
