# auto-generated, remove this line before editing
.equ NR_getpriority, 4096

.text
.set reorder
.global getpriority
.ent getpriority

getpriority:
	li	$2, NR_getpriority
	syscall
	la	$25, unisys
	jr	$25

.end getpriority
