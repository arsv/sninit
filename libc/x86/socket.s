# auto-generated, remove this line before editing
.equ SYS_SOCKET, 1

.text
.global socket

socket:
	movb $SYS_SOCKET, %al
	jmp socketcall

.type socket,@function
.size socket,.-socket
