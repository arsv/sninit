.equ NR_rt_sigprocmask, 14
.equ sizeof_sigset_t, 8

.text
.global sigprocmask

sigprocmask:
	mov	$sizeof_sigset_t, %rcx
	mov	$NR_rt_sigprocmask, %al
	jmp	_syscall

.type sigprocmask,@function
.size sigprocmask,.-sigprocmask
