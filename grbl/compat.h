#ifndef compat_h
#define compat_h


#define sei() EA = 1
#define cli() EA = 0

#define PSTR(s) (const char*)s


#endif