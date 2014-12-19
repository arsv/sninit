# auto-generated, remove this line before editing
.equ NR_sigprocmask, 4126

.text
.set reorder
.global sigprocmask
.ent sigprocmask

sigprocmask:
	li	$2, NR_sigprocmask
	syscall
	la	$25, unisys
	jr	$25

.end sigprocmask
