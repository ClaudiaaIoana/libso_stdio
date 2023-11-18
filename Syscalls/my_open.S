.intel_syntax noprefix
	.global my_open
	.text
my_open:
    mov rax, 2  

    syscall

    test rax, rax        
    js error_occurred    

    ret              
error_occurred:
    mov rax, -1         
    ret     
           