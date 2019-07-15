#include "algebra.hpp"
#include "xalgebra.hpp"
#include <build_options.h>
#include <mpi.h>
#include <cstdlib>

using namespace std;

using real = typename BuildOptions::real;
using tensor = state<real, 3, 0, config.L, config.W, config.H>;
using my_tensor = xfield<real, config.L, config.W, config.H>;
constexpr static size_t scale = config.DOMAIN_SCALE;

inline void setup_MPI(int* argc, char** argv[], int coords[], MPI_Comm& cartcomm){
  constexpr static size_t N = scale;
  constexpr static size_t numproc = N * N * N;
  constexpr static int ndims[3] = {N, N, N};
  constexpr static int periods[3] = {1, 1, 1};

  int rank, runtime_numproc;

  MPI_Init(argc, argv);

  MPI_Comm_size(MPI_COMM_WORLD, &runtime_numproc);

  if(runtime_numproc != numproc){
    cout << "ERROR: Expecting " << numproc << " processes, but getting " << runtime_numproc << ". Goodbye!" << endl;
    _Exit(EXIT_FAILURE);
  }

  MPI_Cart_create(MPI_COMM_WORLD, 3, ndims, periods, 0, &cartcomm);
  MPI_Comm_rank(cartcomm, &rank);
  MPI_Cart_coords(cartcomm, rank, 3, coords);

  cout << "My co-ordinates are: " << coords[0] << " , " << coords[1] << " , " << coords[2] << endl;
}

int main(int argc, char* argv[]){
  MPI_Comm cartcomm;
  int coords[3];
  setup_MPI(&argc, &argv, coords, cartcomm);

  real start[3] = {((real) coords[0]) / scale, ((real) coords[1]) / scale, ((real) coords[2]) / scale};
  real stop[3] = {((real) coords[0] + 1) / scale, ((real) coords[1] + 1) / scale, ((real) coords[2] + 1) / scale};

  my_tensor Y;
  Y.fillTG(start[0], stop[0], start[1], stop[1], start[2], stop[2]); 
  //cout << Y << endl;

  //tensor& V = *(new tensor());
  //V.fillTG();
  //cout << V;

  MPI_Finalize();
}
