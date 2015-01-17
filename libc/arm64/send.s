.equ NR_sendto, 206

.text
.global send

send:
	mov	x4, #0
	mov	x5, #0
	ldr	x8, =NR_sendto
	b	unisys

.type send,function
.size send,.-send
