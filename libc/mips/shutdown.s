# auto-generated, remove this line before editing
.equ NR_shutdown, 4182

.text
.set reorder
.global shutdown
.ent shutdown

shutdown:
	li	$2, NR_shutdown
	syscall
	la	$25, unisys
	jr	$25

.end shutdown
