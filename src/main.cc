#include "algebra.hpp"
#include <build_options.h>

using namespace std;

using field = grid<typename BuildOptions::real, config.L, config.W, config.H, config.VecSize>;

void __attribute__((noinline)) computeN(const field& in, field& out){
  in.computeN(out);
}

int main(){
  field& V = *(new field());
  V.fillRand();
  cout << "Matrix V:" << endl;
  cout << V;

  field& N = *(new field());
  for(u_int i = 0; i < 40; i++){
    N = {};
    computeN(V, N);
  }

  cout << "Matrix N:" << endl;
  cout << N;
}
