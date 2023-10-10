#include <stdlib.h>
#include <stdio.h>
#include <aoc/aoc.h>

typedef uint16_t u16;
typedef uint32_t u32;
typedef int32_t i32;

typedef struct {
  u16 x;
  u16 y;
} point;

#define AOC_T point
#define AOC_T_NAME Point
#include <aoc/array.h>

typedef struct {
  u32 fromX;
  u32 toX;
  u32 fromY;
  u32 toY;
} clay;

#define AOC_T clay
#define AOC_T_NAME Clay
#include <aoc/array.h>

typedef struct {
  AocArrayClay clay;
  u32 minX;
  u32 minY;
  u32 maxX;
  u32 maxY;
} context;

typedef enum {
  TILE_TYPE_EMPTY,
  TILE_TYPE_SOLID,
  TILE_TYPE_FALLING_WATER,
  TILE_TYPE_SURFACE_WATER,
  TILE_TYPE_SETTLED_WATER,
} tile_type;

static const char *tileTypeCharMap[] = {
    [TILE_TYPE_EMPTY] = ".",         [TILE_TYPE_SOLID] = "█",
    [TILE_TYPE_FALLING_WATER] = "|", [TILE_TYPE_SURFACE_WATER] = "~",
    [TILE_TYPE_SETTLED_WATER] = "█",
};

static const char *tileTypeColorMap[] = {
    [TILE_TYPE_EMPTY] = "\e[0;30m",
    [TILE_TYPE_SOLID] = "\e[0;30m",
    [TILE_TYPE_FALLING_WATER] = "\e[0;36m",
    [TILE_TYPE_SURFACE_WATER] = "\e[0;36m",
    [TILE_TYPE_SETTLED_WATER] = "\e[0;34m",
};

typedef struct {
  u32 width;
  u32 height;
  u32 offsetX;
  u32 offsetY;
  tile_type tiles[];
} map;

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

static map *create_map(const context *const ctx) {
  // expand width on the sides by 1 for water on the edges
  const u32 w = ((ctx->maxX + 1) - ctx->minX) + 2;
  const u32 h = ctx->maxY + 1;
  map *m = AocCalloc(1, sizeof(map) + (sizeof(tile_type) * w * h));
  m->width = w;
  m->height = h;
  // account for offset from extra width
  m->offsetX = ctx->minX + 1;
  m->offsetY = ctx->minY;
  for (size_t i = 0; i < ctx->clay.length; ++i) {
    const clay c = ctx->clay.items[i];
    for (u32 y = c.fromY; y < c.toY + 1; ++y) {
      for (u32 x = c.fromX; x < c.toX + 1; ++x) {
        const u32 index = y * m->width + (x - m->offsetX + 2);
        m->tiles[index] = TILE_TYPE_SOLID;
      }
    }
  }
  return m;
}

static i32 clamp(const i32 value, const i32 min, const i32 max) {
  if (value < min)
    return min;
  else if (value > max)
    return max;
  return value;
}

#if 1
static void print_map(const map *const m, const i32 fromY, const i32 toY) {
  for (i32 y = fromY; y < toY; ++y) {
    printf("%4d: ", y);
    for (i32 x = 0; x < m->width; ++x) {
      u32 i = y * m->width + x;
      printf("%s%s\e[0m", tileTypeColorMap[m->tiles[i]],
             tileTypeCharMap[m->tiles[i]]);
    }
    printf("\n");
  }
  printf("\n");
}
#else
static void print_map(const map *const m, const i32 fromY, const i32 toY) {
}
#endif

static inline bool can_be_settled_on(const tile_type t) {
  return t == TILE_TYPE_SOLID || t == TILE_TYPE_SETTLED_WATER;
}

