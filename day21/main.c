#include <stdio.h>
#include <stdlib.h>
#include <aoc/aoc.h>

// clang-format off
typedef enum {
  op_addr, op_addi, op_mulr, op_muli, op_banr, op_bani, op_borr, op_bori, 
  op_setr, op_seti, op_gtir, op_gtri, op_gtrr, op_eqir, op_eqri, op_eqrr
} op_code;
// clang-format on

typedef struct {
  op_code code : 8;
  uint32_t a, b, c;
} instruction;

typedef struct {
  instruction instructions[64];
  uint32_t registers[6];
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
  uint32_t *r = p->registers;
  uint32_t *ip = &r[p->ipIndex];
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

static void print_registers(const uint32_t registers[const 6]) {
  for (uint8_t i = 0; i < 6; ++i)
    printf("[%u]: %-8u  ", i, registers[i]);
  printf("\n");
}

static int solve_part1(program *const p) {
  p->registers[0] = 0;
  for (;;) {
    step(p);
    // instruction 28 checks for equality between register 0 and
    // register 1. if it is equal then it exits the program. so the solution is
    // the value in register 1 after reachign instruction 28 for the first time
    // (least instructions)
    if (p->registers[p->ipIndex] == 28) {
      return p->registers[1];
    }
  }
  // should never reach
  return 0;
}

int main(void) {
  program p = {0};
  AocReadFileLineByLineEx("day21/input.txt", parse, &p);

  const uint32_t part1 = solve_part1(&p);
  printf("%u\n", part1);
}
