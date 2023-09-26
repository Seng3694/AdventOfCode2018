#include <stdio.h>

#define SERIAL_NUMBER 5177 // input
#define GRID_SIZE 300

static void calc_fuel_values(int *const grid) {
  for (int y = 0; y < GRID_SIZE; ++y) {
    for (int x = 0; x < GRID_SIZE; ++x) {
      const int rackId = (x + 1) + 10;
      int powerLevel = rackId * (y + 1);
      powerLevel += SERIAL_NUMBER;
      powerLevel *= rackId;
      powerLevel = (powerLevel / 100) % 10;
      powerLevel -= 5;
      grid[y * GRID_SIZE + x] = powerLevel;
    }
  }
}

static void calc_sum_grid(int *const grid, int *const sumGrid) {
  sumGrid[0] = grid[0];

  // first columns
  for (int i = 0; i < GRID_SIZE * GRID_SIZE; i += GRID_SIZE)
    sumGrid[i] = grid[i];

  // first row
  for (int i = 1; i < GRID_SIZE; ++i)
    sumGrid[i] = sumGrid[i - 1] + grid[i];

  // rows
  for (int y = 0; y < GRID_SIZE; ++y) {
    for (int x = 1; x < GRID_SIZE; ++x) {
      int i = y * GRID_SIZE + x;
      sumGrid[i] = sumGrid[i - 1] + grid[i];
    }
  }

  // columns
  for (int x = 0; x < GRID_SIZE; ++x) {
    for (int y = 1; y < GRID_SIZE; ++y) {
      int i = y * GRID_SIZE + x;
      int prev_i = (y - 1) * GRID_SIZE + x;
      sumGrid[i] += sumGrid[prev_i];
    }
  }
}

static int calc_total_power(const int *const grid, int x, int y, int size) {
  int p = grid[(y + size - 1) * GRID_SIZE + (x + size - 1)];
#define X_SUB_INDEX ((y + size - 1) * GRID_SIZE + (x - 1))
#define Y_SUB_INDEX ((y - 1) * GRID_SIZE + (x + size - 1))
#define CORNER_SUB_INDEX ((y - 1) * GRID_SIZE + (x - 1))
  if (x > 0 && y > 0) {
    p = p - grid[Y_SUB_INDEX] - grid[X_SUB_INDEX] + grid[CORNER_SUB_INDEX];
  } else if (y > 0) {
    p -= grid[Y_SUB_INDEX];
  } else if (x > 0) {
    p -= grid[X_SUB_INDEX];
  }
  return p;
}

static int solve_part1(const int *const grid, const int size, int *const outX,
                       int *const outY) {
  int largestPower = 0;
  for (int y = 0; y < (GRID_SIZE - size); ++y) {
    for (int x = 0; x < (GRID_SIZE - size); ++x) {
      const int totalPower = calc_total_power(grid, x, y, size);
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
  for (int y = 0; y < GRID_SIZE; ++y) {
    for (int x = 0; x < GRID_SIZE; ++x) {
      for (int size = 1; size <= GRID_SIZE; ++size) {
        if (x + size > GRID_SIZE || y + size > GRID_SIZE)
          break;
        const int totalPower = calc_total_power(grid, x, y, size);
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
  int grid[GRID_SIZE * GRID_SIZE] = {0};
  calc_fuel_values(grid);

  int sumGrid[GRID_SIZE * GRID_SIZE] = {0};
  calc_sum_grid(grid, sumGrid);

  int x1 = 0;
  int y1 = 0;
  solve_part1(sumGrid, 3, &x1, &y1);

  int x2 = 0;
  int y2 = 0;
  int size = 0;
  solve_part2(sumGrid, &x2, &y2, &size);

  printf("%d,%d\n", x1, y1);
  printf("%d,%d,%d\n", x2, y2, size);
}
