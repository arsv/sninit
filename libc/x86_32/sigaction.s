.text
.global sigaction

.equ NR_rt_sigaction,	512
.equ SA_RESTORER, 0x04000000

.equ sizeof_sigset_t,	8
.equ offsetof_sigaction_sa_flags,	 8
.equ offsetof_sigaction_sa_restorer,	16

/*    sigaction(RDI=signum, RSI=*act, RDX=*old) */
/* rt_sigaction(RDI=signum, RSI=*act, RDX=*old, RCX=nr) */
sigaction:
	/* flags |= SA_RESTORER */
	mov	8(%rsi), %rax
	or	$SA_RESTORER, %rax
	mov	%rax, 8(%rsi)
	/* sa_restorer = restore */
	lea	sarestore, %rax
	mov	%rax, 16(%rsi)
	/* 4th args = _NSIG/8 */
	xor	%rcx,%rcx
	mov	$sizeof_sigset_t, %cl
	/* syscall(__NR_rt_sigaction) */
	mov	$NR_rt_sigaction, %ax
	jmp	_syscallx

/* dietlibc quoting gcc implies this should be 16-aligned */
/* no idea why, and it seems to work well as it is */
sarestore:
	xor	%rax,%rax
	mov	$15, %al
	syscall
	hlt


.type sigaction,@function
.size sigaction,.-sigaction
