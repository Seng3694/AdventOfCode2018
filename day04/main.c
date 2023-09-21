#include <stdlib.h>
#include <stdio.h>

#define AOC_USE_ARENA_DEFAULT
#include <aoc/aoc.h>
#include <aoc/arena.h>

typedef enum {
  RECORD_TYPE_BEGINS_SHIFT,
  RECORD_TYPE_FALLS_ASLEEP,
  RECORD_TYPE_WAKES_UP,
} record_type;

typedef struct __attribute((packed)) {
  uint16_t minute : 6;
  uint16_t hour : 5;
  uint16_t day : 5;
  uint16_t month : 4;
  uint16_t year : 12;
} datetime;

typedef union {
  struct __attribute__((packed)) {
    uint16_t guardId : 13;
    record_type type : 2;
    datetime time;
  } data;
  uint64_t all;
} record;

#define AOC_T record
#define AOC_T_NAME Record
#include <aoc/array.h>

static inline void parse_datetime(char *line, char **out, datetime *const dt) {
  // "[1518-11-22 00:00] "
  dt->year = strtoul(line + 1, &line, 10);
  dt->month = strtoul(line + 1, &line, 10);
  dt->day = strtoul(line + 1, &line, 10);
  dt->hour = strtoul(line + 1, &line, 10);
  dt->minute = strtoul(line + 1, &line, 10);
  *out = line + 2;
}

static void parse_line(char *line, size_t length, void *userData) {
  (void)length;
  record r = {0};
  parse_datetime(line, &line, &r.data.time);

  // "Guard #1231 begins shift"
  // "falls asleep"
  // "wakes up"
  switch (*line) {
  case 'G':
    r.data.type = RECORD_TYPE_BEGINS_SHIFT;
    r.data.guardId = strtoul(line + 7, NULL, 10);
    break;
  case 'f':
    r.data.type = RECORD_TYPE_FALLS_ASLEEP;
    break;
  case 'w':
    r.data.type = RECORD_TYPE_WAKES_UP;
    break;
  }

  AocArrayRecordPush(userData, r);
}

static inline int compare_datetime(const datetime *const a,
                                   const datetime *const b) {
  return *(const int32_t *const)a - *(const int32_t *const)b;
}

static inline int compare_records(const void *const a, const void *const b) {
  return compare_datetime(&((const record *const)a)->data.time,
                          &((const record *const)b)->data.time);
}

typedef struct {
  uint8_t from;
  uint8_t to;
} range;

#define AOC_T range
#define AOC_T_NAME Range
#include <aoc/array.h>

typedef struct {
  AocArrayRange sleepRanges;
} shift;

#define AOC_T shift
#define AOC_T_NAME Shift
#include <aoc/array.h>

typedef struct {
  uint16_t guardId;
  uint32_t totalSleepTime;
  AocArrayShift shifts;
} guard_schedule;

typedef guard_schedule *schedule_ptr;

#define AOC_T schedule_ptr
#define AOC_T_NAME Schedule
#include <aoc/array.h>

static inline bool try_find_schedule(const AocArraySchedule *const schedules,
                                     const uint16_t guardId,
                                     guard_schedule **const schedule) {
  for (size_t i = 0; i < schedules->length; ++i) {
    if (schedules->items[i]->guardId == guardId) {
      *schedule = schedules->items[i];
      return true;
    }
  }
  return false;
}

static void count_minutes(const AocArrayShift *const shifts,
                          uint16_t minutes[const 60]) {
  for (size_t i = 0; i < shifts->length; ++i) {
    const shift *const s = &shifts->items[i];
    for (uint8_t j = 0; j < s->sleepRanges.length; ++j) {
      const range *const r = &s->sleepRanges.items[j];
      for (uint8_t m = r->from; m < r->to; ++m) {
        minutes[m]++;
      }
    }
  }
}

static void parse_guard_schedules(const AocArrayRecord *const records,
                                  AocArraySchedule *const schedules) {
  uint32_t i = 0;
  while (i < records->length) {
    guard_schedule *schedule = NULL;
    const uint16_t guardId = records->items[i].data.guardId;
    if (!try_find_schedule(schedules, guardId, &schedule)) {
      schedule = AocMalloc(sizeof(guard_schedule));
      schedule->guardId = guardId;
      schedule->totalSleepTime = 0;
      AocArrayShiftCreate(&schedule->shifts, 16);
      AocArraySchedulePush(schedules, schedule);
    }
    i++;

    shift s = {0};
    AocArrayRangeCreate(&s.sleepRanges, 8);
    uint16_t sleepStartTime = 0;

    const record *r = &records->items[i];
    while (i < records->length && r->data.type != RECORD_TYPE_BEGINS_SHIFT) {
      if (r->data.type == RECORD_TYPE_FALLS_ASLEEP) {
        sleepStartTime = r->data.time.minute;
      } else /* wakes up */ {
        range rn = {
            .from = sleepStartTime,
            .to = r->data.time.minute,
        };
        schedule->totalSleepTime += (rn.to - rn.from);
        AocArrayRangePush(&s.sleepRanges, rn);
      }
      i++;
      r = &records->items[i];
    }
    AocArrayShiftPush(&schedule->shifts, s);
  }
}

uint32_t solve_part1(const AocArraySchedule *const schedules) {
  // find guard with longest time asleep
  guard_schedule *schedule = NULL;
  uint32_t longest = 0;

  for (size_t i = 0; i < schedules->length; ++i) {
    if (schedules->items[i]->totalSleepTime > longest) {
      schedule = schedules->items[i];
    }
  }

  uint16_t minutes[60] = {0};
  count_minutes(&schedule->shifts, minutes);

  uint16_t minute = 0;
  uint16_t biggest = 0;
  for (uint8_t i = 0; i < 60; ++i) {
    if (minutes[i] > biggest) {
      biggest = minutes[i];
      minute = i;
    }
  }

  return schedule->guardId * minute;
}

uint32_t solve_part2(const AocArraySchedule *const schedules) {
  uint16_t biggest = 0;
  uint16_t minute = 0;
  uint16_t guardId = 0;

  for (size_t i = 0; i < schedules->length; ++i) {
    const guard_schedule *const schedule = schedules->items[i];
    uint16_t minutes[60] = {0};
    count_minutes(&schedule->shifts, minutes);

    for (uint8_t j = 0; j < 60; ++j) {
      if (minutes[j] > biggest) {
        biggest = minutes[j];
        minute = j;
        guardId = schedule->guardId;
      }
    }
  }

  return minute * guardId;
}

int main(void) {
  aoc_arena arena = {0};
  AocSetArena(&arena);

  AocArrayRecord records = {0};
  AocArrayRecordCreate(&records, 1110);

  AocReadFileLineByLine("day04/input.txt", parse_line, &records);
  qsort(records.items, records.length, sizeof(record), compare_records);

  AocArraySchedule schedules = {0};
  AocArrayScheduleCreate(&schedules, 64);
  parse_guard_schedules(&records, &schedules);

  const uint32_t part1 = solve_part1(&schedules);
  const uint32_t part2 = solve_part2(&schedules);

  printf("%u\n", part1);
  printf("%u\n", part2);

  // don't need to free anything else because everything is allocated with the
  // arena
  AocArenaFree(&arena);
}
