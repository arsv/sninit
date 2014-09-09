.section ".bss"
.align 8
.global errno

errno:
	.long 0

.type errno,@object
.size errno,.-errno
