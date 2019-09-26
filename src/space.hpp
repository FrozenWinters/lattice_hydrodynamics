
// This file contains an answer to the abstract question of:
// "What is a buffered container?"
// It fills in the semantics of the dasein of such a container.
// Such as, what such a container does? How does it sepnd it's time?

#ifndef SPACE_HPP_
#define SPACE_HPP_

#include <iostream>
#include <type_traits>
#include "xtensor/xfixed.hpp"
#include "xtensor/xio.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xarray.hpp"
#include "xtensor/xnoalias.hpp"
#include "xtensor/xadapt.hpp"

namespace meta{
  // The stl helper template index_sequence_for, takes a type parameter pack.
  // We want this for value parameter packs.
  template<size_t... XS>
  using index_sequence_for = std::make_index_sequence<sizeof...(XS)>;

  template <class IXS, class IAS, size_t replacing_value>
  struct replace_prod;

  // More concise implementation using fold, not supported on the intel v18 compiler
  /*template <size_t... XS, int... AS, size_t replacing_value>
  struct replace_prod<std::index_sequence<XS...>, std::integer_sequence<int, AS...>, replacing_value>{
    static constexpr size_t value = ((AS ? replacing_value : XS) * ...);
  };*/

  template <size_t replacing_value>
  struct replace_prod<std::index_sequence<>, std::integer_sequence<int>, replacing_value>{
    static constexpr size_t value = 1;
  };

  // When A is zero we iclude the factor.
  // In practice the non-zero values will be +1 or -1 depending on which side we use.
  template <size_t X, size_t... XS, int A, int... AS, size_t replacing_value>
  struct replace_prod<std::index_sequence<X, XS...>, std::integer_sequence<int, A, AS...>, replacing_value>{
    static constexpr size_t value = (A ? replacing_value : X)
      * replace_prod<std::index_sequence<XS...>, std::integer_sequence<int, AS...>, replacing_value>::value;
  };
}

namespace space{
  using namespace xt;

  template <typename T, size_t buff_len, size_t rank, size_t... XS>
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

    template <int... axis_values>
    static constexpr size_t bufferSize();

    template <int... axis_values>
    void exportBuffer(T* buff) const;

    template <int... axis_values>
    void importBuffer(T* buff);

    template<int... axis_values>
    void print_buffer() {
      auto data_view = view(data, all(),
        (axis_values
          ? (axis_values > 0) ? range(XS + buff_len, XS + 2 * buff_len) : range(0, buff_len)
          : range(buff_len, buff_len + XS)
        )...
      );
      std::cout << "Buffer: " << std::endl;
      std::cout << data_view << std::endl;
    }

    void printAll(){
      std::cout << data << std::endl;
    }

  private:

    friend std::ostream& operator<<(std::ostream& os, const self_t& arg){
      auto data_view = view(arg.data, all(), range(buff_len, XS + buff_len)...);
      return os << data_view;
    }

  protected:
    storage_t data;
  };


  template <typename T, size_t buff_len, size_t... XS>
  class scalar_field : public field<T, buff_len, 1, XS...> {
    using self_t = scalar_field<T, buff_len, XS...>;

  public:
    template <class... Args>
    auto at(Args... args) -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, T&>{
      return this->data(0, (args + buff_len)...);
    }

    template <class... Args>
    auto at(Args... args) const -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, const T&>{
      return this->data(0, (args + buff_len)...);
    }
  };


  template <typename T, size_t buff_len, size_t... XS>
  class vector_field : public field<T, buff_len, sizeof...(XS), XS...> {
    using self_t = vector_field<T, buff_len, XS...>;
    using array_t = std::array<T, self_t::NDIMS>;

  public:
    void fillTG(const array_t& start, const array_t& end);

    template <class... Args>
    auto at(int component, Args... args) -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, T&>{
      return this->data(component, (args + buff_len)...);
    }

    template <class... Args>
    auto at(int component, Args... args) const -> std::enable_if_t<sizeof...(Args) == self_t::NDIMS, const T&>{
      return this->data(component, (args + buff_len)...);
    }

  private:
    template <size_t... IS>
    void fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&);
  };

  // Notice that this uses every single template parameter apart from T!
  // This partially justifies the existence of these parameters.
  template <typename T, size_t buff_len, size_t rank, size_t... XS>
  template <int... axis_values>
  constexpr size_t field<T, buff_len, rank, XS...>::bufferSize(){
    return rank * meta::replace_prod<std::index_sequence<XS...>, std::integer_sequence<int, axis_values...>, buff_len>::value;
  }

  template <typename T, size_t buff_len, size_t rank, size_t... XS>
  template <int... axis_values>
  void field<T, buff_len, rank, XS...>::exportBuffer(T* buff) const{
    auto buffer_view = adapt(buff, bufferSize<axis_values...>(), no_ownership(),
      std::array<size_t, NDIMS + 1>({rank, ((axis_values) ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      (axis_values
        ? (axis_values > 0) ? range(XS, XS + buff_len) : range(buff_len, 2 * buff_len)
        : range(buff_len, buff_len + XS)
      )...
    );
    noalias(buffer_view) = data_view;
  }

  template <typename T, size_t buff_len, size_t rank, size_t... XS>
  template <int... axis_values>
  void field<T, buff_len, rank, XS...>::importBuffer(T* buff){
    auto buffer_view = adapt(buff, bufferSize<axis_values...>(), no_ownership(),
      std::array<size_t, NDIMS + 1>({rank, (axis_values ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      (axis_values
        ? (axis_values > 0) ? range(XS + buff_len, XS + 2 * buff_len) : range(0, buff_len)
        : range(buff_len, buff_len + XS)
      )...
    );
    noalias(data_view) = buffer_view;
  }

  template <typename T, size_t buff_len, size_t... XS>
  template <size_t... IS>
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


  template <typename T, size_t buff_len, size_t... XS>
  void vector_field<T, buff_len, XS...>::fillTG(const array_t& start, const array_t& end){
    fillTG_impl(start, end, meta::index_sequence_for<XS...>());
  }

  template <typename T, size_t buff_len, size_t rank, size_t... XS>
  void field<T, buff_len, rank, XS...>::fillValue(const T& val){
    view(data, all(), range(buff_len, XS + buff_len)...).fill(val);
  }
}

#endif
