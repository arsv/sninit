.equ NR_sendto, 44

.text
.global send

send:
	xor %r8d, %r8d
	xor %r9d, %r9d
	mov $NR_sendto, %al
	jmp unisys

.type send,@function
.size send,.-send
