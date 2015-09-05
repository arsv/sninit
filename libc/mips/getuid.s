# auto-generated, remove this line before editing
.equ NR_getuid, 4024

.text
.set reorder
.global getuid
.ent getuid

getuid:
	li	$2, NR_getuid
	syscall
	la	$25, unisys
	jr	$25

.end getuid
