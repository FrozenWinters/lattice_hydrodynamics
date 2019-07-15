#include <mpi.h>
#include <cstdlib>
#include <build_options.h>
#include <iostream>

using Real = typename BuildOptions::real;
using Vect = std::array<Real, 3>;
using Ind = std::array<int, 3>;

namespace distributed{
  constexpr static size_t N = config.DOMAIN_SCALE;

  inline void setupMPI(int* argc, char** argv[], Ind& cord, MPI_Comm& cartcomm){
    constexpr static size_t numproc = N * N * N;
    constexpr static int ndims[3] = {N, N, N};
    constexpr static int periods[3] = {1, 1, 1};

    int rank, runtime_numproc;

    MPI_Init(argc, argv);

    MPI_Comm_size(MPI_COMM_WORLD, &runtime_numproc);

    if(runtime_numproc != numproc){
      std::cout << "ERROR: Expecting " << numproc << " processes, but getting " << runtime_numproc << ". Goodbye!" << std::endl;
      _Exit(EXIT_FAILURE);
    }

    MPI_Cart_create(MPI_COMM_WORLD, 3, ndims, periods, 0, &cartcomm);
    MPI_Comm_rank(cartcomm, &rank);
    MPI_Cart_coords(cartcomm, rank, 3, cord.data());

    std::cout << "My co-ordinates are: " << cord[0] << " , " << cord[1] << " , " << cord[2] << std::endl;
  }

  inline Vect domainStart(const Ind& cord){
    return {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};;
  }

  inline Vect domainStop(const Ind& cord){
    return {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};;
  }


  inline void domainDecompose(Real (&begin)[3], Real (&end)[3], const Real (&cord)[3]){
    //Real b[] = {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};
    //begin = b;
    //end = {((Real) cord[0] + 1) / N, ((Real) cord[1] + 1) / N, ((Real) cord[2] + 1) / N};
  }
}
