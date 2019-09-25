
// This file implements the communication agency, employing agent taks
// which run send and recieve errands. The agency does the planning for you,
// as it's a good and propoerly managed agency; you simply bring it the data
// and communication chanel, and by the time you leave the door
// (i.e. when the call returns), you may rest assured that the work is done.

#ifndef AGENCY_HPP_
#define AGENCY_HPP_

#include "space.hpp"
#include "distributed.hpp"
#include "stencil.hpp"
#include <future>

namespace distributed{

  namespace detail{
    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    struct _Comm_Tasks{
      // using self_t = _Comm_Tasks<N, T, buff_len, rank, XS...>;
      static constexpr size_t NDIMS = sizeof...(XS);

      template <int... axis_values>
      static void _task_export(
        const space::field<T, buff_len, rank, XS...>& A,
        Communicator<N, NDIMS>& comm
      );

      template <int... axis_values>
      static void _task_import(
        space::field<T, buff_len, rank, XS...>& A,
        Communicator<N, NDIMS>& comm
      );
    };

    // This is repeated for the reason explained in the file distributed.hpp
    template <typename T, size_t buff_len, size_t rank, size_t... XS>
    struct _Comm_Tasks<1, T, buff_len, rank, XS...>{
      // using self_t = _Comm_Tasks<1, T, buff_len, rank, XS...>;
      static constexpr size_t NDIMS = sizeof...(XS);

      template <int... axis_values>
      static void _task_export(
        const space::field<T, buff_len, rank, XS...>& A,
        Communicator<1, NDIMS>& comm
      );

      template <int... axis_values>
      static void _task_import(
        space::field<T, buff_len, rank, XS...>& A,
        Communicator<1, NDIMS>& comm
      );
    };

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<N, T, buff_len, rank, XS...>::_task_export(
      const space::field<T, buff_len, rank, XS...>& A,
      Communicator<N, NDIMS>& comm
    ){
      constexpr size_t len =
        space::field<T, buff_len, rank, XS...>
        ::template bufferSize<axis_values...>();
      T buff[len];
      A.template exportBuffer<axis_values...>(buff);
      comm.template sendTo<axis_values...>(buff, sizeof(T) * len);
    }

    template <typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<1, T, buff_len, rank, XS...>::_task_export(
      const space::field<T, buff_len, rank, XS...>& A,
      Communicator<1, NDIMS>& comm
    ) {};

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<N, T, buff_len, rank, XS...>::_task_import(
      space::field<T, buff_len, rank, XS...>& A,
      Communicator<N, NDIMS>& comm
    ){
      constexpr size_t len =
        space::field<T, buff_len, rank, XS...>
        ::template bufferSize<axis_values...>();
      T buff[len];
      comm.template recvFrom<axis_values...>(buff, sizeof(T) * len);
      A.template importBuffer<axis_values...>(buff);
    }

    template <typename T, size_t buff_len, size_t rank, size_t... XS>
    template <int... axis_values>
    void _Comm_Tasks<1, T, buff_len, rank, XS...>::_task_import(
      space::field<T, buff_len, rank, XS...>& A, Communicator<1, NDIMS>& comm
    ){
      constexpr size_t len =
        space::field<T, buff_len, rank, XS...>
        ::template bufferSize<axis_values...>();
      T buff[len];
      A.template exportBuffer<(- axis_values)...>(buff);
      A.template importBuffer<axis_values...>(buff);
    }

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS, int... axis_values>
    auto launchExport(
      space::field<T, buff_len, rank, XS...>& A,
      Communicator<N, sizeof...(XS)>& comm,
      const std::integer_sequence<int, axis_values...>&
    ){
      return std::async(
        std::launch::async,
        _Comm_Tasks<N, T, buff_len, rank, XS...>::template _task_export<axis_values...>,
        std::ref(A), std::ref(comm)
      );
    }

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS, int... axis_values>
    auto launchImport(
      space::field<T, buff_len, rank, XS...>& A,
      Communicator<N, sizeof...(XS)>& comm,
      const std::integer_sequence<int, axis_values...>&
    ){
      return std::async(
        std::launch::async,
        _Comm_Tasks<N, T, buff_len, rank, XS...>::template _task_import<axis_values...>,
        std::ref(A), std::ref(comm)
      );
    }

    template <size_t N, typename T, size_t buff_len, size_t rank, size_t... XS, class... IS>
    void communicateBuffers_impl(
      space::field<T, buff_len, rank, XS...>& A,
      Communicator<N, sizeof...(XS)>& comm,
      const std::tuple<IS...>&
    ){
      for(auto& task : {launchExport(A, comm, IS())..., launchImport(A, comm, IS())...}) {
        task.wait();
      }
    }
  }

  template <typename T, size_t buff_len, size_t rank, size_t... XS>
  void communicateBuffers(space::field<T, buff_len, rank, XS...>& A, Communicator& comm){
    detail::communicateBuffers_impl(A, comm, meta::strict_neighbour_stencil<sizeof...(XS)>());
  }
}

#endif
