#include "algebra.hpp"
#include "xalgebra.hpp"
#include <build_options.h>
#include <mpi.h>
#include <cstdlib>

using namespace std;

using tensor = state<typename BuildOptions::real, 3, 0, config.L, config.W, config.H>;
using my_tensor = xfield<typename BuildOptions::real, config.L, config.W, config.H>;

inline void setup_MPI(const int& argc, const char* &argv[], MPI_Comm& cartcomm){
  constexpr static size_t N = config.DOMAIN_SCALE;
  constexpr static size_t numproc = N * N * N;
  constexpr static int ndims[3] = {N, N, N};
  constexpr static int periods[3] = {1, 1, 1};

  int rank, runtime_numproc;
  int coords[3];

  MPI_Init(&argc, &argv);

  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &runtime_numproc);

  if(runtime_numproc != numproc){
    cout << "ERROR: Expecting " << numproc << " processes, but getting " << runtime_numproc << ". Goodbye!" << endl;
    _Exit(EXIT_FAILURE);
  }

  MPI_Cart_create(MPI_COMM_WORLD, 3, ndims, periods, 0, &cartcomm);

  cout << "My regular rank is: " << rank << " of " << numproc << endl;

  MPI_Comm_rank(cartcomm, &rank);

  cout << "My cartesian rank is: " << rank << endl;
  MPI_Cart_coords(cartcomm, rank, 3, coords);

  cout << "My co-ordinates are: " << coords[0] << " , " << coords[1] << " , " << coords[2] << endl;
}

int main(int argc, char* argv[]){
  MPI_Comm cartcomm;
  setup_MPI(argc, argv, cartcomm);

  my_tensor Y;
  Y.fillTG();
  //cout << Y << endl;

  //tensor& V = *(new tensor());
  //V.fillTG();
  //cout << V;

  MPI_Finalize();
}
