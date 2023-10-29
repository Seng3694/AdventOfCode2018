#include <stdio.h>
#include <stdlib.h>
#include <aoc/aoc.h>

typedef enum {
  DAMAGE_TYPE_NONE = 0,
  DAMAGE_TYPE_BLUDGEONING = 1 << 0,
  DAMAGE_TYPE_COLD = 1 << 1,
  DAMAGE_TYPE_FIRE = 1 << 2,
  DAMAGE_TYPE_RADIATION = 1 << 3,
  DAMAGE_TYPE_SLASHING = 1 << 4,
} damage_types;

static inline damage_types char_to_damage_type(const char c) {
  // clang-format off
  switch(c) {
  case 'b': return DAMAGE_TYPE_BLUDGEONING;
  case 'c': return DAMAGE_TYPE_COLD;
  case 'f': return DAMAGE_TYPE_FIRE;
  case 'r': return DAMAGE_TYPE_RADIATION;
  case 's': return DAMAGE_TYPE_SLASHING;
  default:  return DAMAGE_TYPE_NONE;
  }
  // clang-format on
}

static inline size_t char_to_damage_type_length(const char c) {
  // clang-format off
  switch(c) {
  case 'b': return 11;
  case 'c': return 4;
  case 'f': return 4;
  case 'r': return 9;
  case 's': return 8;
  default:  return 0;
  }
  // clang-format on
}

typedef struct group {
  int units;
  int hp;
  int ap;
  int init;
  damage_types weaknesses;
  damage_types immunities;
  damage_types attackType;
  struct group *target;
  bool isTargeted;
} group;

#define AOC_T group
#define AOC_T_NAME Group
#include <aoc/array.h>

typedef struct {
  AocArrayGroup immuneSystem;
  AocArrayGroup infection;
} context;

static damage_types parse_damage_types(char *input, char **out) {
  damage_types types = DAMAGE_TYPE_NONE;
  input -= 2;
  do {
    input += 2; // ", "
    types |= char_to_damage_type(*input);
    input += char_to_damage_type_length(*input);
  } while (*input == ',');
  *out = input;
  return types;
}

static group parse_group(char *input, char **out) {
  group g = {0};
  g.units = strtol(input, &input, 10);
  g.hp = strtol(input + 17, &input, 10);
  input += 12;
  if (*input == '(') {
    if (input[1] == 'w') /* weak */ {
      input += 9;
      g.weaknesses = parse_damage_types(input, &input);
      if (*input == ';') {
        input += 12;
        g.immunities = parse_damage_types(input, &input);
      }
    } else /* immune */ {
      input += 11;
      g.immunities = parse_damage_types(input, &input);
      if (*input == ';') {
        input += 10;
        g.weaknesses = parse_damage_types(input, &input);
      }
    }
    input += 2;
  }
  input += 25; // "with an attack that does "
  g.ap = strtol(input, &input, 10);
  input += 1;
  g.attackType = char_to_damage_type(*input);
  input += char_to_damage_type_length(*input) + 22;
  g.init = strtol(input, &input, 10);
  *out = input + 1; // \n
  return g;
}

static context parse(char *input) {
  context ctx = {0};
  AocArrayGroupCreate(&ctx.immuneSystem, 32);
  AocArrayGroupCreate(&ctx.infection, 32);

  input += 15;
  while (*input != '\n')
    AocArrayGroupPush(&ctx.immuneSystem, parse_group(input, &input));

  input += 12;
  while (*input != '\0')
    AocArrayGroupPush(&ctx.infection, parse_group(input, &input));

  return ctx;
}

static inline int64_t effective_power(const group *const g) {
  return (int64_t)g->units * (int64_t)g->ap;
}

static inline int compare_group_by_effective_power(const void *const a,
                                                   const void *const b) {
  const int diff = effective_power(b) - effective_power(a);
  return diff == 0 ? ((group *)b)->init - ((group *)a)->init : diff;
}

static inline int compare_group_by_init(const void *const a,
                                        const void *const b) {
  return ((group *)b)->init - ((group *)a)->init;
}

static inline int compare_group_ptr_by_init(const void *const a,
                                            const void *const b) {
  return compare_group_by_init(*(group **)a, *(group **)b);
}

#define HAS_FLAG(flags, flag) (((flags) & (flag)) == (flag))

