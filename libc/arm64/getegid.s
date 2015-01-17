# auto-generated, remove this line before editing
.equ NR_getegid, 177

.text
.global getegid

getegid:
	mov	x8, NR_getegid
	b	unisys

.type getegid,function
.size getegid,.-getegid
