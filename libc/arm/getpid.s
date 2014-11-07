# auto-generated, remove this line before editing
.equ NR_getpid, 20

.text
.global getpid

getpid:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_getpid
	swi	0
	b	unisys

.type getpid,function
.size getpid,.-getpid
