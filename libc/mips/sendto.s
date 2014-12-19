# auto-generated, remove this line before editing
.equ NR_sendto, 4180

.text
.set reorder
.global sendto
.ent sendto

sendto:
	li	$2, NR_sendto
	syscall
	la	$25, unisys
	jr	$25

.end sendto
