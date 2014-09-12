# auto-generated, remove this line before editing
.equ NR_mremap, 25

.text
.global mremap

mremap:
	mov	$NR_mremap, %al
	jmp	unisys

.type mremap,@function
.size mremap,.-mremap
