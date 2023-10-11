#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <stdio.h>

#define MAP_SIZE 50
#define MAP_TOTAL_SIZE (MAP_SIZE * MAP_SIZE)

typedef enum {
  TILE_TYPE_OPEN,
  TILE_TYPE_TREES,
  TILE_TYPE_LUMBERYARD,
} tile_type;

typedef tile_type map[MAP_SIZE * MAP_SIZE];

static void parse(const char *str, map m) {
  int i = 0;
  do {
    // clang-format off
    switch (*str) {
    case '.': m[i++] = TILE_TYPE_OPEN; break;
    case '#': m[i++] = TILE_TYPE_LUMBERYARD; break;
    case '|': m[i++] = TILE_TYPE_TREES; break;
    }
    // clang-format on
  } while (*(++str));
}

static tile_type transform(const tile_type current, const int counts[const 3]) {
  switch (current) {
  case TILE_TYPE_OPEN:
    return counts[TILE_TYPE_TREES] >= 3 ? TILE_TYPE_TREES : TILE_TYPE_OPEN;
  case TILE_TYPE_TREES:
    return counts[TILE_TYPE_LUMBERYARD] >= 3 ? TILE_TYPE_LUMBERYARD
                                             : TILE_TYPE_TREES;
  case TILE_TYPE_LUMBERYARD:
    return counts[TILE_TYPE_LUMBERYARD] > 0 && counts[TILE_TYPE_TREES] > 0
               ? TILE_TYPE_LUMBERYARD
               : TILE_TYPE_OPEN;
    break;
  }
  // should never reach
  return TILE_TYPE_OPEN;
}

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void count_adjacent(map m, int x, int y, int counts[const 3]) {
  counts[0] = 0;
  counts[1] = 0;
  counts[2] = 0;
  for (int ya = MAX(y - 1, 0); ya < MIN(y + 2, MAP_SIZE); ++ya) {
    for (int xa = MAX(x - 1, 0); xa < MIN(x + 2, MAP_SIZE); ++xa) {
      counts[m[ya * MAP_SIZE + xa]]++;
    }
  }
  counts[m[y * MAP_SIZE + x]]--;
}

static void tick(map front, map back) {
  for (int y = 0; y < MAP_SIZE; ++y) {
    for (int x = 0; x < MAP_SIZE; ++x) {
      int c[3] = {0};
      count_adjacent(front, x, y, c);
      int i = y * MAP_SIZE + x;
      back[i] = transform(front[i], c);
    }
  }
}

static int solve_part1(map m) {
  map buffer = {0};

  tile_type *front = m;
  tile_type *back = buffer;

  for (int i = 0; i < 10; ++i) {
    tick(front, back);
    tile_type *tmp = front;
    front = back;
    back = tmp;
  }

  int counts[3] = {0};
  for (int i = 0; i < MAP_TOTAL_SIZE; ++i)
    counts[m[i]]++;
  return counts[TILE_TYPE_LUMBERYARD] * counts[TILE_TYPE_TREES];
}

int main(void) {
  map m = {0};
  char *contents = NULL;
  size_t length = 0;
  AocReadFileToString("day18/input.txt", &contents, &length);
  parse(contents, m);

  const int part1 = solve_part1(m);
  printf("%d\n", part1);

  AocFree(contents);
}
