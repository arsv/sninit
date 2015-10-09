extern const char* sys_errlist[];
extern const int sys_nerr;

/* Standard prototype for strerror is char*(int), and sys_errlist is
   const char*[]. Clang knows it and complains if strerror is declared
   as const char*(int) as it should be, despite the lack of const
   in the prototype being apparently a remnant of the days when const
   wasn't even a keyword.

   This is bad and clang should feel bad.

   Honoring the fossils, we hereby declare char* strerror BUT we keep in mind
   it is in fact const char* strerror. */

char* strerror(int err)
{
	if(err >= 0 && err < sys_nerr)
		return (char*)sys_errlist[err];
	else
		return "unknown error";
}
