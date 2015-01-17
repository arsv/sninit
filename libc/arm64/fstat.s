# auto-generated, remove this line before editing
.equ NR_fstat, 80

.text
.global fstat

fstat:
	mov	x8, NR_fstat
	b	unisys

.type fstat,function
.size fstat,.-fstat
