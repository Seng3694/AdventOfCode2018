#include <aoc/aoc.h>
#include <stdio.h>

#define EMPTY_POT '.'
#define FULL_POT '#'

typedef struct {
  int64_t id;
  bool full;
} pot;

#define AOC_T pot
#define AOC_T_NAME Pot
#define AOC_BASE2_CAPACITY
#include <aoc/deque.h>

typedef struct {
  char pattern[5];
  char result;
} rule;

typedef struct {
  AocDequePot pots;
  AocDequePot buffer;
  uint8_t ruleCount;
  rule rules[32];
  int64_t minId;
  int64_t maxId;
} context;

static void parse_initial_state(char *line, size_t length, context *const ctx) {
  line += 15;
  AocDequePotCreate(&ctx->pots, 128);
  ctx->minId = 0;
  ctx->maxId = 0;
  for (size_t i = 0; i < length - 16; ++i) {
    const pot p = {.full = line[i] == FULL_POT, .id = ctx->maxId++};
    AocDequePotPushBack(&ctx->pots, p);
  }
}

static void parse_rule(char *line, context *const ctx) {
  rule *r = &ctx->rules[ctx->ruleCount++];
  AocMemCopy(r->pattern, line, sizeof(char) * 5);
  r->result = line[9];
}

static void parse_line(char *line, size_t length, void *userData,
                       const size_t lineNumber) {
  switch (lineNumber) {
  case 0:
    parse_initial_state(line, length, userData);
    break;
  case 1:
    break;
  default:
    parse_rule(line, userData);
    break;
  }
}

static inline pot *get_item_at(const AocDequePot *const d, const int i) {
  return &d->items[(d->head + i) & (d->capacity - 1)];
}

static void tick(context *const ctx) {
  // make sure it starts with at least 3 empty pots
  for (int i = 0; i < 3; ++i) {
    pot *p = get_item_at(&ctx->pots, i);
    if (p->full) {
      for (int j = i; j < 3; ++j) {
        AocDequePotPushFront(&ctx->pots,
                             (pot){.full = false, .id = --ctx->minId});
      }
      break;
    }
  }
  // make sure it ends with at least 3 empty pots
  for (int i = 0; i < 3; ++i) {
    pot *p = get_item_at(&ctx->pots, ctx->pots.length - i - 1);
    if (p->full) {
      for (int j = i; j < 3; ++j) {
        AocDequePotPushBack(&ctx->pots,
                            (pot){.full = false, .id = ctx->maxId++});
      }
      break;
    }
  }

  // copy pots to buffer
  AocDequePotEnsureCapacity(&ctx->buffer, ctx->pots.capacity);
  AocDequePotCopy(&ctx->buffer, &ctx->pots);

  char frame[5] = {0};

  for (int i = 2; i < (int)ctx->pots.length - 2; ++i) {
    // fill frame to compare to
    for (int f = 0; f < 5; ++f) {
      frame[f] =
          get_item_at(&ctx->pots, i - 2 + f)->full ? FULL_POT : EMPTY_POT;
    }

    // search for matching rules
    for (uint8_t j = 0; j < ctx->ruleCount; ++j) {
      rule *r = &ctx->rules[j];
      bool match = true;
      for (uint8_t k = 0; k < 5; ++k) {
        if (r->pattern[k] != frame[k]) {
          match = false;
          break;
        }
      }
      if (match) {
        // apply rule in buffer
        get_item_at(&ctx->buffer, i)->full = r->result == FULL_POT;
        break;
      }
    }
  }

  // copy buffer to pots
  AocDequePotCopy(&ctx->pots, &ctx->buffer);
}

static inline int64_t calc_sum(const AocDequePot *const pots) {
  int64_t sum = 0;
  for (int64_t i = 0; i < (int64_t)pots->length; ++i) {
    const pot *const p = get_item_at(pots, i);
    sum += (p->full * p->id);
  }
  return sum;
}

static int64_t solve_part1(context *const ctx) {
  for (uint64_t i = 0; i < 20; ++i)
    tick(ctx);
  return calc_sum(&ctx->pots);
}

static int64_t solve_part2(context *const ctx, const int64_t ticks) {
  int64_t sumBefore = 0;
  int64_t sum = 0;
  int64_t sumDiffBefore = 0;
  int64_t sumDiff = 0;
  int64_t i = 0;

  // start where part 1 left off at 20
  for (i = 20; i < ticks; ++i) {
    tick(ctx);
    sumBefore = sum;
    sum = calc_sum(&ctx->pots);
    sumDiffBefore = sumDiff;
    sumDiff = sum - sumBefore;
    if (sumDiff == sumDiffBefore)
      break;
  }

  return sum + ((ticks - i - 1) * sumDiff);
}

int main(void) {
  context ctx = {0};
  AocReadFileLineByLineEx("day12/input.txt", parse_line, &ctx);
  AocDequePotDuplicate(&ctx.buffer, &ctx.pots);

  int64_t part1 = solve_part1(&ctx);
  int64_t part2 = solve_part2(&ctx, 50000000000);

  printf("%ld\n", part1);
  printf("%ld\n", part2);

  AocDequePotDestroy(&ctx.pots);
  AocDequePotDestroy(&ctx.buffer);
}
