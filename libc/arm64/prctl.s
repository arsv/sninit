# auto-generated, remove this line before editing
.equ NR_prctl, 167

.text
.global prctl

prctl:
	mov	x8, NR_prctl
	b	unisys

.type prctl,function
.size prctl,.-prctl
