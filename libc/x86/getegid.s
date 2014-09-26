# auto-generated, remove this line before editing
.equ NR_getegid, 50

.text
.global getegid

getegid:
	mov	$NR_getegid, %al
	jmp	unisys

.type getegid,@function
.size getegid,.-getegid
