#include <stdio.h>
#include <aoc/aoc.h>

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

static int solve_part1(const int *const grid, const int squareSize,
                       int *const outX, int *const outY) {
  int largestPower = 0;
  for (int y = 0; y < (GRID_SIZE - squareSize); ++y) {
    for (int x = 0; x < (GRID_SIZE - squareSize); ++x) {
      int totalPower = 0;
      for (int sy = 0; sy < squareSize; ++sy) {
        for (int sx = 0; sx < squareSize; ++sx) {
          const int index = (y + sy) * GRID_SIZE + (x + sx);
          totalPower += grid[index];
        }
      }
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

  for (int size = 1; size <= 300; ++size) {
    int x = 0;
    int y = 0;
    int power = solve_part1(grid, size, &x, &y);
    if (power > largestPower) {
      *outX = x;
      *outY = y;
      *outSize = size;
      largestPower = power;
    }
  }
}

int main(void) {
  int grid[GRID_SIZE * GRID_SIZE] = {0};
  calc_fuel_values(grid);
  int x1 = 0;
  int y1 = 0;
  solve_part1(grid, 3, &x1, &y1);
  int x2 = 0;
  int y2 = 0;
  int size = 0;
  solve_part2(grid, &x2, &y2, &size);
  printf("%d,%d\n", x1, y1);
  printf("%d,%d,%d\n", x2, y2, size);
}
