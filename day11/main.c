#include <stdio.h>
#include <aoc/aoc.h>

#define SERIAL_NUMBER 5177 // input
#define GRID_SIZE 300
#define SQUARE_SIZE 3

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

static void solve_part1(const int *const grid, int *const outX,
                        int *const outY) {
  int largestPower = 0;
  for (int y = 0; y < (GRID_SIZE - SQUARE_SIZE); ++y) {
    for (int x = 0; x < (GRID_SIZE - SQUARE_SIZE); ++x) {
      int totalPower = 0;
      for (int sy = 0; sy < SQUARE_SIZE; ++sy) {
        for (int sx = 0; sx < SQUARE_SIZE; ++sx) {
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
}

int main(void) {
  int grid[GRID_SIZE * GRID_SIZE] = {0};
  calc_fuel_values(grid);
  int x = 0;
  int y = 0;
  solve_part1(grid, &x, &y);
  printf("%d,%d\n", x, y);
}
