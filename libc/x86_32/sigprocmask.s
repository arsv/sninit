.equ NR_rt_sigprocmask, 14
.equ sizeof_sigset_t, 8

.text
.global sigprocmask

sigprocmask:
	mov	$sizeof_sigset_t, %r10d
	mov	$NR_rt_sigprocmask, %al
	jmp	unisys

.type sigprocmask,@function
.size sigprocmask,.-sigprocmask
