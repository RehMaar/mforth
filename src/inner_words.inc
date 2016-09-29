section .data
   out_of_mem_msg: db "Address out of range", 0
   stack_underflow_msg: db "Stack underflow!", 0
;TODO:   stack_overflow_msg: db "Stack overflow!", 0
%define link 0 

%macro native 3
   section .data
    w_%+%2:
   dq link 
   db %1, 0
   db %3 
    xt_%+%2:
   dq %2%+_impl

%define link w_%+%2  

   section .code
    %2%+_impl:
%endmacro

%macro native 2
   native %1, %2, 0
%endmacro

;============
; Arithmetic 
;============

native '+', plus 
   call   check_stack
   pop    rax
   add    [rsp],   rax
   jmp    next   

native '-', minus
   call   check_stack
   pop    rax
   sub    [rsp], rax
   jmp    next

;TODO: devide by zero
native '/', division 
   call   check_stack
   pop    rcx
   pop    rax
   cqo
   idiv   rcx
   push   rax
   jmp    next

native '*', muliple
   call   check_stack
   pop    rax
   imul   qword[rsp]
   mov    [rsp], rax 
   jmp    next

;===========
; Comparison
;===========

native '=', equals
   call   check_stack
   pop    rax
   cmp    [rsp], rax
   je    .equals
   mov    qword[rsp], 0   
   jmp    next
.equals:
   mov    qword[rsp], 1
   jmp    next

native '<', less
   call   check_stack
   pop    rax
   cmp    [rsp], rax
   jl     .true
   mov    qword[rsp], 0
   jmp    next
.true:
   mov   qword[rsp], 1
   jmp   next

;=======
; Logic
;=======
native 'and', conjuction
   call   check_stack
   pop    rax
   and    [rsp], rax
   jmp    next

native   'not', negation
   call   check_stack
   not    qword[rsp]
   jmp    next
;=========
; Stack
;========

native ".", print_stack_peak
   call   check_stack
   mov    rdi, [rsp]
   call   print_int
   call   print_newline
   jmp    next

native "rot", rotate
   call   check_stack
   mov    rdi, [rsp+8]
   xor    rdi, [rsp+16]
   xor    [rsp+8], rdi
   xor    [rsp+16], rdi 
   mov    rdi, [rsp]
   xor    rdi, [rsp+8]
   xor    [rsp], rdi
   xor    [rsp+8], rdi
   jmp    next

native "swap", swap
   call   check_stack
   mov    rdi, [rsp]
   xor    rdi, [rsp+8]
   xor    [rsp], rdi
   xor    [rsp+8], rdi     
   jmp    next

native "drop", drop
   call   check_stack
   add    rsp, 8
   jmp    next

native "dup", dup
   call   check_stack
   mov    rax, [rsp]
   push   rax   
   jmp    next

native ".S", print_all_stack
   call   check_stack
   mov    rax, rsp
.print:
   sub    rdx, 8
   push   rdx
   push   rax
   mov    rdi, [rdx]
   call   print_int
   mov    rdi, " "
   call   print_char
   pop    rax
   pop    rdx
   cmp    rax, rdx
   jne    .print
   call   print_newline
   jmp    next

check_stack:
   lea    rsp, [rsp + 8]
   mov    rdx, [dstack_ptr]
   cmp    rdx, rsp
   jz     stack_underflow
.ret:
   lea    rsp, [rsp - 8]
ret

stack_underflow:
   mov    rdi, stack_underflow_msg
   call   print_string
   call   print_newline
   cmp    byte[state], 2
   je     exit
   jmp    interpreter_loop

;========
; IO
;========

; symbol from stdin (qword[rsp] -- one symbol)
native "key", key
   xor    rdx, rdx
   call   read_word
   mov    dl, byte[rax]
   push   rdx
   jmp    next

; symbol to stdout
native "emit", emit ; check?
   pop    rdi
   call   print_char
   call   print_newline
   jmp    next

native "number", number
   call   read_word
   mov    rdi, rax
   call   parse_int
   push   rax
   jmp    next

;========
; Memory
;========
native "mem", mem
   push   umem
   jmp    next

native "!", write_mem   ; rax = mem + offset, need check
   pop    rax
   cmp    rax, umem
   jl     out_of_mem
   pop    rdx
   mov    [rax], rdx
   jmp    next

native "@", read_mem
   pop    rax
   cmp    rax, umem
   jl     out_of_mem
   mov    rdx, qword[rax]
   push   rdx
   jmp    next

out_of_mem:
   mov    rdi, out_of_mem_msg
   call   print_string
   call   print_newline
   jmp    return_loop ; bad decision ?
 
return_loop:
   jmp    interpreter_loop   

section .data
   xt_exit: dq exit

%macro colon 2
section .data
 w_%+%2:  
    dq link
   db %1, 0
   db 0
%define link w_%+%2 
 xt_%+%2:
   dq docol
%endmacro

colon "or", disjunction
   dq xt_negation
   dq xt_swap
   dq xt_negation
   dq xt_conjuction
   dq xt_negation
   dq xt_exit

;=========
; Compiler
;=========

native ":", colonw
   call   read_word ; rax -- ptr
   mov    rsi, [here] ; rsi <- dmem
   mov    rdi, qword[last_word]
   mov    [rsi], rdi ; [dmem] <- last_word w
   mov    [last_word], rsi ; lw <- w
   add    qword[here], 8
   mov    rdi, rax
   mov    rsi, [here]
   call   string_copy   
   mov    byte[rdi], 2
   inc    rdi
   mov    qword[rdi], docol
   add    rdi, 8
   mov    qword[here], rdi
   mov    byte[state], 1   
   jmp    next

; should check, how?
native ";", semicolon, 1
   mov    rdi, [here]
   mov    qword[rdi], xt_exit
   add    qword[here], 8
   mov    byte[state], 0
   jmp    next

native "lit", lit
   mov    rax, [PC]
   push   rax
   add    PC, 8
   jmp    next

native "branch", branch
   cmp    byte[state], 2
   jne    branch_error   
   mov    rax, [PC]
   add    PC, 8
   imul   rax, 8
   add    PC, rax
   jmp    next

native "branch0", branch0
   cmp    byte[state], 2
   jne    branch_error
   mov    rax, [PC]
   add    PC, 8
   mov    rdx, qword[rsp] 
   test   rdx, rdx
   jnz    next
   imul   rax, 8
   add    PC, rax
   jmp    next 

branch_error:
   mov    rdi, branch_error_msg1
   call   print_string
   call   print_newline
   jmp    next 
