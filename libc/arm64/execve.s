# auto-generated, remove this line before editing
.equ NR_execve, 221

.text
.global execve

execve:
	mov	x8, NR_execve
	b	unisys

.type execve,function
.size execve,.-execve
