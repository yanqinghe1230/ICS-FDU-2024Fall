#ifndef Y86_64_CONST_H
#define Y86_64_CONST_H

#include <cstdint>

//registers
const uint8_t rax=0x0;
const uint8_t rcx=0x1;
const uint8_t rdx=0x2;
const uint8_t rbx=0x3;
const uint8_t rsp=0x4;
const uint8_t rbp=0x5;
const uint8_t rsi=0x6;
const uint8_t rdi=0x7;
const uint8_t r8=0x8;
const uint8_t r9=0x9;
const uint8_t r10=0xA;
const uint8_t r11=0xB;
const uint8_t r12=0xc;
const uint8_t r13=0xD;
const uint8_t r14=0xE;
const uint8_t rnone=0XF;

//condition codes
const uint8_t ZF=0;
const uint8_t SF=1;
const uint8_t OF=2;

//status conditiions
const uint8_t SAOK=1;
const uint8_t SHLT=2;
const uint8_t SADR=3;
const uint8_t SINS=4;

//instructions
const uint8_t halt=0x0;
const uint8_t nop=0x1;
const uint8_t rrmovq=0x2;
const uint8_t irmovq=0x3;
const uint8_t rmmovq=0x4;
const uint8_t mrmovq=0x5;
const uint8_t opq=0x6; //计算
const uint8_t jxx=0x7;
const uint8_t call=0x8;
const uint8_t ret=0x9;
const uint8_t pushq=0xA;
const uint8_t popq=0xB;

//function codes
const uint8_t uncon=0x0;
const uint8_t fle=0x1;
const uint8_t fl=0x2;
const uint8_t fe=0x3;
const uint8_t fne=0x4;
const uint8_t fge=0x5;
const uint8_t fg=0x6;

//operators(can be expanded)
const uint8_t addq=0x0;
const uint8_t subq=0x1;
const uint8_t andq=0x2;
const uint8_t xorq=0x3;
const uint8_t orq=0x4;
const uint8_t shlq=0x5;
const uint8_t shrq=0x6;
const uint8_t mulq=0x7;
const uint8_t divq=0x8;
const uint8_t minq=0x9;
const uint8_t maxq=0xa;
const uint8_t op_inalid=0xb;//set an upper limit aiming to make the expansion easier

struct wire 
{   //fetch
    uint8_t icode=1;
    uint8_t ifun=0;
    uint8_t rA=rnone;
    uint8_t rB=rnone;
    uint64_t valC=0;
    uint64_t valP=0;
    //decode
    uint64_t valA=0;
    uint64_t valB=0;
    uint8_t srcA=rnone;
    uint8_t srcB=rnone;
    //execute
    uint64_t valE=0;
    bool cnd=0;
    //memory
    uint64_t valM=0;
    //writeback
    uint8_t dstE=rnone;
    uint8_t dstM=rnone;
    //update
    uint64_t newpc=0;

    uint8_t status=SAOK;
    bool instr_invalid=0;
    bool imem_error=0;
    bool dmem_error=0;
};
#endif