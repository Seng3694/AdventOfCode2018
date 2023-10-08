#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <aoc/bump.h>

aoc_bump mainBump = {0};
aoc_bump pathFindingBump = {0};
aoc_allocator mainAllocator = {0};
aoc_allocator pathFindingAllocator = {0};

typedef struct {
  uint8_t x;
  uint8_t y;
} point;

typedef enum {
  UNIT_TYPE_GOBLIN,
  UNIT_TYPE_ELF,
} unit_type;

typedef struct {
  unit_type type;
  int32_t hp;
  point pos;
  int8_t id;
} unit;

typedef enum {
  TILE_TYPE_EMPTY,
  TILE_TYPE_WALL,
  TILE_TYPE_UNIT,
} tile_type;

typedef struct {
  tile_type type;
  unit *u;
} tile;

typedef struct {
  tile *data;
  uint8_t size;
} map;

typedef struct {
  map map;
  unit units[2][32];
  uint8_t counts[2];
} context;

#define AOC_T point
#define AOC_T_NAME Point
#include <aoc/array.h>

static inline uint32_t point_hash(const point *const p) {
  return 54812489 * ((uint32_t)p->x ^ 95723417) * ((uint32_t)p->y ^ 69660419);
}

static inline bool point_equals(const point *const a, const point *const b) {
  return a->x == b->x && a->y == b->y;
}

#define AOC_T point
#define AOC_T_NAME Point
#define AOC_T_EMPTY ((point){0, 0})
#define AOC_T_HFUNC point_hash
#define AOC_T_EQUALS point_equals
#define AOC_BASE2_CAPACITY
#include <aoc/hashset.h>

void parse_line(char *line, size_t length, void *userData,
                const size_t lineNumber) {
  context *const ctx = userData;
  ctx->map.size = length - 1;
  if (lineNumber == 0) /* assume w and h to be the same */ {
    ctx->map.data =
        AocAlloc(sizeof(tile) * (size_t)ctx->map.size * (size_t)ctx->map.size);
  }
  const int8_t y = (int8_t)lineNumber;
  for (int8_t x = 0; x < ctx->map.size; ++x) {
    tile t = {0};
    switch (line[x]) {
    case '.':
      t.type = TILE_TYPE_EMPTY;
      break;
    case '#':
      t.type = TILE_TYPE_WALL;
      break;
    case 'G':
      t.type = TILE_TYPE_UNIT;
      ctx->units[UNIT_TYPE_GOBLIN][ctx->counts[UNIT_TYPE_GOBLIN]] = (unit){
          .pos.x = x,
          .pos.y = y,
          .id = ctx->counts[UNIT_TYPE_GOBLIN],
          .hp = 200,
          .type = UNIT_TYPE_GOBLIN,
      };
      t.u = &ctx->units[UNIT_TYPE_GOBLIN][ctx->counts[UNIT_TYPE_GOBLIN]];
      ctx->counts[UNIT_TYPE_GOBLIN]++;
      break;
    case 'E':
      t.type = TILE_TYPE_UNIT;
      ctx->units[UNIT_TYPE_ELF][ctx->counts[UNIT_TYPE_ELF]] = (unit){
          .pos.x = x,
          .pos.y = y,
          .id = ctx->counts[UNIT_TYPE_ELF],
          .hp = 200,
          .type = UNIT_TYPE_ELF,
      };
      t.u = &ctx->units[UNIT_TYPE_ELF][ctx->counts[UNIT_TYPE_ELF]];
      ctx->counts[UNIT_TYPE_ELF]++;
      break;
    default:
      continue;
    }
    ctx->map.data[y * ctx->map.size + x] = t;
  }
}

static void copy_context(context *const dest, const context *const src) {
  dest->map.size = src->map.size;
  AocMemCopy(dest->map.data, src->map.data,
             sizeof(tile) * (size_t)src->map.size * (size_t)src->map.size);
  AocMemCopy(dest->units, src->units, sizeof(unit) * 2 * 32);
  AocMemCopy(dest->counts, src->counts, sizeof(uint8_t) * 2);

  for (uint8_t t = 0; t < 2; ++t) {
    for (uint8_t i = 0; i < src->counts[t]; ++i) {
      const unit *const u = &src->units[t][i];
      dest->map.data[u->pos.y * src->map.size + u->pos.x].u =
          &dest->units[t][i];
    }
  }
}

