.intel_syntax noprefix
	.global my_close
	.text
my_close:
    mov rax, 3

    syscall

    test rax, rax        
    js error_occurred    

    ret              
error_occurred:
    mov rax, -1         
    ret     
           