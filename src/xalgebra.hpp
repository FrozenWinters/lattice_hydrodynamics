#ifndef XALGEBRA_HPP_
#define XALGEBRA_HPP_

#include <iostream>
#include "xtensor/xfixed.hpp"
#include "xtensor/xrandom.hpp"
#include "xtensor/xio.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xarray.hpp"
#include "xtensor/xnoalias.hpp"
#include "xtensor/xadapt.hpp"

namespace algebra{
  using namespace xt;

  //Compile tyme info via errors
  //Usage: TD<declype(var)> info;
  template<typename T>
  struct TD;

  template<typename T, size_t buff_len, size_t... XS>
  class xfield{
    static constexpr size_t NDIMS = sizeof...(XS);
    using array_t = std::array<T, NDIMS>;
    using self_t = xfield<T, buff_len, XS...>;
    using shape_t = xshape<NDIMS, XS...>;
    using storage_shape_t = xshape<NDIMS, (XS + 2 * buff_len)...>;
    using storage_t = xtensor_fixed<T, storage_shape_t>;
    using result_t = xtensor_fixed<T, shape_t>;

  public:
    void fillTG(const array_t& start, const array_t& end);

    template<size_t axis>
    static constexpr size_t bufferSize();

    template<size_t axis, int dir>
    void importBuffer(T* buff);

    template<size_t axis, int dir>
    void exportBuffer(T* buff) const;

    void test();

  private:

    template<size_t... IS>
    void fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&);

    template<size_t axis, int dir, size_t... IS>
    void importBuffer_impl(T* buff, const std::index_sequence<IS...>&);

    template<size_t axis, int dir, size_t... IS>
    void exportBuffer_impl(T* buff, const std::index_sequence<IS...>&) const;

    friend std::ostream& operator<<(std::ostream& os, const self_t& arg){
      auto data_view = view(arg.data, all(), range(buff_len, XS + buff_len)...);
      return os << data_view;
    }

    template<size_t... IS>
    void print_buffer_impl(size_t axis, int dir, const std::index_sequence<IS...>&){
      auto data_view = view(data, all(),
        ((IS != axis)
          ? range(buff_len, buff_len + XS)
          : (dir == 1) ? range(XS + buff_len, XS + 2 * buff_len) :  range(0, buff_len)
        )...
      );
      std::cout << "Buffer from axis: " << axis << " dir: " << dir << std::endl;
      std::cout << data_view << std::endl;
    }

    void print_buffer(size_t axis, int dir){
      print_buffer_impl(axis, dir, std::make_index_sequence<sizeof...(XS)>());
    }

    static constexpr size_t h = 1;

    template<size_t... IS>
    auto offset_view_impl(const size_t& axis, const int& val, const std::index_sequence<IS...>&);

    auto offset_view(const size_t& axis, const int& val);

    // template<size_t... IS>
    // auto offset_view_impl(const size_t& axis, const int& val, const std::index_sequence<IS...>&);

    auto partial1(const size_t& axis){
       return (offset_view(axis, 1) - offset_view(axis, -1)) / (2 * h);
    }
    auto partial1(const size_t& axis, const size_t& var){
       return view(offset_view(axis, 1) - offset_view(axis, -1), var) / (2 * h);
    }
    auto partial2(const size_t& axis){
      return (offset_view(axis, -2) - 2*offset_view(axis, 0) - offset_view(axis, -1)) / (4 * h * h);
    }


    template<size_t... IS>
    auto grad_impl(const std::index_sequence<IS...>&){
      return stack(xtuple(partial1(IS)...));
    }

    auto grad(){
      return grad_impl(std::make_index_sequence<NDIMS>());
    }

    template<size_t... IS>
    auto div_impl(const std::index_sequence<IS...>&){
      return (partial1(IS, IS) + ...);
    }

    auto div(){
      return div_impl(std::make_index_sequence<NDIMS>());
    }

    auto curl(){
      // assert(NDIMS == 3)
      return stack(xtuple(
        partial1(1, 2) - partial1(2, 1),
        partial1(2, 0) - partial1(0, 2),
        partial1(0, 1) - partial1(1, 0)
      ));
    }

    template<size_t... IS>
    auto laplace_impl(const std::index_sequence<IS...>&){
      return (partial2(IS) + ...);
    }

    auto laplace(){
      return laplace_impl(std::make_index_sequence<NDIMS>());
    }




