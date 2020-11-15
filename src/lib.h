#ifndef LIB_H
#define LIB_H

#include <api.h>
#include <finmath/linear_equation.h>
#include <finmath/base_conversion.h>

struct vtable {
  analysis_func run;
};

extern struct vtable exports;

#endif
