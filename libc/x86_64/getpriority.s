# auto-generated, remove this line before editing
.equ NR_getpriority, 140

.text
.global getpriority

getpriority:
	mov	$NR_getpriority, %al
	jmp	unisys

.type getpriority,@function
.size getpriority,.-getpriority
