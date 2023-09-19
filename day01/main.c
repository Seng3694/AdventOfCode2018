#include <stddef.h>

static void *aoc_malloc(size_t size);
static void *aoc_realloc(void *old, size_t newSize);
static void aoc_free(void *ptr);

#define AOC_MALLOC aoc_malloc
#define AOC_FREE aoc_free
#define AOC_REALLOC aoc_realloc

#include <aoc/arena.h>

aoc_arena arena = {0};

static void *aoc_malloc(size_t size) {
  return AocArenaAlloc(&arena, size);
}

static void *aoc_realloc(void *old, size_t newSize) {
  return AocArenaRealloc(&arena, old, newSize);
}

static void aoc_free(void *ptr) {
  (void)ptr;
}

#include <aoc/aoc.h>
#include <stdio.h>
#include <stdlib.h>

#define AOC_T int32_t
#define AOC_T_NAME I32
#include <aoc/array.h>

static inline void parse_line(char *line, size_t length, void *userData) {
  AocArrayI32Push(userData, (int32_t)strtol(line, NULL, 10));
}

static int32_t solve_part1(const AocArrayI32 *const numbers) {
  int32_t sum = 0;
  for (size_t i = 0; i < numbers->length; ++i)
    sum += numbers->items[i];
  return sum;
}

static inline uint32_t i32_hash(const int32_t *const i) {
  return 54812489 * ((uint32_t)(*i) ^ 95723417);
}

static inline bool i32_equals(const int32_t *const a, const int32_t *const b) {
  return *a == *b;
}

#define AOC_T int32_t
#define AOC_T_NAME I32
#define AOC_T_EMPTY ((int32_t){0})
#define AOC_T_HFUNC i32_hash
#define AOC_T_EQUALS i32_equals
#define AOC_BASE2_CAPACITY
#include <aoc/hashset.h>

static int32_t solve_part2(const AocArrayI32 *const numbers) {
  AocHashsetI32 frequencies = {0};
  AocHashsetI32Create(&frequencies, 1 << 18);

  size_t i = 0;
  int32_t frequency = 0;
  for (;;) {
    frequency += numbers->items[i];
    uint32_t hash = 0;
    if (AocHashsetI32Contains(&frequencies, frequency, &hash)) {
      AocHashsetI32Destroy(&frequencies);
      AOC_LOG("SIZE: %zu\n", frequencies.count);
      return frequency;
    } else {
      AocHashsetI32InsertPreHashed(&frequencies, frequency, hash);
    }
    i = (i + 1) % numbers->length;
  }

  // should never reach
  AocHashsetI32Destroy(&frequencies);
  return -1;
}

#include <time.h>

int main(void) {
  AocArenaAlloc(&arena, 1052592);
  AocArenaReset(&arena);

  AocArrayI32 numbers = {0};
  AocArrayI32Create(&numbers, 1000);
  AocReadFileLineByLine("day01/input.txt", parse_line, &numbers);

  const int32_t part1 = solve_part1(&numbers);
  const int32_t part2 = solve_part2(&numbers);

  printf("%d\n", part1);
  printf("%d\n", part2);

  AocArrayI32Destroy(&numbers);
  AocArenaFree(&arena);
}