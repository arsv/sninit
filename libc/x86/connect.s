# auto-generated, remove this line before editing
.equ SYS_CONNECT, 3

.text
.global connect

connect:
	movb $SYS_CONNECT, %al
	jmp socketcall

.type connect,@function
.size connect,.-connect
