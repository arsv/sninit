int currlevel = 0;
int nextlevel = 0;
int warnfd = 2;
int syslogfd = -1;

void* cfg = (void*)0;

extern void nocall(const char* func) __attribute__((noreturn));

#define STUB(fun) \
	void fun(void) { nocall(#fun); }

STUB(configure);
STUB(setnewconf);

STUB(initpass);
STUB(waitpids);

STUB(setinitctl);
STUB(acceptctl);

STUB(spawn);
STUB(stop);
STUB(levelmatch);

STUB(parsecmd);
STUB(dumpstate);
STUB(dumpidof);

STUB(findentry);

STUB(finishinittab);
STUB(rewirepointers);
STUB(setrunflags);

STUB(readinittab);
STUB(readinitdir);

STUB(extendblock);
