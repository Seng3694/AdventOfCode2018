#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>

static inline int8_t get_abs(const int8_t n) {
  const int8_t mask = n >> (sizeof(int8_t) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static uint32_t solve_part1(char *const text, size_t textLength) {
  bool changed = true;
  do {
    changed = false;
    for (int32_t i = (int32_t)textLength - 2; i >= 0; --i) {
      if (get_abs(text[i + 1] - text[i]) == 32) {
        textLength -= 2;
        changed = true;
        AocMemCopy(&text[i], &text[i + 2], textLength - i);
      }
    }
  } while (changed);
  return textLength - 1;
}

static inline size_t custom_copy(char *const dest, const char *const src,
                                 const size_t bytes, const char ignore) {
  size_t length = 0;
  for (size_t i = 0; i < bytes; ++i) {
    if ((src[i] & ~32) != ignore) {
      dest[length++] = src[i];
    }
  }
  return length;
}

static uint32_t solve_part2(char *const copy, const char *const text,
                            const size_t length) {
  uint32_t minLength = (uint32_t)length;
  for (char c = 'A'; c <= 'Z'; ++c) {
    const size_t copyLength = custom_copy(copy, text, length, c);
    const uint32_t newLength = solve_part1(copy, copyLength);
    if (newLength < minLength)
      minLength = newLength;
  }
  return minLength;
}

int main(void) {
  char *text = NULL;
  size_t length = 0;
  AocReadFileToString("day05/input.txt", &text, &length);

  char *copy = malloc(length);
  AocMemCopy(copy, text, length);

  const uint32_t part1 = solve_part1(copy, length);
  const uint32_t part2 = solve_part2(copy, text, length);

  printf("%u\n", part1);
  printf("%u\n", part2);

  free(copy);
  free(text);
}
