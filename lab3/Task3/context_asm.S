.text

.globl __ctx_save
__ctx_save:
    movq %rsp,(%rdi)
    movq %rbp,0x8(%rdi)
    movl $0x0,0x10(%rdi) 
    movq $0x0,0x18(%rdi)
    movq (%rsp),%rax
    movq %rax,0x20(%rdi)
    movl $0x0,%eax
    ret


.globl __ctx_restore
__ctx_restore:
    movq (%rdi), %rsp
    movq 0x8(%rdi), %rbp
    movq 0x20(%rdi), %rax
    movq %rax,(%rsp)
    movl %esi,%eax
    movq 0x18(%rdi),%rsi 
    movl 0x10(%rdi), %edi
    ret
