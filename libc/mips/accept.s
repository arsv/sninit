# auto-generated, remove this line before editing
.equ NR_accept, 4168

.text
.set reorder
.global accept
.ent accept

accept:
	li	$2, NR_accept
	syscall
	la	$25, unisys
	jr	$25

.end accept
