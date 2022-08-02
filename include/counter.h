#ifndef __COUNTER_H_INCLUDE__
#define __COUNTER_H_INCLUDE__

#include <string.h>

#define COUNTER_DECLARE(NAME, TYP, INIT) TYP NAME = INIT, NAME##_old = INIT
#define COUNTER_DECLARE_EXTERN(NAME, TYP) extern TYP NAME, NAME##_old
#define COUNTER(NAME) (NAME)
#define COUNTER_CHANGED(NAME) ((NAME != NAME##_old) ? ((NAME##_old = NAME), TRUE) : (FALSE))
#define COUNTER_SET(NAME, VALUE) (NAME##_old = (VALUE + 1), NAME = (VALUE))
#define COUNTER_RESET(NAME) (NAME##_old = NAME = 0)

#endif