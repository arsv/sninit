# auto-generated, remove this line before editing
.equ NR_setfsuid, 151

.text
.global setfsuid

setfsuid:
	mov	x8, NR_setfsuid
	b	unisys

.type setfsuid,function
.size setfsuid,.-setfsuid
