#include "xalgebra.hpp"
#include "distributed.h"
#include <future>

namespace distributed{

  namespace detail{
    templete<size_t axis, int dir, typename T, size_t buff_len, size_t... XS>
    void _task_export(const xfield<T, buff_len, XS...>& A, Communicator& comm){
      constexpr size_t len = xfield<T, buff_len, XS...>::bufferSize<axis>();
      T buff[len];
      A.exportBuffer<axis, dir>(buff);
      comm.sendToAdjacent(buff, sizeof(T) * len, axis, dir);
    }

    templete<typename T, size_t buff_len, size_t... XS>
    void _task_import(xfield<T, buff_len, XS...>& A, const int& axis, const int& dir, Communicator& comm){
      constexpr size_t len = xfield<T, buff_len, XS...>::bufferSize<axis>();
      T buff[len];
      comm.recvFromAdjacent(buff, sizeof(T) * len, axis, dir);
      A.importBuffer<axis, dir>(buff);
    }

    templete<typename T, size_t buff_len, size_t... XS, size_t.. IS>
    void communicateBuffers_impl(xfield<T, buff_len, XS...>& A, Communicator& comm, const std::index_sequence<IS...>&){
      for(auto& task :
        {std::async(_task_export<IS, +1>, A, comm, std::launch::async)...,
        std::async(_task_export<IS, -1>, A, comm, std::launch::async)...,
        std::async(_task_import<IS, +1>, A, comm, std::launch::async)...,
        std::async(_task_import<IS, -1>, A, comm, std::launch::async)...}
      ) {
        task.wait();
      }
    }
  }


  templete<typename T, size_t buff_len, size_t... XS
  void communicateBuffers(xfield<T, buff_len, XS...>& A, Communicator& comm){
    detail::communicateBuffers_impl(A, comm, std::make_index_sequence<sizeof...(XS)>());
  }
}
