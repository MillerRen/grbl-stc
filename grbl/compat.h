#ifndef compat_h
#define compat_h


#define sei() EA = 1
#define cli() EA = 0

#define PSTR(s) (const char*)s

#define trunc floor
#define lround floor
#define round floor

#define F_CPU 24000000L

#define M_PI		3.14159265358979323846	/* pi */

#endif