# auto-generated, remove this line before editing
.equ NR_setresgid, 119

.text
.global setresgid

setresgid:
	mov	$NR_setresgid, %al
	jmp	unisys

.type setresgid,@function
.size setresgid,.-setresgid