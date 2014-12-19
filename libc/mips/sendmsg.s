# auto-generated, remove this line before editing
.equ NR_sendmsg, 4179

.text
.set reorder
.global sendmsg
.ent sendmsg

sendmsg:
	li	$2, NR_sendmsg
	syscall
	la	$25, unisys
	jr	$25

.end sendmsg
