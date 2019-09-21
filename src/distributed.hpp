
// This file wraps MPI functionality.
// It also determines the region of space for which a process is responsible.

#ifndef DISTRIBUTED_HPP_
#define DISTRIBUTED_HPP_

#include <build_options.h>
#include <array>
#include <iostream>
#include <cstdlib>
#include <mpi.h>
#include <stencil.hpp>

namespace meta{
  template<size_t a, size_t n>
  struct pow{
    static constexpr size_t value = a * pow<a, n-1>::value;
  };

  template<size_t a>
  struct pow<a, 0>{
    static constexpr size_t value = 1;
  };

  template<size_t a, size_t n>
  static constexpr size_t pow_v = pow<a, n>::value;
}

namespace distributed{

  namespace detail{
    template<int... axis_values>
    struct nI;

    template<int val, int... axis_values>
    struct nI<val, axis_values...>{
      static constexpr size_t value = 3 * nI<axis_values...>::value + (val + 1);
    };

    template<>
    struct nI<>{
      static constexpr size_t value = 0;
    };

    template<int... axis_values>
    static constexpr size_t nI_v = nI<axis_values...>::value;

    template <size_t N, size_t dim>
    struct CommunicatorData{
      using self_t = CommunicatorData<N, dim>;
      using Ind = std::array<int, dim>;

      template <int... axis_values>
      void sendTo(void* buff, const size_t& size);

      template <int... axis_values>
      void recvFrom(void* buff, const size_t& size);

      int getRank() const{
        return rank;
      }

      bool shouldIPrint() const{
        return (this->rank == 0);
      }

    protected:
      Ind cord;
      int rank;

      inline static int indToRank(const Ind& loc){
        int result = 0;
        for(int i = 0; i < dim; ++i){
          result *= N;
          result += (loc[i] + N) % N;
        }
        return result;
      }

      inline static Ind rankToInd(int rank){
        Ind result;
        for(int i = 0; i < dim; ++i){
          result[i] = rank % N;
          rank /= N;
        }
        return result;
      }

      template<int... axis_values, int... VS>
      Ind shift_impl(
        const std::integer_sequence<int, axis_values...>&,
        const std::index_sequence<int, VS...>&
      ) const {
        return {(cord[VS] + axis_values)...};
      }

      template<int... axis_values>
      Ind shift(const std::integer_sequence<int, axis_values...>& offset) const {
        return self_t::shift_impl(offset, std::make_index_sequence<dim>);
      }

      template<int... axis_values>
      void setNbr(const std::integer_sequence<int, axis_values...>& offset){
        this->nbrs[nI_v<axis_values...>] = self_t::indToRank(shift(offset));
      }

      template<class... IS>
      void setNbrs_impl(const std::tuple<IS...>&){
        for(auto& offset : {(IS())... }){
          setNbr(offset);
        }
      }

      template<class... IS>
      void setNbrs(){
        cord = self_t::rankToInd(rank);
        setNbrs_impl(meta::neighbour_stencil<dim>());
      }

      private:

      int nbrs[meta::pow_v<3, dim>];
    };

    template <size_t dim>
    struct CommunicatorData<1, dim> {
      constexpr bool shouldIPrint() {
        return true;
      }
    };

    template <size_t N, size_t dim>
    class Communicator : public CommunicatorData<N, dim>{
      using Real = typename BuildOptions::real;
      using Vect = std::array<Real, dim>;

    public:
      Communicator(int* argc, char** argv[]);
      ~Communicator();

      Vect domainStart();
      Vect domainStop();
    };
  }

  using Communicator = detail::Communicator<config.DOMAIN_SCALE, 3>;

  namespace detail{

    template <size_t N, size_t dim>
    template <int... axis_values>
    void CommunicatorData<N, dim>::sendTo(void* buff, const size_t& size){
      MPI_Send(buff, size, MPI_BYTE, nbrs[nI_v<axis_values...>], 0, MPI_COMM_WORLD);
    }

    template <size_t N, size_t dim>
    template <int... axis_values>
    void CommunicatorData<N, dim>::recvFrom(void* buff, const size_t& size){
      MPI_Recv(buff, size, MPI_BYTE, nbrs[nI_v<axis_values...>], MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    template <size_t N, size_t dim>
    Communicator<N, dim>::Communicator(int* argc, char** argv[]) {
      constexpr static size_t numproc = meta::pow_v<N, dim>;

      int runtime_numproc, supported_status;

      MPI_Init_thread(argc, argv, MPI_THREAD_MULTIPLE, &supported_status);

      if(supported_status != MPI_THREAD_MULTIPLE){
        std::cerr << "ERROR: MPI implementation does not support thread"
          << "-concurrent communication; remove this call if strictly necessary."
          << " Goodbye!" << std::endl;
        _Exit(EXIT_FAILURE);
      }

      MPI_Comm_size(MPI_COMM_WORLD, &runtime_numproc);

      if(runtime_numproc != numproc){
        std::cerr << "ERROR: Expecting " << numproc << " processes, but getting "
          << runtime_numproc << ". Goodbye!" << std::endl;
        _Exit(EXIT_FAILURE);
      }

      MPI_Comm_rank(MPI_COMM_WORLD, &this->rank);

      this->setNbrs();
    }

    template <size_t dim>
    Communicator<1, dim>::Communicator(int* argc, char** argv[]) {};

    template <size_t N, size_t dim>
    Communicator<N, dim>::::~Communicator(){
      MPI_Finalize();
    }

    template <size_t dim>
    Communicator<1, dim>::~Communicator() {};

    template <size_t N>
    auto Communicator<N>::domainStart() -> Vect{
      return {((Real) this->cord[0]) / N, ((Real) this->cord[1]) / N, ((Real) this->cord[2]) / N};
    }

    template <>
    auto Communicator<1>::domainStart() -> Vect{
      return {(Real) 0, (Real) 0, (Real) 0};
    }

    template <size_t N>
    auto Communicator<N>::domainStop() -> Vect{
      return {((Real) this->cord[0] + 1) / N, ((Real) this->cord[1] + 1) / N, ((Real) this->cord[2] + 1) / N};
    }

    template <>
    auto Communicator<1>::domainStop() -> Vect{
      return {(Real) 1, (Real) 1, (Real) 1};
    }
  }
}

#endif
