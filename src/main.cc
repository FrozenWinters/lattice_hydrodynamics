#include <build_options.h>
#include "comm.hpp"

using my_tensor = space::vector_field<typename BuildOptions::real, 1, config.L, config.W, config.H>;
using my_scalar = space::scalar_field<typename BuildOptions::real, 1, config.L, config.W, config.H>;

int main(int argc, char* argv[]){
  distributed::Communicator comm(&argc, &argv);
  my_tensor Y;
  my_scalar Z;

  Y.fillTG(comm.domainStart(), comm.domainStop());
  Z.fillValue(comm.getRank());
  distributed::communicateBuffers(Y, comm);
  distributed::communicateBuffers(Z, comm);

  if(comm.shouldIPrint()){
    std::cout << Y << std::endl;
    std::cout << Z << std::endl;
    std::cout << Y.at(0, 0, 0, 0) << std::endl;
    std::cout << Z.at(0, 0, 0) << std::endl;
    Y.print_buffer(0, 1);
    Y.print_buffer(0, -1);
    Z.print_buffer(1, 1);
    Z.print_buffer(1, -1);
  }
}
