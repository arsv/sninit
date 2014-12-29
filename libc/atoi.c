#include <stdlib.h>

int atoi(const char* s)
{
	int v = 0;
	int m = 0;

	switch (*s) {
		case '-': m = 1;
		case '+': ++s;
	}
	while ((unsigned int) (*s - '0') < 10u)
		v = v*10 + (*(s++) - '0');

	return m ? -v : v;
}
