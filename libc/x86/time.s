# auto-generated, remove this line before editing
.equ NR_time, 13

.text
.global time

time:
	mov	$NR_time, %al
	jmp	unisys

.type time,@function
.size time,.-time
