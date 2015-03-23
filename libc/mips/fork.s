# auto-generated, remove this line before editing
.equ NR_fork, 4002

.text
.set reorder
.global fork
.global vfork
.ent fork

vfork:
fork:
	li	$2, NR_fork
	syscall
	la	$25, unisys
	jr	$25

.end fork

.type vfork,function
.size vfork,0
