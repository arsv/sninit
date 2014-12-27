# auto-generated, remove this line before editing
.equ NR_alarm, 27

.text
.global alarm

alarm:
	mov	$NR_alarm, %al
	jmp	unisys

.type alarm,@function
.size alarm,.-alarm
