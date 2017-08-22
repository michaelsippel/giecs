
#pragma once

#include <asm/ptrace.h>
#include <cstdlib>
#include <iostream>
#include <giecs/ll.hpp>

namespace giecs
{

namespace ll
{

struct System
{
    LLFN(syscall)
    {
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

        static constexpr int syscall_params[][5] =
        {
            {NONE, NONE, NONE, NONE, NONE},
            {INT, NONE, NONE, NONE, NONE}, // 0x01 - exit
            {PTR+sizeof(struct pt_regs), NONE, NONE, NONE, NONE}, // 0x02 - fork
            {INT, PTR_L_DX, INT, NONE, NONE}, // 0x03 - read
            {INT, PTR_L_DX, INT, NONE, NONE}, // 0x04 - write
            {STR, INT, INT, NONE, NONE}, // 0x05 - open
            {INT, NONE, NONE, NONE, NONE}, // 0x06 - close
            {INT, PTR+1, INT, NONE, NONE} // 0x07 - waitpid
        };

        val_t regs_in[6];
        stack.pop(6, regs_in);

        long sys_id = regs_in[0];
        void* regs_out[6] = {(void*)sys_id,0,0,0,0,0};
        val_t* buf[5] = {NULL, NULL, NULL, NULL, NULL};
        size_t sizes[5] = {0, 0, 0, 0, 0};

        for(int i = 0; i < 5; ++i)
        {
            val_t c;
            int j;
            switch(syscall_params[sys_id][i])
            {
                case NONE:
                case INT:
                    regs_out[i+1] =(void*)((uintptr_t)regs_in[i+1]);
                    continue;
                case STR:
                    j = 0;
                    do
                    {
                        c = stack.read(addr_t(regs_in[i+1])+addr_t(j));
                        j++;
                    }
                    while(char(c) != '\0');
                    sizes[i] = (size_t)j;
                    break;
                case PTR_L_BX:
                    sizes[i] = regs_in[1];
                    break;
                case PTR_L_CX:
                    sizes[i] = regs_in[2];
                    break;
                case PTR_L_DX:
                    sizes[i] = regs_in[3];
                    break;
                case PTR_L_SI:
                    sizes[i] = regs_in[4];
                    break;
                case PTR_L_DI:
                    sizes[i] = regs_in[5];
                    break;
                default: // pointer
                    sizes[i] = syscall_params[sys_id][i] - PTR;
                    break;
            }
            sizes[i] = 1+ (sizes[i] * bitsize<align_t>()-1) / bitsize<val_t>();
            buf[i] = (val_t*) malloc(sizes[i] * sizeof(val_t));
            stack.read(addr_t(regs_in[i+1]), sizes[i], buf[i]);
            regs_out[i+1] = (void*) buf[i];
        }

        int retv = 0;
        asm("int $0x80;"
            : "=a" (retv) : "a" (regs_out[0]),  "b" (regs_out[1]), "c"(regs_out[2]), "d"(regs_out[3]), "S"(regs_out[4]), "D"(regs_out[5]));
        stack.push(val_t(retv));

        for(int i = 0; i < 5; ++i)
        {
            if(buf[i] != NULL)
            {
                stack.write(addr_t(regs_in[i+1]), sizes[i], buf[i]);
                free(buf[i]);
            }
        }
    }
};

} // namespace ll

} // namespace giecs

