section .text


; Find word in the dictionary
; IN: rdi <-- string to compare
; OUT: rax <-- xt or 0
find_word:
   mov     rsi, [last_word] ; последнее слово
.loop:
   lea     rsi, [rsi + 8] 
   push    rdi
   push    rsi
   call    string_equals 
   pop     rsi
   pop     rdi
   lea     rsi, [rsi-8] ; rsi <- начало слова
   test    rax, rax ; rax -- 0
   jnz     .return
   mov     rsi, [rsi] ; rsi адрес link1 ; link2
   test    rsi, rsi
   jz      .return
   jmp     .loop 
.return:                                              
   mov     rax, rsi 
ret 

; Code From Address
; IN: rdi <- address
; OUT: rax <- XT address
cfa:
   lea     rdi, [rax+8]
   call    string_length   
   lea     rax, [rdi+(rax+1)+1] 
ret

next:
   mov      W, PC
   add      PC, 8  
   mov      W, [W]
   jmp      [W]

docol:
   mov      byte[state], 2 ; for colon-word execution
   add      RST, 8
   mov      [RST], PC
   add      W, 8
   mov      PC, W
   jmp      next

exit:
   mov     byte[state], 0 ; for colon-word execution
   mov     PC, [RST]
   sub     RST, 8
   jmp     next
