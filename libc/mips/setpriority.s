# auto-generated, remove this line before editing
.equ NR_setpriority, 4097

.text
.set reorder
.global setpriority
.ent setpriority

setpriority:
	li	$2, NR_setpriority
	syscall
	la	$25, unisys
	jr	$25

.end setpriority
