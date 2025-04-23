#include "y86.h"
#include<string>
using namespace std;

bool in(std::initializer_list<uint8_t> ins)
{
    auto code=ins.begin();
    for(auto n=code+1;n<ins.end();n++)
    {
        if(*code==*n) return true;
    }
    return false;
}

int hex_to_dec(char ch)
{
    if(ch>='0' && ch<='9')
    {
        return ch-'0';
    }
    else if(ch>='a' && ch<='f')
    {
        return ch-'a'+10;
    }
    else if(ch>='A' && ch<='F')
    {
        return ch-'A'+10;
    }
    else
    {
        return -1;
    }
}

int getadr(char* line)
{
    int address=0;
    for(int i=2;line[i]!=':';i++)
    {
        int temp=hex_to_dec(line[i]);
        if(temp==-1)
        {
            std::cout<<"invalid address"<<std::endl;
        }
        else
        {
            address=address*16+temp;
        }
    }
    return address;
}

uint8_t cpu::gethigh(uint8_t val)
{
    return (val&0xf0)>>4;
}

uint8_t cpu::getlow(uint8_t val)
{
    return val&0xf;
}

uint64_t cpu::getval(uint64_t offset)
{
    return *(uint64_t*)(MEM+offset);
}

void cpu::fetch()
{
    route.imem_error=PC<0 || PC>=mem_size;
    if(route.imem_error)
    {
        route.icode=nop;
        route.ifun=0;
    }
    else
    {
        route.icode=gethigh(MEM[PC]);
        //cout<<(int)route.icode<<'\n';
        route.ifun=getlow(MEM[PC]);
        //cout<<(int)route.ifun<<'\n';
        route.instr_invalid=invalid();
        //cout<<(int)route.instr_invalid<<'\n';
        std::initializer_list need_regs_ins={route.icode,rrmovq,irmovq,rmmovq,mrmovq,opq,pushq,popq};
        bool need_regs=in(need_regs_ins);
        if(need_regs)
        {
            route.rA=gethigh(MEM[PC+1]);
            route.rB=getlow(MEM[PC+1]);
        }
        std::initializer_list need_ins_valc={route.icode,irmovq,rmmovq,mrmovq,jxx,call};
        bool need_valC=in(need_ins_valc);
        if(need_valC)
        {
            route.valC=getval(PC+1+need_regs);
            //cout<<(long long)route.valC<<'\n';
        }
        route.valP=PC+1+need_regs+8*need_valC;
        //cout<<(long long)route.valP<<'\n';
    }
}

bool cpu::invalid()
{
    std::initializer_list no_fun={route.icode,halt,nop,irmovq,rmmovq,mrmovq,call,ret,pushq,popq};
    if(route.icode>0xB)
    {
        //cout<<'1';
        return 1;
    }
    else if(route.icode==rrmovq && route.ifun>0x6)
    {
        //cout<<'2';
        return 1;
    }
    else if(route.icode==jxx && route.ifun>0x6)
    {
        //cout<<'3';
        return 1;
    }
    else if(route.icode==opq && route.ifun>=op_inalid)
    {
        //cout<<'4';
        return 1;
    }
    else if(in(no_fun) && route.ifun)
    {
        //cout<<'5';
        return 1;
    }
    return 0;
}

void cpu::decode()
{
    setsrcA();
    setsrcB();
    setdstE();
    setdstM();
    route.valA=REG[route.srcA];
    route.valB=REG[route.srcB];
}

void cpu::setsrcA()
{
    std::initializer_list ins_A={route.icode,rrmovq,rmmovq,opq,pushq};
    std::initializer_list ins_P={route.icode,popq,ret};
    if(in(ins_A))
    {
        route.srcA=route.rA;
    }
    else if(in(ins_P))
    {
        route.srcA=rsp;
    }
    else
    {
        route.srcA=rnone;
    }
}

void cpu::setsrcB()
{
    std::initializer_list ins_B={route.icode,opq,rmmovq,mrmovq};
    std::initializer_list ins_P={route.icode,pushq,popq,call,ret};
    if(in(ins_B))
    {
        route.srcB=route.rB;
    }
    else if(in(ins_P))
    {
        route.srcB=rsp;
    }
    else
    {
        route.srcB=rnone;
    }
}

