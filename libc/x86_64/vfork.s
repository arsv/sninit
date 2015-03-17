.equ NR_vfork, 58

.text
.global vfork

vfork:
	mov	$NR_vfork, %al
	pop	%rdx
	syscall
	push	%rdx
	jmp	uniret

.type vfork,@function
.size vfork,.-vfork
