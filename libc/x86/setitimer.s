# auto-generated, remove this line before editing
.equ NR_setitimer, 104

.text
.global setitimer

setitimer:
	mov	$NR_setitimer, %al
	jmp	unisys

.type setitimer,@function
.size setitimer,.-setitimer
