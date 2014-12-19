# auto-generated, remove this line before editing
.equ NR_ppoll, 4302

.text
.set reorder
.global ppoll
.ent ppoll

ppoll:
	li	$2, NR_ppoll
	syscall
	la	$25, unisys
	jr	$25

.end ppoll
