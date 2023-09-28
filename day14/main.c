#include <stdio.h>
#include <math.h>

#define AOC_SIZE_T int
#include <aoc/aoc.h>

#define AOC_T int
#define AOC_T_NAME Int
#include <aoc/array.h>

static void tick(AocArrayInt *const recipes, int *const a, int *const b) {
  int sum = recipes->items[*a] + recipes->items[*b];
  const int newRecipe2 = sum % 10;
  sum /= 10;
  if (sum > 0)
    AocArrayIntPush(recipes, sum % 10); // newRecipe1
  AocArrayIntPush(recipes, newRecipe2);
  *a = (*a + (recipes->items[*a] + 1)) % recipes->length;
  *b = (*b + (recipes->items[*b] + 1)) % recipes->length;
}

static void solve_part1(const int input) {
  AocArrayInt recipes = {0};
  AocArrayIntCreate(&recipes, 240000);
  AocArrayIntPush(&recipes, 3);
  AocArrayIntPush(&recipes, 7);

  int a = 0;
  int b = 1;
  while (recipes.length <= input + 10)
    tick(&recipes, &a, &b);

  for (int i = 0; i < 10; ++i)
    putchar(recipes.items[input + i] + '0');
  printf("\n");

  AocArrayIntDestroy(&recipes);
}

int main(void) {
  const int input = 236021;
  solve_part1(input);
}
