#define ASM     1
.global system_calls, invalid_call, system_call_done, sys_call_table
system_calls:
    pushl %esp
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushfl

    pushl %edx
    pushl %ecx
    pushl %ebx

    cmpl $1, %eax
    jl invalid_call
    cmpl $10, %eax
    jg invalid_call

    call *sys_call_table(, %eax, 4)
    jmp system_call_done

invalid_call:
    movl $-1, %eax

system_call_done:
    addl $12, %esp
    popfl
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popl %esp
    iret

sys_call_table:
    .long 0x0
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap

