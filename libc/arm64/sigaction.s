.equ NR_rt_sigaction, 134
.equ sizeof_sigset_t,	8

.text
.global sigaction

/*    sigaction(x0=signum, x1=*act, x2=*old) */
/* rt_sigaction(x0=signum, x1=*act, x2=*old, x3=nr) */
sigaction:
	mov	x8, NR_rt_sigaction
	mov	x3, sizeof_sigset_t
	b	_syscall

.type sigaction,function
.size sigaction,.-sigaction
