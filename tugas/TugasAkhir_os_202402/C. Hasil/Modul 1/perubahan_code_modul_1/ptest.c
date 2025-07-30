#include "types.h"
#include "stat.h"
#include "user.h"

struct pinfo {
  int pid[64];
  int mem[64];
  char name[64][16];
};

int main() {
  struct pinfo p;
  getpinfo(&p);

  printf(1, "PID\tMEM\tNAME\n");
  for(int i = 0; i < 64 && p.pid[i]; i++) {
    printf(1, "%d\t%d\t%s\n", p.pid[i], p.mem[i], p.name[i]);
  }
  exit();
}