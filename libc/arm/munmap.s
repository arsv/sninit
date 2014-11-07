# auto-generated, remove this line before editing
.equ NR_munmap, 91

.text
.global munmap

munmap:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_munmap
	swi	0
	b	unisys

.type munmap,function
.size munmap,.-munmap
