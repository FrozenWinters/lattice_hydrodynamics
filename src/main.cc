#include "algebra.hpp"
#include "xalgebra.hpp"
#include <build_options.h>

using namespace std;

using tensor = state<typename BuildOptions::real, 3, 0, config.L, config.W, config.H>;
using my_tensor = xfield<typename BuildOptions::real, config.L, config.W, config.H>;
//using scalar = state<typename BuildOptions::real, 1, 0, config.L, config.W, config.H>;

/*using my_shape = xt::xshape<3, config.L, config.W, config.H>;
using mat = xt::xtensor_fixed<double, my_shape>;*/

/*void __attribute__((noinline)) computeN(const field& in, field& out){
  in.computeN(out);
}*/

/*template<typename T, size_t L, size_t W, size_t H>
void fill_mat(xt::xtensor_fixed<T, xt::xshape<3, L, W, H>>& dest)
{
  static const T pi = acos(-1);

  auto x_space = xt::linspace<T>(pi / L, 2 * pi - pi / L, L);
  auto y_space = xt::linspace<T>(pi / W, 2 * pi - pi / W, W);
  auto z_space = xt::linspace<T>(pi / H, 2 * pi - pi / H, H);

  auto mesh = xt::meshgrid(z_space, y_space, x_space);

  auto X = xt::cos(get<2>(mesh)) * xt::sin(get<1>(mesh)) * xt::sin(get<0>(mesh));
  auto Y = -0.5 * xt::cos(get<1>(mesh)) * xt::sin(get<0>(mesh)) * xt::sin(get<2>(mesh));
  auto Z = -0.5 * xt::cos(get<0>(mesh)) * xt::sin(get<2>(mesh)) * xt::sin(get<1>(mesh));

  dest = xt::stack(xt::xtuple(X, Y, Z));
}*/

int main(){

  my_tensor Y;
  Y.fillTG();
  //cout << Y << endl;

  //tensor& V = *(new tensor());
  //V.fillTG();
  //cout << "Taylor-Green Vortex:" << endl;
  //cout << V;


  //mat A = xt::random::rand<double>(my_shape());
  /*size_t SIDE = 5;
  auto spacing = xt::linspace<double>(pi / SIDE, 2 * pi - pi / SIDE, SIDE);
  auto mesh = xt::meshgrid(spacing, spacing, spacing);
  auto X = xt::cos(get<2>(mesh)) * xt::sin(get<1>(mesh)) * xt::sin(get<0>(mesh));*/
  //cout << get<2>(mesh) << endl;
  //cout << A << endl;
  /*tensor& L = *(new tensor());
  tensor& N = *(new tensor());
  tensor& V2 = *(new tensor());

  V.fillTG();
  cout << "Taylor-Green Vortex:" << endl;
  cout << V;

  V.vectLap(L);
  cout << "Laplacian:" << endl;
  cout << L;

  V.dVfvf(N);
  cout << "Non-linear term:" << endl;
  cout << N;

  N.scale(-1, N);
  L.scale(0.2, L);
  N.sum(L, V2);
  cout << "V2:" << endl;
  cout << V2;*/

  /*field& N = *(new field());
  for(u_int i = 0; i < 40; i++){
    N = {};
    computeN(V, N);
  }

  cout << "Matrix N:" << endl;
  cout << N;*/
}
