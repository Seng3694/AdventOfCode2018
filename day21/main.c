#include <stdio.h>
#include <stdint.h>

int main(void) {
  uint32_t arr[1 << 14] = {0};
  uint32_t a = 0, b = 0, c = 0, length = 0;
  for (;;) {
    b = a | 0x10000;
    a = 6780005;
    for (;;) {
      c = b & 0xff;
      a = 0xffffff & (65899 * (0xffffff & (a + c)));
      if (b >= 0x100) {
        b /= 0x100;
      } else {
        break;
      }
    }
    for (uint32_t i = 0; i < length; ++i)
      if (arr[i] == a)
        goto done;
    arr[length++] = a;
  }
done:
  printf("%u\n%u\n", arr[0], arr[length - 1]);
}
