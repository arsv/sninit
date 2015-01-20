# auto-generated, remove this line before editing
.equ NR_setfsuid, 4138

.text
.set reorder
.global setfsuid
.ent setfsuid

setfsuid:
	li	$2, NR_setfsuid
	syscall
	la	$25, unisys
	jr	$25

.end setfsuid
