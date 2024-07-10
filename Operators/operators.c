#ifdef __cplusplus
    extern "C" {
#endif

#include <stdio.h>

#if defined(BUILD_FROM_GUI)
#include "operators.h"
#else
#include "operators/operators.h"
#endif

#ifdef __cplusplus
}
#endif