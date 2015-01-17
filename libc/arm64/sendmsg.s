# auto-generated, remove this line before editing
.equ NR_sendmsg, 211

.text
.global sendmsg

sendmsg:
	mov	x8, NR_sendmsg
	b	unisys

.type sendmsg,function
.size sendmsg,.-sendmsg
