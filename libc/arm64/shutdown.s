# auto-generated, remove this line before editing
.equ NR_shutdown, 210

.text
.global shutdown

shutdown:
	mov	x8, NR_shutdown
	b	unisys

.type shutdown,function
.size shutdown,.-shutdown
