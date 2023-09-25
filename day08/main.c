#include <stdio.h>
#include <stdlib.h>

#include <aoc/aoc.h>
#include <aoc/bump.h>

typedef struct node {
  int *metadata;
  int childCount;
  int metadataCount;
  struct node *childs[];
} node;

static node *parse_node(char *str, char **out) {
  const int childCount = strtol(str, &str, 10);
  node *n = AocAlloc(sizeof(node) + childCount * sizeof(node *));
  n->childCount = childCount;
  n->metadataCount = strtol(str + 1, &str, 10);
  n->metadata = AocAlloc(sizeof(int) * n->metadataCount);
  for (int i = 0; i < n->childCount; ++i)
    n->childs[i] = parse_node(str + 1, &str);
  for (int i = 0; i < n->metadataCount; ++i)
    n->metadata[i] = strtol(str + 1, &str, 10);
  *out = str;
  return n;
}

static inline node *parse_tree(char *str) {
  return parse_node(str, &str);
}

static int solve_part1(const node *const n) {
  int sum = 0;
  for (int i = 0; i < n->childCount; ++i)
    sum += solve_part1(n->childs[i]);
  for (int i = 0; i < n->metadataCount; ++i)
    sum += n->metadata[i];
  return sum;
}

static int solve_part2(const node *const n) {
  int value = 0;
  if (n->childCount > 0) {
    for (int i = 0; i < n->metadataCount; ++i) {
      const int index = n->metadata[i] - 1;
      if (index < n->childCount)
        value += solve_part2(n->childs[index]);
    }
  } else {
    for (int i = 0; i < n->metadataCount; ++i)
      value += n->metadata[i];
  }
  return value;
}

int main(void) {
  aoc_bump bump = {0};
  AocBumpInit(&bump, 160000);
  aoc_allocator allocator = AocBumpCreateAllocator(&bump);
  AocMemSetAllocator(&allocator);

  char *contents = NULL;
  size_t length = 0;
  AocReadFileToString("day08/input.txt", &contents, &length);

  node *root = parse_tree(contents);

  const int part1 = solve_part1(root);
  const int part2 = solve_part2(root);

  printf("%d\n", part1);
  printf("%d\n", part2);

  AocBumpDestroy(&bump);
}
