#include <stdio.h>
#include "cachelab.h"

static inline int kernel(int *w)
{
    // we set alpha to 1 and delta_t to 1.
    // equals to w[0] + alpha * (w[-1] - 2 * w[0] + w[1]);
    // example usage: A[t + 1][x] = kernel(&A[t][x]);
    return (w[-1] - w[0] + w[1]);
}

char heat_sim_desc[] = "Heat Simulate submission";
void heat_sim_example(int T, int N, int A[T][N])
{
    for (int t = 0; t < T - 1; t++)
        for (int x = 1; x < N - 1; x++)
            A[t + 1][x] = kernel(&A[t][x]);
}


void heat_sim(int T, int N, int A[T][N])
{
    for(int t0=0;t0<T;t0+=5)
    {
        int x_length=5;
        for(int t=t0;t<t0+5;t++)
        {
            for(int x=1;x<x_length;x++)
            {
                A[t+1][x]=kernel(&A[t][x]);
            }
            x_length--;
        }
        //x_length=5;
        int x_end=5;
        int x_start=5;
        while(x_end<N-2)
        {
            for(int t=t0;t<t0+5;t++)
            {
                int x0=x_start+t0-t;
                x_end=x_start+5;
                //printf("%d\n",x_end);
                for(int i=x0;i<x_end;i++)
                {
                    A[t+1][i]=kernel(&A[t][i]);
                }
            }
            x_start=x_end;    
        }
        for(int t=t0;t<t0+5;t++)
        {
            int x0=x_end+t0-t;
            for(int i=x0;i<=x_end;i++)
            {
                A[t+1][i]=kernel(&A[t][i]);
            }
        }
    }
}

void heat_sim2(int T, int N, int A[T][N])
{
    for(int t0=0;t0<T;t0+=5)
    {
        int x_length=5;
        for(int t=t0;t<t0+5;t++)
        {
            for(int x=1;x<x_length;x++)
            {
                A[t+1][x]=kernel(&A[t][x]);
            }
            x_length--;
        }
        int x_end=5;
        int x_start=5;
        while(x_end<N-3)
        {
            for(int t=t0;t<t0+5;t++)
            {
                int x0=x_start+t0-t;
                x_end=x_start+4;
                for(int i=x0;i<x_end;i++)
                {
                    //printf("%d %d\n",t,i);
                    A[t+1][i]=kernel(&A[t][i]);
                }
            }
            x_start=x_end;    
        }
        x_end=N-2;
        for(int t=t0;t<t0+5;t++)
        {
            x_start=x_end+t0-t-1;
            for(int x=x_start;x<=x_end;x++)
            {
                A[t+1][x]=kernel(&A[t][x]);
            }
        }
    }
}



void registerHeatFunctions()
{
    //registerHeatFunction(heat_sim_example, heat_sim_desc);
    registerHeatFunction(heat_sim2, heat_sim_desc);
}