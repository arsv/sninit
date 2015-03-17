/* The word "static" has at least two meanings in C: declare function
   local, and move variable from stack to .bss (or .data) section. 
   For tests to work, it is necessary to cancel the first meaning
   without touching the second one. Since -Dstatic= invariably
   redefines both, the first meaning is expressed with (redefinable)
   "local" keyword instead. */

#ifndef exportall
# define local static
#else
# define local
#endif

/* As for the second meaning of "static", there are no reenterable
   functions in sninit, so static is not really the right word;
   it's not about the value being static, it's only about moving
   variables out of the stack.

   Why would one want to move variables out of the stack?
   Well, overrunning an in-stack buffer means trouble, overrunning
   something in .bss, not so much. Due to its design, sninit invariably
   has a lot of free space in bss and an lot of free space in stack,
   so it's more about preferring bss free space over stack free space
   actually. Note "a lot" here means a bit less than a full 4KB page. */

#define bss static

/* Defining a lot of stuff static or extern at the start of the file puts
   unnecessary emphasis on less-important code and hides the functions
   the file was created for in the first place. To counteract that,
   the primary entry points are declared alongside statics and externs,
   using no-op "export" keyword. */

#define export

/* This is only used from drop-in initial configuration override.
   Default value is hard-coded into init.c, but adding builtin.c
   replaces that with a pre-compiled struct config. */

#define weak __attribute__((weak))
