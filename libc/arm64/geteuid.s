# auto-generated, remove this line before editing
.equ NR_geteuid, 175

.text
.global geteuid

geteuid:
	mov	x8, NR_geteuid
	b	unisys

.type geteuid,function
.size geteuid,.-geteuid
