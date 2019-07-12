#include "algebra.hpp"
#include <build_options.h>

using namespace std;

using tensor = state<typename BuildOptions::real, 3, 0, config.L, config.W, config.H>;
using scalar = state<typename BuildOptions::real, 1, 0, config.L, config.W, config.H>;

/*void __attribute__((noinline)) computeN(const field& in, field& out){
  in.computeN(out);
}*/

int main(){
  tensor& V = *(new tensor());
  tensor& L = *(new tensor());
  tensor& N = *(new tensor());
  tensor& V2 = *(new tensor());

  V.fillTG();
  cout << "Taylor-Green Vortex:" << endl;
  cout << V;

  V.vectLap(L);
  cout << "Laplacian:" << endl;
  cout << L;

  V.dVfvf(N);
  cout << "Non-linear term:" << endl;
  cout << N;

  N.scale(-1, N);
  L.scale(0.2, L);
  N.sum(L, V2);
  cout << "V2:" << endl;
  cout << V2;

  /*field& N = *(new field());
  for(u_int i = 0; i < 40; i++){
    N = {};
    computeN(V, N);
  }

  cout << "Matrix N:" << endl;
  cout << N;*/
}
