#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  char buf[10];
  printf(1, "Read Count Sebelum: %d\n", getreadcount());
  read(0, buf, 5); // baca stdin
  printf(1, "Read Count Setelah: %d\n", getreadcount());
  exit();
}