/*
 *   a simple-minded procedure for mapping names to addresses
 *
 *
 */


#include "xkernel.h"
#include "romopt.h"

#define DNS_MAP_SIZE           16
#define MAX_XKHOST_NAMELENGTH  64

static struct namenumber {
  char h_name[MAX_XKHOST_NAMELENGTH];
  IPhost h_addr; } hostnametable[DNS_MAP_SIZE];
static initialized = 0;


#define min(a,b) ((a)<(b) ? (a):(b))

static xkern_return_t addnametotable(str, nfields, line, arg )
     char **str;
     int nfields, line;
     VOID *arg;
{
  char hn[MAX_XKHOST_NAMELENGTH];
  int i;

  bzero(hn, MAX_XKHOST_NAMELENGTH);
  strncpy(hn, str[1], min(strlen(str[1]), MAX_XKHOST_NAMELENGTH));
  for (i=0; i<DNS_MAP_SIZE; i++) {
    if (!strcmp(hostnametable[i].h_name, hn))
      /* don't overwrite existing entries; should really print warning */
      break;
    else if (!hostnametable[i].h_name[0]) {
      strcpy(hostnametable[i].h_name, hn);
      str2ipHost(&hostnametable[i].h_addr, str[2]);
      break;
    }
  }
  return XK_SUCCESS;
}

xkern_return_t
  xknameresolve(n, a) char *n; IPhost *a;
{
  int i;

  for (i=0; i<DNS_MAP_SIZE; i++) {
    if (!strcmp(hostnametable[i].h_name, n))
      { *a = hostnametable[i].h_addr; return XK_SUCCESS; }
  }
  return XK_FAILURE;
}

/* looking for lines like
 *
 *   dns umbra 192.12.69.97
 *
 */

xkern_return_t
xk_gethostbyname(namestr, addr) 
     char *namestr;
     IPhost *addr;
{
  char hn[MAX_XKHOST_NAMELENGTH];
  RomOpt options[] = { {"", 3, addnametotable} };

  if (!initialized) {
    bzero((char *)hostnametable, sizeof(hostnametable));
    findRomOpts("dns", options, 1, 0);
    initialized = 1;
  }
  
  bzero(hn, MAX_XKHOST_NAMELENGTH);
  strncpy(hn, namestr, min(strlen(namestr), MAX_XKHOST_NAMELENGTH));
  return xknameresolve(hn, addr);
}
