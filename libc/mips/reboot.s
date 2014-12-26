# auto-generated, remove this line before editing
.equ NR_reboot, 4088
.equ LINUX_REBOOT_MAGIC1, 0xfee1dead
.equ LINUX_REBOOT_MAGIC2, 672274793

.text
.set reorder
.global reboot
.ent reboot

.equ zero, 0
.equ v0, 2
.equ a0, 4
.equ a1, 5
.equ a2, 6
.equ t9, 25

reboot:
	li	$v0, NR_reboot
	move	$a2, $a0
	li	$a0, LINUX_REBOOT_MAGIC1
	li	$a1, LINUX_REBOOT_MAGIC2
	syscall
	la	$t9, unisys
	jr	$t9

.end reboot
