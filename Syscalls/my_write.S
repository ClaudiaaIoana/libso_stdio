.intel_syntax noprefix
	.global my_write
	.text
my_write:
    mov rax, 1

    syscall

    test rax, rax        
    js error_occurred    

    ret              
error_occurred:
    mov rax, -1         
    ret     
           