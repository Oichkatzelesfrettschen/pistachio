/*

*/

#include <fcntl.h>

main(argc, argv) char **argv;
{
  int i, fd;
  char c[6], d;

  if (argc>2   && (fd = open(argv[1], O_RDONLY)) > 0) {
    i = atoi(argv[2]);
    while (i && read(fd, &d, 1)>0) {
      if (d == '\n') i--;
    }
    while (!i && read(fd, &d, 1)>0) {
      write(1, &d, 1);
      if (d == '\n') i--;
    }
  }
}
