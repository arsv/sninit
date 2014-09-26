.text
.global sigaction

.equ NR_sigaction,	67
.equ sizeof_sigset_t,	8

/* sigaction(signum, act, oldact) := linux_sigaction(signum, act, oldact, sizeof(sigset_t)) */

sigaction:
	movl	$sizeof_sigset_t, 4*4(%esp)
	mov	$NR_sigaction, %al
	jmp	unisys


.type sigaction,@function
.size sigaction,.-sigaction
