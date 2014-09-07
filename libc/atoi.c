#include <stdlib.h>

int atoi(const char* s)
{
	int v = 0;
	int m = 0;

	while(*s == ' '  ||  (unsigned int)(*s - 9) < 5u) s++;
	switch (*s) {
		case '-': m = 1;
		case '+': ++s;
	}
	while ((unsigned int) (*s - '0') < 10u)
		v = v*10 + (*(s++) - '0');

	return m ? -v : v;
}
