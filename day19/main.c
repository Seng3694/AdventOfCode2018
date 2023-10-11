#define USE_COMPUTED_GOTO

#include <stdio.h>
#include <stdlib.h>
#include <aoc/aoc.h>

typedef int32_t i32;
typedef uint8_t u8;
typedef i32 registers[6];

#define OP_CODE_LIST(f)                                                        \
  f(nop), f(addr), f(addi), f(mulr), f(muli), f(banr), f(bani), f(borr),       \
      f(bori), f(setr), f(seti), f(gtir), f(gtri), f(gtrr), f(eqir), f(eqri),  \
      f(eqrr)

#define OP_CODE_ENUM(name) op_##name

typedef enum {
  OP_CODE_LIST(OP_CODE_ENUM),
} op_code;

typedef struct __attribute__((packed)) {
  op_code code : 8;
  u8 a, b, c;
} instruction;

typedef struct {
  instruction instructions[64];
  registers registers;
  u8 instructionCount;
  i32 *ip;
} program;

static void parse(char *s, size_t length, void *userData,
                  const size_t lineNumber) {
  (void)length;
  program *const p = userData;
  if (lineNumber == 0) {
    p->ip = &p->registers[strtol(s + 4, NULL, 10)];
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

static i32 run(program *const p) {
  i32 *r = p->registers;
  register i32 *ip = p->ip;
  instruction *instructions = p->instructions;

#define INSTR instructions[*ip]

#ifdef USE_COMPUTED_GOTO

#define OP_CODE_LABEL(name) op_label_##name
  static const void *dispatchTable[] = {
      OP_CODE_LIST(&&OP_CODE_LABEL),
  };
#define LOOP
#define SWITCH(x)
#define DISPATCH() goto *dispatchTable[instructions[++(*ip)].code]

  (*ip)--;
  DISPATCH();

#else

#define OP_CODE_LABEL(name) case OP_CODE_ENUM(name)
#define LOOP for (;;)
#define SWITCH(x) switch (x)
#define DISPATCH()                                                             \
  (*ip)++;                                                                     \
  break

#endif

  LOOP {
    // clang-format off
    SWITCH(INSTR.code) {
    OP_CODE_LABEL(addr): r[INSTR.c] = r[INSTR.a] + r[INSTR.b]; DISPATCH(); 
    OP_CODE_LABEL(addi): r[INSTR.c] = r[INSTR.a] + INSTR.b;    DISPATCH();
    OP_CODE_LABEL(mulr): r[INSTR.c] = r[INSTR.a] * r[INSTR.b]; DISPATCH();
    OP_CODE_LABEL(muli): r[INSTR.c] = r[INSTR.a] * INSTR.b;    DISPATCH();
    OP_CODE_LABEL(banr): r[INSTR.c] = r[INSTR.a] & r[INSTR.b]; DISPATCH();
    OP_CODE_LABEL(bani): r[INSTR.c] = r[INSTR.a] & INSTR.b;    DISPATCH();
    OP_CODE_LABEL(borr): r[INSTR.c] = r[INSTR.a] | r[INSTR.b]; DISPATCH();
    OP_CODE_LABEL(bori): r[INSTR.c] = r[INSTR.a] | INSTR.b;    DISPATCH();
    OP_CODE_LABEL(setr): r[INSTR.c] = r[INSTR.a];              DISPATCH();
    OP_CODE_LABEL(seti): r[INSTR.c] = INSTR.a;                 DISPATCH();
    OP_CODE_LABEL(gtir): r[INSTR.c] = INSTR.a > r[INSTR.b];    DISPATCH();
    OP_CODE_LABEL(gtri): r[INSTR.c] = r[INSTR.a] > INSTR.b;    DISPATCH();
    OP_CODE_LABEL(gtrr): r[INSTR.c] = r[INSTR.a] > r[INSTR.b]; DISPATCH();
    OP_CODE_LABEL(eqir): r[INSTR.c] = INSTR.a == r[INSTR.b];   DISPATCH();
    OP_CODE_LABEL(eqri): r[INSTR.c] = r[INSTR.a] == INSTR.b;   DISPATCH();
    OP_CODE_LABEL(eqrr): r[INSTR.c] = r[INSTR.a] == r[INSTR.b];DISPATCH();
    OP_CODE_LABEL(nop):  goto done;
    }
    // clang-format on
  }
done:
  return r[0];
}

int main(void) {
  program p = {0};
  AocReadFileLineByLineEx("day19/input.txt", parse, &p);
  const i32 part1 = run(&p);

  printf("%d\n", part1);
}
