#include <stdio.h>
#include <stdlib.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <aoc/bump.h>

#define AOC_T int32_t
#define AOC_T_NAME I32
#include <aoc/array.h>

static inline void parse_line(char *line, size_t length, void *userData) {
  (void)length;
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

int main(void) {
  aoc_bump bump = {0};
  AocBumpInit(&bump, 1052592);

  aoc_allocator allocator = AocBumpCreateAllocator(&bump);
  AocMemSetAllocator(&allocator);

  AocArrayI32 numbers = {0};
  AocArrayI32Create(&numbers, 1000);
  AocReadFileLineByLine("day01/input.txt", parse_line, &numbers);

  const int32_t part1 = solve_part1(&numbers);
  const int32_t part2 = solve_part2(&numbers);

  printf("%d\n", part1);
  printf("%d\n", part2);

  AocBumpDestroy(&bump);
}
