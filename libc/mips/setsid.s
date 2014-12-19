# auto-generated, remove this line before editing
.equ NR_setsid, 4066

.text
.set reorder
.global setsid
.ent setsid

setsid:
	li	$2, NR_setsid
	syscall
	la	$25, unisys
	jr	$25

.end setsid
