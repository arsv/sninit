# auto-generated, remove this line before editing
.equ NR_sendmsg, 518

.text
.global sendmsg

sendmsg:
	mov	$NR_sendmsg, %ax
	jmp	unisysx

.type sendmsg,@function
.size sendmsg,.-sendmsg
