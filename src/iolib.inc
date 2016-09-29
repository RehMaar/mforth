section .text

%define NEWLINE 0xA

string_length:
   xor     rax, rax
.loop:
   cmp    byte[rdi+rax], 0
   je     .null
   inc    rax
   jmp    .loop
.null:
ret

print_string:
   call string_length
   mov    rdx, rax
   mov    rsi, rdi
   mov    rax, 1
   mov    rdi, 1
   syscall
ret

print_char:
   push   rdi
   mov    rsi, rsp
   mov    rdi, 1
   mov    rax, 1
   mov    rdx, 1
   syscall
   pop    rdi
ret

print_newline:
   mov    rdi, NEWLINE
   call   print_char
ret

print_uint:
   push   rbx
   xor    rbx, rbx
   xor    rcx, rcx
   mov    rax, rdi
   mov    rcx, 0xA
.loop:
   xor    rdx, rdx
   div    rcx
   add    rdx, '0'
   mov    byte[rsp+rbx], dl
   dec    rbx
   test   rax, rax
   jnz    .loop
.print:
   inc    rbx
   lea    rsi, [rsp+rbx]
   dec    rbx
   neg    rbx  
   mov    rdx, rbx
   mov    rax, 1
   mov    rdi, 1 
   syscall 
   pop    rbx    
ret

print_int:
   cmp    rdi, 0
   jns    .print
   push   rdi
   mov    rdi, '-'
   call   print_char
   pop    rdi
   neg    rdi
.print:
   call   print_uint
ret   

; rdi - first ptr; rsi -- second ptr
; rax - 0 if not equal, 1 otherwise
string_equals:
   call   string_length 
   push   rax
   xchg   rdi, rsi
   call   string_length 
   pop    rcx
   mov    rdx, rax
   xor    rax, rax
   cmp    rcx, rdx 
   jne    .exit
   test   rcx, rdx
   jz     .equal
.loop:
   cmpsb
   jne    .exit
   loop   .loop
.equal:
   inc    rax
.exit:
ret

read_char:
   push    0
   mov    rdx, 1
   mov    rax, 0
   mov    rsi, rsp
   mov    rdi, 0
   syscall
   pop    rax
ret 

section .data
word_buffer: times 256 db 0x00

section .text

read_word:
   push    r15
   xor     r15, r15
.next_char:
   call   read_char
   test   al, al
   jz     .write_buf
   cmp    al, 0x20
   jg     .write_buf  
   jl     .null     
   test   r15, r15
   jz     .next_char
.null:         
   mov    al, 0
.write_buf:
   mov    byte[word_buffer+r15], al
   inc    r15
   test   al, al
   jnz   .next_char
.exit:
   mov    rax, word_buffer
   mov    rdx, r15
   pop    r15
ret

; rdi points to a string
; returns rax -- number, rdx -- length 
parse_uint:
   xor    rcx, rcx
   xor    rax, rax
   xor    rsi, rsi
   mov    r11, 0xA

   mov    al, byte[rdi]
   test   al, al
   jz     .clear 
   cmp    al, '9'
   jg     .clear 
   cmp     al, '0'
   jl     .clear 
   sub    al, '0'
   inc    rsi   


.get_next:
   mov    cl, byte[rdi+rsi]
   test   cl, cl
   jz     .exit
   cmp    cl, '9'
   jg     .exit
   cmp    cl, '0'
   jl     .exit
   sub    cl, '0'
   inc    rsi
   mul    r11
   add    rax, rcx
   jmp    .get_next
   jmp    .exit
.clear:
   xor    rax, rax
.exit:
   mov    rdx, rsi
ret

; rdi points to a string
; returns rax: number, rdx : length
parse_int:
   mov    al, [rdi]
   cmp    al, '-'
   jz     .neg
   call   parse_uint
   jmp    .exit
.neg:
   inc    rdi
   call   parse_uint
   test   rdx, rdx
   jz     .exit
   inc    rdx
   neg    rax
.exit:   
ret 

; rdi str1, rsi buffer 
string_copy:
   call   string_length
   inc    rax
   mov    rcx, rax
   xchg   rdi, rsi
.loop:
   movsb
   loop   .loop
ret
