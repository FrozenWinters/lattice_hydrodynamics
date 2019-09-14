#ifndef COMM_HPP_
#define COMM_HPP_

#include "space.hpp"
#include "distributed.hpp"
#include <future>

namespace meta{
  template <class TA, class TB>
  struct concatenate;

  class <class... AS, class... BS>
  struct concatenate<std::tuple<AS...>, std::tuple<BS...>> {
    using type = std::tuple<AS..., BS...>;
  };

  template <class... TS>
  struct cartesian_product;

  template <>
  struct cartesian_product<>{
    using type = void;
  }

  template <class TB>
  struct cartesian_product<std::tuple<>, TB> {
    using type = std::tuple<>;
  }

  template <class A, class... AS, class... BS>
  struct cartesian_product<std::tuple<A, AS...>, std::tuple<BS...>> {
    using type =
      concatenate<
        std::tuple< std::tuple<A, BS>... >,
        cartesian_product<std::tuple<AS...>, std::tuple<BS...>>
      >::type;
  };

  /*template <int A, int... AS, int... BS>
  struct cartesian_product<std::integer_sequence<int, A, AS...>, std::integer_sequence<int, BS...>> {
    using type =
  }*/
}



namespace distributed{

  namespace detail{
    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    struct _Comm_Tasks{
      template <int... axis_values>
      static void _task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm);

      template <int... axis_values>
      static void _task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm);
    };

    template <typename T, size_t buff_len, size_t rank, size_t... XS>
    struct _Comm_Tasks<1, T, buff_len, rank, XS...>{
      template <int... axis_values>
      static void _task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm);

      template <int... axis_values>
      static void _task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm);
    };

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<N, T, buff_len, rank, XS...>::_task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm){
      constexpr size_t len = space::field<T, buff_len, rank, XS...>::template bufferSize<axis_values...>();
      T buff[len];
      A.template exportBuffer<axis_values...>(buff);
      comm.sendToAdjacent(buff, sizeof(T) * len, axis_values...);
    }

    template <typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<1, T, buff_len, rank, XS...>::_task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm) {};

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<N, T, buff_len, rank, XS...>::_task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm){
      constexpr size_t len = space::field<T, buff_len, rank, XS...>::template bufferSize<axis_values...>();
      T buff[len];
      comm.recvFromAdjacent(buff, sizeof(T) * len, axis_values...);
      A.template importBuffer<axis_values...>(buff);
    }

    template <typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<1, T, buff_len, rank, XS...>::_task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm){
      constexpr size_t len = space::field<T, buff_len, rank, XS...>::template bufferSize<axis_values...>();
      T buff[len];
      A.template exportBuffer<(- axis_values)...>(buff);
      A.template importBuffer<axis_values...>(buff);
    }

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS, size_t... IS>
    void communicateBuffers_impl(space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm, const std::index_sequence<IS...>&){
      for(auto& task :
        {
          std::async(std::launch::async, _Comm_Tasks<N, T, buff_len, rank, XS...>::template _task_export<IS, +1>, std::ref(A), std::ref(comm))...,
          std::async(std::launch::async, _Comm_Tasks<N, T, buff_len, rank, XS...>::template _task_export<IS, -1>, std::ref(A), std::ref(comm))...,
          std::async(std::launch::async, _Comm_Tasks<N, T, buff_len, rank, XS...>::template _task_import<IS, +1>, std::ref(A), std::ref(comm))...,
          std::async(std::launch::async, _Comm_Tasks<N, T, buff_len, rank, XS...>::template _task_import<IS, -1>, std::ref(A), std::ref(comm))...
        }
      ) {
        task.wait();
      }
    }
  }


  template <typename T, size_t buff_len, size_t rank, size_t... XS>
  void communicateBuffers(space::field<T, buff_len, rank, XS...>& A, Communicator& comm){
    detail::communicateBuffers_impl(A, comm, std::make_index_sequence<sizeof...(XS)>());
  }
}

#endif
