# auto-generated, remove this line before editing
.equ NR_shutdown, 48

.text
.global shutdown

shutdown:
	mov	$NR_shutdown, %al
	jmp	unisys

.type shutdown,@function
.size shutdown,.-shutdown
