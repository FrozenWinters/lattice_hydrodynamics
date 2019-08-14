/*#include "distributed.h"
#include <iostream>
#include <cstdlib>

namespace distributed{
  Communicator::Communicator(int* argc, char** argv[]){
    constexpr static size_t numproc = N * N * N;
    constexpr static int sides[3] = {N, N, N};
    constexpr static int periodic[3] = {1, 1, 1};

    int runtime_numproc, supported_status;

    MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &supported_status);

    if(supported_status != MPI_THREAD_MULTIPLE){
      std::cerr << "ERROR: MPI implementation does not support thread-concurrent communication. Goodbye!" << std::endl;
      _Exit(EXIT_FAILURE);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &runtime_numproc);

    if(runtime_numproc != numproc){
      std::cerr << "ERROR: Expecting " << numproc << " processes, but getting " << runtime_numproc << ". Goodbye!" << std::endl;
      _Exit(EXIT_FAILURE);
    }

    MPI_Cart_create(MPI_COMM_WORLD, 3, sides, periodic, 0, &cartcomm);
    MPI_Comm_rank(cartcomm, &rank);
    MPI_Cart_coords(cartcomm, rank, 3, cord.data());

    for(int i = 0; i < 3; ++i){
      MPI_Cart_shift(cartcomm, i, 1, &nbrs[i][0], &nbrs[i][1]);
    }
  }

  Communicator::~Communicator(){
    MPI_Finalize();
  }

  auto Communicator::domainStart() -> Vect{
    return {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};
  }

  auto Communicator::domainStop() -> Vect{
    return {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};
  }

  void Communicator::sendToAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir){
    MPI_Send(buff, size, MPI_BYTE, nbrs[axis][dir == 1], MPI_ANY_TAG, MPI_COMM_WORLD);
  }

  void Communicator::recvFromAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir){
    MPI_Recv(buff, size, MPI_BYTE, nbrs[axis][dir == 1], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
}*/
