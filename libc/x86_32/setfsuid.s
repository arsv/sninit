# auto-generated, remove this line before editing
.equ NR_setfsuid, 122

.text
.global setfsuid

setfsuid:
	mov	$NR_setfsuid, %al
	jmp	unisys

.type setfsuid,@function
.size setfsuid,.-setfsuid
