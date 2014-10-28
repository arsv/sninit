extern const char* sys_errlist[];
extern const int sys_nerr;

const char* strerror(int err)
{
	if(err >= 0 && err < sys_nerr)
		return sys_errlist[err];
	else
		return "unknown error";
}