void cpu::setdstE()
{
    std::initializer_list dst_B={route.icode,rrmovq,irmovq,opq};
    std::initializer_list dst_P={route.icode,pushq,popq,call,ret};
    if(in(dst_B))
    {
        route.dstE=route.rB;
    }
    else if(in(dst_P))
    {
        route.dstE=rsp;
    }
    else
    {
        route.dstE=rnone;
    }
}

void cpu::setdstM()
{
    std::initializer_list dst_A={route.icode,mrmovq,popq};
    if(in(dst_A))
    {
        route.dstM=route.rA;
    }
    else
    {
        route.dstM=rnone;
    }
}

void cpu::execute()
{
    bool set_cc=0;
    if(route.icode==opq)
    {
        set_cc=1;
    }
    long long alua=getvalA();
    long long alub=getvalB();
    route.valE=alu(alua,alub);
    //cout<<alua<<'\n';
    //cout<<alub<<'\n';
    //cout<<route.valE<<'\n';
    if(set_cc)
    {
        CC[ZF]=(route.valE==0);
        CC[SF]=((long long)route.valE<0);
        CC[OF]=(((long long)alua>0) && ((long long)alub>0) &&((long long)route.valE<0)) || (((long long)alua<0) && ((long long)alub<0) && ((long long)route.valE>0));
    }
    route.cnd=set_cnd();
    if(route.icode==rrmovq && !route.cnd)
    {
        route.dstE=rnone;
    }
}

long long cpu::getvalA()
{
    std::initializer_list ins_valc={route.icode,irmovq,rmmovq,mrmovq};
    if(route.icode==rrmovq || route.icode==opq)
    {
        return route.valA;
    }
    else if(in(ins_valc))
    {
        return route.valC;
    }
    else if(route.icode==call || route.icode==pushq)
    {
        return -8;
    }
    else if(route.icode==ret || route.icode==popq)
    {
        return 8;
    }
    return 0;
}

long long cpu::getvalB()
{
    std::initializer_list valb_ins={route.icode,rmmovq,mrmovq,opq,call,pushq,ret,popq};
    if(in(valb_ins))
    {
        return route.valB;
    }
    else if(route.icode==rrmovq || route.icode==irmovq)
    {
        return 0;
    }
    return 0;
}

uint64_t cpu::alu(long long vala,long long valb)
{
    if(route.icode==opq)
    {
        switch(route.ifun)
        {
            case addq: return (valb+vala);
            case subq: return (valb-vala);
            case andq: return (valb&vala);
            case xorq: return (valb^vala);
            case orq:  return (valb|vala);
            case shlq: return (valb<<vala);
            case shrq: return (valb>>vala);
            case mulq: return (valb*vala);
            case divq: return (valb/vala);
            case minq: return (valb<vala?valb:vala);
            case maxq: return (valb>vala?valb:vala);
        }
    }

    return (valb+vala);

}

bool cpu::set_cnd()
{
    switch(route.ifun)
    {
        case uncon: return 1;
        case fle: return CC[ZF] || (CC[SF]^CC[OF]);
        case fl: return CC[SF]^CC[OF];
        case fe: return CC[ZF];
        case fne: return !CC[ZF];
        case fge: return CC[ZF] || (!(CC[SF]^CC[OF]));
        case fg: return !(CC[SF]^CC[OF]);
    }
    return 0;
}

void cpu::memory()
{
    uint64_t mem_address=getaddress();
    //cout<<(long long)mem_address<<'\n';
    uint64_t mem_data=getdata();
    std::initializer_list ins_readmem={route.icode,mrmovq,popq,ret};
    std::initializer_list ins_writemem={route.icode,rmmovq,pushq,call};
    bool mem_read=in(ins_readmem);
    bool mem_write=in(ins_writemem);
    if(mem_read)
    {
        route.dmem_error=readmem(mem_address);
    }
    else if(mem_write)
    {
        route.dmem_error=writemem(mem_address,mem_data);
    }
    set_state();
    STAT=route.status;
}
uint64_t cpu::getaddress()
{
    std::initializer_list ins_vale={route.icode,rmmovq,pushq,call,mrmovq};
    std::initializer_list ins_vala={route.icode,popq,ret};
    if(in(ins_vale))
    {
        return route.valE;
    }
    else if(in(ins_vala))
    {
        return route.valA;
    }
    return 0;
}

