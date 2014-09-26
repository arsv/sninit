# auto-generated, remove this line before editing
.equ NR_prctl, 172

.text
.global prctl

prctl:
	mov	$NR_prctl, %al
	jmp	unisys

.type prctl,@function
.size prctl,.-prctl
