# auto-generated, remove this line before editing
.equ NR_prctl, 4192

.text
.set reorder
.global prctl
.ent prctl

prctl:
	li	$2, NR_prctl
	syscall
	la	$25, unisys
	jr	$25

.end prctl
