.equ NR_vfork, 190

/* vfork is a special syscall for achitectures that keep
   return address in the stack. For x86, it may not use
   standard trampoline for instance. */

.text
.global vfork

vfork:
	popl	%edx
	xorl	%eax,%eax
	movb	$NR_vfork, %al
	int	$0x80
	jmpl	*%edx

.type vfork,@function
.size vfork,.-vfork
