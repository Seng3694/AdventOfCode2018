#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <aoc/aoc.h>

typedef struct {
  int x, y, z;
} point;

typedef struct {
  point position;
  int radius;
} nanobot;

#define AOC_T nanobot
#define AOC_T_NAME Bot
#include <aoc/array.h>

static void parse(char *line, size_t length, void *userData) {
  (void)length;
  nanobot b = {0};
  b.position.x = strtol(line + 5, &line, 10);
  b.position.y = strtol(line + 1, &line, 10);
  b.position.z = strtol(line + 1, &line, 10);
  b.radius = strtol(line + 5, NULL, 10);
  AocArrayBotPush(userData, b);
}

static inline int compare_bots(const void *const a, const void *const b) {
  return ((nanobot *)b)->radius - ((nanobot *)a)->radius;
}

static inline int fast_abs(const int n) {
  const int mask = n >> (sizeof(int) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static inline int manhattan_distance(const point a, const point b) {
  return fast_abs(a.x - b.x) + fast_abs(a.y - b.y) + fast_abs(a.z - b.z);
}

static int solve_part1(AocArrayBot *const bots) {
  // assume bots are sorted in descending order (radius)
  nanobot strongest = bots->items[0];
  int inRange = 1;
  for (size_t i = 1; i < bots->length; ++i) {
    if (manhattan_distance(strongest.position, bots->items[i].position) <=
        strongest.radius)
      inRange++;
  }
  return inRange;
}

int main(void) {
  AocArrayBot bots = {0};
  AocArrayBotCreate(&bots, 1 << 12);
  AocReadFileLineByLine("day23/input.txt", parse, &bots);
  qsort(bots.items, bots.length, sizeof(nanobot), compare_bots);

  const int part1 = solve_part1(&bots);

  printf("%d\n", part1);

  AocArrayBotDestroy(&bots);
}
