# auto-generated, remove this line before editing
.equ NR_bind, 200

.text
.global bind

bind:
	mov	x8, NR_bind
	b	_syscall

.type bind,function
.size bind,.-bind
