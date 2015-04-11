#
# mmap takes 6 parameters - ie more than can be passed in registers via the
# regular syscall interface. Instead, parameters are passed on the stack.
#
# On entry, the compiler will have already placed the fifth and sixth
# parameters on the stack - all we need do here is push the first four and
# call the syscall.
#

.equ NR_mmap2, 192

.text 
.global mmap

mmap:
	str     r5, [sp, #-4]!
	ldr     r5, [sp, #8]
	str     r4, [sp, #-4]!
	ldr     r4, [sp, #8]
	mov	ip, r7
	mov	r7, #NR_mmap2
	svc	0x00000000
	mov	r7, ip
	ldr     r4, [sp], #4
	ldr     r5, [sp], #4
	cmn	r0, #4096
	mov	pc, lr			/* return */

.type mmap,function
.size mmap,.-mmap
