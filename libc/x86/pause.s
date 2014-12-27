# auto-generated, remove this line before editing
.equ NR_pause, 29

.text
.global pause

pause:
	mov	$NR_pause, %al
	jmp	unisys

.type pause,@function
.size pause,.-pause
