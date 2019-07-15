#include "algebra.hpp"
#include "xalgebra.hpp"
#include "distributed.hpp"

using tensor = state<Real, 3, 0, config.L, config.W, config.H>;
using my_tensor = algebra::xfield<Real, config.L, config.W, config.H>;

int main(int argc, char* argv[]){
  MPI_Comm cartcomm;
  Ind cord;
  distributed::setupMPI(&argc, &argv, cord, cartcomm);

  Vect start = distributed::domainStart(cord);
  Vect stop = distributed::domainStop(cord);

  my_tensor Y;
  Y.fillTG(start, stop);
  //std::cout << Y << std::endl;

  //tensor& V = *(new tensor());
  //V.fillTG();
  //cout << V;

  MPI_Finalize();
}
