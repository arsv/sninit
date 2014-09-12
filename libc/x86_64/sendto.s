# auto-generated, remove this line before editing
.equ NR_sendto, 44

.text
.global sendto

sendto:
	mov	$NR_sendto, %al
	jmp	unisys

.type sendto,@function
.size sendto,.-sendto
