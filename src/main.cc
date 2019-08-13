#include "algebra.hpp"
#include "xalgebra.hpp"
#include "distributed.hpp"
#include "comm.hpp"
#include <build_options.h>

using Real = typename BuildOptions::real;
using tensor = state<Real, 3, 0, config.L, config.W, config.H>;
using my_tensor = algebra::xfield<Real, 2, config.L, config.W, config.H>;
using Vect = std::array<Real, 3>;
using Ind = std::array<int, 3>;

int main(int argc, char* argv[]){
  distributed::Communicator comm(&argc, &argv);

  Vect start = distributed::domainStart(comm.cord);
  Vect stop = distributed::domainStop(comm.cord);

  my_tensor Y;
  Y.fillTG(start, stop);
  distributed::communicateBuffers(Y);
  //std::cout << Y << std::endl;


  //Y.test();

  /*tensor& V = *(new tensor());
  V.fillTG();
  std::cout << V;*/
}
