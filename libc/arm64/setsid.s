# auto-generated, remove this line before editing
.equ NR_setsid, 157

.text
.global setsid

setsid:
	mov	x8, NR_setsid
	b	unisys

.type setsid,function
.size setsid,.-setsid
