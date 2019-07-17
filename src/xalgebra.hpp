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

  template<typename T, size_t... XS>
  class xfield{
    constexpr static size_t NDIMS = sizeof...(XS);
    using array_t = std::array<T, NDIMS>;
    using self_t = xfield<T, XS...>;
    using shape_t = xshape<NDIMS, XS...>;
    using storage_shape_t = xshape<NDIMS, (XS + 4)...>;
    using storage_t = xtensor_fixed<T, storage_shape_t>;

  public:
    void fillTG(const array_t& start, const array_t& end);

    void fillBoundaryBuffer(const size_t &axis, const int &dir, T* buff);


    /*template<size_t m>
    void fillBoundaryBuffer(const int &axis, const int &dir, std::array<T, m> &buff);*/

  private:
    template<size_t... IS>
    void fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&);

    template<size_t... IS>
    void fillBoundaryBuffer_impl(const size_t &axis, const int &dir, T* buff, const std::index_sequence<IS...>&);

    friend std::ostream & operator<<(std::ostream &os, const self_t& arg)
    {
      auto data_view = view(arg.data, all(), range(2, XS + 2)...);
      return os << data_view;
    }

    storage_t data;
  };

  /*namespace meta{

    template <size_t I, size_t... XS>
    struct at{
      constexpr static size_t arr[sizeof...(XS)] = {XS...};
      constexpr static size_t value = (IDX < sizeof...(XS)) ? arr[I] : 0;
    };

    template <size_t... XS, size_t... IS>
    size_t drop_prod(const size_t &axis){
      return (((IS == axis) ? 1 : XS) * ...);
    }

    template <size_t I, size_t... X>
    size_t range_begin(size_t axis, int direction){
      if(axis == index || direction == 1){
        return 2;
      } else{
        return (at<index, I...>::value - 1);
      }
    }

    template <size_t I, size_t... X>
    size_t range_end(size_t axis, int direction){
      if(axis == index || direction == 0){
        return 2;
      } else{
        return (at<index, I...>::value - 1);
      }
    }
  }*/

  template<typename T, size_t... XS>
  template<size_t... IS>
  void xfield<T, XS...>::fillBoundaryBuffer_impl(const size_t &axis, const int &dir, T* buff, const std::index_sequence<IS...>&){
    auto A = adapt(buff, std::vector<size_t>({NDIMS, ((IS == axis) ? 2 : XS)... }));
    auto data_view = view(data, all(),
      range(
        (IS == axis || (dir == 1)) ? 2 : (XS + 1),
        (IS == axis || (dir == -1)) ? XS+2 : 4
      )...
    );
    A = data_view;
  }

  template<typename T, size_t... XS>
  void xfield<T, XS...>::fillBoundaryBuffer(const size_t &axis, const int &dir, T* buff){
    fillBoundaryBuffer_impl(axis, dir, buff, std::make_index_sequence<sizeof...(XS)>());
  }

  template<typename T, size_t... XS>
  template<size_t... IS>
  void xfield<T, XS...>::fillTG_impl(const array_t& start, const array_t& end, const std::index_sequence<IS...>&){
    using std::get;
    static const T pi = acos(-1);
    const array_t len = {(end[IS] - start[IS])... };

    auto mesh = meshgrid(linspace<T>(pi * (2 * start[IS] + len[IS] / XS), pi * (2 * end[IS] - len[IS] / XS), XS)... );

    auto X = cos(get<2>(mesh)) * sin(get<1>(mesh)) * sin(get<0>(mesh));
    auto Y = -0.5 * cos(get<1>(mesh)) * sin(get<0>(mesh)) * sin(get<2>(mesh));
    auto Z = -0.5 * cos(get<0>(mesh)) * sin(get<2>(mesh)) * sin(get<1>(mesh));

    auto data_view = view(data, all(), range(2, XS + 2)... );

    noalias(data_view) = stack(xtuple(X, Y, Z));
  }


  template<typename T, size_t... XS>
  void xfield<T, XS...>::fillTG(const array_t& start, const array_t& end){
    fillTG_impl (start, end, std::make_index_sequence<sizeof...(XS)>());
  }
}
