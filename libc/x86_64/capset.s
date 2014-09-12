# auto-generated, remove this line before editing
.equ NR_capset, 126

.text
.global capset

capset:
	mov	$NR_capset, %al
	jmp	unisys

.type capset,@function
.size capset,.-capset
