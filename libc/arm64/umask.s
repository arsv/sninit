# auto-generated, remove this line before editing
.equ NR_umask, 166

.text
.global umask

umask:
	mov	x8, NR_umask
	b	unisys

.type umask,function
.size umask,.-umask
