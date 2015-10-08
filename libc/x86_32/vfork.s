# auto-generated, remove this line before editing
.equ NR_vfork, 58

.text
.global vfork

vfork:
	pop	%rdx
	mov	$(0x40000000 | NR_vfork), %rax
	syscall
	push	%rdx
	mov	%rax, %rdi
	jmp	_sysret

.type vfork,@function
.size vfork,.-vfork
