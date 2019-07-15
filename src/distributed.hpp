#include <mpi.h>
#include <cstdlib>
#include <build_options.h>
#include <iostream>

using Real = typename BuildOptions::real;

namespace distrubuted{
  constexpr static size_t N = config.DOMAIN_SCALE;

  inline void setupMPI(int* argc, char** argv[], int cord[], MPI_Comm& cartcomm){
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
    MPI_Cart_coords(cartcomm, rank, 3, cord);

    cout << "My co-ordinates are: " << cord[0] << " , " << cord[1] << " , " << cord[2] << endl;
  }

  inline void domainDecompose(Real begin[3], Real end [3], const int cord[3]){
    begin = {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};
    end = {((Real) cord[0] + 1) / N, ((Real) cord[1] + 1) / N, ((Real) cord[2] + 1) / N};
  }
}
