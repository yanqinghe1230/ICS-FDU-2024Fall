#include "context.h"
#include "assert.h"
#include "stdlib.h"
#include "string.h"
#include <stdio.h>
#include <stdint.h>
__generator __main_gen;
__generator* __now_gen = &__main_gen;

// Task 2

void __err_stk_push(__ctx* ctx){
    assert(ctx != 0);
    __err_stk_node* newnode=(__err_stk_node*)malloc(sizeof(__err_stk_node));
    newnode->ctx=ctx;
    newnode->prev=__now_gen->__err_stk_head;
    __now_gen->__err_stk_head=newnode;
}

__ctx* __err_stk_pop(){
    assert(__now_gen->__err_stk_head != 0);
    __err_stk_node* temp=__now_gen->__err_stk_head;
    __now_gen->__err_stk_head=temp->prev;
    __ctx* content=temp->ctx;
    free(temp);
    return content;

}

void __err_cleanup(const int* error){
    int __err_try=*error;
    if(__err_try==0) __err_stk_pop();
}

int __check_err_stk(){
    return __now_gen->__err_stk_head == 0;
}

// Task 3

int send(__generator* gen, int value) {
    if (gen == 0) throw(ERR_GENNIL);
    gen->data = value;

    gen->caller=__now_gen; 
    int status=__ctx_save(&__now_gen->ctx); 
    if(status==0) 
    {
        __now_gen=gen;
        __ctx_restore(&gen->ctx,1);
    }
    else
    {
        __now_gen=__now_gen->caller;
    }

    return gen->data;
}

int yield(int value) {
    if (__now_gen->caller == 0) throw(ERR_GENNIL);
    __now_gen->data = value;
    int status= 0;
    status=__ctx_save(&__now_gen->ctx);
    if(status==0) //首次调用生成器
    {
        __ctx_restore(&__now_gen->caller->ctx,1);
    }
    return __now_gen->data;

}
void start(int arg,__generator* gen)
{
    gen->f(arg);
    throw(ERR_GENEND);   
}
__generator* generator(void (*f)(int), int arg) {
    __generator* gen=malloc(sizeof(*gen));
    memset(gen,0,sizeof(*gen)); //初始化
    gen->f=f;
    gen->stack=malloc(8192);//给generator的栈分配8192字节的空间
    uint64_t* ptr = (uint64_t*)gen->ctx;
    void* retaddress=start;
    ptr[0]=(uint64_t)(gen->stack+8176); 
    ptr[1]=(uint64_t)(gen->stack+8176); 
    ptr[2]=arg;
    ptr[4]=(uint64_t)retaddress;
    ptr[3]=(uint64_t)gen;
    return gen;
}
void generator_free(__generator** gen) {
    if (*gen == NULL) throw(ERR_GENNIL);
    free((*gen)->stack);
    while ((*gen)->__err_stk_head != 0) {
        __err_stk_node* tmp = (*gen)->__err_stk_head;
        (*gen)->__err_stk_head = (*gen)->__err_stk_head->prev;
        free(tmp->ctx);
        free(tmp);
    }
    free(*gen);
    *gen = NULL;
}                                                                           