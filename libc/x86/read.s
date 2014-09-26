# auto-generated, remove this line before editing
.equ NR_read, 3

.text
.global read

read:
	mov	$NR_read, %al
	jmp	unisys

.type read,@function
.size read,.-read
