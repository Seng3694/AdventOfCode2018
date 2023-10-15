#include <stdio.h>
#include <aoc/aoc.h>
#include <aoc/mem.h>
#include <aoc/bump.h>

aoc_bump bump = {0};

typedef enum {
  TILE_TYPE_UNKNOWN,
  TILE_TYPE_EMPTY,
  TILE_TYPE_WALL,
  TILE_TYPE_DOOR,
} tile_type;

typedef struct {
  int x;
  int y;
} point;

typedef struct {
  tile_type type;
  point position;
} tile;

static inline uint32_t point_hash(const point *const p) {
  return (54812489 * ((uint32_t)p->x ^ 95723417) * ((uint32_t)p->y ^ 69660419));
}

static inline bool point_equals(const point *const a, const point *const b) {
  return a->x == b->x && a->y == b->y;
}

static const point emptyPoint = {INT32_MIN, INT32_MIN};

#define AOC_KEY_T point
#define AOC_KEY_T_NAME Point
#define AOC_KEY_T_HFUNC point_hash
#define AOC_KEY_T_EQUALS point_equals
#define AOC_KEY_T_EMPTY emptyPoint
#define AOC_VALUE_T tile
#define AOC_VALUE_T_NAME Tile
#define AOC_BASE2_CAPACITY
#include <aoc/hashmap.h>

typedef struct {
  AocHashmapPointTile *hm;
  size_t current;
} hm_iterator;

static bool hm_iterate(hm_iterator *const iterator, tile *const outTile) {
  point current = emptyPoint;
  do {
    current = iterator->hm->keys[iterator->current];
    iterator->current++;
  } while (iterator->current < iterator->hm->capacity &&
           point_equals(&current, &emptyPoint));
  if (iterator->current < iterator->hm->capacity) {
    *outTile = iterator->hm->values[iterator->current];
    return true;
  }
  return false;
}

typedef enum {
  TOKEN_TYPE_EMPTY,
  TOKEN_TYPE_DIRECTIONS,
  TOKEN_TYPE_GROUP,
} token_type;

typedef struct token {
  token_type type;
  struct token *lnext;
  struct token *gnext;
  struct token *parent;
  union {
    struct {
      const char *start;
      int length;
    } dir;
    struct {
      struct token *first;
    } group;
  } data;
} token;

static token *parse_expression(char *str, char **out);

static token *parse_directions(char *str, char **out) {
  token *t = AocBumpAlloc(&bump, sizeof(token));
  t->lnext = NULL;
  t->gnext = NULL;
  t->parent = NULL;
  t->type = TOKEN_TYPE_DIRECTIONS;
  t->data.dir.start = str;
  for (;;) {
    switch (*str) {
    case 'N':
    case 'E':
    case 'S':
    case 'W':
      str++;
      break;
    default:
      goto done;
    }
  }
done:
  t->data.dir.length = str - t->data.dir.start;
  *out = str;
  return t;
}

static token *parse_group(char *str, char **out) {
  AOC_ASSERT(*str == '(');
  str++; // '('

  token *grp = AocBumpAlloc(&bump, sizeof(token));
  grp->type = TOKEN_TYPE_GROUP;
  grp->lnext = NULL;
  grp->gnext = NULL;
  grp->parent = NULL;

  token *prev = parse_expression(str, &str);
  grp->data.group.first = prev;
  prev->parent = grp;

  token *current = NULL;
  while (*str == '|') {
    str++; // '|'
    current = parse_expression(str, &str);
    current->parent = grp;
    prev->gnext = current;
    prev = current;
  }

  AOC_ASSERT(*str == ')');
  str++; // ')'
  *out = str;

  return grp;
}

