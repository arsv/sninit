# auto-generated, remove this line before editing
.equ NR_sigaction, 4067

.text
.set reorder
.global sigaction
.ent sigaction

sigaction:
	li	$2, NR_sigaction
	syscall
	la	$25, _syscall
	jr	$25

.end sigaction
