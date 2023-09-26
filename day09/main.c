#include <stdio.h>
#include <stdlib.h>

#include <aoc/aoc.h>
#include <aoc/bump.h>

typedef struct marble {
  struct marble *left;
  struct marble *right;
  uint32_t value;
} marble;

static void parse(char *line, uint32_t *const players,
                  uint32_t *const lastMarbleWorth) {
  *players = strtol(line, &line, 10);
  *lastMarbleWorth = strtol(line + 31, &line, 10);
}

static inline marble *find_cw(marble *m, uint32_t index) {
  while (index != 0) {
    m = m->right;
    index--;
  }
  return m;
}

static inline marble *find_ccw(marble *m, uint32_t index) {
  while (index != 0) {
    m = m->left;
    index--;
  }
  return m;
}

static marble *insert_at_cw(marble *m, uint32_t index, uint32_t value) {
  m = find_cw(m, index);
  marble *new = AocAlloc(sizeof(marble));
  new->value = value;
  new->left = m;
  new->right = m->right;
  m->right->left = new;
  m->right = new;
  return new;
}

static marble *remove_at_ccw(marble *m, uint32_t index, uint32_t *const score) {
  m = find_ccw(m, index);
  *score += m->value;
  marble *left = m->left;
  marble *right = m->right;
  left->right = right;
  right->left = left;
  return right;
}

static uint32_t solve(const uint32_t players, const uint32_t lastMarbleWorth) {
  uint32_t *scores = AocCalloc(players, sizeof(uint32_t));

  marble *current = AocAlloc(sizeof(marble));
  current->left = current;
  current->right = current;
  current->value = 0;

  uint32_t i = 1;
  for (;;) {
    for (uint32_t p = 0; p < players; ++p) {
      if (i % 23 == 0) {
        scores[p] += i;
        current = remove_at_ccw(current, 7, &scores[p]);
      } else {
        current = insert_at_cw(current, 1, i);
      }
      i++;
      if (i == lastMarbleWorth)
        goto done;
    }
  }

done:;

  uint32_t highestScore = 0;
  for (uint32_t i = 0; i < players; ++i) {
    if (scores[i] > highestScore)
      highestScore = scores[i];
  }

  return highestScore;
}

int main(void) {
  aoc_bump bump = {0};
  AocBumpInit(&bump, 220000000);
  aoc_allocator allocator = AocBumpCreateAllocator(&bump);
  AocMemSetAllocator(&allocator);

  char *contents = NULL;
  size_t length = 0;
  AocReadFileToString("day09/input.txt", &contents, &length);

  uint32_t players = 0;
  uint32_t lastMarbleWorth = 0;
  parse(contents, &players, &lastMarbleWorth);

  const uint32_t part1 = solve(players, lastMarbleWorth);
  AocBumpReset(&bump);
  const uint32_t part2 = solve(players, lastMarbleWorth * 100);

  printf("%u\n", part1);
  printf("%u\n", part2);

  AocBumpDestroy(&bump);
}
