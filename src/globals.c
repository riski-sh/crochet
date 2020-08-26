#include <globals.h>

bool
globals_continue(bool *val)
{
  static bool is_safe = true;
  if (val == NULL) {
    return is_safe;
  } else {
    is_safe = *val;
    return is_safe;
  }
}
