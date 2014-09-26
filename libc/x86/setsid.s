# auto-generated, remove this line before editing
.equ NR_setsid, 66

.text
.global setsid

setsid:
	mov	$NR_setsid, %al
	jmp	unisys

.type setsid,@function
.size setsid,.-setsid
