#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>

#define MAP_WIDTH 32
#define MAP_HEIGHT 32

#define GOBLIN_AP 3
#define GOBLIN_HP 200

#define ELF_AP 3
#define ELF_HP 200

#define INVALID_ID ((int8_t)-1)

typedef struct {
  int16_t hp;
  int8_t x;
  int8_t y;
  int8_t id;
} unit;

typedef enum {
  TILE_TYPE_EMPTY,
  TILE_TYPE_WALL,
  TILE_TYPE_GOBLIN,
  TILE_TYPE_ELF,
} tile_type;

typedef struct {
  tile_type type;
  int8_t id;
} tile;

typedef struct {
  tile map[MAP_WIDTH * MAP_HEIGHT];
  unit elves[32];
  unit goblins[32];
  uint8_t elfCount;
  uint8_t goblinCount;
} context;

static void parse_line(char *line, size_t length, void *userData,
                       const size_t lineNumber) {
  (void)length;
  context *const ctx = userData;
  const int8_t y = (int8_t)lineNumber;
  for (int8_t x = 0; x < MAP_WIDTH; ++x) {
    tile t = {.id = INVALID_ID};
    switch (line[x]) {
    case '.':
      t.type = TILE_TYPE_EMPTY;
      break;
    case '#':
      t.type = TILE_TYPE_WALL;
      break;
    case 'G':
      t.type = TILE_TYPE_GOBLIN;
      t.id = ctx->goblinCount;
      ctx->goblins[ctx->goblinCount] = (unit){
          .x = x,
          .y = y,
          .id = ctx->goblinCount,
          .hp = GOBLIN_HP,
      };
      ctx->goblinCount++;
      break;
    case 'E':
      t.type = TILE_TYPE_ELF;
      t.id = ctx->elfCount;
      ctx->elves[ctx->elfCount] = (unit){
          .x = x,
          .y = y,
          .id = ctx->elfCount,
          .hp = ELF_HP,
      };
      ctx->elfCount++;
      break;
    default:
      continue;
    }
    ctx->map[y * MAP_WIDTH + x] = t;
  }
}

static void print_map(const tile *const map) {
  static const char char_map[] = {
      [TILE_TYPE_EMPTY] = '.',
      [TILE_TYPE_WALL] = '#',
      [TILE_TYPE_GOBLIN] = 'G',
      [TILE_TYPE_ELF] = 'E',
  };
  static const char *color_map[] = {
      [TILE_TYPE_EMPTY] = "\e[0;36m",
      [TILE_TYPE_WALL] = "\e[0;37m",
      [TILE_TYPE_GOBLIN] = "\e[0;31m",
      [TILE_TYPE_ELF] = "\e[0;32m",
  };
  const tile *current = map;
  for (int8_t y = 0; y < MAP_HEIGHT; ++y) {
    for (int8_t x = 0; x < MAP_WIDTH; ++x) {
      printf("%s%c\e[0m", color_map[current[x].type],
             char_map[current[x].type]);
    }
    printf("\n");
    current += MAP_WIDTH;
  }
  printf("\n");
}

static void print_units(const unit *const units, const uint8_t unitCount,
                        const char *title) {
  printf("%s:\n", title);
  for (uint8_t i = 0; i < unitCount; ++i) {
    printf(" %2d: x=%2d y=%2d hp=%3d\n", units[i].id, units[i].x, units[i].y,
           units[i].hp);
  }
  printf("\n");
}

static inline int compare_unit(const void *const left,
                               const void *const right) {
  const unit *const a = left;
  const unit *const b = right;
  return (((int)a->y << 8) | a->x) - (((int)b->y << 8) | b->x);
}

static inline sort_units(const unit *const units, const uint8_t count) {
  qsort(units, count, sizeof(unit), compare_unit);
}

static inline uint8_t fast_abs(const int8_t n) {
  const uint8_t mask = n >> (sizeof(uint8_t) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static inline uint8_t taxicab(const int8_t x1, const int8_t y1, const int8_t x2,
                              const int8_t y2) {
  return fast_abs(x1 - x2) + fast_abs(y1 - y2);
}

static void solve_part1(context *const ctx) {
  unit allUnits[64] = {0};
  const uint8_t allUnitsCount = ctx->goblinCount + ctx->elfCount;
  AocMemCopy(allUnits, ctx->goblins, ctx->goblinCount * sizeof(unit));
  AocMemCopy(&allUnits[ctx->goblinCount], ctx->elves,
             ctx->elfCount * sizeof(unit));

  print_units(allUnits, allUnitsCount, "all units");
}

int main(void) {
  context ctx = {0};
  AocReadFileLineByLineEx("day15/input.txt", parse_line, &ctx);

  solve_part1(&ctx);
}
