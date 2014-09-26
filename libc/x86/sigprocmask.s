.equ NR_sigprocmask, 126
.equ sizeof_sigset_t, 8

.text
.global sigprocmask

/* sigprocmask(how, set, old) := linux_sigprocmask(how, set, old, sizeof(sigset_t)) */

sigprocmask:
	pushl	$sizeof_sigset_t
	mov	$NR_sigprocmask, %al
	call	unisys
	add	$4, %esp
	ret

.type sigprocmask,@function
.size sigprocmask,.-sigprocmask
