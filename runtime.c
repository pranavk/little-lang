#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

int64_t __l_input() {
  int64_t n;
  scanf("%"SCNi64, &n);
  return n;
}

void __l_print_int(int64_t x) {
  printf("%"PRIi64, x);
}

void __l_print_string(char* str) {
  printf("%s", str);
}

void __l_abort() {
  abort();
}

uint64_t __l_pow(uint64_t x, uint64_t n) {
  uint64_t result = 1;
  while (n--) {
    result *= x;
  }
  return result;
}
