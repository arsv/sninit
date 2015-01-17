# auto-generated, remove this line before editing
.equ NR_getdents64, 61

.text
.global getdents64

getdents64:
	mov	x8, NR_getdents64
	b	unisys

.type getdents64,function
.size getdents64,.-getdents64
