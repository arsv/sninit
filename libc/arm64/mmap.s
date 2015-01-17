.equ NR_mmap2, 222

.text
.global mmap

mmap:
	mov	x8, NR_mmap2
	b	unisys

.type mmap,function
.size mmap,.-mmap
