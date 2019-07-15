#include "algebra.hpp"
#include "xalgebra.hpp"
#include "distributed.hpp"

using tensor = state<Real, 3, 0, config.L, config.W, config.H>;
using my_tensor = algebra::xfield<Real, config.L, config.W, config.H>;

int main(int argc, char* argv[]){
  MPI_Comm cartcomm;
  int cord[3];
  distrubuted::setup_MPI(&argc, &argv, cord, cartcomm);

  Real begin[3], end[3];
  distrubuted::domainDecompose(begin, end);

  my_tensor Y;
  Y.fillTG(start, stop);
  //cout << Y << endl;

  //tensor& V = *(new tensor());
  //V.fillTG();
  //cout << V;

  MPI_Finalize();
}
