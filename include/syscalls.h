#ifndef _syscalls_h_
#define _syscalls_h_

#include <asm/ptrace.h>

#include <context.h>

enum syscall_param_type
{
    NONE,
    INT,
    STR,
    PTR_L_BX,
    PTR_L_CX,
    PTR_L_DX,
    PTR_L_SI,
    PTR_L_DI,
    PTR, // 8 + len
};

int syscall_params[][5] =
{

    {NONE, NONE, NONE, NONE, NONE},
    {INT, NONE, NONE, NONE, NONE}, // 0x01 - exit
    {PTR+sizeof(struct pt_regs), NONE, NONE, NONE, NONE}, // 0x02 - fork
    {INT, PTR_L_DX, INT, NONE, NONE}, // 0x03 - read
    {INT, PTR_L_DX, INT, NONE, NONE}, // 0x04 - write
    {STR, INT, INT, NONE, NONE}, // 0x05 - open
    {INT, NONE, NONE, NONE, NONE}, // 0x06 - close
    {INT, PTR+VWORD_SIZE, INT, NONE, NONE} // 0x07 - waitpid
};

#endif

