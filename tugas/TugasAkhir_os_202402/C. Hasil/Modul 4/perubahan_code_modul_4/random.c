// random.c
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h" 
#include "file.h"

static uint next_random = 1; 

struct spinlock random_lock;

void
randominit(void)
{
  initlock(&random_lock, "random");
 
}

int
randomread(struct inode *ip, char *dst, int n)
{
  acquire(&random_lock);
  int bytes_written = 0;
  for (int i = 0; i < n; i++) {
    if (bytes_written >= n) break; 
    next_random = next_random * 1103515245 + 1234577;
    char byte = (char)(next_random % 256); 

    *dst = byte;
    dst++;
    bytes_written++;
  }
  release(&random_lock);
  return bytes_written;
}