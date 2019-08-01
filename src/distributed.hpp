#include <mpi.h>
#include <cstdlib>
#include <build_options.h>
#include <iostream>

using Real = typename BuildOptions::real;
using Vect = std::array<Real, 3>;
using Ind = std::array<int, 3>;

namespace distributed{
  constexpr static size_t N = config.DOMAIN_SCALE;

  struct Communicator{
    Communicator(int* argc, char** argv[]);
    ~Communicator();

    MPI_Comm cartcomm;
    Ind cord;
    int rank;
    int nbrs[3][2];

    std::array<Real, 6 * N> send_buffs[3][2];
    std::array<Real, 6 * N> rcv_buffs[3][2];
  }

  inline Communicator::Communicator(int* argc, char** argv[]){
    constexpr static size_t numproc = N * N * N;
    constexpr static int sides[3] = {N, N, N};
    constexpr static int periodic[3] = {1, 1, 1};

    int runtime_numproc;

    MPI_Init(argc, argv);

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

    std::cout << "My co-ordinates are: " << cord[0] << " , " << cord[1] << " , " << cord[2] << std::endl;
  }

  inline Communicator::~Communicator(){
    MPI_Finalize();
  }

  inline Vect domainStart(const Ind& cord){
    return {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};;
  }

  inline Vect domainStop(const Ind& cord){
    return {((Real) cord[0]) / N, ((Real) cord[1]) / N, ((Real) cord[2]) / N};;
  }
}
