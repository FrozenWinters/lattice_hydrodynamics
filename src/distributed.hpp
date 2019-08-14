#ifndef DISTRIBUTED_HPP_
#define DISTRIBUTED_HPP_

#include <build_options.h>
#include <array>
#include <iostream>
#include <cstdlib>

namespace distributed{

  namespace detail{
    template<size_t N>
    struct CommunicatorData{
    // protected:
      using Ind = std::array<int, 3>;
      MPI_Comm cartcomm;
      Ind cord;
      int rank;
      int nbrs[3][2];
    };

    // template<>
    // struct CommunicatorData<1> {};

    template<size_t N>
    class Communicator : CommunicatorData<N>{
      using Real = typename BuildOptions::real;
      using Vect = std::array<Real, 3>;

    public:
      Communicator(int* argc, char** argv[]);
      ~Communicator();

      void sendToAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir);
      void recvFromAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir);

      Vect domainStart();
      Vect domainStop();
    };
  }

  using Communicator = detail::Communicator<config.DOMAIN_SCALE>;

  namespace detail{
    template<size_t N>
    Communicator<N>::Communicator(int* argc, char** argv[]){
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

      MPI_Cart_create(MPI_COMM_WORLD, 3, sides, periodic, 0, &this->cartcomm);
      MPI_Comm_rank(this->cartcomm, &this->rank);
      MPI_Cart_coords(this->cartcomm, this->rank, 3, this->cord.data());

      for(int i = 0; i < 3; ++i){
        MPI_Cart_shift(this->cartcomm, i, 1, &this->nbrs[i][0], &this->nbrs[i][1]);
      }
    }

    // template<>
    // Communicator<1>::Communicator(int* argc, char** argv[]){};

    template<size_t N>
    Communicator<N>::~Communicator(){
      MPI_Finalize();
    }

    // template<>
    // Communicator<1>::~Communicator() {};

    template<size_t N>
    auto Communicator<N>::domainStart() -> Vect{
      return {((Real) this->cord[0]) / N, ((Real) this->cord[1]) / N, ((Real) this->cord[2]) / N};
    }

    // template<>
    // auto Communicator<1>::domainStart() -> Vect{
    //   return {(Real) 0, (Real) 0, (Real) 0};
    // }

    template<size_t N>
    auto Communicator<N>::domainStop() -> Vect{
      return {((Real) this->cord[0] + 1) / N, ((Real) this->cord[1] + 1) / N, ((Real) this->cord[2] + 1) / N};
    }

    // template<>
    // auto Communicator<1>::domainStop() -> Vect{
    //   return {(Real) 1, (Real) 1, (Real) 1};
    // }

    template<size_t N>
    void Communicator<N>::sendToAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir){
      MPI_Send(buff, size, MPI_BYTE, this->nbrs[axis][dir == 1], MPI_ANY_TAG, MPI_COMM_WORLD);
    }

    template<size_t N>
    void Communicator<N>::recvFromAdjacent(void* buff, const size_t& size, const size_t& axis, const int& dir){
      MPI_Recv(buff, size, MPI_BYTE, this->nbrs[axis][dir == 1], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
  }
}

#endif
