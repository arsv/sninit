# auto-generated, remove this line before editing
.equ NR_recvmsg, 4177

.text
.set reorder
.global recvmsg
.ent recvmsg

recvmsg:
	li	$2, NR_recvmsg
	syscall
	la	$25, unisys
	jr	$25

.end recvmsg
