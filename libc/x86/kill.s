# auto-generated, remove this line before editing
.equ NR_kill, 37

.text
.global kill

kill:
	mov	$NR_kill, %al
	jmp	unisys

.type kill,@function
.size kill,.-kill
