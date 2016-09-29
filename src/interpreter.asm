%define W r15
%define PC r14
%define RST r13

%include "iolib.inc"
%include "inner_words.inc"
%include "inner_functions.inc"

section .bss
   umem: resb 65536 
   dmem: resb 65536 
   rstack_mem: resb 256

section .data
   dstack_ptr:   dq 0
   last_word: dq link
   state: db 0
   here: dq dmem

   branch_error_msg1: db "Invalid usage!", 0
   nowd_message: db "Unknown word >> ", 0
 
   program_stub: dq 0  
   xt_interpreter: dq .interpreter 
   .interpreter: dq interpreter_loop 

section .text
global _start

interpreter_loop:
   call   read_word     ; read word from stdin
   mov    rdi, rax
   call   string_length ; check the length, rdi point to the word
   test   rax, rax      ; if zero -- exit
   jz     .exit

   mov   dl, byte[state] ; check the flag of the current mode.
   test    dl, dl
   jnz    .compile

   push   rdi
   call   parse_int
   pop    rdi
   test   rdx, rdx
   jnz   .push_number

   call   find_word
   test   rax, rax
   jz    .unknown_word
   call   cfa
   .interpret:
   mov    [program_stub], rax
   mov    PC,  program_stub
   jmp    next

   .push_number:
   push   rax
   jmp    interpreter_loop

   .unknown_word:
   push   rdi
   mov    rdi, nowd_message
   call   print_string
   pop    rdi
   call   print_string
   call   print_newline
   jmp    interpreter_loop

   .compile:
   push   rdi
   call   find_word
   pop    rdi
   test   rax, rax
   jz     .parse_number
   call   cfa   
   mov    rdx, [rax-1]
   cmp    dl,1
   je     .interpret
   mov    rdi, qword[here]
   mov    [rdi], rax
   add    rdi, 8
   mov    qword[here], rdi
   jmp    interpreter_loop   
   .parse_number:
   call   parse_int
   test   rdx, rdx
   jz    .unknown_word
   .compile_number:
   mov    rdx, [here]
   mov    rdx, [rdx-8]
   cmp    rdx, xt_branch
   je     .add_offset
   cmp    rdx, xt_branch0
   jne    .lit_number
   .add_offset:
   mov    rdi, [here]
   mov    [rdi], rax
   add    rdi, 8
   mov    qword[here], rdi
   jmp    interpreter_loop
    .lit_number:
   mov    rdi, [here]
   mov    qword[rdi], xt_lit
   add    rdi, 8
   mov    [rdi], rax
   add    rdi, 8
   mov    qword[here], rdi
   jmp    interpreter_loop

   .exit:
   mov    rsp, [dstack_ptr]
   jmp    exit_mforth

_start:
   xor    PC, PC
   xor    W, W
   mov    RST, rstack_mem
   mov    qword[dstack_ptr], rsp
   mov    PC, xt_interpreter
   jmp    next      

exit_mforth:
   mov    rax, 60   
   xor    rdi, rdi
   syscall