uint64_t cpu::getdata()
{
    std::initializer_list ins_vala={route.icode,rmmovq,pushq};
    if(in(ins_vala))
    {
        return route.valA;
    }
    else if(route.icode==call)
    {
        return route.valP;
    }
    return 0;
}

bool cpu::writemem(uint64_t address,uint64_t data)
{
    if(address>mem_size)
    {
        return 1;
    }

    *(long long*)(MEM+address)=data;
    return 0;
    
}

bool cpu::readmem(uint64_t address)
{
    if(address+8>mem_size)
    {
        return 1;
    }
    else
    {
        route.valM=*(uint64_t*)(MEM+address);
        return 0;
    }
}

void cpu::set_state()
{
    if(route.imem_error || route.dmem_error)
    {
        route.status=SADR;
    }
    else if(route.instr_invalid)
    {
        route.status=SINS;
    }
    else if(route.icode==halt)
    {
        route.status=SHLT;
    }
    else
    {
        route.status=SAOK;
    }
}

void cpu::writeback()
{
    if(route.dstE!=rnone && !route.dmem_error)
    {
        REG[route.dstE]=route.valE;
    }
    if(route.dstM!=rnone)
    {
        REG[route.dstM]=route.valM;
    }
}

void cpu::updatepc()
{
    route.newpc=newpc();
    if(STAT==SAOK)
    {
        PC=route.newpc;
    }
}
uint64_t cpu::newpc()
{
    if(route.icode==call)
    {
        return route.valC;
    }
    else if((route.icode==jxx) && route.cnd)
    {
        return route.valC;
    }
    else if(route.icode==ret)
    {
        return route.valM;
    }
    else
    {
        return route.valP;
    }
}

void cpu::runcpu()
{
    fetch();
    decode();
    execute();
    memory();
    writeback();
    updatepc();
}
void cpu::input()
{
    char line[160];

    while(std::cin.getline(line,160))
    {
        if(line[0]=='0' && hex_to_dec(line[7])!=-1)
        {
            int address=getadr(line);
            //cout<<address<<'\n';
            char val[16];
            int index=0,temp=0;
            for(int i=7;hex_to_dec(line[i])!=-1;i+=2,index++)
            {
                temp=hex_to_dec(line[i])*16+hex_to_dec(line[i+1]);
                val[index]=temp;
            }
            for(int i=0;i<index;i++)
            {
                MEM[address+i]=val[i];
            }
        }
    }
}

void cpu::output()
{
    cout << "- "<<"PC: " << (uint64_t)PC << "\n";


    cout << "  REG: \n";
    cout <<"    "<<"rax: " << (long long)REG[rax]<<'\n';
    cout <<"    "<<"rcx: " << (long long)REG[rcx]<<'\n';
    cout <<"    "<<"rdx: " << (long long)REG[rdx]<<'\n';
    cout <<"    "<<"rbx: " << (long long)REG[rbx]<<'\n';
    cout <<"    "<<"rsp: " << (long long)REG[rsp]<<'\n';
    cout <<"    "<<"rbp: " << (long long)REG[rbp]<<'\n';
    cout <<"    "<<"rsi: " << (long long)REG[rsi]<<'\n';
    cout <<"    "<<"rdi: " << (long long)REG[rdi]<<'\n';
    cout <<"    "<<"r8: " << (long long)REG[r8]<<'\n';
    cout <<"    "<<"r9: " << (long long)REG[r9]<<'\n';
    cout <<"    "<<"r10: " << (long long)REG[r10]<<'\n';
    cout <<"    "<<"r11: " << (long long)REG[r11]<<'\n';
    cout <<"    "<<"r12: " << (long long)REG[r12]<<'\n';
    cout <<"    "<<"r13: " << (long long)REG[r13]<<'\n';
    cout <<"    "<<"r14: " << (long long)REG[r14]<<'\n';


    cout<< "  MEM: \n";
    for (int i = 0; i < mem_size/8; i += 8) {
        long long content = *(long long*)(MEM + i);
        if (content != 0) {
            cout<< "    " << i << ": " << content << "\n";
        }
    }

    cout << "  CC: \n";
    cout << "    ZF: " << CC[ZF] << "\n";
    cout << "    SF: " << CC[SF] << "\n";
    cout << "    OF: " << CC[OF] << "\n";

    cout<< "  STAT: " << (int)STAT << "\n";

}

