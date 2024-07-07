.section .text
.global print
.intel_syntax noprefix

print:
    cmp rdx, 1
    je .print_byte
    cmp rdx, 2
    je .print_word
    cmp rdx, 4
    je .print_dword
    cmp rdx, 8
    je .print_qword
    cmp rdx, 10
    je .print_string
    cmp rdx, 11
    je .print_char
    ret

.print_byte:
    mov rcx, 10
    mov rsi, rsp
    add rsi, 32
    mov byte ptr [rsi], 0xA
    dec rsi
    xor rbx, rbx
    mov bl, BYTE PTR [rdi]
.byte_loop:
    mov rax, rbx
    div rcx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test rax, rax
    jnz .byte_loop
    inc rsi
    jmp .write_string

.print_word:
    mov rcx, 10
    mov rsi, rsp
    add rsi, 32
    mov byte ptr [rsi], 0xA
    dec rsi
    xor rdx, rdx
    mov dx, WORD PTR [rdi]
.word_loop:
    mov rax, rdx
    div rcx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test rax, rax
    jnz .word_loop
    inc rsi
    jmp .write_string

.print_dword:
    mov rcx, 10
    mov rsi, rsp
    add rsi, 32
    mov byte ptr [rsi], 0xA
    dec rsi
    xor rdx, rdx
    mov edx, DWORD PTR [rdi]
.dword_loop:
    mov rax, rdx
    div rcx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test rax, rax
    jnz .dword_loop
    inc rsi
    jmp .write_string

.print_qword:
    mov rcx, 10
    mov rsi, rsp
    add rsi, 32
    mov byte ptr [rsi], 0xA
    dec rsi
    xor rdx, rdx
    mov rdx, QWORD PTR [rdi]
.qword_loop:
    mov rax, rdx
    div rcx
    add dl, '0'
    mov [rsi], dl
    dec rsi
    test rax, rax
    jnz .qword_loop
    inc rsi
    jmp .write_string

.print_string:
    mov rsi, rdi
    xor rax, rax
.strlen_loop:
    cmp byte ptr [rsi + rax], 0
    je .write_string
    inc rax
    jmp .strlen_loop

.print_char:
    mov rsi, rsp
    add rsi, 32
    mov byte ptr [rsi], al
    mov byte ptr [rsi + 1], 0xA
    mov rdx, 2
    jmp .write_string

.write_string:
    mov rax, 1
    mov rdi, 1
    syscall
    ret
