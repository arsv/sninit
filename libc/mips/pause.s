# auto-generated, remove this line before editing
.equ NR_pause, 4029

.text
.set reorder
.global pause
.ent pause

pause:
	li	$2, NR_pause
	syscall
	la	$25, unisys
	jr	$25

.end pause