static token *parse_expression(char *str, char **out) {
  token *root = NULL;

  switch (*str) {
  case '(':
    root = parse_group(str, &str);
    break;
  case 'N':
  case 'E':
  case 'S':
  case 'W':
    root = parse_directions(str, &str);
    break;
  default: {
    if ((str[-1] == '|' && str[0] == ')') ||
        (str[-1] == '(' && str[0] == '|') ||
        (str[-1] == '|' && str[0] == '|')) {
      root = AocBumpAlloc(&bump, sizeof(token));
      root->lnext = NULL;
      root->gnext = NULL;
      root->parent = NULL;
      root->type = TOKEN_TYPE_EMPTY;
      goto done;
    }
    break;
  }
  }

  token *prev = root;
  token *current = NULL;
  while (*str != '$') {
    switch (*str) {
    case '(':
      current = parse_group(str, &str);
      break;
    case 'N':
    case 'E':
    case 'S':
    case 'W':
      current = parse_directions(str, &str);
      break;
    default: {
      if ((str[-1] == '|' && str[0] == ')') ||
          (str[-1] == '(' && str[0] == '|') ||
          (str[-1] == '|' && str[0] == '|')) {
        current = AocBumpAlloc(&bump, sizeof(token));
        current->lnext = NULL;
        current->gnext = NULL;
        current->parent = NULL;
        current->type = TOKEN_TYPE_EMPTY;
        prev->lnext = current;
      }
      goto done;
    }
    }
    prev->lnext = current;
    prev = current;
  }
done:
  if (out != NULL)
    *out = str;
  return root;
}

static void print_token(token *t, int depth) {
  for (int i = 0; i < depth; ++i)
    printf("  ");

  switch (t->type) {
  case TOKEN_TYPE_EMPTY:
    printf("EMPTY\n");
    break;
  case TOKEN_TYPE_GROUP:
    printf("GROUP\n");
    token *current = t->data.group.first;
    while (current) {
      print_token(current, depth + 1);
      current = current->gnext;
    }
    break;
  case TOKEN_TYPE_DIRECTIONS:
    printf("D: \"%.*s\"\n", t->data.dir.length, t->data.dir.start);
    break;
  }

  if (t->lnext != NULL)
    print_token(t->lnext, depth);
}

static const token *get_next(const token *t) {
  while (t->lnext != NULL) {
    t = t->parent;
    if (t == NULL)
      return NULL;
  }
  return t->lnext;
}

static void add_new_tiles(const tile *const newTiles, const uint8_t tileCount,
                          AocHashmapPointTile *const tiles) {
  for (uint8_t i = 0; i < tileCount; ++i) {
    uint32_t hash = 0;
    if (AocHashmapPointTileContains(&tiles, newTiles[i].position, &hash)) {
      tile t;
      AocHashmapPointTileGetPrehashed(&tiles, newTiles[i].position, hash, &t);
      if (t.type == TILE_TYPE_UNKNOWN &&
          newTiles[i].type != TILE_TYPE_UNKNOWN) {
        AocHashmapPointTileRemovePreHashed(&tiles, newTiles[i].position, hash);
        AocHashmapPointTileInsertPreHashed(&tiles, newTiles[i].position,
                                           newTiles[i], hash);
      }
    } else {
      AocHashmapPointTileInsertPreHashed(&tiles, newTiles[i].position,
                                         newTiles[i], hash);
    }
  }
}

static void move_up(point *const p, AocHashmapPointTile *const tiles) {
  //  #?#
  //  ?.?
  //   -
  p->y -= 2;
  const tile newTiles[] = {
      {.position = *p, .type = TILE_TYPE_EMPTY},
      {.position = (point){p->x - 1, p->y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 1, p->y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y - 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y + 1}, .type = TILE_TYPE_DOOR},
      {.position = (point){p->x + 1, p->y - 1}, .type = TILE_TYPE_WALL},
      {.position = (point){p->x - 1, p->y - 1}, .type = TILE_TYPE_WALL},
  };
  add_new_tiles(newTiles, sizeof(newTiles) / sizeof(tile), tiles);
}

static void move_down(point *const p, AocHashmapPointTile *const tiles) {
  //   -
  //  ?.?
  //  #?#
  p->y += 2;
  const tile newTiles[] = {
      {.position = *p, .type = TILE_TYPE_EMPTY},
      {.position = (point){p->x - 1, p->y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 1, p->y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y + 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y - 1}, .type = TILE_TYPE_DOOR},
      {.position = (point){p->x + 1, p->y + 1}, .type = TILE_TYPE_WALL},
      {.position = (point){p->x - 1, p->y + 1}, .type = TILE_TYPE_WALL},
  };
  add_new_tiles(newTiles, sizeof(newTiles) / sizeof(tile), tiles);
}

