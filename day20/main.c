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

static inline uint32_t tile_hash(const tile *const t) {
  return (54812489 * ((uint32_t)t->position.x ^ 95723417) *
          ((uint32_t)t->position.y ^ 69660419));
}

static inline bool tile_equals(const point *const a, const point *const b) {
  return a->x == b->x && a->y == b->y;
}

static const tile emptyTile = {.position = {INT32_MIN, INT32_MIN},
                               .type = TILE_TYPE_UNKNOWN};

#define AOC_T tile
#define AOC_T_NAME Tile
#define AOC_T_EMPTY emptyTile
#define AOC_T_HFUNC tile_hash
#define AOC_T_EQUALS tile_equals
#define AOC_BASE2_CAPACITY
#include <aoc/hashset.h>

typedef struct {
  AocHashsetTile *hs;
  size_t current;
} hs_iterator;

static bool hs_iterate(hs_iterator *const iterator, tile *const outTile) {
  tile current = emptyTile;
  do {
    current = iterator->hs->entries[iterator->current];
    iterator->current++;
  } while (iterator->current < iterator->hs->capacity &&
           tile_equals(&current, &emptyTile));
  if (iterator->current < iterator->hs->capacity) {
    *outTile = iterator->hs->entries[iterator->current];
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

int main(void) {
  AocBumpInit(&bump, 1 << 20);

  char *regex = NULL;
  size_t length = 0;
  AocReadFileToString("day20/input.txt", &regex, &length);
  AocTrimRight(regex, &length);

  token *root = parse_expression(regex + 1, NULL);
  // print_token(root, 0);

  AocFree(regex);
  AocBumpDestroy(&bump);
}
