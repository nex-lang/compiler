section .text
global print

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
    call .strlen
    mov rax, 1
    mov rdi, 1
    syscall
    ret

.strlen:
    push rbp
    mov rbp, rsp

    xor rdx, rdx

.strlen_loop:
    cmp byte [rsi+rdx], 0x0
    je .strlen_end
    inc rdx
    jmp .strlen_loop
.strlen_end:
    pop rbp
    ret
    