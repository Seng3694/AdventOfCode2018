#include <stdio.h>
#include <math.h>

#define AOC_SIZE_T int
#include <aoc/aoc.h>

#define AOC_T int
#define AOC_T_NAME Int
#include <aoc/array.h>

static int append_sum_digits(AocArrayInt *const recipes, const int sum) {
  AocArrayIntEnsureCapacity(recipes, recipes->length + 2);
  const int onesDigit = sum % 10;
  const int tensDigit = sum / 10;
  recipes->items[recipes->length] = tensDigit;
  recipes->items[recipes->length + tensDigit] = onesDigit;
  recipes->length += (1 + tensDigit);
  return 1 + tensDigit;
}

static void solve_part1(const int input) {
  AocArrayInt recipes = {0};
  AocArrayIntCreate(&recipes, input + 1000);
  AocArrayIntPush(&recipes, 3);
  AocArrayIntPush(&recipes, 7);

  int a = 0;
  int b = 1;

  while (recipes.length <= input + 10) {
    append_sum_digits(&recipes, recipes.items[a] + recipes.items[b]);
    a = (a + (recipes.items[a] + 1)) % recipes.length;
    b = (b + (recipes.items[b] + 1)) % recipes.length;
  }

  for (int i = 0; i < 10; ++i)
    putchar(recipes.items[input + i] + '0');
  printf("\n");

  AocArrayIntDestroy(&recipes);
}

int main(void) {
  const int input = 236021;
  solve_part1(input);
}
