#include <stdio.h>
#include <math.h>

#include <aoc/aoc.h>

#define AOC_T int8_t
#define AOC_T_NAME Int
#include <aoc/array.h>

static int8_t append_sum_digits(AocArrayInt *const recipes, const int8_t sum) {
  AocArrayIntEnsureCapacity(recipes, recipes->length + 2);
  const int8_t onesDigit = sum % 10;
  const int8_t tensDigit = sum / 10;
  recipes->items[recipes->length] = tensDigit;
  recipes->items[recipes->length + tensDigit] = onesDigit;
  recipes->length += (1 + tensDigit);
  return 1 + tensDigit;
}

static inline bool match(const int8_t *const recipes,
                         const int8_t *const digits, const int8_t digitCount) {
  for (int8_t i = 0; i < digitCount; ++i) {
    if (recipes[-i] != digits[i])
      return false;
  }
  return true;
}

static void solve(const size_t input, char part1[const 11],
                  int64_t *const part2) {
  AocArrayInt recipes = {0};
  AocArrayIntCreate(&recipes, input + 1000);
  AocArrayIntPush(&recipes, 3);
  AocArrayIntPush(&recipes, 7);

  int8_t digits[10];
  size_t digitCount = log10(input) + 1;
  // add digits in reverse order
  for (size_t i = 0, tmp = input; i < digitCount; ++i, tmp /= 10) {
    digits[i] = (int8_t)(tmp % 10);
  }

  int64_t a = 0;
  int64_t b = 1;

  while (recipes.length <= input + 10) {
    append_sum_digits(&recipes, recipes.items[a] + recipes.items[b]);
    a = (a + (recipes.items[a] + 1)) % recipes.length;
    b = (b + (recipes.items[b] + 1)) % recipes.length;
  }

  for (int8_t i = 0; i < 10; ++i)
    part1[i] = recipes.items[input + i] + '0';

  *part2 = -1;
  for (size_t i = digitCount; i < recipes.length; ++i) {
    if (recipes.items[i] == digits[0]) {
      if (match(&recipes.items[i], digits, digitCount)) {
        *part2 = i - digitCount + 1;
        break;
      }
    }
  }

  if (*part2 == -1) {
    for (;;) {
      int8_t newRecipes =
          append_sum_digits(&recipes, recipes.items[a] + recipes.items[b]);
      a = (a + (recipes.items[a] + 1)) % recipes.length;
      b = (b + (recipes.items[b] + 1)) % recipes.length;

      for (int8_t d = 0; d < newRecipes; ++d) {
        size_t index = recipes.length - 1 - d;
        if (recipes.items[index] == digits[0]) {
          if (match(&recipes.items[index], digits, digitCount)) {
            *part2 = index - digitCount + 1;
            goto done;
          }
        }
      }
    }
  }

done:

  AocArrayIntDestroy(&recipes);
}

int main(void) {
  const int64_t input = 236021;

  char part1[11] = {0};
  int64_t part2 = 0;
  solve(input, part1, &part2);

  printf("%s\n", part1);
  printf("%ld\n", part2);
}