static void clone_context(context *const dest, const context *const src) {
  dest->map.data =
      AocAlloc(sizeof(tile) * (size_t)src->map.size * (size_t)src->map.size);
  copy_context(dest, src);
}

static inline int compare_point(const point *const a, const point *const b) {
  return (((int)a->y << 8) | a->x) - (((int)b->y << 8) | b->x);
}

static inline int compare_unit_ptr(const void *const left,
                                   const void *const right) {
  const unit *const *const a = left;
  const unit *const *const b = right;
  return compare_point(&(*a)->pos, &(*b)->pos);
}

static inline void sort_unit_ptrs(unit **const units, const uint8_t count) {
  qsort(units, count, sizeof(unit *), compare_unit_ptr);
}

void get_valid_adjacent_points(const map *const m, const point pos,
                               const unit_type targetType,
                               point positions[const 4],
                               uint8_t *const positionCount) {
  *positionCount = 0;
  // reading order
  const point adjacent[4] = {
      {pos.x + 0, pos.y - 1}, // up
      {pos.x - 1, pos.y + 0}, // left
      {pos.x + 1, pos.y + 0}, // right
      {pos.x + 0, pos.y + 1}, // down
  };
  for (uint8_t i = 0; i < 4; ++i) {
    int index = adjacent[i].y * m->size + adjacent[i].x;
    if (m->data[index].type <= TILE_TYPE_EMPTY ||
        (m->data[index].type == TILE_TYPE_UNIT &&
         m->data[index].u->type == targetType))
      positions[(*positionCount)++] = adjacent[i];
  }
}

typedef struct {
  point position;
  point startingPoint;
  point lastPoint;
} bfs_data;

#define AOC_T bfs_data
#define AOC_T_NAME BfsData
#include <aoc/array.h>

int32_t shortest_path_to_target(const map *const m, const point from,
                                const point to, const unit_type targetType,
                                point *const nextPosition,
                                point *const nextTargetPosition) {
  // todo: could probably do something a lot faster with A*
  AocBumpReset(&pathFindingBump);
  AocMemSetAllocator(&pathFindingAllocator);

  AocArrayBfsData data = {0};
  AocArrayBfsDataCreate(&data, 1 << 12);

  AocHashsetPoint visited = {0};
  AocHashsetPointCreate(&visited, 1 << 12);
  AocHashsetPointInsert(&visited, from);

  point fromAdjacent[4] = {0};
  uint8_t fromAdjacentCount = 0;
  get_valid_adjacent_points(m, from, targetType, fromAdjacent,
                            &fromAdjacentCount);

  point toAdjacent[4] = {0};
  uint8_t toAdjacentCount = 0;
  get_valid_adjacent_points(m, to, targetType, toAdjacent, &toAdjacentCount);

  int32_t shortestPathLength = INT32_MAX;
  point bestStart = {0};
  point bestEnd = {0};
  bool foundPath = false;

  for (uint8_t fi = 0; fi < fromAdjacentCount; ++fi) {
    for (uint8_t ti = 0; ti < toAdjacentCount; ++ti) {
      AocArrayBfsDataClear(&data);
      AocHashsetPointClear(&visited);
      bfs_data d = {
          .lastPoint = from,
          .startingPoint = fromAdjacent[fi],
          .position = fromAdjacent[fi],
      };
      AocArrayBfsDataPush(&data, d);

      int32_t pathLength = 0;
      point start = {0};
      point end = {0};
      point toAdj = toAdjacent[ti];

      while (data.length > 0) {
        const size_t length = data.length;
        for (size_t i = 0; i < length; ++i) {
          const bfs_data *const current = &data.items[i];

          if (current->position.x == toAdj.x &&
              current->position.y == toAdj.y) {
            start = current->startingPoint;
            end = current->lastPoint;
            foundPath = true;
            goto done;
          }

          point adjacent[4] = {0};
          uint8_t adjacentCount = 0;

          get_valid_adjacent_points(m, current->position, targetType, adjacent,
                                    &adjacentCount);
          for (uint8_t j = 0; j < adjacentCount; ++j) {
            uint32_t hash = 0;
            if (!AocHashsetPointContains(&visited, adjacent[j], &hash)) {
              AocHashsetPointInsertPreHashed(&visited, adjacent[j], hash);
              const bfs_data d = {
                  .lastPoint = current->position,
                  .startingPoint = current->startingPoint,
                  .position = adjacent[j],
              };
              AocArrayBfsDataPush(&data, d);
            }
          }
        }

        const size_t newLength = data.length - length;
        for (size_t j = 0; j < newLength; ++j)
          data.items[j] = data.items[length + j];
        data.length = newLength;
        pathLength++;
      }
      pathLength = -1;

    done:
      if (pathLength != -1 && pathLength < shortestPathLength) {
        bestStart = start;
        bestEnd = end;
        shortestPathLength = pathLength;
      } else if (pathLength == shortestPathLength) {
        const int startComparison = compare_point(&bestStart, &start);
        const int endComparison = compare_point(&bestEnd, &end);
        if (endComparison > 0 || (endComparison == 0 && startComparison > 0)) {
          bestStart = start;
          bestEnd = end;
        }
      }
    }
  }

  if (!foundPath) {
    shortestPathLength = -1;
  } else {
    *nextPosition = bestStart;
    *nextTargetPosition = bestEnd;
  }

  AocHashsetPointDestroy(&visited);
  AocArrayBfsDataDestroy(&data);

  AocMemSetAllocator(&mainAllocator);
  return shortestPathLength;
}

