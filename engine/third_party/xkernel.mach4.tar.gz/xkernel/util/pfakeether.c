/*

*/

main(argc, argv) char **argv;
{
  int i, base;
  char *ea;

  if (argc > 2 ) {
    ea = argv[1];
    base = atoi(argv[2]);
    i = 0;
    while (*ea) {
      if (*ea != '.') i = 10*i + *ea - '0';
      else
	{ printf("%.2x:", i&0xff); i = 0; }
      ea++;
    }
    printf("%.2x:%.2x:%.2x", i&0xff, (base>>8)&0xff, base&0xff);
  }
}
