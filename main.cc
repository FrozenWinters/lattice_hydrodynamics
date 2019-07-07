#include "algebra.hpp"
#include <omp.h>
#include <iostream>

using namespace std;

typedef grid<float, 200, 200, 200, 16> field;

void __attribute__((noinline)) computeN(const field& in, field& out){
  in.computeN(out);
}

int main(){
  cout << omp_get_max_threads() << endl;
  field& V = *(new field());
  V.fillRand();
  //cout << "Matrix V:" << endl;
  //cout << V;

  field& N = *(new field());
  for(u_int i = 0; i < 40; i++){
    //N = {};
    computeN(V, N);
    //V.computeN(N);
  }

  //cout << "Matrix N:" << endl;
  //cout << N;
}
