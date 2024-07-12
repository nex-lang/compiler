section .bss
    number resb 22

section .text
global print
global number

print:
    cmp rcx, 10
    je .print_string
    cmp rcx, 9
    je .print_char
    cmp rcx, 8
    je .print_int
    ret

.print_int:
    mov rsi, number

    mov rdi, 1
    mov rcx, 1
    mov rbx, 10
    
    cmp rax, 0
    jge .positive

    mov byte [rsi], '-'
    inc rdi
    neg rax

    .positive:
        push rax
    .get_devisor:
        xor rdx, rdx
        div rbx
        
        cmp rax, 0
        je ._after

        imul rcx, 10 ; increase devisor
        inc rdi ; strlen
        jmp .get_devisor

    ._after:
        pop rax ; restore
        push rdi
    
    .to_string:
        xor rdx, rdx        
        div rcx

        add al, 48 ; asci
        mov [rsi + 1], al
        inc rsi

        push rdx

        xor rdx, rdx
        mov rax, rcx
        mov rbx, 10
        div rbx
        mov rcx, rax

        pop rax

        cmp rcx, 0
        jg .to_string
    
    pop rdx

    mov byte [rsi + 1], 10
    inc rdx

    mov rax, 1
    mov rdi, 1
    mov rsi, number

    syscall
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
    