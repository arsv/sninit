.equ NR_socketcall, 102

.text
.global socketcall

socketcall:
	leal 4(%esp), %ecx
	pushl %ecx
	movzbl %al,%eax
	pushl %eax
	movb $NR_socketcall, %al
	call unisys
	popl %ecx
	popl %ecx
	ret

.type socketcall,@function
.size socketcall,.-socketcall
