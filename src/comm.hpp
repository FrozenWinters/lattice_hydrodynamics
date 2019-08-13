#include "xalgebra.hpp"
#include "distributed.h"
#include <future>

namespace distributed{

  namespace detail{

    template<size_t axis, int dir, typename T, size_t buff_len, size_t... XS>
    void _task_export(const algebra::xfield<T, buff_len, XS...>& A, Communicator& comm){
      constexpr size_t len = algebra::xfield<T, buff_len, XS...>::template bufferSize<axis>();
      T buff[len];
      A.template exportBuffer<axis, dir>(buff);
      comm.sendToAdjacent(buff, sizeof(T) * len, axis, dir);
    }

    template<size_t axis, int dir, typename T, size_t buff_len, size_t... XS>
    void _task_import(algebra::xfield<T, buff_len, XS...>& A, Communicator& comm){
      constexpr size_t len = algebra::xfield<T, buff_len, XS...>::template bufferSize<axis>();
      T buff[len];
      comm.recvFromAdjacent(buff, sizeof(T) * len, axis, dir);
      A.template importBuffer<axis, dir>(buff);
    }

    template<typename T, size_t buff_len, size_t... XS, size_t... IS>
    void communicateBuffers_impl(algebra::xfield<T, buff_len, XS...>& A, Communicator& comm, const std::index_sequence<IS...>&){
      for(auto& task :
        {
          std::async(std::launch::async, _task_export<IS, +1, T, buff_len, XS...>, std::ref(A), std::ref(comm))...,
          std::async(std::launch::async, _task_export<IS, -1, T, buff_len, XS...>, std::ref(A), std::ref(comm))...,
          std::async(std::launch::async, _task_import<IS, +1, T, buff_len, XS...>, std::ref(A), std::ref(comm))...,
          std::async(std::launch::async, _task_import<IS, -1, T, buff_len, XS...>, std::ref(A), std::ref(comm))...
        }
      ) {
        task.wait();
      }
    }
  }


  template<typename T, size_t buff_len, size_t... XS>
  void communicateBuffers(algebra::xfield<T, buff_len, XS...>& A, Communicator& comm){
    detail::communicateBuffers_impl(A, comm, std::make_index_sequence<sizeof...(XS)>());
  }
}
