# auto-generated, remove this line before editing
.equ NR_unshare, 4303

.text
.set reorder
.global unshare
.ent unshare

unshare:
	li	$2, NR_unshare
	syscall
	la	$25, unisys
	jr	$25

.end unshare
