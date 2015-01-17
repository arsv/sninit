.equ NR_wait4, 260

.text
.global waitpid

waitpid:
	mov	x3, #0
	mov	x8, NR_wait4
	b	unisys

.type waitpid,function
.size waitpid,.-waitpid