static inline int64_t calc_damage(const group *const atk,
                                  const group *const def) {
  int multiplier = 1;
  if (HAS_FLAG(def->weaknesses, atk->attackType))
    multiplier = 2;
  else if (HAS_FLAG(def->immunities, atk->attackType))
    multiplier = 0;

  return multiplier * effective_power(atk);
}

static void select_targets(AocArrayGroup *const a, AocArrayGroup *const b) {
  // this function assumes `a` and `b` to be sorted in desencding effective
  // power in case of ties it also assumes it's sorted by initiative in
  // descending order
  for (size_t i = 0; i < a->length; ++i) {
    a->items[i].target = NULL;
    // dead units can't select targets
    if (a->items[i].units <= 0)
      continue;
    int64_t highestDamage = 0;
    for (size_t j = 0; j < b->length; ++j) {
      // dead or already targeted units can't be targeted
      if (b->items[j].units <= 0 || b->items[j].isTargeted)
        continue;
      const int64_t damage = calc_damage(&a->items[i], &b->items[j]);
      if (damage > highestDamage) {
        highestDamage = damage;
        // if another target was selected before, deselect it
        if (a->items[i].target != NULL)
          a->items[i].target->isTargeted = false;
        a->items[i].target = &b->items[j];
        b->items[j].isTargeted = true;
      }
    }
  }
}

static void simulate(context *const ctx, int *const immuneUnits,
                     int *const infectionUnits) {
  size_t count = ctx->immuneSystem.length + ctx->infection.length;
  group **groups = AocAlloc(sizeof(group *) * count);
  for (size_t i = 0; i < ctx->immuneSystem.length; ++i)
    groups[i] = &ctx->immuneSystem.items[i];
  for (size_t i = ctx->immuneSystem.length; i < count; ++i)
    groups[i] = &ctx->infection.items[i - ctx->immuneSystem.length];

  size_t immuneCount = ctx->immuneSystem.length;
  size_t infectionCount = ctx->infection.length;

  while (immuneCount > 0 && infectionCount > 0) {
    qsort(ctx->immuneSystem.items, ctx->immuneSystem.length, sizeof(group),
          compare_group_by_effective_power);
    qsort(ctx->infection.items, ctx->infection.length, sizeof(group),
          compare_group_by_effective_power);
    qsort(groups, count, sizeof(group *), compare_group_ptr_by_init);

    // selection phase
    select_targets(&ctx->immuneSystem, &ctx->infection);
    select_targets(&ctx->infection, &ctx->immuneSystem);

    // attack phase
    for (size_t i = 0; i < count; ++i) {
      if (groups[i]->units <= 0 || groups[i]->target == NULL)
        continue;
      groups[i]->target->units -=
          (calc_damage(groups[i], groups[i]->target) / groups[i]->target->hp);
    }

    immuneCount = 0;
    infectionCount = 0;
    for (size_t i = 0; i < ctx->immuneSystem.length; ++i) {
      ctx->immuneSystem.items[i].isTargeted = false;
      if (ctx->immuneSystem.items[i].units > 0)
        immuneCount++;
    }
    for (size_t i = 0; i < ctx->infection.length; ++i) {
      ctx->infection.items[i].isTargeted = false;
      if (ctx->infection.items[i].units > 0)
        infectionCount++;
    }
  }

  *immuneUnits = 0;
  *infectionUnits = 0;
  for (size_t i = 0; i < ctx->immuneSystem.length; ++i)
    if (ctx->immuneSystem.items[i].units > 0)
      (*immuneUnits) += ctx->immuneSystem.items[i].units;
  for (size_t i = 0; i < ctx->infection.length; ++i)
    if (ctx->infection.items[i].units > 0)
      (*infectionUnits) += ctx->infection.items[i].units;

  AocFree(groups);
}

static int solve_part1(context *const ctx) {
  int immuneUnits, infectionUnits;
  simulate(ctx, &immuneUnits, &infectionUnits);
  return immuneUnits + infectionUnits;
}

int main(void) {
  char *input = NULL;
  size_t length = 0;
  AocReadFileToString("day24/input.txt", &input, &length);
  context ctx = parse(input);
  AocFree(input);

  const int part1 = solve_part1(&ctx);
  printf("%d\n", part1);

  AocArrayGroupDestroy(&ctx.immuneSystem);
  AocArrayGroupDestroy(&ctx.infection);
}
