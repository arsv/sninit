# auto-generated, remove this line before editing
.equ NR_bind, 4169

.text
.set reorder
.global bind
.ent bind

bind:
	li	$2, NR_bind
	syscall
	la	$25, unisys
	jr	$25

.end bind
