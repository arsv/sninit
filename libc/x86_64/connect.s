# auto-generated, remove this line before editing
.equ NR_connect, 42

.text
.global connect

connect:
	mov	$NR_connect, %al
	jmp	unisys

.type connect,@function
.size connect,.-connect
