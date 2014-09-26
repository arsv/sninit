# auto-generated, remove this line before editing
.equ NR_setresuid, 164

.text
.global setresuid

setresuid:
	mov	$NR_setresuid, %al
	jmp	unisys

.type setresuid,@function
.size setresuid,.-setresuid
