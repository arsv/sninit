# auto-generated, remove this line before editing
.equ NR_kill, 4037

.text
.set reorder
.global kill
.ent kill

kill:
	li	$2, NR_kill
	syscall
	la	$25, unisys
	jr	$25

.end kill
