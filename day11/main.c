#include <stdio.h>

#define SERIAL_NUMBER 5177 // input
#define GRID_SIZE 300

static inline int calc_fuel_value(const int x, const int y) {
  const int rackId = (x + 1) + 10;
  return (((((rackId * (y + 1)) + SERIAL_NUMBER) * rackId) / 100) % 10) - 5;
}

static void create_sum_grid(int *const sumGrid) {
  sumGrid[0] = calc_fuel_value(0, 0);

  // first column
  for (int i = 0; i < GRID_SIZE * GRID_SIZE; i += GRID_SIZE)
    sumGrid[i] = calc_fuel_value(0, i);

  // first row
  for (int i = 1; i < GRID_SIZE; ++i)
    sumGrid[i] = sumGrid[i - 1] + calc_fuel_value(i, 0);

  // rows
  for (int y = 0; y < GRID_SIZE; ++y) {
    for (int x = 1; x < GRID_SIZE; ++x) {
      const int i = y * GRID_SIZE + x;
      sumGrid[i] = sumGrid[i - 1] + calc_fuel_value(x, y);
    }
  }

  // columns
  for (int x = 0; x < GRID_SIZE; ++x) {
    for (int y = 1; y < GRID_SIZE; ++y) {
      const int i = y * GRID_SIZE + x;
      sumGrid[i] += sumGrid[i - GRID_SIZE];
    }
  }
}

static inline int calc_power_zero(const int *const grid, const int size) {
  return grid[(size - 1) * GRID_SIZE + (size - 1)];
}

static inline int calc_power_first_row(const int *const grid, const int x,
                                       const int size) {
  return grid[(size - 1) * GRID_SIZE + (x + size - 1)] -
         grid[(size - 1) * GRID_SIZE + (x - 1)];
}

static inline int calc_power_first_column(const int *const grid, const int y,
                                          const int size) {
  return grid[(y + size - 1) * GRID_SIZE + (size - 1)] -
         grid[(y - 1) * GRID_SIZE + (size - 1)];
}

static inline int calc_power(const int *const grid, const int x, const int y,
                             const int size) {
  return grid[(y + size - 1) * GRID_SIZE + (x + size - 1)] -
         grid[(y - 1) * GRID_SIZE + (x + size - 1)] -
         grid[(y + size - 1) * GRID_SIZE + (x - 1)] +
         grid[(y - 1) * GRID_SIZE + (x - 1)];
}

static int solve_part1(const int *const grid, int *const outX,
                       int *const outY) {
  int largestPower = calc_power_zero(grid, 3);

  for (int x = 1; x < (GRID_SIZE - 3); ++x) {
    const int totalPower = calc_power_first_row(grid, x, 3);
    if (totalPower > largestPower) {
      largestPower = totalPower;
      *outX = x + 1;
      *outY = 1;
    }
  }
  for (int y = 1; y < (GRID_SIZE - 3); ++y) {
    const int totalPower = calc_power_first_column(grid, y, 3);
    if (totalPower > largestPower) {
      largestPower = totalPower;
      *outX = 1;
      *outY = y + 1;
    }
  }

  for (int y = 1; y < (GRID_SIZE - 3); ++y) {
    for (int x = 1; x < (GRID_SIZE - 3); ++x) {
      const int totalPower = calc_power(grid, x, y, 3);
      if (totalPower > largestPower) {
        largestPower = totalPower;
        *outX = x + 1;
        *outY = y + 1;
      }
    }
  }
  return largestPower;
}

static void solve_part2(const int *const grid, int *const outX, int *const outY,
                        int *const outSize) {
  int largestPower = 0;

  for (int size = 1; size <= GRID_SIZE; ++size) {
    const int totalPower = calc_power_zero(grid, size);
    if (totalPower > largestPower) {
      largestPower = totalPower;
      *outX = 1;
      *outY = 1;
    }
  }

  for (int x = 1; x < (GRID_SIZE - 3); ++x) {
    for (int size = 1; size <= GRID_SIZE; ++size) {
      if (x + size > GRID_SIZE)
        break;
      const int totalPower = calc_power_first_row(grid, x, size);
      if (totalPower > largestPower) {
        largestPower = totalPower;
        *outX = x + 1;
        *outY = 1;
      }
    }
  }

  for (int y = 1; y < (GRID_SIZE - 3); ++y) {
    for (int size = 1; size <= GRID_SIZE; ++size) {
      if (y + size > GRID_SIZE)
        break;
      const int totalPower = calc_power_first_column(grid, y, size);
      if (totalPower > largestPower) {
        largestPower = totalPower;
        *outX = 1;
        *outY = y + 1;
      }
    }
  }

  for (int y = 1; y < GRID_SIZE; ++y) {
    for (int x = 1; x < GRID_SIZE; ++x) {
      for (int size = 1; size <= GRID_SIZE; ++size) {
        if (x + size > GRID_SIZE || y + size > GRID_SIZE)
          break;
        const int totalPower = calc_power(grid, x, y, size);
        if (totalPower > largestPower) {
          largestPower = totalPower;
          *outX = x + 1;
          *outY = y + 1;
          *outSize = size;
        }
      }
    }
  }
}

int main(void) {
  int sumGrid[GRID_SIZE * GRID_SIZE] = {0};
  create_sum_grid(sumGrid);

  int x1 = 0;
  int y1 = 0;
  solve_part1(sumGrid, &x1, &y1);

  int x2 = 0;
  int y2 = 0;
  int size = 0;
  solve_part2(sumGrid, &x2, &y2, &size);

  printf("%d,%d\n", x1, y1);
  printf("%d,%d,%d\n", x2, y2, size);
}
