.section text
.global print
.intel_syntax noprefix

print:
    cmp rcx, 10
    je .print_string
    cmp rcx, 9
    je .print_char
    ret

.print_char:
    mov rdx, 1
    mov rax, 1
    mov rdi, 1
    syscall
    ret

.print_string:
    mov rdx, 20
    mov rax, 1
    mov rdi, 1
    syscall
    ret

.strlen:
    push rbp
    mov rbp, rsp

    xor rdx, rdx

.strlen_loop:
    mov cl, [rsi+rdx]
    cmp cl, 0x0
    je .strlen_end
    
    inc rdx
    jmp .strlen_loop

.strlen_end:
    pop rbp
    ret