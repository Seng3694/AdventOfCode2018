#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#define AOC_USE_ARENA_DEFAULT
#include <aoc/aoc.h>
#include <aoc/arena.h>

aoc_arena defaultArena = {0};
aoc_arena listCopyArena = {0};

typedef struct node {
  struct node *next;
  char value;
} node;

static node *parse(const char *const text, const size_t length) {
  node *root = AocMalloc(sizeof(node));
  root->value = text[0];
  node *current = root;
  for (size_t i = 1; i < length; ++i) {
    current->next = AocMalloc(sizeof(node));
    current->next->value = text[i];
    current->next->next = NULL;
    current = current->next;
  }
  return root;
}

static inline int8_t get_abs(const int8_t n) {
  const int8_t mask = n >> (sizeof(int8_t) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static uint32_t solve_part1(node *list, size_t length) {
  bool changed = true;
  do {
    changed = false;
    node *previous = NULL;
    node *current = list;
    node *next = current->next;

    while (next != NULL) {
      if (get_abs(current->value - next->value) == 32) {
        length -= 2;
        changed = true;
        if (previous != NULL) {
          previous->next = next->next;
        } else {
          list = next->next;
        }
        current = next->next;
        next = current->next;
      } else {
        previous = current;
        current = next;
        next = current->next;
      }
    }

  } while (changed);

  return length;
}

static size_t custom_copy(node *src, const char ignore, node **out) {
  size_t length = 0;

  AocArenaReset(&listCopyArena);
  AocSetArena(&listCopyArena);

  node *currentSrc = src;

  while ((currentSrc->value & ~32) == ignore)
    currentSrc = currentSrc->next;

  node *dest = AocMalloc(sizeof(node));
  dest->next = NULL;
  dest->value = currentSrc->value;

  node *currentDest = dest;

  while (currentSrc->next != NULL) {
    if ((currentSrc->next->value & ~32) != ignore) {
      currentDest->next = AocMalloc(sizeof(node));
      currentDest->next->next = NULL;
      currentDest->next->value = currentSrc->next->value;
      currentDest = currentDest->next;
      length++;
    }
    currentSrc = currentSrc->next;
  }

  AocSetArena(&defaultArena);

  *out = dest;
  return length;
}

static uint32_t solve_part2(node *list, const size_t length) {
  uint32_t minLength = (uint32_t)length;
  for (char c = 'A'; c <= 'Z'; ++c) {
    node *copy = NULL;
    const size_t copyLength = custom_copy(list, c, &copy);
    const uint32_t newLength = solve_part1(copy, copyLength);
    if (newLength < minLength)
      minLength = newLength;
  }
  return minLength;
}

int main(void) {
  AocArenaAlloc(&defaultArena, 1200024);
  AocArenaReset(&defaultArena);
  AocArenaAlloc(&listCopyArena, 260000);
  AocArenaReset(&listCopyArena);

  AocSetArena(&defaultArena);

  char *text = NULL;
  size_t length = 0;
  AocReadFileToString("day05/input.txt", &text, &length);

  node *list = parse(text, length);

  const uint32_t part1 = solve_part1(list, length - 1);
  const uint32_t part2 = solve_part2(list, length);

  printf("%u\n", part1);
  printf("%u\n", part2);

  AocArenaFree(&defaultArena);
  AocArenaFree(&listCopyArena);
  free(text);
}
