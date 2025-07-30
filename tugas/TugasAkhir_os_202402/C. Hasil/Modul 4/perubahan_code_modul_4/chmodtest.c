#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main() {
  int fd = open("myfile.txt", O_CREATE | O_RDWR);
  write(fd, "hello", 5);
  close(fd);

  chmod("myfile.txt", 1);  // set read-only

  fd = open("myfile.txt", O_RDWR);
  if(write(fd, "world", 5) < 0)
    printf(1, "Write blocked as expected\n");
  else
    printf(1, "Write allowed unexpectedly\n");

  close(fd);
  exit();
}