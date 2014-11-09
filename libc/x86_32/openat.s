# auto-generated, remove this line before editing
.equ NR_openat, 257

.text
.global openat

openat:
	mov	$NR_openat, %ax
	jmp	unisysx

.type openat,@function
.size openat,.-openat
