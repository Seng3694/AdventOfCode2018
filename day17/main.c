#include <stdlib.h>
#include <stdio.h>
#include <aoc/aoc.h>

typedef uint16_t u16;
typedef uint32_t u32;

typedef struct {
  u16 x;
  u16 y;
} point;

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

typedef struct {
  u16 fromX;
  u16 toX;
  u16 fromY;
  u16 toY;
} clay;

#define AOC_T clay
#define AOC_T_NAME Clay
#include <aoc/array.h>

typedef struct {
  AocArrayClay clay;
  u16 minX;
  u16 minY;
  u16 maxX;
  u16 maxY;
} context;

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static void parse(char *line, size_t length, void *userData) {
  (void)length;
  context *const ctx = userData;
  clay c = {0};
  if (*line == 'x') {
    c.fromX = strtol(line + 2, &line, 10);
    c.toX = c.fromX;
    c.fromY = strtol(line + 4, &line, 10);
    c.toY = strtol(line + 2, &line, 10);
  } else {
    c.fromY = strtol(line + 2, &line, 10);
    c.toY = c.fromY;
    c.fromX = strtol(line + 4, &line, 10);
    c.toX = strtol(line + 2, &line, 10);
  }
  ctx->minX = MIN(ctx->minX, c.fromX);
  ctx->maxX = MAX(ctx->maxX, c.toX);
  ctx->minY = MIN(ctx->minY, c.fromY);
  ctx->maxY = MAX(ctx->maxY, c.toY);
  AocArrayClayPush(&ctx->clay, c);
}

static u32 solve_part1(const context *const ctx) {
  AocArrayPoint current = {0};
  AocArrayPointCreate(&current, 128);
  AocArrayPointPush(&current, (point){500, 0});

  AocHashsetPoint solid = {0};
  AocHashsetPointCreate(&solid, 1 << 12);
  AocHashsetPoint water = {0};
  AocHashsetPointCreate(&water, 1 << 12);

  for (size_t i = 0; i < ctx->clay.length; ++i) {
    clay c = ctx->clay.items[i];
    for (u16 y = c.fromY; y < c.toY + 1; ++y) {
      for (u16 x = c.fromX; x < c.toX + 1; ++x) {
        AocHashsetPointInsert(&solid, (point){x, y});
      }
    }
  }

  while (current.length > 0) {
    const size_t length = current.length;
    for (size_t i = 0; i < length; ++i) {
      point p = current.items[i];

      if (p.y < ctx->maxY) {
        point newPos = {p.x, p.y + 1};
        u32 hash = 0;
        if (AocHashsetPointContains(&solid, newPos, &hash)) {
          point left = {newPos.x - 1, newPos.y};
          point right = {newPos.x + 1, newPos.y};
          point belowLeft = {left.x, left.y + 1};
          point belowRight = {right.x, right.y + 1};
          bool hasWallLeft = false;
          bool hasWallRight = false;
          bool hasGroundLeft = false;
          bool hasGroundRight = false;

          do {
            newPos.y--;
            left = (point){newPos.x - 1, newPos.y};
            right = (point){newPos.x + 1, newPos.y};
            belowLeft = (point){left.x, left.y + 1};
            belowRight = (point){right.x, right.y + 1};

            hasWallLeft = false;
            hasWallRight = false;
            hasGroundLeft = false;
            hasGroundRight = false;

            while (
                (hasGroundLeft =
                     AocHashsetPointContains(&solid, belowLeft, NULL)) &&
                !(hasWallLeft = AocHashsetPointContains(&solid, left, NULL))) {
              left.x--;
              belowLeft.x--;
            }

            // search for right wall or no ground  |____|  or _____
            while ((hasGroundRight =
                        AocHashsetPointContains(&solid, belowRight, NULL)) &&
                   !(hasWallRight =
                         AocHashsetPointContains(&solid, right, NULL))) {
              right.x++;
              belowRight.x++;
            }

            u16 fromX = left.x + hasWallLeft;
            u16 toX = right.x - hasWallRight;
            for (u16 x = fromX; x < newPos.x; ++x) {
              AocHashsetPointInsert(&water, (point){x, newPos.y});
              printf("%3u %3u\n", x, newPos.y);
            }
            for (u16 x = newPos.x + 1; x < toX + 1; ++x) {
              AocHashsetPointInsert(&water, (point){x, newPos.y});
              printf("%3u %3u\n", x, newPos.y);
            }

            if (hasWallLeft && hasWallRight) {
              // create physical layer
              for (u16 x = fromX; x < toX + 1; ++x)
                AocHashsetPointInsert(&solid, (point){x, newPos.y});
            }
          } while (hasWallLeft && hasWallRight);

          if (!hasGroundLeft)
            AocArrayPointPush(&current, left);
          if (!hasGroundRight)
            AocArrayPointPush(&current, right);

        } else {
          // one drop
          AocHashsetPointInsertPreHashed(&water, newPos, hash);
          AocArrayPointPush(&current, newPos);
          printf("%3u %3u\n", newPos.x, newPos.y);
        }
      }
    }

    const size_t newLength = current.length - length;
    for (size_t j = 0; j < newLength; ++j)
      current.items[j] = current.items[length + j];
    current.length = newLength;
  }

  AocHashsetPointDestroy(&water);
  AocHashsetPointDestroy(&solid);
  AocArrayPointDestroy(&current);
  return (u32)water.count;
}

int main(void) {
  context ctx = {.minX = UINT16_MAX, .minY = UINT16_MAX};
  AocArrayClayCreate(&ctx.clay, 1600);

  AocReadFileLineByLine("day17/input.txt", parse, &ctx);

  // printf("%u..%u %u..%u\n", ctx.minX, ctx.maxX, ctx.minY, ctx.maxY);
  // for (size_t i = 0; i < ctx.clay.length; ++i) {
  //   printf(" x:%u..%u y:%u..%u\n", ctx.clay.items[i].fromX,
  //          ctx.clay.items[i].toX, ctx.clay.items[i].fromY,
  //          ctx.clay.items[i].toY);
  // }

  const u32 part1 = solve_part1(&ctx);
  printf("%u\n", part1);

  AocArrayClayDestroy(&ctx.clay);
}
