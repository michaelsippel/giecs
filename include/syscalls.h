#ifndef _syscalls_h_
#define _syscalls_h_


enum syscall_param_type
{
    NONE,
    INT,
    STR,
    PTR, // 3 + len
};


enum syscall_param_type syscall_params[][5] =
{

    {NONE, NONE, NONE, NONE, NONE},
    {INT, NONE, NONE, NONE, NONE}, // 0x01 - exit
    {PTR, NONE, NONE, NONE, NONE}, // 0x02 - fork
    {INT, STR, INT, NONE, NONE}, // 0x03 - read
    {INT, STR, INT, NONE, NONE}, // 0x04 - write
    {STR, INT, NONE, NONE, NONE}, // 0x05 - open
    {INT, NONE, NONE, NONE, NONE} // 0x06 - close

};

#endif

