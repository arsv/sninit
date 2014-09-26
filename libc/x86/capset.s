# auto-generated, remove this line before editing
.equ NR_capset, 185

.text
.global capset

capset:
	mov	$NR_capset, %al
	jmp	unisys

.type capset,@function
.size capset,.-capset
