#define bcopy_by_int(from, to, n) asm volatile("movd %0, r0; movd %1, r1; movd %2, r2; movsd" : : "g" ((n)), "g" (from), "g" (to): "r0", "r1", "r2");
#define bcopy_by_char(from, to, n) asm volatile("movd %0, r0; movd %1, r1; movd %2, r2; movsb" : : "g" ((n)), "g" (from), "g" (to): "r0", "r1", "r2");
#define rbcopy_by_int(from, to, n) asm volatile("movd %0, r0; movd %1, r1; movd %2, r2; movsd [b]" : : "g" ((n)), "g" (from), "g" (to): "r0", "r1", "r2");
#define rbcopy_by_char(from, to, n) asm volatile("movd %0, r0; movd %1, r1; movd %2, r2; movsb [b]" : : "g" ((n)), "g" (from), "g" (to): "r0", "r1", "r2");

bcopy(char *from, char *to, int n)
{
  if((n > 0) && (from != to))
    {
      if ((to < from) || (to >= from + n))
	{
	  /* Forward copy */
	  int t;

	  /* Copy bytes til from is aligned */
	  t = ((int) from) & (sizeof(int) - 1);
	  if (t <= n)
	    {
	      bcopy_by_char(from, to, t);
	      n -= t;
	      from += t;
	      to += t;
	    }

	  /* Copy as many whole words as possible */
	  t = n / sizeof(int);
	  bcopy_by_int(from, to, t);
	  t *= sizeof(int);
	  n -= t;
	  to += t;
	  from += t;

	  /* Copy any remaining bytes */
	  bcopy_by_char(from, to, n);
	}
      else
	{
	  /* Reverse copy */
	  int t;
	  from += n;
	  to += n;

	  /* Copy bytes til from is aligned */
	  t = ((int) from) & (sizeof(int) - 1);
	  if (t <= n)
	    {
	      rbcopy_by_char(from - sizeof(char), to - sizeof(char), t);
	      n -= t;
	      from -= t;
	      to -= t;
	    }

	  /* Copy as many whole words as possible */
	  t = n / sizeof(int);
	  rbcopy_by_int(from - sizeof(int), to - sizeof(int), t);
	  t *= sizeof(int);
	  n -= t;
	  from -= t;
	  to -= t;

	  /* Copy any remaining bytes */
	  rbcopy_by_char(from - sizeof(char), to - sizeof(char), n);
	}

    }
  return (0);
}
    
