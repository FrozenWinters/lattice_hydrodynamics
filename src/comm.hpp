#ifndef COMM_HPP_
#define COMM_HPP_

#include "space.hpp"
#include "distributed.hpp"
#include <future>

namespace distributed{

  namespace detail{
    template<size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    struct _Comm_Tasks{
      template<size_t axis, int dir>
      static void _task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm);

      template<size_t axis, int dir>
      static void _task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm);
    };

    template<typename T, size_t buff_len, size_t rank, size_t... XS>
    struct _Comm_Tasks<1, T, buff_len, rank, XS...>{
      template<size_t axis, int dir>
      static void _task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm);

      template<size_t axis, int dir>
      static void _task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm);
    };

    template<size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    template<size_t axis, int dir>
    void _Comm_Tasks<N, T, buff_len, rank, XS...>::_task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm){
      constexpr size_t len = space::field<T, buff_len, rank, XS...>::template bufferSize<axis>();
      T buff[len];
      A.template exportBuffer<axis, dir>(buff);
      comm.sendToAdjacent(buff, sizeof(T) * len, axis, dir);
    }

    template<typename T, size_t buff_len, size_t rank, size_t... XS>
    template<size_t axis, int dir>
    void _Comm_Tasks<1, T, buff_len, rank, XS...>::_task_export(const space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm) {};

    template<size_t N, typename T, size_t buff_len, size_t rank, size_t... XS>
    template<size_t axis, int dir>
    void _Comm_Tasks<N, T, buff_len, rank, XS...>::_task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<N>& comm){
      constexpr size_t len = space::field<T, buff_len, rank, XS...>::template bufferSize<axis>();
      T buff[len];
      comm.recvFromAdjacent(buff, sizeof(T) * len, axis, dir);
      A.template importBuffer<axis, dir>(buff);
    }

    template<typename T, size_t buff_len, size_t rank, size_t... XS>
    template<size_t axis, int dir>
    void _Comm_Tasks<1, T, buff_len, rank, XS...>::_task_import(space::field<T, buff_len, rank, XS...>& A, Communicator<1>& comm){
      constexpr size_t len = space::field<T, buff_len, rank, XS...>::template bufferSize<axis>();
      T buff[len];
      A.template exportBuffer<axis, -1 * dir>(buff);
      A.template importBuffer<axis, dir>(buff);
    }

    template<size_t N, typename T, size_t buff_len, size_t rank, size_t... XS, size_t... IS>
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


  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  void communicateBuffers(space::field<T, buff_len, rank, XS...>& A, Communicator& comm){
    detail::communicateBuffers_impl(A, comm, std::make_index_sequence<sizeof...(XS)>());
  }
}

#endif
