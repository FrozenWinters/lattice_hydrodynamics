#include "algebra.h"
#include iostream;

using namespace std;

typedef grid<double, 24, 24, 24> field;

int main(){
  field V, N;
  V.fillRand();
  cout << "Matrix V:" << endl;
  cout << V;
  V.computeN(&N);

  out << "Matrix N:" << endl;
  cout << N;
}
