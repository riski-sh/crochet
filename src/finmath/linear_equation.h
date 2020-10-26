#ifndef FINMATH_LINEAR_EQUATION_H
#define FINMATH_LINEAR_EQUATION_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Defines a linear equation.
 *
 * This linear equation has a domain in N but a range of R
 */
struct linear_equation;

enum LINEAR_EQUATION_DIRECTION {
  LINEAR_EQUATION_DIRECTION_BELOW = 0,
  LINEAR_EQUATION_DIRECTION_ABOVE = 1,
  LINEAR_EQUATION_DIRECTION_EQUAL = 2
};

/*
 * Creates a new linear equation from two points
 * @param x1 The x coordiant of point 1
 * @param y1 The y coordiant of point 1
 * @param x2 The x coordiant of point 2
 * @param y2 The y coordiant of point 2
 */
struct linear_equation *linear_equation_new(
    int64_t x1, int64_t y1, int64_t x2, int64_t y2);

/*
 * Evaluates a linear equation at a point
 * @param eq The equation
 * @param z The point to evaluate at
 * @return The Y coordinate at z.
 */
int64_t linear_equation_eval(struct linear_equation *eq, int64_t z);

/*
 * Determines if a given y coordiant is above or below the y
 * coordiant of m*z + b for a given eq.
 * @param eq The linear equation
 * @param z The x coordiant on the line
 * @param y The y coordiant to test against
 * @return The direction
 */
enum LINEAR_EQUATION_DIRECTION linear_equation_direction(
    struct linear_equation *eq, int64_t z, int64_t y);

/*
 * Frees a linear equation allocated by linear_equation_new
 * @param eq The equation to free
 */
void linear_equation_free(struct linear_equation **eq);

#endif
