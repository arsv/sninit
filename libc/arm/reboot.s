# auto-generated, remove this line before editing
.equ NR_reboot, 88

.text
.global reboot

reboot:
        stmfd	sp!,{r4,r5,r7,lr}
	ldr	r4, [sp,#16]
	ldr	r5, [sp,#20]
        ldr     r7, =NR_reboot
	swi	0
	b	unisys

.type reboot,function
.size reboot,.-reboot
