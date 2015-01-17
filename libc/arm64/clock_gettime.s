# auto-generated, remove this line before editing
.equ NR_clock_gettime, 113

.text
.global clock_gettime

clock_gettime:
	mov	x8, NR_clock_gettime
	b	unisys

.type clock_gettime,function
.size clock_gettime,.-clock_gettime
