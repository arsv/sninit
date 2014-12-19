# auto-generated, remove this line before editing
.equ NR_reboot, 4088

.text
.set reorder
.global reboot
.ent reboot

reboot:
	li	$2, NR_reboot
	syscall
	la	$25, unisys
	jr	$25

.end reboot
