#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <aoc/aoc.h>

typedef struct {
  int x, y, z, w;
} point;

#define AOC_T point
#define AOC_T_NAME Point
#include <aoc/array.h>

typedef struct {
  AocArrayPoint points;
} constellation;

#define AOC_T constellation
#define AOC_T_NAME Constellations
#include <aoc/array.h>

#define AOC_T size_t
#define AOC_T_NAME Index
#include <aoc/array.h>

static void parse(char *line, size_t length, void *userData) {
  (void)length;
  point p = {0};
  p.x = strtol(line, &line, 10);
  p.y = strtol(line + 1, &line, 10);
  p.z = strtol(line + 1, &line, 10);
  p.w = strtol(line + 1, &line, 10);
  AocArrayPointPush(userData, p);
}

static inline int fast_abs(const int n) {
  const int mask = n >> (sizeof(int) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static inline int manhattan_distance(const point a, const point b) {
  return fast_abs(a.x - b.x) + fast_abs(a.y - b.y) + fast_abs(a.z - b.z) +
         fast_abs(a.w - b.w);
}

static size_t solve(const AocArrayPoint *const points) {
  AocArrayConstellations constellations = {0};
  AocArrayConstellationsCreate(&constellations, 1 << 8);
  AocArrayIndex indices = {0};
  AocArrayIndexCreate(&indices, 1 << 8);

  for (size_t i = 0; i < points->length; ++i) {
    AocArrayIndexClear(&indices);
    for (size_t j = 0; j < constellations.length; ++j) {
      bool found = false;
      for (size_t k = 0; k < constellations.items[j].points.length; ++k) {
        point p = constellations.items[j].points.items[k];
        if (manhattan_distance(p, points->items[i]) <= 3) {
          found = true;
          break;
        }
      }
      if (found) {
        AocArrayIndexPush(&indices, j);
      }
    }
    if (indices.length == 0) {
      constellation c = {0};
      AocArrayPointCreate(&c.points, 1 << 6);
      AocArrayPointPush(&c.points, points->items[i]);
      AocArrayConstellationsPush(&constellations, c);
    } else if (indices.length == 1) {
      AocArrayPointPush(&constellations.items[indices.items[0]].points,
                        points->items[i]);
    } else {
      // merge constellations
      constellation *const dest = &constellations.items[indices.items[0]];
      AocArrayPointPush(&dest->points, points->items[i]);
      for (size_t j = indices.length - 1; j > 0; --j) {
        constellation *const src = &constellations.items[indices.items[j]];

        for (size_t k = 0; k < src->points.length; ++k)
          AocArrayPointPush(&dest->points, src->points.items[k]);

        AocArrayPointDestroy(&src->points);
        *src = constellations.items[constellations.length - 1];
        constellations.length--;
      }
    }
  }

  for (size_t i = 0; i < constellations.length; ++i)
    AocArrayPointDestroy(&constellations.items[i].points);
  AocArrayIndexDestroy(&indices);
  AocArrayConstellationsDestroy(&constellations);
  return constellations.length;
}

int main(void) {
  AocArrayPoint points = {0};
  AocArrayPointCreate(&points, 1 << 11);
  AocReadFileLineByLine("day25/input.txt", parse, &points);
  const size_t solution = solve(&points);
  printf("%zu\n", solution);
  AocArrayPointDestroy(&points);
}
