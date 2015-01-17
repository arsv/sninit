# only used in tests btw
.equ NR_fstatat, 79
.equ AT_FDCWD, -100

.text
.global stat

stat:
	mov	x2, x1
	mov	x1, x0
	mov	x0, AT_FDCWD
	mov	x3, #0	
	mov	x8, NR_fstatat
	b	unisys

.type stat,function
.size stat,.-stat
