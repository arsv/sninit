# auto-generated, remove this line before editing
.equ NR_mmap, 9

.text
.global mmap

mmap:
	mov	$NR_mmap, %al
	jmp	_syscall

.type mmap,@function
.size mmap,.-mmap
