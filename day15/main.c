#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <aoc/aoc.h>
#include <aoc/mem.h>

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
  int16_t hp;
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

#define UNIT_TO_STR(type) (type == UNIT_TYPE_GOBLIN ? "goblin" : "elf")

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

static const int8_t unit_ap[2] = {
    [UNIT_TYPE_GOBLIN] = 3,
    [UNIT_TYPE_ELF] = 3,
};

static const int16_t unit_hp[2] = {
    [UNIT_TYPE_GOBLIN] = 200,
    [UNIT_TYPE_ELF] = 200,
};

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
          .hp = unit_hp[UNIT_TYPE_GOBLIN],
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
          .hp = unit_hp[UNIT_TYPE_ELF],
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

static inline int compare_point(const point *const a, const point *const b) {
  return (((int)a->y << 8) | a->x) - (((int)b->y << 8) | b->x);
}

static inline int compare_unit(const void *const left,
                               const void *const right) {
  const unit *const a = left;
  const unit *const b = right;
  return compare_point(&a->pos, &b->pos);
}

static inline int compare_unit_ptr(const void *const left,
                                   const void *const right) {
  const unit *const *const a = left;
  const unit *const *const b = right;
  return compare_point(&(*a)->pos, &(*b)->pos);
}

static inline void sort_units(unit *const units, const uint8_t count) {
  qsort(units, count, sizeof(unit), compare_unit);
}

static inline void sort_unit_ptrs(unit **const units, const uint8_t count) {
  qsort(units, count, sizeof(unit *), compare_unit_ptr);
}

static inline uint8_t fast_abs(const int8_t n) {
  const uint8_t mask = n >> (sizeof(uint8_t) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static inline uint8_t taxicab(const int8_t x1, const int8_t y1, const int8_t x2,
                              const int8_t y2) {
  return fast_abs(x1 - x2) + fast_abs(y1 - y2);
}

static inline uint8_t taxicab2(const point p1, const point p2) {
  return taxicab(p1.x, p1.y, p2.x, p2.y);
}

static void print_map(const map *const m) {
  const tile *current = m->data;
  for (int8_t y = 0; y < m->size; ++y) {
    printf(" ");
    for (int8_t x = 0; x < m->size; ++x) {
      switch (current[x].type) {
      case TILE_TYPE_EMPTY:
        printf("\e[0;30m.\e[0m ");
        break;
      case TILE_TYPE_WALL:
        printf("\e[0;30m■\e[0m ");
        break;
      case TILE_TYPE_UNIT:
        if (current[x].u->type == UNIT_TYPE_GOBLIN) {
          printf("\e[0;31mG\e[0m ");
        } else {
          printf("\e[0;32mE\e[0m ");
        }
        break;
      }
    }

    for (uint8_t x = 0; x < m->size; ++x) {
      if (current[x].u != NULL) {
        if (current[x].u->type == UNIT_TYPE_GOBLIN) {
          printf(" \e[0;31mG(%i)\e[0m", current[x].u->hp);
        } else {
          printf(" \e[0;32mE(%i)\e[0m", current[x].u->hp);
        }
      }
    }

    printf("\n");
    current += m->size;
  }
  printf("\n");
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

uint32_t solve_part1(context *const ctx) {
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
        target->hp -= unit_ap[u->type];
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

  return rounds * totalHp;
}

int main(void) {
  context ctx = {0};
  AocReadFileLineByLineEx("day15/input.txt", parse_line, &ctx);

  const uint32_t part1 = solve_part1(&ctx);

  printf("%u\n", part1);

  AocFree(ctx.map.data);
}
