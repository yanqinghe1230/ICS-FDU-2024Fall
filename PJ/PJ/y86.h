#ifndef Y86_64_H
#define Y86_64_H

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include "predefine.h"

#define mem_size 4096

class cpu
{
    private:
    uint64_t PC=0; 
    uint64_t REG[15]={0}; //15个64位的通用寄存器
    uint8_t MEM[mem_size]={0};
    bool CC[3]={1,0,0};
    wire route;

    public:
    uint8_t STAT=SAOK;

    //tools
    uint8_t gethigh(uint8_t val);
    uint8_t getlow(uint8_t val);
    uint64_t getval(uint64_t offset);

    //6 stages
    void fetch();
    void decode();
    void execute();
    void memory();
    void writeback();
    void updatepc();

    //fetch
    bool invalid();

    //decode
    void setsrcA();
    void setsrcB();
    void setdstE();
    void setdstM();

    //execute
    long long getvalA();
    long long getvalB();
    uint64_t alu(long long vala,long long valb);
    bool set_cnd();

    //memory
    uint64_t getaddress();
    uint64_t getdata();
    bool writemem(uint64_t address,uint64_t data);
    bool readmem(uint64_t address);
    void set_state();
    
    //update
    uint64_t newpc();
    //run
    void runcpu();
    //input
    void input();
    //output
    void output();
    //function
    void outins();
    std::string getreg(uint8_t reg);
    std::string getop(uint8_t op);
    std::string getjxx(uint8_t fn);
    std::string getstat(uint8_t stat);
};
#endif