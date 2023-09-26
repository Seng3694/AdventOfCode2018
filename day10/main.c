#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <aoc/aoc.h>

typedef struct {
  int x;
  int y;
} point;

static inline uint32_t point_hash(const point *const p) {
  return 54812489 * ((uint32_t)p->x ^ 95723417) * ((uint32_t)p->y ^ 69660419);
}

static inline bool point_equals(const point *const a, const point *const b) {
  return a->x == b->x && a->y == b->y;
}

#define AOC_T point
#define AOC_T_NAME Point
#define AOC_T_EMPTY ((point){0, 0})
#define AOC_T_HFUNC point_hash
#define AOC_T_EQUALS point_equals
#define AOC_BASE2_CAPACITY
#include <aoc/hashset.h>

typedef struct {
  point position;
  point velocity;
} star;

#define AOC_T star
#define AOC_T_NAME Star
#include <aoc/array.h>

static inline void skip_spaces(char *str, char **out) {
  while (*str == ' ')
    str++;
  *out = str;
}

static point parse_point(char *str, char **out) {
  point p = {0};
  skip_spaces(str, &str);
  p.x = strtol(str, &str, 10);
  skip_spaces(str + 1, &str);
  p.y = strtol(str, &str, 10);
  *out = str;
  return p;
}

static void parse_line(char *line, size_t length, void *userData) {
  (void)length;
  // "position=< 9,  1> velocity=< 0,  2>"
  star s = {0};
  s.position = parse_point(line + 10, &line);
  s.velocity = parse_point(line + 12, &line);
  AocArrayStarPush(userData, s);
}

static inline point move(point position, point velocity, int time) {
  return (point){
      position.x + velocity.x * time,
      position.y + velocity.y * time,
  };
}

static bool intersection(star a, star b, point *const p, int *const time) {
  // x1 + v1*t = x2 + v2*t
  // x1 + v1*t - x2 - v2*t = 0
  // v1*t - v2*t + x1 - x2 = 0
  // t(v1 - v2) = x2 - x1
  // t = (x2 - x1) / (v1 - v2)

  // no intersection
  if (a.velocity.x - b.velocity.x == 0 || a.velocity.y - b.velocity.y == 0)
    return false;

  int tx = (b.position.x - a.position.x) / (a.velocity.x - b.velocity.x);
  int ty = (b.position.y - a.position.y) / (a.velocity.y - b.velocity.y);
  if (tx == ty) {
    *time = tx;
    *p = move(a.position, a.velocity, tx);
    return true;
  }

  return false;
}

static void solve(const AocArrayStar *const stars) {
  int64_t time = 0;
  int count = 0;

  for (size_t i = 0; i < stars->length - 1; ++i) {
    for (size_t j = i + 1; j < stars->length; ++j) {
      point p = {0};
      int t = 0;
      if (intersection(stars->items[i], stars->items[j], &p, &t)) {
        time += t;
        count++;
      }
    }
  }

  time = (int64_t)round(time / (double)count);

  AocHashsetPoint points = {0};
  AocHashsetPointCreate(&points, stars->capacity);
  point min = {INT32_MAX, INT32_MAX};
  point max = {0};

  for (size_t i = 0; i < stars->length; ++i) {
    const star *const s = &stars->items[i];
    const point p = move(s->position, s->velocity, time);
    if (p.x < min.x)
      min.x = p.x;
    if (p.y < min.y)
      min.y = p.y;
    if (p.x > max.x)
      max.x = p.x;
    if (p.y > max.y)
      max.y = p.y;
    uint32_t hash = 0;
    if (!AocHashsetPointContains(&points, p, &hash))
      AocHashsetPointInsertPreHashed(&points, p, hash);
  }

  int w = max.x - min.x + 1;
  int h = max.y - min.y + 1;

  for (int y = 0; y < h; ++y) {
    point p = {.y = y + min.y};
    for (int x = 0; x < w; ++x) {
      p.x = x + min.x;
      printf("%c", AocHashsetPointContains(&points, p, NULL) ? '#' : '.');
    }
    printf("\n");
  }

  printf("%ld\n", time);
  AocHashsetPointDestroy(&points);
}

int main(void) {
  AocArrayStar stars = {0};
  AocArrayStarCreate(&stars, 512);

  AocReadFileLineByLine("day10/input.txt", parse_line, &stars);

  solve(&stars);

  AocArrayStarDestroy(&stars);
}
