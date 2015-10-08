# auto-generated, remove this line before editing
.equ NR_socket, 4183

.text
.set reorder
.global socket
.ent socket

socket:
	li	$2, NR_socket
	syscall
	la	$25, _syscall
	jr	$25

.end socket
