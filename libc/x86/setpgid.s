# auto-generated, remove this line before editing
.equ NR_setpgid, 57

.text
.global setpgid

setpgid:
	mov	$NR_setpgid, %al
	jmp	unisys

.type setpgid,@function
.size setpgid,.-setpgid
