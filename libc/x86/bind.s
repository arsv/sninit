# auto-generated, remove this line before editing
.equ SYS_BIND, 2

.text
.global bind

bind:
	movb $SYS_BIND, %al
	jmp socketcall

.type bind,@function
.size bind,.-bind
