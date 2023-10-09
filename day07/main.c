#include <stdio.h>
#include <aoc/aoc.h>

typedef uint8_t u8;

typedef struct {
  u8 adjacent[26];
  u8 count;
  bool visited;
} node;

typedef struct {
  node nodes[26];
  u8 indegree[26];
  u8 vertexCount;
} graph;

typedef struct {
  char data[27];
  u8 length;
} result;

static void add_edge(graph *const g, const u8 v, const u8 w) {
  g->nodes[v].adjacent[g->nodes[v].count++] = w;
  g->indegree[w]++;
}

static inline void parse(char *line, size_t length, void *userData) {
  add_edge(userData, line[5] - 'A', line[36] - 'A');
}

static void topsort(graph *const g, result *const r) {
  if (r->length == g->vertexCount)
    return;

  for (u8 i = 0; i < g->vertexCount; ++i) {
    if (g->indegree[i] == 0 && !g->nodes[i].visited) {

      for (u8 j = 0; j < g->nodes[i].count; ++j)
        g->indegree[g->nodes[i].adjacent[j]]--;

      r->data[r->length++] = (i + 'A');
      g->nodes[i].visited = true;

      topsort(g, r);
    }
  }
}

int main(void) {
  graph g = {.vertexCount = 26};
  AocReadFileLineByLine("day07/input.txt", parse, &g);

  // part1
  result r = {0};
  topsort(&g, &r);
  printf("%s\n", r.data);
}
