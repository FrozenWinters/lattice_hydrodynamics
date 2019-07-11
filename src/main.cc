#include "algebra.hpp"
#include <build_options.h>

using namespace std;

using tensor = state<typename BuildOptions::real, 3, 2, config.L, config.W, config.H>;
using scalar = state<typename BuildOptions::real, 1, 2, config.L, config.W, config.H>;

/*void __attribute__((noinline)) computeN(const field& in, field& out){
  in.computeN(out);
}*/

int main(){
  tensor& V = *(new tensor());
  V.fillRand();
  cout << "Matrix V:" << endl;
  cout << V;

  /*field& N = *(new field());
  for(u_int i = 0; i < 40; i++){
    N = {};
    computeN(V, N);
  }

  cout << "Matrix N:" << endl;
  cout << N;*/
}
