#include "common.h"
#include "syscall.h"
#include "stdio.h"

int __sys_xxxhandler(struct pcb_t *caller, struct sc_regs* regs)
{
    /*TODO: implement syscall job */
    __sys_listsyscall(caller, regs);
    __sys_ni_syscall(caller, regs);
    __sysmemmap(caller, regs);
    __sys_killall(caller, regs);
    return 0;
}