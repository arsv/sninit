.equ NR_rt_sigprocmask, 135
.equ sizeof_sigset_t, 8

.text
.global sigprocmask

sigprocmask:
	mov	x3, sizeof_sigset_t
	mov	x8, NR_rt_sigprocmask
	b	unisys

.type sigprocmask,function
.size sigprocmask,.-sigprocmask
