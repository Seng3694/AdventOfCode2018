#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aoc/aoc.h>

typedef struct {
  char data[32];
} box_id;

#define AOC_T box_id
#define AOC_T_NAME BoxId
#include <aoc/array.h>

static void parse_line(char *line, size_t length, void *userData) {
  box_id id = {0};
  AocTrimRight(line, &length);
  AocMemCopy(id.data, line, length);
  AocArrayBoxIdPush(userData, id);
}

static int32_t solve_part1(const AocArrayBoxId *const ids) {
  int32_t twos = 0;
  int32_t threes = 0;
  const size_t idLength = strlen(ids->items[0].data);

  for (size_t i = 0; i < ids->length; ++i) {
    // count letter occurences
    int8_t occurences[26] = {0};
    for (size_t j = 0; j < idLength; ++j)
      occurences[ids->items[i].data[j] - 'a']++;

    // set flags for each occurence. Bit 2 means there is a character with 2
    // occurences and so on. can't be more than 32 because ids are smaller
    uint32_t counts = 0;
    for (size_t j = 0; j < 26; ++j)
      counts = AOC_SET_BIT(counts, occurences[j]);

    twos += AOC_CHECK_BIT(counts, 2);
    threes += AOC_CHECK_BIT(counts, 3);
  }
  return twos * threes;
}

static void solve_part2(const AocArrayBoxId *const ids, char output[const 32]) {
  const size_t idLength = strlen(ids->items[0].data);
  for (size_t i = 0; i < ids->length - 1; ++i) {
    const box_id *const a = &ids->items[i];
    for (size_t j = i + 1; j < ids->length; ++j) {
      const box_id *const b = &ids->items[j];
      int8_t differences = 0;
      size_t lastDifference = 0;
      for (size_t k = 0; k < idLength; ++k) {
        if (a->data[k] != b->data[k]) {
          differences++;
          lastDifference = k;
        }
      }
      if (differences == 1) {
        AocMemCopy(output, a->data, lastDifference);
        AocMemCopy(output + lastDifference, a->data + lastDifference + 1,
                   idLength - lastDifference);
        return;
      }
    }
  }
}

int main(void) {
  AocArrayBoxId ids = {0};
  AocArrayBoxIdCreate(&ids, 250);

  AocReadFileLineByLine("day02/input.txt", parse_line, &ids);

  const int32_t part1 = solve_part1(&ids);
  char part2[32] = {0};
  solve_part2(&ids, part2);

  printf("%d\n", part1);
  printf("%s\n", part2);

  AocArrayBoxIdDestroy(&ids);
}
