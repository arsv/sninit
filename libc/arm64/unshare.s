# auto-generated, remove this line before editing
.equ NR_unshare, 97

.text
.global unshare

unshare:
	mov	x8, NR_unshare
	b	unisys

.type unshare,function
.size unshare,.-unshare
