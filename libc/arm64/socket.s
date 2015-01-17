# auto-generated, remove this line before editing
.equ NR_socket, 198

.text
.global socket

socket:
	mov	x8, NR_socket
	b	unisys

.type socket,function
.size socket,.-socket
