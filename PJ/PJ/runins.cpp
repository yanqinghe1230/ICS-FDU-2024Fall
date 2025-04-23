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
        mycpu.output();
        mycpu.outins();
    }

    return 0;
}
