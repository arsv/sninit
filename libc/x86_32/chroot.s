# auto-generated, remove this line before editing
.equ NR_chroot, 161

.text
.global chroot

chroot:
	mov	$NR_chroot, %al
	jmp	unisys

.type chroot,@function
.size chroot,.-chroot
