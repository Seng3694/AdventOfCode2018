#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <assert.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <aoc/bump.h>

static aoc_allocator defaultAllocator = {0};
static aoc_allocator copyAllocator = {0};

typedef struct node {
  struct node *next;
  char value;
} node;

static node *parse(const char *const text, const size_t length) {
  node *root = AocAlloc(sizeof(node));
  root->value = text[0];
  node *current = root;
  for (size_t i = 1; i < length; ++i) {
    current->next = AocAlloc(sizeof(node));
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
  node *currentSrc = src;

  while ((currentSrc->value & ~32) == ignore)
    currentSrc = currentSrc->next;

  AocMemSetAllocator(&copyAllocator);
  AocBumpReset(copyAllocator.allocator);

  node *dest = AocAlloc(sizeof(node));
  dest->next = NULL;
  dest->value = currentSrc->value;

  node *currentDest = dest;

  while (currentSrc->next != NULL) {
    if ((currentSrc->next->value & ~32) != ignore) {
      currentDest->next = AocAlloc(sizeof(node));
      currentDest->next->next = NULL;
      currentDest->next->value = currentSrc->next->value;
      currentDest = currentDest->next;
      length++;
    }
    currentSrc = currentSrc->next;
  }

  AocMemSetAllocator(&defaultAllocator);

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
  aoc_bump defaultAlloc = {0};
  aoc_bump copyAlloc = {0};
  AocBumpInit(&defaultAlloc, 850024);
  AocBumpInit(&copyAlloc, 169712);
  defaultAllocator = AocBumpCreateAllocator(&defaultAlloc);
  copyAllocator = AocBumpCreateAllocator(&copyAlloc);

  AocMemSetAllocator(&defaultAllocator);

  char *text = NULL;
  size_t length = 0;
  AocReadFileToString("day05/input.txt", &text, &length);

  node *list = parse(text, length);

  const uint32_t part1 = solve_part1(list, length - 1);
  const uint32_t part2 = solve_part2(list, length);

  printf("%u\n", part1);
  printf("%u\n", part2);

  AocBumpDestroy(&defaultAlloc);
  AocBumpDestroy(&copyAlloc);
}