static void move_left(point *const p, AocHashmapPointTile *const tiles) {
  //  #?
  //  ?.|
  //  #?
  p->x -= 2;
  const tile newTiles[] = {
      {.position = *p, .type = TILE_TYPE_EMPTY},
      {.position = (point){p->x - 1, p->y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y - 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y + 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 1, p->y + 0}, .type = TILE_TYPE_DOOR},
      {.position = (point){p->x - 1, p->y + 1}, .type = TILE_TYPE_WALL},
      {.position = (point){p->x - 1, p->y - 1}, .type = TILE_TYPE_WALL},
  };
  add_new_tiles(newTiles, sizeof(newTiles) / sizeof(tile), tiles);
}

static void move_right(point *const p, AocHashmapPointTile *const tiles) {
  //   ?#
  //  |.?
  //   ?#
  p->x += 2;
  const tile newTiles[] = {
      {.position = *p, .type = TILE_TYPE_EMPTY},
      {.position = (point){p->x + 1, p->y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y - 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x + 0, p->y + 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p->x - 1, p->y + 0}, .type = TILE_TYPE_DOOR},
      {.position = (point){p->x + 1, p->y + 1}, .type = TILE_TYPE_WALL},
      {.position = (point){p->x + 1, p->y - 1}, .type = TILE_TYPE_WALL},
  };
  add_new_tiles(newTiles, sizeof(newTiles) / sizeof(tile), tiles);
}

static void initial_tiles(const point p, AocHashmapPointTile *const tiles) {
  //  #?#
  //  ?.?
  //  #?#
  const tile newTiles[] = {
      {.position = p, .type = TILE_TYPE_EMPTY},
      {.position = (point){p.x - 1, p.y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p.x + 1, p.y + 0}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p.x + 0, p.y - 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p.x + 0, p.y + 1}, .type = TILE_TYPE_UNKNOWN},
      {.position = (point){p.x + 1, p.y + 1}, .type = TILE_TYPE_WALL},
      {.position = (point){p.x + 1, p.y - 1}, .type = TILE_TYPE_WALL},
      {.position = (point){p.x - 1, p.y + 1}, .type = TILE_TYPE_WALL},
      {.position = (point){p.x - 1, p.y - 1}, .type = TILE_TYPE_WALL},
  };
  add_new_tiles(newTiles, sizeof(newTiles) / sizeof(tile), tiles);
}

static void solve(const token *t, int depth, point position,
                  AocHashmapPointTile *const tiles) {

  while (t != NULL) {
    while (t->type == TOKEN_TYPE_EMPTY) {
      t = get_next(t);
      if (t == NULL)
        return;
    }
    switch (t->type) {
    case TOKEN_TYPE_DIRECTIONS:
      for (int i = 0; i < t->data.dir.length; ++i) {
        // clang-format off
      switch(t->data.dir.start[i]){
        case 'N': move_up(&position, tiles);    break;
        case 'E': move_right(&position, tiles); break;
        case 'S': move_down(&position, tiles);  break;
        case 'W': move_left(&position, tiles);  break;
      }
        // clang-format on
      }
      t = get_next(t);
      break;
    case TOKEN_TYPE_GROUP:
      solve(t->data.group.first, depth + 1, position, tiles);
      t = get_next(t);
      break;
    }
  }
}

int main(void) {
  AocBumpInit(&bump, 1 << 20);

  char *regex = NULL;
  size_t length = 0;
  AocReadFileToString("day20/input.txt", &regex, &length);
  AocTrimRight(regex, &length);

  token *root = parse_expression(regex + 1, NULL);
  // print_token(root, 0);
  AocHashmapPointTile tiles = {0};
  AocHashmapPointTileCreate(&tiles, 1 << 14);
  initial_tiles((point){0, 0}, &root);

  solve(root, 0, (point){0, 0}, &tiles);

  AocFree(regex);
  AocHashmapPointTileDestroy(&tiles);
  AocBumpDestroy(&bump);
}
