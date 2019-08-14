#include <build_options.h>
#include "comm.hpp"

using my_tensor = algebra::xfield<typename BuildOptions::real, 2, config.L, config.W, config.H>;

int main(int argc, char* argv[]){
  distributed::Communicator comm(&argc, &argv);
  my_tensor Y;
  Y.fillTG(comm.domainStart(), comm.domainStop());
  distributed::communicateBuffers(Y, comm);
  if(comm.shouldIPrint()){
    std::cout << Y << std::endl;
  }
}
