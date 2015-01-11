# auto-generated, remove this line before editing
.equ NR_setpgid, 4057

.text
.set reorder
.global setpgid
.ent setpgid

setpgid:
	li	$2, NR_setpgid
	syscall
	la	$25, unisys
	jr	$25

.end setpgid