bool get_adjacent_target(const map *const m, const unit *const u,
                         unit **outTarget) {
  const uint32_t adjacentIndices[] = {
      (u->pos.y - 1) * m->size + (u->pos.x + 0),
      (u->pos.y + 0) * m->size + (u->pos.x - 1),
      (u->pos.y + 0) * m->size + (u->pos.x + 1),
      (u->pos.y + 1) * m->size + (u->pos.x + 0),
  };

  int16_t minHp = INT16_MAX;
  int8_t index = -1;
  for (uint8_t i = 0; i < 4; ++i) {
    const unit *const mu = m->data[adjacentIndices[i]].u;
    if (mu != NULL && mu->type != u->type && (index == -1 || mu->hp < minHp)) {
      index = i;
      minHp = mu->hp;
    }
  }
  if (index != -1) {
    *outTarget = m->data[adjacentIndices[index]].u;
    return true;
  }
  return false;
}

static void solve_part1(context *const ctx, const uint32_t elfAp,
                        uint32_t *const result,
                        uint32_t *const remainingElves) {
  unit *allUnits[64] = {0};
  uint8_t allUnitsCount = 0;
  uint8_t remainingCounts[2] = {
      [UNIT_TYPE_GOBLIN] = ctx->counts[UNIT_TYPE_GOBLIN],
      [UNIT_TYPE_ELF] = ctx->counts[UNIT_TYPE_ELF],
  };

  for (uint8_t i = 0; i < ctx->counts[UNIT_TYPE_GOBLIN]; ++i)
    allUnits[allUnitsCount++] = &ctx->units[UNIT_TYPE_GOBLIN][i];
  for (uint8_t i = 0; i < ctx->counts[UNIT_TYPE_ELF]; ++i)
    allUnits[allUnitsCount++] = &ctx->units[UNIT_TYPE_ELF][i];

  const uint32_t ap[] = {
      [UNIT_TYPE_GOBLIN] = 3,
      [UNIT_TYPE_ELF] = elfAp,
  };

  uint32_t rounds = 0;
  for (rounds = 0;; ++rounds) {
    sort_unit_ptrs(allUnits, allUnitsCount);
    if (rounds == 10)
      rounds = rounds;

    for (uint8_t i = 0; i < allUnitsCount; ++i) {
      unit *const u = allUnits[i];
      if (u->hp <= 0)
        continue;

      const unit_type targetType = (unit_type)(((int)u->type + 1) & 1);

      if (remainingCounts[targetType] == 0)
        goto done;

      unit *const targets = ctx->units[targetType];
      const uint8_t count = ctx->counts[targetType];

      unit *target = NULL;
      if (get_adjacent_target(&ctx->map, u, &target)) {
        goto attack;
      }

      point nextPosition = u->pos;
      point nextTargetPosition = {ctx->map.size, ctx->map.size};
      int32_t shortestPath = INT32_MAX;
      bool pathFound = false;

      for (uint8_t j = 0; j < count; ++j) {
        if (targets[j].hp <= 0)
          continue;

        point pos = {0};
        point targetPos = {0};
        const int32_t pathLength = shortest_path_to_target(
            &ctx->map, u->pos, targets[j].pos, targetType, &pos, &targetPos);
        if (pathLength != -1) {
          if (pathLength < shortestPath ||
              (pathLength == shortestPath &&
               compare_point(&nextTargetPosition, &targetPos) > 0) ||
              (pathLength == shortestPath &&
               compare_point(&nextTargetPosition, &targetPos) == 0 &&
               compare_point(&nextPosition, &pos) > 0)) {
            nextPosition = pos;
            nextTargetPosition = targetPos;
            shortestPath = pathLength;
            pathFound = true;
          }
        }
      }

      if (pathFound) {
        // move
        ctx->map.data[u->pos.y * ctx->map.size + u->pos.x].u = NULL;
        ctx->map.data[u->pos.y * ctx->map.size + u->pos.x].type =
            TILE_TYPE_EMPTY;
        u->pos = nextPosition;
        ctx->map.data[u->pos.y * ctx->map.size + u->pos.x].u = u;
        ctx->map.data[u->pos.y * ctx->map.size + u->pos.x].type =
            TILE_TYPE_UNIT;
      }

      if (get_adjacent_target(&ctx->map, u, &target)) {
      attack:
        target->hp -= ap[u->type];
        if (target->hp <= 0) {
          ctx->map.data[target->pos.y * ctx->map.size + target->pos.x].u = NULL;
          ctx->map.data[target->pos.y * ctx->map.size + target->pos.x].type =
              TILE_TYPE_EMPTY;
          target->hp = 0;
          remainingCounts[targetType]--;
        }
      }
    }
  }

done:;
  uint32_t totalHp = 0;
  for (uint8_t i = 0; i < ctx->counts[UNIT_TYPE_GOBLIN]; ++i)
    totalHp += ctx->units[UNIT_TYPE_GOBLIN][i].hp;
  for (uint8_t i = 0; i < ctx->counts[UNIT_TYPE_ELF]; ++i)
    totalHp += ctx->units[UNIT_TYPE_ELF][i].hp;

  *result = totalHp * rounds;
  if (remainingElves != NULL)
    *remainingElves = remainingCounts[UNIT_TYPE_ELF];
}

