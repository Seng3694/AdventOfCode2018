#include <stdio.h>
#include <aoc/aoc.h>

typedef uint8_t u8;
typedef uint32_t u32;

#define VERTEX_COUNT 26
#define WORKER_COUNT 5
#define EXTRA_SECONDS 60

typedef struct {
  u8 adjacent[VERTEX_COUNT];
  u8 count;
  bool visited;
  bool assigned;
  u8 seconds;
} node;

typedef struct {
  node nodes[VERTEX_COUNT];
  u8 indegree[VERTEX_COUNT];
} graph;

typedef struct {
  node *n;
} worker;

typedef struct {
  char data[VERTEX_COUNT + 1];
  u8 length;
  worker workers[WORKER_COUNT];
  u32 seconds;
} result;

static void add_edge(graph *const g, const u8 v, const u8 w) {
  g->nodes[v].adjacent[g->nodes[v].count++] = w;
  g->nodes[v].seconds = EXTRA_SECONDS + v + 1;
  g->nodes[w].seconds = EXTRA_SECONDS + w + 1;
  g->indegree[w]++;
}

static inline void parse(char *line, size_t length, void *userData) {
  (void)length;
  add_edge(userData, line[5] - 'A', line[36] - 'A');
}

static void topsort(graph *const g, result *const r) {
  while (r->length != VERTEX_COUNT) {
    for (u8 i = 0; i < VERTEX_COUNT; ++i) {
      if (g->indegree[i] == 0 && !g->nodes[i].visited) {

        for (u8 j = 0; j < g->nodes[i].count; ++j)
          g->indegree[g->nodes[i].adjacent[j]]--;

        r->data[r->length++] = (i + 'A');
        g->nodes[i].visited = true;
        break;
      }
    }
  }
}

static void solve_part2(graph *const g, result *const r) {
  while (r->length != VERTEX_COUNT) {
    // assign workers
    bool workerAvailable = false;
    for (u8 i = 0; i < VERTEX_COUNT; ++i) {
      workerAvailable = false;
      if (g->indegree[i] == 0 && !g->nodes[i].assigned &&
          !g->nodes[i].visited) {
        for (u8 j = 0; j < WORKER_COUNT; ++j) {
          if (r->workers[j].n == NULL) {
            r->workers[j].n = &g->nodes[i];
            g->nodes[i].assigned = true;
            workerAvailable = true;
            break;
          }
        }
        if (!workerAvailable)
          break;
      }
    }

    // let workers work
    workerAvailable = false;
    while (!workerAvailable) {
      for (u8 i = 0; i < WORKER_COUNT; ++i) {
        if (r->workers[i].n == NULL)
          continue;
        if ((--r->workers[i].n->seconds) == 0) {
          r->workers[i].n->visited = true;
          const u8 nodeIndex = r->workers[i].n - g->nodes;
          for (u8 j = 0; j < g->nodes[nodeIndex].count; ++j)
            g->indegree[g->nodes[nodeIndex].adjacent[j]]--;
          r->data[r->length++] = nodeIndex + 'A';
          r->workers[i].n = NULL;
          workerAvailable = true;
        }
      }
      r->seconds++;
    }
  }
}

int main(void) {
  graph g1 = {0};
  AocReadFileLineByLine("day07/input.txt", parse, &g1);
  graph g2 = g1;

  result part1 = {0};
  topsort(&g1, &part1);

  result part2 = {0};
  solve_part2(&g2, &part2);

  printf("%s\n", part1.data);
  printf("%u\n", part2.seconds);
}
