#include <bits/syscall.h>

#ifdef __PIC__
#define syscall_weak(name,wsym,sym) \
.text; \
.type wsym,@function; \
.weak wsym; \
wsym: ; \
.type sym,@function; \
.global sym; \
sym: \
	mov	$__NR_##name,%al; \
	jmp	__unified_syscall@PLT

#define syscall(name,sym) \
.text; \
.type sym,@function; \
.global sym; \
sym: \
.ifge __NR_##name-256 ; \
	mov	$__NR_##name,%ax; \
	jmp	__unified_syscall_16bit@PLT;  \
.else ; \
	mov	$__NR_##name,%al; \
	jmp	__unified_syscall@PLT;  \
.endif

#else

#define syscall_weak(name,wsym,sym) \
.text; \
.type wsym,@function; \
.weak wsym; \
wsym: ; \
.type sym,@function; \
.global sym; \
sym: \
.ifge __NR_##name-256 ; \
	mov	$__NR_##name,%ax; \
	jmp	__unified_syscall_16bit;  \
.else ; \
	mov	$__NR_##name,%al; \
	jmp	__unified_syscall;  \
.endif

#define syscall(name,sym) \
.text; \
.type sym,@function; \
.global sym; \
sym: \
.ifge __NR_##name-256 ; \
	mov	$__NR_##name,%ax; \
	jmp	__unified_syscall_16bit; \
.else ; \
	mov	$__NR_##name,%al; \
	jmp	__unified_syscall; \
.endif
#endif
