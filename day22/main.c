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
  const int w = ctx->target.x * 10;
  const int h = ctx->target.y * 10;
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

typedef enum {
  TOOL_NONE,
  TOOL_TORCH,
  TOOL_CLIMBING_GEAR,
} tool;

static const tool validTools[][2] = {
    [TILE_TYPE_ROCKY] = {TOOL_TORCH, TOOL_CLIMBING_GEAR},
    [TILE_TYPE_WET] = {TOOL_CLIMBING_GEAR, TOOL_NONE},
    [TILE_TYPE_NARROW] = {TOOL_TORCH, TOOL_NONE},
};

#define IS_VALID_TOOL(tileType, t)                                             \
  (validTools[tileType][0] == t || validTools[tileType][1] == t)

typedef struct {
  point position;
  point previous;
  tool tool;
  int timer;
} bfs_data;

static inline int bfs_data_compare(const bfs_data *const a,
                                   const bfs_data *const b) {
  return a->timer - b->timer;
}

#define AOC_T bfs_data
#define AOC_T_NAME BfsData
#define AOC_T_COMPARE bfs_data_compare
#include <aoc/heap.h>

static inline uint32_t bfs_data_hash(const bfs_data *const d) {
  return 47254019u ^ (((*(uint32_t *)&d->position.x) * 84560501u) ^
                      ((*(uint32_t *)&d->position.y) * 57798941u) ^
                      ((*(uint32_t *)&d->tool) * 37853173u));
}

static inline bool bfs_data_equals(const bfs_data *const a,
                                   const bfs_data *const b) {
  return a->position.x == b->position.x && a->position.y == b->position.y &&
         a->tool == b->tool;
}

static const bfs_data emptyBfsData = {.position = {INT32_MIN, INT32_MIN},
                                      .timer = -1};

#define AOC_T bfs_data
#define AOC_T_NAME BfsData
#define AOC_T_HFUNC bfs_data_hash
#define AOC_T_EQUALS bfs_data_equals
#define AOC_T_EMPTY emptyBfsData
#define AOC_BASE2_CAPACITY
#include <aoc/hashset.h>

static void get_adjacent(const bfs_data *const current,
                         bfs_data adjacent[const 12],
                         uint8_t *const adjacentCount, const map *const m) {
  *adjacentCount = 0;
  const point adjacentPoints[4] = {
      {current->position.x - 1, current->position.y + 0},
      {current->position.x + 1, current->position.y + 0},
      {current->position.x + 0, current->position.y + 1},
      {current->position.x + 0, current->position.y - 1},
  };

  const tile currentTile =
      m->tiles[current->position.y * m->width + current->position.x];
  for (uint8_t i = 0; i < 4; ++i) {
    point p = adjacentPoints[i];
    if (p.x < 0 || p.y < 0 ||
        (p.x == current->previous.x && p.y == current->previous.y))
      continue;
    tile mapTile = m->tiles[p.y * m->width + p.x];

    for (tool t = 0; t < 3; ++t) {
      if (!IS_VALID_TOOL(mapTile.type, t) ||
          !IS_VALID_TOOL(currentTile.type, t))
        continue;
      bfs_data data = {
          .position = p,
          .timer = t == current->tool ? 1 : 8,
          .tool = t,
          .previous = current->position,
      };
      adjacent[(*adjacentCount)++] = data;
    }
  }
}

static int solve_part2(const context *const ctx, const map *const m) {
  AocMinHeapBfsData current = {0};
  AocHashsetBfsData visited = {0};

  AocMinHeapBfsDataCreate(&current, 1 << 15);
  AocHashsetBfsDataCreate(&visited, 1 << 21);

  bfs_data start = {.position = {0, 0}, .timer = 0, .tool = TOOL_TORCH};
  AocMinHeapBfsDataPush(&current, start);

  bfs_data adjacent[12] = {0};
  uint8_t adjacentCount = 0;
  int time = 0;

  while (current.count > 0) {
    bfs_data b;
    while (AocMinHeapBfsDataPeek(&current).timer == 0) {
      b = AocMinHeapBfsDataPop(&current);

      uint32_t hash = 0;
      if (!AocHashsetBfsDataContains(&visited, b, &hash))
        AocHashsetBfsDataInsertPreHashed(&visited, b, hash);
      else
        continue;

      if (b.position.x == ctx->target.x && b.position.y == ctx->target.y) {
        if (b.tool != TOOL_TORCH) {
          b.tool = TOOL_TORCH;
          b.timer = 7;
          AocMinHeapBfsDataPush(&current, b);
          continue;
        }
        goto done;
      }

      get_adjacent(&b, adjacent, &adjacentCount, m);
      for (uint8_t i = 0; i < adjacentCount; ++i) {
        AocMinHeapBfsDataPush(&current, adjacent[i]);
      }
    }

    for (size_t i = 0; i < current.count; ++i)
      current.items[i].timer--;

    time++;
  }

done:
  AocMinHeapBfsDataDestroy(&current);
  AocHashsetBfsDataDestroy(&visited);

  return time;
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
  const int part2 = solve_part2(&ctx, m);

  printf("%d\n", part1);
  printf("%d\n", part2);

  AocFree(m);
}