    storage_t data;
  };

  namespace meta{
    template<size_t axis, typename, size_t... XS>
    struct drop_prod_impl;


    // More concise implementation using fold, not supported on the intel v18 compiler
    /*template<size_t axis, size_t... XS, size_t... IS>
    struct drop_prod_impl<axis, std::index_sequence<IS...>, XS...>{
        static constexpr size_t value = (((IS == axis) ? 1 : XS) * ...);
    };*/

    template<size_t axis>
    struct drop_prod_impl<axis, std::index_sequence<>>{
        static constexpr size_t value = 1;
    };

    template<size_t axis, size_t X, size_t... XS, size_t I, size_t... IS>
    struct drop_prod_impl<axis, std::index_sequence<I, IS...>, X, XS...>{
        static constexpr size_t value = ((I == axis) ? 1 : X) * drop_prod_impl<axis, std::index_sequence<IS...>, XS...>::value;
    };

    template<size_t axis, size_t... XS>
    struct drop_prod : drop_prod_impl<
      axis, decltype(std::make_index_sequence<sizeof...(XS)>()), XS...
    > {};
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t axis>
  constexpr size_t xfield<T, buff_len, XS...>::bufferSize(){
    return buff_len * NDIMS * meta::drop_prod<axis, XS...>::value;
  }


  template<typename T, size_t buff_len, size_t... XS>
  void xfield<T, buff_len, XS...>::test(){

      // data =

      T buffer[(int) 1e6];



      // dVdt();
      data *= 0;
      data += 1337;


      int val = 0;
      view(data, 0, range(buff_len+1, buff_len+2), range(buff_len+1, buff_len+2), range(buff_len+1, buff_len+2)) = val++;
      view(data, 0, range(buff_len, buff_len+1), range(buff_len+1, buff_len+2), range(buff_len+1, buff_len+2)) = val++;
      view(data, 0, range(buff_len+2, buff_len+3), range(buff_len+1, buff_len+2), range(buff_len+1, buff_len+2)) = val++;
      view(data, 0, range(buff_len+1, buff_len+2), range(buff_len, buff_len+1), range(buff_len+1, buff_len+2)) = val++;
      view(data, 0, range(buff_len+1, buff_len+2), range(buff_len+2, buff_len+3), range(buff_len+1, buff_len+2)) = val++;

      std::cout << view(data, 0) << std::endl << std::endl;


      for (size_t i = 0; i < NDIMS; i++) {
        for (auto j: {-1, 1}) {
          exportBuffer(i, +j, buffer);
          importBuffer(i, -j, buffer);


          std::cout << view(data, 0) << std::endl << std::endl;


          view(data, 0, range(buff_len, buff_len+3), range(buff_len, buff_len+3)) +=10;
        }
      }


      std::cout << view(data, range(0, 1), all(), all()) << std::endl << std::endl;
      std::cout << offset_view(1, 00)[{1,2,3}] << std::endl << std::endl;
      std::cout << view(offset_view(1, -1), range(0, 1), all(), all()) << std::endl << std::endl;
      std::cout << view(offset_view(1, +1), range(0, 1), all(), all()) << std::endl << std::endl;

      std::cout << "d/dx:\n" << partial1(0) << std::endl << std::endl;
      std::cout << "d/dy:\n" << partial1(1) << std::endl << std::endl;
      std::cout << "div:\n" << div() << std::endl << std::endl;
      std::cout << "grad:\n" << grad() << std::endl << std::endl;
      std::cout << "laplace:\n" << laplace() << std::endl << std::endl;
      // std::cout << offset_view(0, -2) << std::endl << std::endl;
      // std::cout << offset_view(0, 2) << std::endl << std::endl
      // std::cout << offset_view(0, 0) << std::endl << std::endl;
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t... IS>
  auto xfield<T, buff_len, XS...>::offset_view_impl(const size_t& axis, const int& val, const std::index_sequence<IS...>&){
    return view(data, all(),
      ((IS != axis)
        ? range(buff_len, buff_len + XS)
        : range(buff_len + val, buff_len + XS + val)
      )...
    );
  }

  template<typename T, size_t buff_len, size_t... XS>
  auto xfield<T, buff_len, XS...>::offset_view(const size_t& axis, const int& val){
    return offset_view_impl(axis, val, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t axis, int dir, size_t... IS>
  void xfield<T, buff_len, XS...>::exportBuffer_impl(T* buff, const std::index_sequence<IS...>& seq) const{
    auto buffer_view = adapt(buff, bufferSize<axis>(), no_ownership(),
      std::array<size_t, NDIMS + 1>({NDIMS, ((IS == axis) ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      ((IS != axis)
        ? range(buff_len, buff_len + XS)
        : (dir == 1) ? range(XS, XS + buff_len) : range(buff_len, 2 * buff_len)
      )...
    );
    noalias(buffer_view) = data_view;
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t axis, int dir>
  void xfield<T, buff_len, XS...>::exportBuffer(T* buff) const{
    exportBuffer_impl<axis, dir>(buff, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t axis, int dir, size_t... IS>
  void xfield<T, buff_len, XS...>::importBuffer_impl(T* buff, const std::index_sequence<IS...>& seq){
    auto buffer_view = adapt(buff, bufferSize<axis>(), no_ownership(),
      std::array<size_t, NDIMS + 1>({NDIMS, ((IS == axis) ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      ((IS != axis)
        ? range(buff_len, buff_len + XS)
        : (dir == 1) ? range(XS + buff_len, XS + 2 * buff_len) :  range(0, buff_len)
      )...
    );
    noalias(data_view) = buffer_view;
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t axis, int dir>
  void xfield<T, buff_len, XS...>::importBuffer(T* buff){
    importBuffer_impl<axis, dir>(buff, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t... IS>
  void xfield<T, buff_len, XS...>::fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&){

    using std::get;
    static const T pi = acos(-1);
    const array_t len = {(end[IS] - start[IS])... };

    auto mesh = meshgrid(linspace<T>(pi * (2 * start[IS] + len[IS] / XS), pi * (2 * end[IS] - len[IS] / XS), XS)... );

    auto X = cos(get<2>(mesh)) * sin(get<1>(mesh)) * sin(get<0>(mesh));
    auto Y = -0.5 * cos(get<1>(mesh)) * sin(get<0>(mesh)) * sin(get<2>(mesh));
    auto Z = -0.5 * cos(get<0>(mesh)) * sin(get<2>(mesh)) * sin(get<1>(mesh));

    auto data_view = view(data, all(), range(buff_len, XS + buff_len)... );

    noalias(data_view) = stack(xtuple(X, Y, Z));
  }


  template<typename T, size_t buff_len, size_t... XS>
  void xfield<T, buff_len, XS...>::fillTG(const array_t& start, const array_t& end){
    fillTG_impl(start, end, std::make_index_sequence<sizeof...(XS)>());
  }
}

#endif
