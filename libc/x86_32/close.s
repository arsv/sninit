# auto-generated, remove this line before editing
.equ NR_close, 3

.text
.global close

close:
	mov	$NR_close, %al
	jmp	unisys

.type close,@function
.size close,.-close
