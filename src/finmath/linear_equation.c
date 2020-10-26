#include <finmath/linear_equation.h>

// Defines a linear equation
struct linear_equation {
  int64_t a;
  int64_t b;
  int64_t c;
  int64_t d;
};

struct linear_equation *
linear_equation_new(int64_t x1, int64_t y1, int64_t x2, int64_t y2)
{
  struct linear_equation *eq =
      (struct linear_equation *)malloc(1 * sizeof(struct linear_equation));

  eq->a = x1;
  eq->b = y1;
  eq->c = x2;
  eq->d = y2;

  return eq;
}

// Evaluates mx+b and returns y
int64_t
linear_equation_eval(struct linear_equation *eq, int64_t x)
{
  struct linear_equation e = (*eq);
  const int64_t num = (e.d - e.b) * (x - e.c);
  const int64_t den = (e.c - e.a);
  return (num / den) + e.d;
}

enum LINEAR_EQUATION_DIRECTION
linear_equation_direction(struct linear_equation *eq, int64_t z, int64_t y)
{
  if (linear_equation_eval(eq, z) == y)
    return LINEAR_EQUATION_DIRECTION_EQUAL;
  else if (linear_equation_eval(eq, z) > y)
    return LINEAR_EQUATION_DIRECTION_BELOW;
  else
    return LINEAR_EQUATION_DIRECTION_ABOVE;
}

// Frees the linear equation
void
linear_equation_free(struct linear_equation **eq)
{
  free(*eq);
  *eq = NULL;
}
