# auto-generated, remove this line before editing
.equ NR_unshare, 310

.text
.global unshare

unshare:
	mov	$NR_unshare, %ax
	jmp	unisysx

.type unshare,@function
.size unshare,.-unshare