unsigned long strlen(const char* s)
{
	int l = 0;
	while(*(s++)) l++;
	return l;
}
