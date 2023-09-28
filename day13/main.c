#include <stdio.h>
#include <stdlib.h>

#include <aoc/aoc.h>

typedef enum {
  TILE_TYPE_EMPTY,
  TILE_TYPE_HORIZONTAL,
  TILE_TYPE_VERTICAL,
  TILE_TYPE_CORNER_TR,
  TILE_TYPE_CORNER_BR,
  TILE_TYPE_CORNER_BL,
  TILE_TYPE_CORNER_TL,
  TILE_TYPE_INTERSECTION,
} tile_type;

#define AOC_T tile_type
#define AOC_T_NAME Tile
#include <aoc/array.h>

typedef struct {
  AocArrayTile tiles;
  int16_t width;
  int16_t height;
} map;

// don't change order
typedef enum {
  DIRECTION_RIGHT,
  DIRECTION_DOWN,
  DIRECTION_LEFT,
  DIRECTION_UP,
} direction;

static inline direction turn_right(const direction dir) {
  return (direction)(((int)dir + 1) % 4);
}

static inline direction turn_left(const direction dir) {
  return dir == DIRECTION_RIGHT ? DIRECTION_UP : (direction)((int)dir - 1);
}

// don't change order
typedef enum {
  CART_STATE_TURN_LEFT,
  CART_STATE_GO_STRAIGHT,
  CART_STATE_TURN_RIGHT,
} cart_state;

typedef struct {
  direction dir;
  cart_state state;
  int16_t x;
  int16_t y;
  int16_t prevX;
  int16_t prevY;
  int8_t id;
} cart;

#define AOC_T cart
#define AOC_T_NAME Cart
#include <aoc/array.h>

typedef struct {
  map map;
  AocArrayCart *carts;
} context;

static void parse_line(char *line, size_t length, void *userData) {
  context *const ctx = userData;
  ctx->map.width = --length;

  for (size_t i = 0; i < length; ++i) {
    tile_type tile = TILE_TYPE_EMPTY;
    switch (line[i]) {
    case '|':
      tile = TILE_TYPE_VERTICAL;
      break;
    case '-':
      tile = TILE_TYPE_HORIZONTAL;
      break;
    case '/': {
      if (i == 0 || ctx->map.height == 0) {
        tile = TILE_TYPE_CORNER_TL;
      } else if (i == (size_t)ctx->map.width - 1) {
        tile = TILE_TYPE_CORNER_BR;
      } else {
        tile_type left =
            ctx->map.tiles.items[ctx->map.height * ctx->map.width + (i - 1)];
        if (left == TILE_TYPE_HORIZONTAL || left == TILE_TYPE_INTERSECTION ||
            left == TILE_TYPE_CORNER_TL || left == TILE_TYPE_CORNER_BL)
          tile = TILE_TYPE_CORNER_BR;
        else
          tile = TILE_TYPE_CORNER_TL;
      }

      break;
    }
    case '\\': {
      if (i == 0) {
        tile = TILE_TYPE_CORNER_BL;
      } else if (i == (size_t)ctx->map.width - 1 ||
                 (size_t)ctx->map.height == 0) {
        tile = TILE_TYPE_CORNER_TR;
      } else {
        tile_type left =
            ctx->map.tiles.items[ctx->map.height * ctx->map.width + (i - 1)];
        if (left == TILE_TYPE_HORIZONTAL || left == TILE_TYPE_INTERSECTION ||
            left == TILE_TYPE_CORNER_TL || left == TILE_TYPE_CORNER_BL)
          tile = TILE_TYPE_CORNER_TR;
        else
          tile = TILE_TYPE_CORNER_BL;
      }
      break;
    }
    case '+':
      tile = TILE_TYPE_INTERSECTION;
      break;
    case '<': {
      tile = TILE_TYPE_HORIZONTAL;
      cart c = (cart){.dir = DIRECTION_LEFT,
                      .x = i,
                      .y = ctx->map.height,
                      .id = ctx->carts->length};
      AocArrayCartPush(ctx->carts, c);
      line[i] = '-';
      break;
    }
    case '>': {
      tile = TILE_TYPE_HORIZONTAL;
      cart c = (cart){.dir = DIRECTION_RIGHT,
                      .x = i,
                      .y = ctx->map.height,
                      .id = ctx->carts->length};
      AocArrayCartPush(ctx->carts, c);
      line[i] = '-';
      break;
    }
    case '^': {
      tile = TILE_TYPE_VERTICAL;
      cart c = (cart){.dir = DIRECTION_UP,
                      .x = i,
                      .y = ctx->map.height,
                      .id = ctx->carts->length};
      AocArrayCartPush(ctx->carts, c);
      line[i] = '|';
      break;
    }
    case 'v': {
      tile = TILE_TYPE_VERTICAL;
      cart c = (cart){.dir = DIRECTION_DOWN,
                      .x = i,
                      .y = ctx->map.height,
                      .id = ctx->carts->length};
      AocArrayCartPush(ctx->carts, c);
      line[i] = '|';
      break;
    }
    default:
      break;
    }
    AocArrayTilePush(&ctx->map.tiles, tile);
  }
  ctx->map.height++;
}

