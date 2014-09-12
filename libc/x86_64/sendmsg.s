# auto-generated, remove this line before editing
.equ NR_sendmsg, 46

.text
.global sendmsg

sendmsg:
	mov	$NR_sendmsg, %al
	jmp	unisys

.type sendmsg,@function
.size sendmsg,.-sendmsg
