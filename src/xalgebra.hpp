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
    constexpr static size_t NDIMS = sizeof...(XS);
    using array_t = std::array<T, NDIMS>;
    using self_t = xfield<T, buff_len, XS...>;
    using shape_t = xshape<NDIMS, XS...>;
    using storage_shape_t = xshape<NDIMS, (XS + 2 * buff_len)...>;
    using storage_t = xtensor_fixed<T, storage_shape_t>;

  public:
    void fillTG(const array_t& start, const array_t& end);

    void importBuffer(const size_t &axis, const int &dir,T* buff);

    void exportBuffer(const size_t &axis, const int &dir,T* buff);

  private:
    template<size_t... IS>
    void fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&);

    template<size_t... IS>
    void importBuffer_impl(const size_t &axis, const int &dir, T* buff, const std::index_sequence<IS...>&);

    template<size_t... IS>
    void exportBuffer_impl(const size_t &axis, const int &dir, T* buff, const std::index_sequence<IS...>&);

    friend std::ostream & operator<<(std::ostream &os, const self_t& arg)
    {
      auto data_view = view(arg.data, all(), range(buff_len, XS + buff_len)...);
      return os << data_view;
    }

    storage_t data;
  };

  namespace meta{
    template <size_t... XS, size_t... IS>
    size_t drop_prod(const size_t &axis, const std::index_sequence<IS...>&){
      return (((IS == axis) ? 1 : XS) * ...);
    }
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t... IS>
  void xfield<T, buff_len, XS...>::exportBuffer_impl(const size_t &axis, const int &dir, T* buff, const std::index_sequence<IS...>& seq){
    size_t buffer_size = buff_len * NDIMS * meta::drop_prod<XS...>(axis, seq);

    auto buffer_view = adapt(buff, buffer_size, no_ownership(),
      std::array<size_t, NDIMS + 1>({NDIMS, ((IS == axis) ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      range(
        (IS != axis || (dir == 1)) ? buff_len : XS,
        (IS != axis || (dir == -1)) ? (XS + buff_len) : (2 * buff_len)
      )...
    );

    noalias(buffer_view) = data_view;
  }

  template<typename T, size_t buff_len, size_t... XS>
  void xfield<T, buff_len, XS...>::exportBuffer(const size_t &axis, const int &dir, T* buff){
    exportBuffer_impl(axis, dir, buff, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t buff_len, size_t... XS>
  template<size_t... IS>
  void xfield<T, buff_len, XS...>::importBuffer_impl(const size_t &axis, const int &dir, T* buff, const std::index_sequence<IS...>& seq){
    size_t buffer_size = buff_len * NDIMS * meta::drop_prod<XS...>(axis, seq);

    auto buffer_view = adapt(buff, buffer_size, no_ownership(),
      std::array<size_t, NDIMS + 1>({NDIMS, ((IS == axis) ? buff_len : XS)... })
    );
    auto data_view = view(data, all(),
      range(
        (IS != axis) ? buff_len : ((dir == 1) ? (XS + buff_len) : 0),
        (IS != axis) ? (XS + buff_len) : ((dir == 1) ? (XS + 2 * buff_len) : buff_len)
      )...
    );

    noalias(data_view) = buffer_view;
  }

  template<typename T, size_t buff_len, size_t... XS>
  void xfield<T, buff_len, XS...>::importBuffer(const size_t &axis, const int &dir, T* buff){
    importBuffer_impl(axis, dir, buff, std::make_index_sequence<sizeof...(XS)>());
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
