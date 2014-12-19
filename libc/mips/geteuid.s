# auto-generated, remove this line before editing
.equ NR_geteuid, 4049

.text
.set reorder
.global geteuid
.ent geteuid

geteuid:
	li	$2, NR_geteuid
	syscall
	la	$25, unisys
	jr	$25

.end geteuid
