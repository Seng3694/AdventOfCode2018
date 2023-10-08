#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <aoc/aoc.h>

typedef uint32_t u32;
typedef uint8_t u8;
typedef uint32_t registers[4];

#define OP_CODE_LIST(f)                                                        \
  f(addr), f(addi), f(mulr), f(muli), f(banr), f(bani), f(borr), f(bori),      \
      f(setr), f(seti), f(gtir), f(gtri), f(gtrr), f(eqir), f(eqri), f(eqrr)

static inline void addr(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] + r[b];
}
static inline void addi(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] + b;
}
static inline void mulr(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] * r[b];
}
static inline void muli(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] * b;
}
static inline void banr(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] & r[b];
}
static inline void bani(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] & b;
}
static inline void borr(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] | r[b];
}
static inline void bori(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] | b;
}
static inline void setr(registers r, const u8 a, const u8 b, const u8 c) {
  (void)b;
  r[c] = r[a];
}
static inline void seti(registers r, const u8 a, const u8 b, const u8 c) {
  (void)b;
  r[c] = a;
}
static inline void gtir(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = a > r[b];
}
static inline void gtri(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] > b;
}
static inline void gtrr(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] > r[b];
}
static inline void eqir(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = a == r[b];
}
static inline void eqri(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] == b;
}
static inline void eqrr(registers r, const u8 a, const u8 b, const u8 c) {
  r[c] = r[a] == r[b];
}

#define OP_CODE_ENUM(name) op_code_##name

typedef enum {
  OP_CODE_LIST(OP_CODE_ENUM),
} op_code;

typedef void (*op_code_func)(registers r, const u8, const u8, const u8);
#define OP_CODE_FUNC(name) [OP_CODE_ENUM(name)] = &name

static const op_code_func opCodeFuncs[] = {
    OP_CODE_LIST(OP_CODE_FUNC),
};

typedef struct __attribute__((packed)) {
  u8 code : 4;
  u8 a : 4;
  u8 b : 4;
  u8 c : 4;
} instruction;

typedef struct {
  registers before;
  registers after;
  instruction instr;
} sample;

#define AOC_T instruction
#define AOC_T_NAME Instr
#include <aoc/array.h>

#define AOC_T sample
#define AOC_T_NAME Sample
#include <aoc/array.h>

static void skip_whitespace(char *str, char **out) {
  for (;;) {
    switch (*str) {
    case '\r':
    case '\n':
    case '\t':
    case ' ':
      str++;
      break;
    default:
      *out = str;
      return;
    }
  }
}

static void parse_registers(char *str, char **out, registers r) {
  // "[0, 3, 0, 3]"
  r[0] = strtol(str + 1, &str, 10);
  r[1] = strtol(str + 2, &str, 10);
  r[2] = strtol(str + 2, &str, 10);
  r[3] = strtol(str + 2, &str, 10);
  *out = str + 1;
}

static instruction parse_instruction(char *str, char **out) {
  // "9 0 0 1"
  instruction instr = {0};
  instr.code = strtol(str, &str, 10);
  instr.a = strtol(str + 1, &str, 10);
  instr.b = strtol(str + 1, &str, 10);
  instr.c = strtol(str + 1, &str, 10);
  *out = str;
  return instr;
}

static sample parse_sample(char *str, char **out) {
  // "Before: [0, 3, 0, 3]
  // 9 0 0 1
  // After:  [0, 0, 0, 3]"
  sample s = {0};
  parse_registers(str + 8, &str, s.before);
  skip_whitespace(str, &str);
  s.instr = parse_instruction(str, &str);
  skip_whitespace(str, &str);
  parse_registers(str + 8, &str, s.after);
  skip_whitespace(str, &str);
  *out = str;
  return s;
}

typedef struct {
  AocArraySample samples;
  AocArrayInstr instructions;
} context;

static void parse(char *str, context *const ctx) {
  AocArrayInstrCreate(&ctx->instructions, 1200);
  AocArraySampleCreate(&ctx->samples, 1000);

  while (*str == 'B')
    AocArraySamplePush(&ctx->samples, parse_sample(str, &str));

  while (*str) {
    AocArrayInstrPush(&ctx->instructions, parse_instruction(str, &str));
    skip_whitespace(str, &str);
  }
}

static void check_sample(registers r, const sample *const s,
                         uint16_t *const possibleOpCodeBits) {
  uint16_t bits = 0;
  for (op_code c = 0; c < 16; ++c) {
    memcpy(r, s->before, sizeof(registers));
    opCodeFuncs[c](r, s->instr.a, s->instr.b, s->instr.c);
    if (memcmp(r, s->after, sizeof(registers)) == 0) {
      bits = AOC_SET_BIT(bits, c);
    }
  }
  *possibleOpCodeBits = bits;
}

static void resolve_opcode_numbers(uint16_t mappingBits[const 16],
                                   op_code mapping[const 16]) {
  bool allSingleBit = false;
  while (!allSingleBit) {
    allSingleBit = true;
    for (u8 i = 0; i < 16; ++i) {
      if (__builtin_popcount(mappingBits[i]) == 1) {
        for (u8 j = 0; j < 16; ++j) {
          if (j != i) {
            mappingBits[j] &= (~mappingBits[i]);
          }
        }
      } else {
        allSingleBit = false;
      }
    }
  }
  for (u8 i = 0; i < 16; ++i)
    mapping[i] = (32 - __builtin_clz(mappingBits[i])) - 1;
}

static void solve(const context *const ctx, u32 *const part1,
                  u32 *const part2) {
  uint16_t mappingBits[16] = {0};
  for (u8 i = 0; i < 16; ++i)
    mappingBits[i] = UINT16_MAX;

  u32 solution = 0;
  for (size_t i = 0; i < ctx->samples.length; ++i) {
    registers r = {0};
    uint16_t possibleOpCodes = 0;
    check_sample(r, &ctx->samples.items[i], &possibleOpCodes);
    const u32 count = __builtin_popcount(possibleOpCodes);
    if (count >= 3)
      solution++;
    mappingBits[ctx->samples.items[i].instr.code] &= possibleOpCodes;
  }

  op_code mapping[16] = {0};
  resolve_opcode_numbers(mappingBits, mapping);

  registers r = {0};
  for (size_t i = 0; i < ctx->instructions.length; ++i) {
    const instruction instr = ctx->instructions.items[i];
    opCodeFuncs[mapping[instr.code]](r, instr.a, instr.b, instr.c);
  }

  *part1 = solution;
  *part2 = r[0];
}

int main(void) {
  char *input = NULL;
  size_t length = 0;
  AocReadFileToString("day16/input.txt", &input, &length);

  context ctx = {0};
  parse(input, &ctx);
  AocFree(input);

  u32 part1 = 0;
  u32 part2 = 0;
  solve(&ctx, &part1, &part2);

  printf("%u\n", part1);
  printf("%u\n", part2);

  AocArraySampleDestroy(&ctx.samples);
  AocArrayInstrDestroy(&ctx.instructions);
}
