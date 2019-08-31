#ifndef SPACE_HPP_
#define SPACE_HPP_

#include <iostream>
#include <type_traits>
#include "xtensor/xfixed.hpp"
#include "xtensor/xrandom.hpp"
#include "xtensor/xio.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xarray.hpp"
#include "xtensor/xnoalias.hpp"
#include "xtensor/xadapt.hpp"

namespace space{
  using namespace xt;

  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  class field{
    using self_t = field<T, buff_len, rank, XS...>;

  protected:
    static constexpr size_t NDIMS = sizeof...(XS);
    using shape_t = xshape<rank, XS...>;
    using storage_shape_t = xshape<rank, (XS + 2 * buff_len)...>;
    using storage_t = xtensor_fixed<T, storage_shape_t>;
    using result_t = xtensor_fixed<T, shape_t>;

  public:
    void fillValue(const T& val);

    template<size_t axis>
    static constexpr size_t bufferSize();

    template<size_t axis, int dir>
    void importBuffer(T* buff);

    template<size_t axis, int dir>
    void exportBuffer(T* buff) const;

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

  private:

    template<size_t axis, int dir, size_t... IS>
    void importBuffer_impl(T* buff, const std::index_sequence<IS...>&);

    template<size_t axis, int dir, size_t... IS>
    void exportBuffer_impl(T* buff, const std::index_sequence<IS...>&) const;

    friend std::ostream& operator<<(std::ostream& os, const self_t& arg){
      auto data_view = view(arg.data, all(), range(buff_len, XS + buff_len)...);
      return os << data_view;
    }

  protected:
    storage_t data;
  };


  template<typename T, size_t buff_len, size_t... XS>
  class scalar_field : public field<T, buff_len, 1, XS...> {
    using self_t = scalar_field<T, buff_len, XS...>;

  public:
    template<class... Args>
    auto at(Args... args) -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, T&>{
      return this->data(0, (args + buff_len)...);
    }

    template<class... Args>
    auto at(Args... args) const -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, const T&>{
      return this->data(0, (args + buff_len)...);
    }
  };


  template<typename T, size_t buff_len, size_t... XS>
  class vector_field : public field<T, buff_len, sizeof...(XS), XS...> {
    using self_t = vector_field<T, buff_len, XS...>;
    using array_t = std::array<T, self_t::NDIMS>;

  public:
    void fillTG(const array_t& start, const array_t& end);

    template<class... Args>
    auto at(int component, Args... args) -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, T&>{
      return this->data(component, (args + buff_len)...);
    }

    template<class... Args>
    auto at(int component, Args... args) const -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, const T&>{
      return this->data(component, (args + buff_len)...);
    }

  private:
    template<size_t... IS>
    void fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&);
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

  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  template<size_t axis>
  constexpr size_t field<T, buff_len, rank, XS...>::bufferSize(){
    return buff_len * rank * meta::drop_prod<axis, XS...>::value;
  }

  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  template<size_t axis, int dir, size_t... IS>
  void field<T, buff_len, rank, XS...>::exportBuffer_impl(T* buff, const std::index_sequence<IS...>& seq) const{
    auto buffer_view = adapt(buff, bufferSize<axis>(), no_ownership(),
      std::array<size_t, NDIMS + 1>({rank, ((IS == axis) ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      ((IS != axis)
        ? range(buff_len, buff_len + XS)
        : (dir == 1) ? range(XS, XS + buff_len) : range(buff_len, 2 * buff_len)
      )...
    );
    noalias(buffer_view) = data_view;
  }

  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  template<size_t axis, int dir>
  void field<T, buff_len, rank, XS...>::exportBuffer(T* buff) const{
    exportBuffer_impl<axis, dir>(buff, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  template<size_t axis, int dir, size_t... IS>
  void field<T, buff_len, rank, XS...>::importBuffer_impl(T* buff, const std::index_sequence<IS...>& seq){
    auto buffer_view = adapt(buff, bufferSize<axis>(), no_ownership(),
      std::array<size_t, NDIMS + 1>({rank, ((IS == axis) ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      ((IS != axis)
        ? range(buff_len, buff_len + XS)
        : (dir == 1) ? range(XS + buff_len, XS + 2 * buff_len) :  range(0, buff_len)
      )...
    );
    noalias(data_view) = buffer_view;
  }

  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  template<size_t axis, int dir>
  void field<T, buff_len, rank, XS...>::importBuffer(T* buff){
    importBuffer_impl<axis, dir>(buff, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t... IS>
  void vector_field<T, buff_len, XS...>::fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&){

    using std::get;
    static const T pi = acos(-1);
    const array_t len = {(end[IS] - start[IS])... };

    auto mesh = meshgrid(linspace<T>(pi * (2 * start[IS] + len[IS] / XS), pi * (2 * end[IS] - len[IS] / XS), XS)... );

    auto X = cos(get<2>(mesh)) * sin(get<1>(mesh)) * sin(get<0>(mesh));
    auto Y = -0.5 * cos(get<1>(mesh)) * sin(get<0>(mesh)) * sin(get<2>(mesh));
    auto Z = -0.5 * cos(get<0>(mesh)) * sin(get<2>(mesh)) * sin(get<1>(mesh));

    auto data_view = view(this->data, all(), range(buff_len, XS + buff_len)... );

    noalias(data_view) = stack(xtuple(X, Y, Z));
  }


  template<typename T, size_t buff_len, size_t... XS>
  void vector_field<T, buff_len, XS...>::fillTG(const array_t& start, const array_t& end){
    fillTG_impl(start, end, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t buff_len, size_t rank, size_t... XS>
  void field<T, buff_len, rank, XS...>::fillValue(const T& val){
    view(data, all(), range(buff_len, XS + buff_len)...).fill(val);
  }
}

#endif
