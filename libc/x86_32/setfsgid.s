# auto-generated, remove this line before editing
.equ NR_setfsgid, 123

.text
.global setfsgid

setfsgid:
	mov	$NR_setfsgid, %al
	jmp	unisys

.type setfsgid,@function
.size setfsgid,.-setfsgid
