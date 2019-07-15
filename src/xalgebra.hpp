#include <iostream>
#include "xtensor/xfixed.hpp"
#include "xtensor/xrandom.hpp"
#include "xtensor/xio.hpp"
#include "xtensor/xview.hpp"
#include "xtensor/xarray.hpp"
#include "xtensor/xnoalias.hpp"

using std::get;
using namespace xt;


//Compile tyme info via errors
//Usage: TD<declype(var)> info;
template<typename T>
struct TD;

template<typename T, size_t L, size_t W, size_t H>
class xfield{
  using self_t = xfield<T, L, W, H>;
  using shape_t = xshape<3, L, W, H>;
  using storage_shape_t = xshape<3, L+4, W+4, H+4>;
  using storage_t = xtensor_fixed<T, storage_shape_t>;

public:
  void fillTG(const T&, const T&, const T&, const T&, const T&, const T&);

private:
  friend std::ostream & operator<<(std::ostream &os, const self_t& arg)
  {
    auto data_view = view(arg.data, all(), range(2, L + 2), range(2, W + 2), range(2, H + 2));
    return os << data_view;
  }

  storage_t data;
};

template<typename T, size_t L, size_t W, size_t H>
void xfield<T, L, W, H>::fillTG(const T& x_0, const T& x_1, const T& y_0, const T& y_1, const T& z_0, const T& z_1){
  static const T pi = acos(-1);

  auto x_space = linspace<T>(x_0 * (pi / L), x_1 * (2 * pi - pi / L), L);
  auto y_space = linspace<T>(y_0 * (pi / W), y_1 * (2 * pi - pi / W), W);
  auto z_space = linspace<T>(z_0 * (pi / H), z_1 * (2 * pi - pi / H), H);

  auto mesh = meshgrid(z_space, y_space, x_space);

  auto X = cos(get<2>(mesh)) * sin(get<1>(mesh)) * sin(get<0>(mesh));
  auto Y = -0.5 * cos(get<1>(mesh)) * sin(get<0>(mesh)) * sin(get<2>(mesh));
  auto Z = -0.5 * cos(get<0>(mesh)) * sin(get<2>(mesh)) * sin(get<1>(mesh));

  auto data_view = view(data, all(), range(2, L + 2), range(2, W + 2), range(2, H + 2));

  noalias(data_view) = stack(xtuple(X, Y, Z));
}
