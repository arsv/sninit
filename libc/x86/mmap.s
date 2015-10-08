/* NR_mmap is an old 4-argument pass-stuff-in-memory call version.
   Standard mmap interface is provided by NR_mmap2 instead.
   Ref. http://www.win.tue.nl/~aeb/linux/lk/lk-4.html */

.equ NR_mmap2, 192

.text
.global mmap

mmap:
	mov	$NR_mmap2, %al
	jmp	_syscall

.type mmap,@function
.size mmap,.-mmap
