/*

*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

struct hostent *gethostbyname();

main(argc, argv) char **argv;
{
  char *n, **hn;
  struct hostent *h;
  int i;

  if (argc > 1 && (h=gethostbyname(argv[1]))) {
    if (h->h_length > 0 && h->h_addr_list) {
      hn = h->h_addr_list;
      n = *hn;
      if (n) for (i=0; i<h->h_length-2; i++) printf("%d.", *n++ & 0xff);
      printf("%d", *n &0xff);
    }
  }
}
