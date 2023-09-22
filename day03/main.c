#include <stdlib.h>
#include <stdio.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <aoc/arena.h>

typedef struct {
  uint32_t left;
  uint32_t top;
  uint32_t width;
  uint32_t height;
  bool intersects;
} rectangle;

#define AOC_T rectangle
#define AOC_T_NAME Rect
#include <aoc/array.h>

typedef struct {
  uint32_t x;
  uint32_t y;
} point;

static inline uint32_t point_hash(const point *const p) {
  return 54812489 * (p->x ^ 95723417) * (p->y ^ 69660419);
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

static void parse_line(char *line, size_t length, void *userData) {
  (void)length;
  rectangle claim = {0};
  while (*line != '@')
    line++;
  claim.left = strtoul(line + 1, &line, 10);
  claim.top = strtoul(line + 1, &line, 10);
  claim.width = strtoul(line + 2, &line, 10);
  claim.height = strtoul(line + 1, NULL, 10);
  claim.intersects = false;
  AocArrayRectPush(userData, claim);
}

static inline uint32_t max(const uint32_t a, const uint32_t b) {
  return a > b ? a : b;
}

static inline uint32_t min(const uint32_t a, const uint32_t b) {
  return a < b ? a : b;
}

static bool get_intersection_area(const rectangle *const a,
                                  const rectangle *const b,
                                  rectangle *const intersection) {
  const uint32_t interLeft = max(a->left, b->left);
  const uint32_t interRight = min(a->left + a->width, b->left + b->width);
  if (interLeft >= interRight)
    return false;

  const uint32_t interTop = max(a->top, b->top);
  const uint32_t interBottom = min(a->top + a->height, b->top + b->height);
  if (interTop >= interBottom)
    return false;

  intersection->left = interLeft;
  intersection->top = interTop;
  intersection->width = interRight - interLeft;
  intersection->height = interBottom - interTop;
  return true;
}

static void solve_both(AocArrayRect *const claims, uint32_t *const part1,
                       uint32_t *const part2) {
  AocArrayRect intersections = {0};
  AocArrayRectCreate(&intersections, 1 << 11);

  for (size_t i = 0; i < claims->length - 1; ++i) {
    rectangle *const a = &claims->items[i];
    for (size_t j = i + 1; j < claims->length; ++j) {
      rectangle *const b = &claims->items[j];

      rectangle intersection = {0};
      if (get_intersection_area(a, b, &intersection)) {
        a->intersects = true;
        b->intersects = true;
        AocArrayRectPush(&intersections, intersection);
      }
    }
  }

  AocHashsetPoint points = {0};
  AocHashsetPointCreate(&points, 1 << 18);

  for (size_t i = 0; i < intersections.length; ++i) {
    const rectangle *const intersection = &intersections.items[i];
    for (uint32_t y = 0; y < intersection->height; ++y) {
      point p = {.y = intersection->top + y};
      for (uint32_t x = 0; x < intersection->width; ++x) {
        p.x = intersection->left + x;
        uint32_t hash = 0;
        if (!AocHashsetPointContains(&points, p, &hash))
          AocHashsetPointInsertPreHashed(&points, p, hash);
      }
    }
  }

  *part1 = (uint32_t)points.count;

  for (size_t i = 0; i < claims->length; ++i) {
    if (!claims->items[i].intersects) {
      *part2 = (uint32_t)(i + 1);
      break;
    }
  }
}

int main(void) {
  aoc_arena arena = {0};
  AocArenaAlloc(&arena, 2179096);
  AocArenaReset(&arena);

  aoc_allocator allocator = AocArenaCreateAllocator(&arena);
  AocMemSetAllocator(&allocator);

  AocArrayRect claims = {0};
  AocArrayRectCreate(&claims, 1 << 11);

  AocReadFileLineByLine("day03/input.txt", parse_line, &claims);

  uint32_t part1 = 0;
  uint32_t part2 = 0;
  solve_both(&claims, &part1, &part2);

  printf("%u\n%u\n", part1, part2);

  AocArenaFree(&arena);
}
