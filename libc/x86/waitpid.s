# auto-generated, remove this line before editing
.equ NR_waitpid, 7

.text
.global waitpid

waitpid:
	mov	$NR_waitpid, %al
	jmp	unisys

.type waitpid,@function
.size waitpid,.-waitpid
