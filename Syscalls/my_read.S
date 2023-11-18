.intel_syntax noprefix
	.global my_read
	.text
my_read:
    mov rax, 0 

    syscall

    test rax, rax        
    js error_occurred    

    ret              
error_occurred:
    mov rax, -1         
    ret     
           