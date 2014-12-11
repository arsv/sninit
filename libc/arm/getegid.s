# auto-generated, remove this line before editing
.equ NR_getegid, 50

.text
.global getegid

getegid:
	stmfd	sp!,{r4,r5,r7,lr}
	ldr	r7, =NR_getegid
	b	unisys

.type getegid,function
.size getegid,.-getegid
