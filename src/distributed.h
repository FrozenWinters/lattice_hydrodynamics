#ifndef DISTRIBUTED_H_
#define DISTRIBUTED_H_

#include <mpi.h>
#include <build_options.h>

namespace distributed{
  class Communicator{
    constexpr static size_t N = config.DOMAIN_SCALE;
    using Real = typename BuildOptions::real;
    using Vect = std::array<Real, 3>;
    using Ind = std::array<int, 3>;

  public:
    Communicator(int* argc, char** argv[]);
    ~Communicator();

    void sendToAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir);
    void recvFromAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir);

    Vect domainStart();
    Vect domainStop();

  private:
    MPI_Comm cartcomm;
    Ind cord;
    int rank;
    int nbrs[3][2];
  };
}

#endif
