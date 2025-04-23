#include<iostream>
#include<fstream>
#include "y86.h"
#include "y86.cpp"

int main()
{
    cpu mycpu;
    mycpu.input();

    while(mycpu.STAT==SAOK)
    {
        mycpu.runcpu();
        //mycpu.output();
    }
    if(mycpu.STAT==SHLT)
    {
        cout<<"Execution successful!"<<'\n';
    }
    else
    {
        std::string status=mycpu.getstat(mycpu.STAT);
        cout<<status<<'\n';
        mycpu.outins();
    }

    return 0;
}