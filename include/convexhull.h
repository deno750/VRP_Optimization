#ifndef CONVEX_HULL_H
#define CONVEX_HULL_H

#include "utility.h"
#include "convexhull.h"

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

static int comparePoints(const void *lhs, const void *rhs);

static bool ccw(const node *a, const node *b, const node *c);

node* convexHull(node *p, int len, int* hsize);

#endif