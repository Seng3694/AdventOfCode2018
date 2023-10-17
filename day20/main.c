#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <stdio.h>
#include <limits.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
  int x;
  int y;
} point;

#define AOC_T point
#define AOC_T_NAME Point
#include <aoc/array.h>

static inline uint32_t point_hash(const point *const p) {
  return 47254019u ^ (((*(uint32_t *)&p->x) * 84560501u) ^
                      ((*(uint32_t *)&p->y) * 57798941u));
}

static inline bool point_equals(const point *const a, const point *const b) {
  return a->x == b->x && a->y == b->y;
}

static const point emptyPoint = {INT32_MIN, INT32_MIN};

#define AOC_T point
#define AOC_T_NAME Point
#define AOC_T_HFUNC point_hash
#define AOC_T_EQUALS point_equals
#define AOC_T_EMPTY emptyPoint
#define AOC_BASE2_CAPACITY
#include <aoc/hashset.h>

typedef struct {
  const AocHashsetPoint *hs;
  size_t current;
} hs_iterator;

static bool hs_iterate(hs_iterator *const iterator, point *const outPoint) {
  point current = emptyPoint;
  while (iterator->current < iterator->hs->capacity &&
         point_equals(&current, &emptyPoint)) {
    current = iterator->hs->entries[iterator->current];
    iterator->current++;
  }
  if (iterator->current < iterator->hs->capacity) {
    *outPoint = iterator->hs->entries[(iterator->current - 1)];
    return true;
  }
  return false;
}

typedef enum {
  TILE_TYPE_EMPTY,
  TILE_TYPE_WALL,
  TILE_TYPE_DOOR,
} tile_type;

typedef struct {
  int width;
  int height;
  point start;
  tile_type tiles[];
} map;

static inline int fast_abs(const int n) {
  const int mask = n >> (sizeof(int) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static map *create_map(const AocHashsetPoint *const spaces,
                       const AocHashsetPoint *const doors, const int minX,
                       const int maxX, const int minY, const int maxY) {
  const int width = fast_abs(maxX - minX) + 1;
  const int height = fast_abs(maxY - minY) + 1;
  const int totalSize = width * height;
  map *m = AocAlloc(sizeof(map) + sizeof(tile_type) * totalSize);
  for (int i = 0; i < totalSize; ++i)
    m->tiles[i] = TILE_TYPE_WALL;

  m->width = width;
  m->height = height;
  const int xOffset = 0 - minX;
  const int yOffset = 0 - minY;
  m->start.x = xOffset;
  m->start.y = yOffset;

  hs_iterator iter = {.hs = spaces};
  point p = {0};
  while (hs_iterate(&iter, &p)) {
    const int i = (p.y + yOffset) * width + (p.x + xOffset);
    m->tiles[i] = TILE_TYPE_EMPTY;
  }
  iter.current = 0;
  iter.hs = doors;
  while (hs_iterate(&iter, &p)) {
    const int i = (p.y + yOffset) * width + (p.x + xOffset);
    m->tiles[i] = TILE_TYPE_DOOR;
  }
  return m;
}

static map *parse(const char *expression) {
  AocHashsetPoint spaces = {0};
  AocHashsetPointCreate(&spaces, 1 << 14);
  AocHashsetPoint doors = {0};
  AocHashsetPointCreate(&doors, 1 << 14);
  AocArrayPoint stack = {0};
  AocArrayPointCreate(&stack, 1 << 8);
  int minX = -1;
  int maxX = 1;
  int minY = -1;
  int maxY = 1;

  point current = {0};
  AocHashsetPointInsert(&spaces, current);
  for (;;) {
    const char c = *expression;
    switch (c) {
    case '(':
      AocArrayPointPush(&stack, current);
      break;
    case '|':
      current = *AocArrayPointLast(&stack);
      break;
    case ')':
      current = *AocArrayPointLast(&stack);
      AocArrayPointPop(&stack);
      break;
    case '$':
      goto done;
    case '^':
      break;
    default: {
      point space = current;
      point door = current;
      uint32_t hash = 0;
      switch (c) {
      case 'N':
        space.y -= 2;
        door.y -= 1;
        minY = MIN(minY, space.y - 1);
        break;
      case 'E':
        space.x += 2;
        door.x += 1;
        maxX = MAX(maxX, space.x + 1);
        break;
      case 'S':
        space.y += 2;
        door.y += 1;
        maxY = MAX(maxY, space.y + 1);
        break;
      case 'W':
        space.x -= 2;
        door.x -= 1;
        minX = MIN(minX, space.x - 1);
        break;
      }
      if (!AocHashsetPointContains(&spaces, space, &hash))
        AocHashsetPointInsertPreHashed(&spaces, space, hash);
      if (!AocHashsetPointContains(&doors, door, &hash))
        AocHashsetPointInsertPreHashed(&doors, door, hash);
      current = space;
      break;
    }
    }
    expression++;
  }

done:;
  map *m = create_map(&spaces, &doors, minX, maxX, minY, maxY);
  AocArrayPointDestroy(&stack);
  AocHashsetPointDestroy(&doors);
  AocHashsetPointDestroy(&spaces);
  return m;
}

static void get_adjacent_points(const map *const m, const point p,
                                point points[const 4], uint8_t *const count) {
  *count = 0;
  const point offsets[] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
  for (uint8_t i = 0; i < 4; ++i) {
    const int index = (p.y + offsets[i].y) * m->width + (p.x + offsets[i].x);
    if (m->tiles[index] == TILE_TYPE_DOOR) {
      points[*count] = (point){
          p.x + (offsets[i].x * 2),
          p.y + (offsets[i].y * 2),
      };
      (*count)++;
    }
  }
}

static void solve(const map *const m, uint32_t *const part1,
                  uint32_t *const part2) {
  AocHashsetPoint visited = {0};
  AocHashsetPointCreate(&visited, 1 << 6);
  AocArrayPoint current = {0};
  AocArrayPointCreate(&current, 1 << 14);

  AocHashsetPointInsert(&visited, m->start);
  AocArrayPointPush(&current, m->start);

  uint32_t pathLength = 0;
  point adjacent[4] = {0};
  uint8_t adjacentCount = 0;
  uint32_t hash = 0;

  while (current.length > 0) {
    const size_t length = current.length;
    for (size_t i = 0; i < length; ++i) {
      point p = current.items[i];
      get_adjacent_points(m, p, adjacent, &adjacentCount);
      for (uint8_t i = 0; i < adjacentCount; ++i) {
        if (!AocHashsetPointContains(&visited, adjacent[i], &hash)) {
          AocHashsetPointInsertPreHashed(&visited, adjacent[i], hash);
          AocArrayPointPush(&current, adjacent[i]);
        }
      }
    }
    const size_t newLength = current.length - length;
    for (size_t j = 0; j < newLength; ++j)
      current.items[j] = current.items[length + j];
    current.length = newLength;
    pathLength++;
    if (pathLength >= 1000)
      *part2 += newLength;
  }
  AocArrayPointDestroy(&current);
  AocHashsetPointDestroy(&visited);
  *part1 = pathLength - 1;
}

int main(void) {
  char *expression = NULL;
  size_t length = 0;
  AocReadFileToString("day20/input.txt", &expression, &length);

  map *m = parse(expression);
  AocFree(expression);

  uint32_t part1 = 0, part2 = 0;
  solve(m, &part1, &part2);
  printf("%u\n%u\n", part1, part2);

  AocFree(m);
}