static void fill_reservoir(map *const m, const point p,
                           AocArrayPoint *const newFalling) {
  // # # |      # # |      # # |      # #^^^^^|
  // # # |  #   # # |  #   # #~~~~#   # #~~~~#
  // #   |  #   #~~~~~~#   #~~~~~~#   #~~~~~~#
  // ########   ########   ########   ########
  tile_type *const t = m->tiles;

  u32 baseIndex = p.y * m->width + p.x;
  u32 left, right, belowLeft, belowRight;
  bool leftCanBeSettledOn, rightCanBeSettledOn;

  do {
    left = baseIndex - 1;
    right = baseIndex + 1;
    belowLeft = left + m->width;
    belowRight = right + m->width;
    leftCanBeSettledOn = false;
    rightCanBeSettledOn = false;

    // search for left boundary or empty space
    while ((leftCanBeSettledOn = can_be_settled_on(t[belowLeft])) &&
           t[left] != TILE_TYPE_SOLID) {
      left--;
      belowLeft--;
    }
    // now "left" is either here
    //  #L  |
    //  ########
    // or here
    //  L   |
    //   #######

    while ((rightCanBeSettledOn = can_be_settled_on(t[belowRight])) &&
           t[right] != TILE_TYPE_SOLID) {
      right++;
      belowRight++;
    }

    const u32 from = left + leftCanBeSettledOn;
    const u32 to = right - rightCanBeSettledOn;

    // cases
    // #F  T#   F    T#   #F    T  F      T
    // ######    ######   ######    ######
    // case 1: create settled water from F to T
    // case 2: create surface water from F+1 to T and falling at F
    // case 3: create surface water from F to T-1 and falling at F
    // case 4: create surface water from F+1 to T-1 and falling at F and T

    // other example for case 2
    // #F   T#
    // # #####

    const tile_type waterType = leftCanBeSettledOn && rightCanBeSettledOn
                                    ? TILE_TYPE_SETTLED_WATER
                                    : TILE_TYPE_SURFACE_WATER;

    for (u32 i = from; i < to + 1; ++i) {
      t[i] = waterType;
    }

    if (!leftCanBeSettledOn) {
      t[from] = TILE_TYPE_FALLING_WATER;
      AocArrayPointPush(newFalling, (point){from % m->width, from / m->width});
    }
    if (!rightCanBeSettledOn) {
      t[to] = TILE_TYPE_FALLING_WATER;
      AocArrayPointPush(newFalling, (point){to % m->width, to / m->width});
    }

    baseIndex -= m->width;
  } while (leftCanBeSettledOn && rightCanBeSettledOn);
}

static void solve(map *const m, u32 *const part1, u32 *const part2) {
  AocArrayPoint current = {0};
  AocArrayPointCreate(&current, 128);
  AocArrayPointPush(&current, (point){500 - m->offsetX + 2, 0});

  while (current.length > 0) {
    const size_t length = current.length;
    for (size_t i = 0; i < length; ++i) {
      point p = current.items[i];
      if (p.y < m->height - 1) {
        point newPos = {p.x, p.y + 1};
        u32 index = newPos.y * m->width + newPos.x;

        switch (m->tiles[index]) {
        case TILE_TYPE_EMPTY:
          m->tiles[index] = TILE_TYPE_FALLING_WATER;
          AocArrayPointPush(&current, newPos);
          break;
        case TILE_TYPE_SOLID:
        case TILE_TYPE_SETTLED_WATER:
          fill_reservoir(m, p, &current);
          break;
        default:
          // ignore
          break;
        }
      }
    }

    const size_t newLength = current.length - length;
    for (size_t j = 0; j < newLength; ++j)
      current.items[j] = current.items[length + j];
    current.length = newLength;
  }

  const u32 mapSize = m->width * m->height;
  for (u32 i = 0; i < mapSize; ++i)
    *part1 += (m->tiles[i] >= TILE_TYPE_FALLING_WATER);
  for (u32 i = 0; i < mapSize; ++i)
    *part2 += (m->tiles[i] == TILE_TYPE_SETTLED_WATER);

  print_map(m, 0, m->height);

  AocArrayPointDestroy(&current);
}

int main(void) {
  context ctx = {.minX = UINT16_MAX, .minY = UINT16_MAX};
  AocArrayClayCreate(&ctx.clay, 1600);
  AocReadFileLineByLine("day17/input.txt", parse, &ctx);
  map *m = create_map(&ctx);

  u32 part1 = 0;
  u32 part2 = 0;
  solve(m, &part1, &part2);
  // TODO: this is actually not the correct answer
  // it's off by a few for most inputs. 2 in my case
  printf("%u\n", part1);
  printf("%u\n", part2);

  AocFree(m);
  AocArrayClayDestroy(&ctx.clay);
}
