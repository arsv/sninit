# auto-generated, remove this line before editing
.equ NR_getegid, 4050

.text
.set reorder
.global getegid
.ent getegid

getegid:
	li	$2, NR_getegid
	syscall
	la	$25, unisys
	jr	$25

.end getegid