void cpu::outins()
{
    std::string ra,rb,cn,op;
    cout<<"Instructions: ";
    switch(route.icode)
    {
        case halt: 
        {
            cout<<"halt"<<'\n';
            break;
        }
        case nop: 
        {
            cout<<"nop"<<'\n';
            break;
        }
        case rrmovq:
        {
            ra=getreg(route.rA);
            rb=getreg(route.rB);  
            if(!route.ifun)
            {
                cout<<"rrmovq"<<' '<<ra<<", "<<rb<<'\n';
                break;
            }
            else
            {
                cn=getjxx(route.ifun);
                cout<<"cmov"<<cn<<' '<<ra<<", "<<rb<<'\n';
                break;
            }
        }
        case irmovq:
        {
            long long val=route.valC;
            rb=getreg(route.rB);
            cout<<"irmovq"<<' '<<"$"<<val<<","<<rb<<'\n';
            break;
        }
        case rmmovq:
        {
            ra=getreg(route.rA);
            rb=getreg(route.rB); 
            long long offset=route.valC;
            if(offset)
            {
                cout<<"rmmovq"<<' '<<ra<<','<<offset<<'('<<rb<<')'<<'\n';
                break;
            }
            else
            {
                cout<<"rmmovq"<<' '<<ra<<",("<<rb<<')'<<'\n';
                break;
            }
        }
        case mrmovq:
        {
            ra=getreg(route.rA);
            rb=getreg(route.rB); 
            long long offset=route.valC;
            if(offset)
            {
                cout<<"mrmovq"<<' '<<offset<<'('<<rb<<"),"<<ra<<'\n';
                break;
            }
            else
            {
                cout<<"mrmovq"<<' '<<'('<<rb<<"),"<<ra<<'\n';
                break;
            }
        }
        case opq:
        {
            ra=getreg(route.rA);
            rb=getreg(route.rB);
            op=getop(route.ifun);
            cout<<op<<' '<<ra<<','<<rb<<'\n';
            break;
        }
        case jxx:
        {
            long long dst=route.valC;
            if(route.ifun)
            {
                cn=getjxx(route.ifun);
                cout<<'j'<<cn<<' '<<dst<<'\n';
                break;
            }
            else
            {
                cout<<"jmp"<<' '<<dst<<'\n';
                break;
            }
        }
        case call:
        {
            long long dst=route.valC;
            cout<<"call"<<' '<<dst<<'\n';
            break;
        }
        case ret:
        {
            cout<<"ret"<<'\n';
            break;
        }
        case pushq:
        {
            ra=getreg(route.rA);
            cout<<"pushq"<<' '<<ra<<'\n';
            break;
        }
        case popq:
        {
            ra=getreg(route.rA);
            cout<<"popq"<<' '<<ra<<'\n';
            break;
        }
    }
}

std::string cpu::getreg(uint8_t reg)
{
    switch(reg)
    {
        case rax: return "%rax";
        case rcx: return "%rcx";
        case rdx: return "%rdx";
        case rbx: return "%rbx";
        case rsp: return "%rsp";
        case rbp: return "%rbp";
        case rsi: return "%rsi";
        case rdi: return "%rdi";
        case r8: return "%r8";
        case r9: return "%r9";
        case r10: return "%r10";
        case r11: return "%r11";
        case r12: return "%r12";
        case r13: return "%r13";
        case r14: return "%r14";
    }
    return " ";
}

std::string cpu::getop(uint8_t op)
{
    switch(op)
    {
        case addq: return "addq";
        case subq: return "subq";
        case andq: return "andq";
        case xorq: return "xorq";
        case orq: return "orq";
        case shlq: return "shlq";
        case shrq: return "shrq";
        case mulq: return "mulq";
        case divq: return "divq";
        case minq: return "minq";
        case maxq: return "maxq";
    }
    return " ";
}

std::string cpu::getjxx(uint8_t fn)
{
    switch(fn)
    {
        case fle: return "le";
        case fl: return "l";
        case fe: return "e";
        case fne: return "ne";
        case fge: return "ge";
        case fg: return "g";
    }
    return " ";
}

std::string cpu::getstat(uint8_t stat)
{
    switch(stat)
    {
        case SADR: return "INVALID ADDRESS";
        case SINS: return "INVALID INSTRUCTION";
    }
    return " ";
}