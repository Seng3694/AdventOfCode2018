#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include <aoc/aoc.h>
#include <aoc/arena.h>
#include <aoc/mem.h>
#include <aoc/image.h>

typedef struct {
  int32_t x;
  int32_t y;
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

static void parse_line(char *line, size_t length, void *userData) {
  (void)length;
  point p = {0};
  p.x = (int32_t)strtol(line, &line, 10);
  p.y = (int32_t)strtol(line + 2, NULL, 10);
  AocArrayPointPush(userData, p);
}

typedef struct {
  int32_t left;
  int32_t top;
  int32_t right;
  int32_t bottom;
} rectangle;

static rectangle find_bounds(const AocArrayPoint *const points) {
  rectangle bounds = {
      .left = INT32_MAX,
      .top = INT32_MAX,
      .right = INT32_MIN,
      .bottom = INT32_MIN,
  };

  for (size_t i = 0; i < points->length; ++i) {
    const point *const p = &points->items[i];
    if (p->x < bounds.left)
      bounds.left = p->x;
    if (p->x > bounds.right)
      bounds.right = p->x;
    if (p->y < bounds.top)
      bounds.top = p->y;
    if (p->y > bounds.bottom)
      bounds.bottom = p->y;
  }

  return bounds;
}

static inline int32_t get_abs(const int32_t n) {
  const int32_t mask = n >> (sizeof(int32_t) * CHAR_BIT - 1);
  return (n + mask) ^ mask;
}

static inline uint32_t manhattan_distance(const point from, const point to) {
  return (uint32_t)(get_abs(from.x - to.x) + get_abs(from.y - to.y));
}

typedef enum {
  RECT_ITER_STATE_TOP,
  RECT_ITER_STATE_RIGHT,
  RECT_ITER_STATE_BOTTOM,
  RECT_ITER_STATE_LEFT,
  RECT_ITER_STATE_DONE,
} rect_iter_state;

typedef struct {
  rect_iter_state state;
  rectangle rect;
  uint32_t i;
} rect_iter;

static bool iterate_rectangle(rect_iter *const iter, point *const p) {
  switch (iter->state) {
  case RECT_ITER_STATE_TOP:
    p->x = iter->rect.left + iter->i;
    p->y = iter->rect.top;
    if (p->x == iter->rect.right) {
      iter->state++;
      iter->i = 0;
    }
    break;
  case RECT_ITER_STATE_RIGHT:
    p->x = iter->rect.right;
    p->y = iter->rect.top + iter->i + 1;
    if (p->y == iter->rect.bottom - 1) {
      iter->state++;
      iter->i = 0;
    }
    break;
  case RECT_ITER_STATE_BOTTOM:
    p->x = iter->rect.left + iter->i;
    p->y = iter->rect.bottom;
    if (p->x == iter->rect.right) {
      iter->state++;
      iter->i = 0;
    }
    break;
  case RECT_ITER_STATE_LEFT:
    p->x = iter->rect.left;
    p->y = iter->rect.top + iter->i + 1;
    if (p->y == iter->rect.bottom - 1) {
      iter->state++;
      iter->i = 0;
    }
    break;
  default:
    return false;
  }
  iter->i++;
  return iter->state != RECT_ITER_STATE_DONE;
}

static AocArrayPoint get_finite_points(const AocArrayPoint *const points,
                                       const rectangle bounds) {
  AocHashsetPoint pointsToRemove = {0};
  AocHashsetPointCreate(&pointsToRemove, 64);

  for (size_t i = 0; i < points->length; ++i) {
    rect_iter iter = {.rect = bounds};
    point p = {0};
    while (iterate_rectangle(&iter, &p)) {
      uint32_t dist1 = manhattan_distance(points->items[i], p);
      for (size_t j = 0; j < points->length; ++j) {
        if (i == j)
          continue;
        uint32_t dist2 = manhattan_distance(points->items[j], p);
        if (dist2 <= dist1)
          goto next;
      }
      uint32_t hash = 0;
      if (!AocHashsetPointContains(&pointsToRemove, points->items[i], &hash))
        AocHashsetPointInsertPreHashed(&pointsToRemove, points->items[i], hash);
    next:;
    }
  }

  AocArrayPoint finitePoints = {0};
  AocArrayPointCreate(&finitePoints, points->length - pointsToRemove.count);

  for (size_t i = 0; i < points->length; ++i) {
    if (AocHashsetPointContains(&pointsToRemove, points->items[i], NULL))
      goto next2;
    AocArrayPointPush(&finitePoints, points->items[i]);
  next2:;
  }

  return finitePoints;
}

static void get_new_points(const point center, const point current,
                           point newPoints[const 4],
                           uint8_t *const newPointCount) {
  // if center is current, then all 4 cardinal directions
  if (center.x == current.x && center.y == current.y) {
    newPoints[0] = (point){current.x - 1, current.y};
    newPoints[1] = (point){current.x + 1, current.y};
    newPoints[2] = (point){current.x, current.y - 1};
    newPoints[3] = (point){current.x, current.y + 1};
    *newPointCount = 4;
  } else {
    //
    //   N
    // W C E
    //   S
    //
    // for N or any point on the same x coordinate as center and above
    // center.y expand like this
    //
    //   1
    //   N 2
    // W C E
    //   S
    //
    // so top and right. for E it's right and down. for S it's down and left.
    // for W it's left and up for diagonals it's only one direction
    //
    //     n
    //     N n
    //   c c D d
    // c c C c W
    //   c c c
    //     c

    if (center.x == current.x) {
      // N case -> 2 points
      if (current.y < center.y) {
        newPoints[0] = (point){current.x, current.y - 1}; // top
        newPoints[1] = (point){current.x + 1, current.y}; // right
      } else /* S case -> 2 points */ {
        newPoints[0] = (point){current.x, current.y + 1}; // bottom
        newPoints[1] = (point){current.x - 1, current.y}; // left
      }
      *newPointCount = 2;
    } else if (center.y == current.y) {
      // W case -> 2 points
      if (current.x < center.x) {
        newPoints[0] = (point){current.x - 1, current.y}; // left
        newPoints[1] = (point){current.x, current.y - 1}; // top
      } else /* E case -> 2 points */ {
        newPoints[0] = (point){current.x + 1, current.y}; // right
        newPoints[1] = (point){current.x, current.y + 1}; // bottom
      }
      *newPointCount = 2;
    } else /* diagonals */ {
      if (current.x > center.x && current.y < center.y) /* tr */ {
        newPoints[0] = (point){current.x + 1, current.y}; // right
      } else if (current.x > center.x && current.y > center.y) /* br*/ {
        newPoints[0] = (point){current.x, current.y + 1}; // down
      } else if (current.x < center.x && current.y > center.y) /* bl */ {
        newPoints[0] = (point){current.x - 1, current.y}; // left
      } else /* tl */ {
        newPoints[0] = (point){current.x, current.y - 1}; // up
      }
      *newPointCount = 1;
    }
  }
}

static uint32_t calculate_area(const AocArrayPoint *const points,
                               AocArrayPoint *const surroundingPoints,
                               const point center) {
  uint32_t area = 1;
  AocArrayPointClear(surroundingPoints);
  AocArrayPointPush(surroundingPoints, center);

  point newPoints[4] = {0};
  uint8_t newPointCount = 0;

  while (surroundingPoints->length > 0) {
    const size_t length = surroundingPoints->length;
    for (size_t i = 0; i < length; ++i) {
      const point p = surroundingPoints->items[i];
      get_new_points(center, p, newPoints, &newPointCount);

      for (size_t j = 0; j < newPointCount; ++j) {
        uint32_t distance = manhattan_distance(center, newPoints[j]);
        for (size_t k = 0; k < points->length; ++k) {
          if (points->items[k].x == center.x && points->items[k].y == center.y)
            continue;
          uint32_t dist = manhattan_distance(points->items[k], newPoints[j]);
          if (dist <= distance)
            goto not_closest;
        }
        area++;
        AocArrayPointPush(surroundingPoints, newPoints[j]);
      not_closest:;
      }
    }

    const size_t newLength = surroundingPoints->length - length;
    for (size_t j = 0; j < newLength; ++j)
      surroundingPoints->items[j] = surroundingPoints->items[length + j];

    surroundingPoints->length = newLength;
  }

  return area;
}

static inline uint32_t min(const uint32_t a, const uint32_t b) {
  return a < b ? a : b;
}

static uint32_t solve_part1(const AocArrayPoint *const points) {
  uint32_t largestArea = 0;
  const rectangle bounds = find_bounds(points);
  AocArrayPoint finitePoints = get_finite_points(points, bounds);

  AocArrayPoint surroundingPoints = {0};
  AocArrayPointCreate(&surroundingPoints, 1 << 10);
  for (size_t i = 0; i < finitePoints.length; ++i) {
    const point p = finitePoints.items[i];

    const uint32_t area = calculate_area(points, &surroundingPoints, p);
    if (area > largestArea)
      largestArea = area;
  }

  return largestArea;
}

static point find_center(const AocArrayPoint *const points) {
  int64_t sumX = 0;
  int64_t sumY = 0;
  for (size_t i = 0; i < points->length; ++i) {
    sumX += points->items[i].x;
    sumY += points->items[i].y;
  }
  return (point){
      sumX / points->length,
      sumY / points->length,
  };
}

static inline uint64_t sum_of_distances(const AocArrayPoint *const points,
                                        const point p) {
  uint64_t sum = 0;
  for (size_t i = 0; i < points->length; ++i)
    sum += manhattan_distance(p, points->items[i]);
  return sum;
}

typedef enum {
  DIAMOND_ITER_STATE_START,
  DIAMOND_ITER_STATE_T_R,
  DIAMOND_ITER_STATE_R_B,
  DIAMOND_ITER_STATE_B_L,
  DIAMOND_ITER_STATE_L_T,
} diamond_iter_state;

typedef struct {
  diamond_iter_state state;
  point center;
  point current;
  int32_t layer;
} diamond_iter;

static bool iterate_diamond(diamond_iter *const iter, point *const p) {
  bool isOnTheSameLayer = true;
  switch (iter->state) {
  case DIAMOND_ITER_STATE_START:
    iter->current.y--;
    iter->state = DIAMOND_ITER_STATE_T_R;
    iter->layer++;
    break;
  case DIAMOND_ITER_STATE_T_R:
    iter->current.x++;
    iter->current.y++;
    if (iter->current.x == iter->layer)
      iter->state = DIAMOND_ITER_STATE_R_B;
    break;
  case DIAMOND_ITER_STATE_R_B:
    iter->current.x--;
    iter->current.y++;
    if (iter->current.x == 0)
      iter->state = DIAMOND_ITER_STATE_B_L;
    break;
  case DIAMOND_ITER_STATE_B_L:
    iter->current.x--;
    iter->current.y--;
    if (iter->current.x == -iter->layer)
      iter->state = DIAMOND_ITER_STATE_L_T;
    break;
  case DIAMOND_ITER_STATE_L_T:
    iter->current.x++;
    iter->current.y--;
    if (iter->current.x == 0) {
      iter->state = DIAMOND_ITER_STATE_T_R;
      iter->current.y--;
      iter->layer++;
      isOnTheSameLayer = false;
    }
    break;
  }
  p->x = iter->center.x + iter->current.x;
  p->y = iter->center.y + iter->current.y;
  return isOnTheSameLayer;
}

static uint32_t solve_part2(const AocArrayPoint *const points) {
  const point center = find_center(points);

  uint32_t area = 0;
  uint32_t previousArea = 0;

  diamond_iter iterator = {.center = center};
  point current = center;

  for (;;) {
    previousArea = area;

    // iterate one diamond layer
    do {
      const uint32_t sum = sum_of_distances(points, current);
      if (sum < 10000)
        area++;
    } while (iterate_diamond(&iterator, &current));

    if (previousArea == area)
      break;
  }

  return area;
}

int main(void) {
  aoc_arena arena = {0};
  AocArenaAlloc(&arena, 9360);
  AocArenaReset(&arena);

  aoc_allocator allocator = AocArenaCreateAllocator(&arena);
  AocMemSetAllocator(&allocator);

  AocArrayPoint points = {0};
  AocArrayPointCreate(&points, 50);

  AocReadFileLineByLine("day06/input.txt", parse_line, &points);

  const uint32_t part1 = solve_part1(&points);
  const uint32_t part2 = solve_part2(&points);

  printf("%u\n", part1);
  printf("%u\n", part2);

  AocArenaFree(&arena);
}
