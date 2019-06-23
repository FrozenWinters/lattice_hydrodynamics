#include "algebra.hpp"
#include <iostream>

using namespace std;

typedef grid<float, 8, 8, 8> field;

int main(){
  field V;
  V.fillRand();
  //cout << "Matrix V:" << endl;
  //cout << V;

  field N;
  for(u_int i = 0; i < 1'000'000; i++){
    N = {};
    V.computeN(N);
  }

  //cout << "Matrix N:" << endl;
  //cout << N;
}
