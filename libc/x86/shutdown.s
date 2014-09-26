# auto-generated, remove this line before editing
.equ SYS_SHUTDOWN, 13

.text
.global shutdown

shutdown:
	movb $SYS_SHUTDOWN, %al
	jmp socketcall

.type shutdown,@function
.size shutdown,.-shutdown
