# auto-generated, remove this line before editing
.equ NR_socket, 281

.text
.global socket

socket:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_socket
	b	unisys

.type socket,function
.size socket,.-socket
