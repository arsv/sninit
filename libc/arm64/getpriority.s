# auto-generated, remove this line before editing
.equ NR_getpriority, 141

.text
.global getpriority

getpriority:
	mov	x8, NR_getpriority
	b	unisys

.type getpriority,function
.size getpriority,.-getpriority
