#include <bits/syscall.h>

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
	jmp	unisysx;  \
.else ; \
	mov	$__NR_##name,%al; \
	jmp	unisys;  \
.endif

#define syscall(name,sym) \
.text; \
.type sym,@function; \
.global sym; \
sym: \
.ifge __NR_##name-256 ; \
	mov	$__NR_##name,%ax; \
	jmp	unisysx; \
.else ; \
	mov	$__NR_##name,%al; \
	jmp	unisys; \
.endif
