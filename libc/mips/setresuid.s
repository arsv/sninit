# auto-generated, remove this line before editing
.equ NR_setresuid, 4185

.text
.set reorder
.global setresuid
.ent setresuid

setresuid:
	li	$2, NR_setresuid
	syscall
	la	$25, _syscall
	jr	$25

.end setresuid
