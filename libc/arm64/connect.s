# auto-generated, remove this line before editing
.equ NR_connect, 203

.text
.global connect

connect:
	mov	x8, NR_connect
	b	_syscall

.type connect,function
.size connect,.-connect