static void move_cart(cart *const c, const map *const m) {
  c->prevX = c->x;
  c->prevY = c->y;
  // clang-format off
  switch (c->dir) {
  case DIRECTION_RIGHT: c->x++; break;
  case DIRECTION_DOWN:  c->y++; break; 
  case DIRECTION_LEFT:  c->x--; break; 
  case DIRECTION_UP:    c->y--; break; 
  }
  // clang-format on

  // update direction and state
  const tile_type currentType = m->tiles.items[c->y * m->width + c->x];
  switch (currentType) {
  case TILE_TYPE_EMPTY:
    AOC_ASSERT(false);
    break;
  case TILE_TYPE_CORNER_TR: /* -\ */ {
    // either goes right and wants down or goes up and wants left
    c->dir = c->dir == DIRECTION_RIGHT ? DIRECTION_DOWN : DIRECTION_LEFT;
    break;
  }
  case TILE_TYPE_CORNER_BR: /* -/ */ {
    // either goes right and wants up or goes down and wants left
    c->dir = c->dir == DIRECTION_RIGHT ? DIRECTION_UP : DIRECTION_LEFT;
    break;
  }
  case TILE_TYPE_CORNER_BL: /* \- */ {
    // either goes left and wants up or goes down and wants right
    c->dir = c->dir == DIRECTION_LEFT ? DIRECTION_UP : DIRECTION_RIGHT;
    break;
  }
  case TILE_TYPE_CORNER_TL: /* /- */ {
    // either goes left and wants down or goes up and wants right
    c->dir = c->dir == DIRECTION_LEFT ? DIRECTION_DOWN : DIRECTION_RIGHT;
    break;
  }
  case TILE_TYPE_INTERSECTION: {
    switch (c->state) {
    case CART_STATE_TURN_LEFT:
      c->dir = turn_left(c->dir);
      break;
    case CART_STATE_TURN_RIGHT:
      c->dir = turn_right(c->dir);
      break;
    default:
      break;
    }
    c->state = (cart_state)(((int)c->state + 1) % 3);
    break;
  }

  default:
    break;
  }
}

static inline bool carts_collide(const cart *const a, const cart *const b) {
  return (a->x == b->x && a->y == b->y) ||
         (a->x == b->prevX && a->y == b->prevY);
}

static int compare_carts(const void *const c1, const void *const c2) {
  const cart *const cart1 = c1;
  const cart *const cart2 = c2;
  return ((cart1->y << 16) | cart1->x) - ((cart2->y << 16) | cart2->x);
}

static void solve_part1(context *const ctx, int16_t *const outX,
                        int16_t *const outY) {
  map *const m = &ctx->map;
  AocArrayCart *const carts = ctx->carts;

  for (;;) {
    qsort(carts->items, carts->length, sizeof(cart), compare_carts);

    for (size_t i = 0; i < carts->length; ++i)
      move_cart(&carts->items[i], m);

    for (size_t i = 0; i < carts->length - 1; ++i) {
      const cart *const a = &carts->items[i];
      for (size_t j = i + 1; j < carts->length; ++j) {
        const cart *const b = &carts->items[j];
        if (carts_collide(a, b)) {
          if (outX)
            *outX = a->x;
          if (outY)
            *outY = a->y;

          // remove carts
          carts->items[j] = carts->items[--carts->length];
          carts->items[i] = carts->items[--carts->length];
          return;
        }
      }
    }
  }
}

static void solve_part2(context *const ctx, int16_t *const outX,
                        int16_t *const outY) {
  while (ctx->carts->length != 1)
    solve_part1(ctx, NULL, NULL);
  cart const *c = AocArrayCartFirst(ctx->carts);
  *outX = c->x;
  *outY = c->y;
}

int main(void) {
  AocArrayCart carts = {0};
  AocArrayCartCreate(&carts, 32);
  context ctx = {.carts = &carts};
  AocArrayTileCreate(&ctx.map.tiles, 150 * 150);

  AocReadFileLineByLine("day13/input.txt", parse_line, &ctx);

  int16_t x1 = 0;
  int16_t y1 = 0;
  solve_part1(&ctx, &x1, &y1);

  int16_t x2 = 0;
  int16_t y2 = 0;
  solve_part2(&ctx, &x2, &y2);

  printf("%d,%d\n", x1, y1);
  printf("%d,%d\n", x2, y2);

  AocArrayTileDestroy(&ctx.map.tiles);
  AocArrayCartDestroy(&carts);
}
