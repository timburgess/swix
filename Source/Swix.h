//
//  Swix.h
//  Swix
//
//  Created by Tim Burgess on 23/11/2015.
//  Copyright © 2015 Tim Burgess. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <Accelerate/Accelerate.h>

//! Project version number for Swix.
FOUNDATION_EXPORT double SwixVersionNumber;

//! Project version string for Swix.
FOUNDATION_EXPORT const unsigned char SwixVersionString[];

// In this header, you should import all the public headers of your framework using statements like #import <Swix/PublicHeader.h>


// operations and indexing
#import <stdint.h>

void test(){
  int N = 1000;
  double * x = (double*)malloc(sizeof(double) * N);
  double * y = (double*)malloc(sizeof(double) * N);
  for (int i=0; i<N; i++){ x[i] = i; y[i] = 0;}
  
  dispatch_apply(N, dispatch_get_global_queue(0, 0), ^(size_t i){
    y[i] = x[i]+1;
  });
  
  for (int i=0; i<N; i++){
    double value = x[i]+1 - y[i];
    if (!(value==0)){
      printf("%.2e, ", value);
    }
  }
  printf("\n");
}

// UNOPTIMIZED
void index_xa_b_objc(double* x, double* a, double* b, int N){
  int * j = (int *)malloc(sizeof(int) * N);
  vDSP_vfix32D(a, 1, j, 1, N); // double to int
  
  for (int i=0; i<N; i++){
    x[j[i]] = b[i];
  }
  free(j);
}

// OPTIMIZED
double* zeros_objc(int N){
  double * x = (double *)malloc(sizeof(double) * N);
  double value = 0.0;
  vDSP_vfillD(&value, x, 1, N);
  return x;
}

void svd_objc(double * xx, int m, int n, double* s, double* vt, double* u, int compute_uv){
  // adapted from the buggy code at http://stackoverflow.com/questions/5047503/lapack-svd-singular-value-decomposition
  
  char job;
  if      (compute_uv == 0) job = 'N';
  else if (compute_uv == 1) job = 'A';
  
  __CLPK_integer lda = (__CLPK_integer)m;
  long numberOfSingularValues = m < n ? m : n;
  
  // Workspace and status variables:
  __CLPK_integer _n = n;
  __CLPK_integer _m = m;
  double* work = (double*)malloc(sizeof(double) * 2);
  __CLPK_integer lwork = -1;
  __CLPK_integer * iwork = (__CLPK_integer *)malloc(sizeof(__CLPK_integer) * 8 * numberOfSingularValues);
  __CLPK_integer info = 0;
  
  // Call dgesdd_ with lwork = -1 to query optimal workspace size:
  dgesdd_(&job, &_m, &_n, xx, &lda, s, u, &_m, vt, &_n, work, &lwork, iwork, &info);
  
  // Optimal workspace size is returned in work[0].
  lwork = work[0];
  free(work);
  work = (double *)malloc(lwork * sizeof(double));
  
  // Call dgesdd_ to do the actual computation:
  dgesdd_(&job, &_m, &_n, xx, &lda, s, u, &_m, vt, &_n, work, &lwork, iwork, &info);
  
  free(work);
  free(iwork);
}



