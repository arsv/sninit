# auto-generated, remove this line before editing
.equ NR_alarm, 4027

.text
.set reorder
.global alarm
.ent alarm

alarm:
	li	$2, NR_alarm
	syscall
	la	$25, unisys
	jr	$25

.end alarm
