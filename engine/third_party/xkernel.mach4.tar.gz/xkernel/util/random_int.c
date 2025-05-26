main( argc, argv) char **argv;
{
  int mod = atoi(argv[1]), rn;
  int i, p;
  
  srand(p=getpid());
  rn = (rand() >> 16) &0xffff;
  srand(rn);

  rn = rand() % mod;
  printf("%d", rn);
}
