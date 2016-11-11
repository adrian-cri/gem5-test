#include <stdio.h>
#include <stdlib.h>
#include "m5/m5op.h"

#ifndef N
# define N 1000
#endif

int main()
{
  int i;
  double a[N],b[N],c[N],d[N];
  double k = 2.3;

  double *t = (double *) hbw_malloc(sizeof(double)*1000);

  for (i=0; i<N; i++) {
    c[i] = a[i] + b[i];
    d[i] = k * a[i];
  }

  // prevent from deleting code
  printf("%f %f %f %f\n",a[11],b[11],c[11],d[11]);

  exit(0);
}
