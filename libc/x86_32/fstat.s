# auto-generated, remove this line before editing
.equ NR_fstat, 5

.text
.global fstat

fstat:
	mov	$NR_fstat, %al
	jmp	unisys

.type fstat,@function
.size fstat,.-fstat
