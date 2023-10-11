#include <stdio.h>
#include <stdlib.h>
#include <aoc/aoc.h>

// clang-format off
typedef enum {
  op_addr, op_addi, op_mulr, op_muli, op_banr, op_bani, op_borr, op_bori, 
  op_setr, op_seti, op_gtir, op_gtri, op_gtrr, op_eqir, op_eqri, op_eqrr
} op_code;
// clang-format on

typedef struct __attribute__((packed)) {
  op_code code : 8;
  uint8_t a, b, c;
} instruction;

typedef struct {
  instruction instructions[64];
  int registers[6];
  uint8_t instructionCount;
  uint8_t ipIndex;
} program;

static void parse(char *s, size_t length, void *userData,
                  const size_t lineNumber) {
  (void)length;
  program *const p = userData;
  if (lineNumber == 0) {
    p->ipIndex = strtol(s + 4, NULL, 10);
    return;
  }
  instruction i = {0};
  // clang-format off
  switch (*s) {
  case 'a': i.code = op_addr + (s[3] == 'i'); break;
  case 'm': i.code = op_mulr + (s[3] == 'i'); break;
  case 'b': i.code = op_banr + ((s[1] == 'o') * 2) + (s[3] == 'i'); break;
  case 's': i.code = op_setr + (s[3] == 'i'); break;
  case 'g': i.code = op_gtir + (s[2] == 'r') + (s[2] == 'r' && s[3] == 'r'); break;
  case 'e': i.code = op_eqir + (s[2] == 'r') + (s[2] == 'r' && s[3] == 'r'); break;
  }
  // clang-format on
  i.a = strtol(s + 5, &s, 10);
  i.b = strtol(s + 1, &s, 10);
  i.c = strtol(s + 1, &s, 10);
  p->instructions[p->instructionCount++] = i;
}

static void step(program *const p) {
  int *r = p->registers;
  int *ip = &r[p->ipIndex];
  instruction instr = p->instructions[*ip];
  // clang-format off
  switch(instr.code) {
  case op_addr: r[instr.c] = r[instr.a] + r[instr.b];  break; 
  case op_addi: r[instr.c] = r[instr.a] + instr.b;     break;
  case op_mulr: r[instr.c] = r[instr.a] * r[instr.b];  break;
  case op_muli: r[instr.c] = r[instr.a] * instr.b;     break;
  case op_banr: r[instr.c] = r[instr.a] & r[instr.b];  break;
  case op_bani: r[instr.c] = r[instr.a] & instr.b;     break;
  case op_borr: r[instr.c] = r[instr.a] | r[instr.b];  break;
  case op_bori: r[instr.c] = r[instr.a] | instr.b;     break;
  case op_setr: r[instr.c] = r[instr.a];               break;
  case op_seti: r[instr.c] = instr.a;                  break;
  case op_gtir: r[instr.c] = instr.a > r[instr.b];     break;
  case op_gtri: r[instr.c] = r[instr.a] > instr.b;     break;
  case op_gtrr: r[instr.c] = r[instr.a] > r[instr.b];  break;
  case op_eqir: r[instr.c] = instr.a == r[instr.b];    break;
  case op_eqri: r[instr.c] = r[instr.a] == instr.b;    break;
  case op_eqrr: r[instr.c] = r[instr.a] == r[instr.b]; break;
  }
  // clang-format on
  (*ip)++;
}

static int solve(program *const p) {
  // arbitrary value. 20 seems to be enough
  for (int i = 0; i < 20; ++i)
    step(p);

  const int destination = p->registers[1];
  int solution = 0;
  int lastValue = 0;
  for (int i = 1;; ++i) {
    if (destination % i == 0) {
      solution += i;
      lastValue = i;
      if (lastValue == destination)
        break;
    }
  }
  return solution;
}

int main(void) {
  program p1 = {0};
  AocReadFileLineByLineEx("day19/input.txt", parse, &p1);
  program p2 = p1;
  p2.registers[0] = 1;

  const int part1 = solve(&p1);
  const int part2 = solve(&p2);
  printf("%d\n%d\n", part1, part2);
}
