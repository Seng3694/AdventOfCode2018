#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>

#define EMPTY_CHAR '#'

static inline int8_t get_abs(const int8_t n) {
  const int8_t mask = n >> (sizeof(int8_t) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static uint32_t solve_part1(char *const text, const size_t textLength,
                            uint32_t polymerLength) {
  bool changed = true;
  size_t lastNonEmptyIndex = textLength - 1;
  while (changed) {
    changed = false;
    for (int32_t i = (int32_t)textLength - 2; i >= 0; --i) {
      if (text[i] == EMPTY_CHAR)
        continue;

      if (get_abs(text[lastNonEmptyIndex] - text[i]) == 32) {
        text[i] = EMPTY_CHAR;
        text[lastNonEmptyIndex] = EMPTY_CHAR;
        polymerLength -= 2;
        changed = true;

        while (i >= 0) {
          if (text[i] != EMPTY_CHAR) {
            lastNonEmptyIndex = i;
            break;
          }
          i--;
        }

      } else {
        lastNonEmptyIndex = i;
      }
    }
  }
  return polymerLength;
}

static uint32_t solve_part2(char *const copy, const char *const text,
                            const size_t length) {
  uint32_t minLength = UINT32_MAX;
  for (char c = 'A'; c <= 'Z'; ++c) {
    AocMemCopy(copy, text, length + 1);

    // flag chars
    uint32_t flaggedChars = 0;
    for (char *current = copy; *current != '\0'; ++current) {
      if (*current == c || *current == c + 32) {
        *current = EMPTY_CHAR;
        flaggedChars++;
      }
    }
    const uint32_t newLength = solve_part1(copy, length, length - flaggedChars);
    if (newLength < minLength) {
      minLength = newLength;
    }
  }
  return minLength;
}

int main(void) {
  char *text = NULL;
  size_t length = 0;
  AocReadFileToString("day05/input.txt", &text, &length);
  AocTrimRight(text, &length);

  char *copy = malloc(length + 1);
  AocMemCopy(copy, text, length + 1);

  const uint32_t part1 = solve_part1(copy, length, length);

  const uint32_t part2 = solve_part2(copy, text, length);

  printf("%u\n", part1);
  printf("%u\n", part2);

  free(copy);
  free(text);
}
