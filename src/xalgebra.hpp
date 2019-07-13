#include <iostream>
#include "xtensor/xfixed.hpp"
#include "xtensor/xrandom.hpp"
#include "xtensor/xio.hpp"

using std::get;

template<typename T, size_t L, size_t W, size_t H>
class xfield{
  using field_t = xfield<T, L, W, H>;
  using shape_t = xt::xshape<3, L, W, H>;
  using storage_t = xt::xtensor_fixed<T, shape_t>;

public:

  void fillTG();

private:
  friend std::ostream & operator<<(std::ostream &os, const field_t& arg)
  {
    return os << arg.data;
  }

  storage_t data;
};

template<typename T, size_t L, size_t W, size_t H>
void xfield<T, L, W, H>::fillTG(){
  static const T pi = acos(-1);

  auto x_space = xt::linspace<T>(pi / L, 2 * pi - pi / L, L);
  auto y_space = xt::linspace<T>(pi / W, 2 * pi - pi / W, W);
  auto z_space = xt::linspace<T>(pi / H, 2 * pi - pi / H, H);

  auto mesh = xt::meshgrid(z_space, y_space, x_space);

  auto X = xt::cos(get<2>(mesh)) * xt::sin(get<1>(mesh)) * xt::sin(get<0>(mesh));
  auto Y = -0.5 * xt::cos(get<1>(mesh)) * xt::sin(get<0>(mesh)) * xt::sin(get<2>(mesh));
  auto Z = -0.5 * xt::cos(get<0>(mesh)) * xt::sin(get<2>(mesh)) * xt::sin(get<1>(mesh));

  data = xt::stack(xt::xtuple(X, Y, Z));
}
