# auto-generated, remove this line before editing
.equ NR_setpgid, 154

.text
.global setpgid

setpgid:
	mov	x8, NR_setpgid
	b	unisys

.type setpgid,function
.size setpgid,.-setpgid
