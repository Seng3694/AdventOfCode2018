#include <stdlib.h>
#include <stdio.h>
#include <aoc/aoc.h>
#include <aoc/mem.h>

typedef struct {
  int x;
  int y;
} point;

typedef struct {
  int depth;
  point target;
} context;

typedef enum {
  TILE_TYPE_ROCKY = 0,
  TILE_TYPE_WET = 1,
  TILE_TYPE_NARROW = 2,
} tile_type;

typedef struct {
  tile_type type;
  int erosion;
} tile;

typedef struct {
  int width;
  int height;
  tile tiles[];
} map;

static inline void parse(char *input, context *const ctx) {
  ctx->depth = strtol(input + 7, &input, 10);
  ctx->target.x = strtol(input + 9, &input, 10);
  ctx->target.y = strtol(input + 1, &input, 10);
}

#define GEOLOGICAL_INDEX_ROW0(x) ((x) * 16807)
#define GEOLOGICAL_INDEX_COL0(y) ((y) * 48271)
#define GEOLOGICAL_INDEX(top, left) ((top) * (left))
#define EROSION_LEVEL(geoIndex, depth) (((geoIndex) + (depth)) % 20183)
#define EROSION_LEVEL_TO_TILE_TYPE(level) ((tile_type)((level) % 3))

static map *create_map(const context *const ctx) {
  const int w = ctx->target.x * 2;
  const int h = ctx->target.y * 2;
  map *m = AocAlloc(sizeof(map) + (sizeof(tile) * w * h));
  m->width = w;
  m->height = h;
  tile *t = m->tiles;
  t[0].erosion = EROSION_LEVEL(0, ctx->depth);
  t[0].type = EROSION_LEVEL_TO_TILE_TYPE(t[0].erosion);

  for (int x = 1; x < w; ++x) {
    t[x].erosion = EROSION_LEVEL(GEOLOGICAL_INDEX_ROW0(x), ctx->depth);
    t[x].type = EROSION_LEVEL_TO_TILE_TYPE(t[x].erosion);
  }
  for (int y = 1; y < h; ++y) {
    t[y * w].erosion = EROSION_LEVEL(GEOLOGICAL_INDEX_COL0(y), ctx->depth);
    t[y * w].type = EROSION_LEVEL_TO_TILE_TYPE(t[y * w].erosion);
  }

  t += w;
  for (int y = 1; y < h; ++y) {
    for (int x = 1; x < w; ++x) {
      if (y == ctx->target.y && x == ctx->target.x) {
        t[x].erosion = EROSION_LEVEL(0, ctx->depth);
      } else {
        t[x].erosion = EROSION_LEVEL(
            GEOLOGICAL_INDEX(t[x - w].erosion, t[x - 1].erosion), ctx->depth);
      }
      t[x].type = EROSION_LEVEL_TO_TILE_TYPE(t[x].erosion);
    }
    t += w;
  }
  return m;
}

static int solve_part1(const context *const ctx, const map *const m) {
  int riskLevel = 0;
  const tile *t = m->tiles;
  for (int y = 0; y <= ctx->target.y; ++y) {
    for (int x = 0; x <= ctx->target.x; ++x) {
      riskLevel += (int)t[x].type;
    }
    t += m->width;
  }
  return riskLevel;
}

int main(void) {
  char *input = NULL;
  size_t length = 0;
  AocReadFileToString("day22/input.txt", &input, &length);
  context ctx = {0};
  parse(input, &ctx);
  AocFree(input);

  map *m = create_map(&ctx);
  const int part1 = solve_part1(&ctx, m);

  printf("%d\n", part1);

  AocFree(m);
}