static uint32_t solve_part2(const context *const baseCtx, context *const ctx) {
  // todo: could probably do binary search instead
  uint32_t elfAp = 4;
  uint32_t result = 0;
  uint32_t remainingElves = 0;
  do {
    copy_context(ctx, baseCtx);
    solve_part1(ctx, elfAp, &result, &remainingElves);
    elfAp++;
  } while (remainingElves != baseCtx->counts[UNIT_TYPE_ELF]);
  return result;
}

int main(void) {
  AocBumpInit(&mainBump, 40000);
  AocBumpInit(&pathFindingBump, 40000);

  mainAllocator = AocBumpCreateAllocator(&mainBump);
  pathFindingAllocator = AocBumpCreateAllocator(&pathFindingBump);
  AocMemSetAllocator(&mainAllocator);

  context ctx = {0};
  AocReadFileLineByLineEx("day15/input.txt", parse_line, &ctx);
  context clone = {0};
  clone_context(&clone, &ctx);

  uint32_t part1 = 0;
  solve_part1(&clone, 3, &part1, NULL);
  const uint32_t part2 = solve_part2(&ctx, &clone);

  printf("%u\n", part1);
  printf("%u\n", part2);

  AocBumpDestroy(&pathFindingBump);
  AocBumpDestroy(&mainBump);
}
