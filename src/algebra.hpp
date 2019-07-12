#include <random>
#include <iostream>
#include <cmath>

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
class state;

template<typename T>
static inline T clearZero(const T& val){
  static constexpr T EPSILON = std::numeric_limits<T>::epsilon();
  return (val > EPSILON || val < - EPSILON) ? val : 0;
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
std::ostream& operator<<(std::ostream& os, const state<T, rank, buff, L, W, H>& gr){
  using S = state<T, rank, buff, L, W, H>;

  for(u_int z = 0; z < H; ++z){
    os << "[ ";
    for(u_int y = 0; y < W; ++y){
      os << std::endl << "    [ ";
      for(u_int x = 0; x < L; ++x){
        if constexpr (rank == 1){
          os << clearZero(gr.data[S::I(0, x, y, z)]);
        } else{
          os << "< ";
          for(int i = 0; i < rank; ++i){
            os << clearZero(gr.data[S::I(i, x, y, z)]);
            if(i != rank - 1 ){
              os << " , ";
            }
          }
          os << " > ";
        }
      }
      os << "] ";
    }
    os << "]" << std::endl;
  }

  /*for(int i = 0; i < rank; ++i){
    os << gr.data[S::I(i, 9, 12, 17)];
    if(i != rank - 1 ){
      os << " , ";
    }
  }
  os << std::endl;*/

  return os;
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
class state{
public:
  using state_t = state<T, rank, buff, L, W, H>;

  //void computeN(state_t& dest) const;

  void fillRand();

  void scale(const T& scalar, state_t& dest) const;

  void sum(const state_t& arg, state_t& dest) const;

  std::enable_if_t<rank == 3> fillTG();

  std::enable_if_t<rank == 3> vectLap(state_t& dest) const;

  std::enable_if_t<rank == 3> dVfvf(state_t& dest) const;

private:
  static constexpr size_t Lb = L + 2 * buff;
  static constexpr size_t Wb = W + 2 * buff;
  static constexpr size_t Hb = H + 2 * buff;

  static inline int I(int A, int x, int y, int z) {
    if constexpr (buff != 0){
      const size_t xb = x + buff;
      const size_t yb = y + buff;
      const size_t zb = z + buff;
      return A * Lb * Wb * Hb + xb + yb * Wb + zb * Wb * Hb;
    } else{
      const size_t xw = (x + L) % L;
      const size_t yw = (y + W) % W;
      const size_t zw = (z + H) % H;
      return A * L * W * H + xw + yw * W + zw * W * H;
    }
  }

  static inline int I_off(int A, int x, int y, int z, int B, int off) {
    switch(B)
    {
      case 0:
        return I(A, x + off, y, z);
      case 1:
        return I(A, x, y + off, z);
      default:
        return I(A, x, y, z + off);
    }
  }

  friend std::ostream & operator<<<T, rank, buff, L, W, H>(std::ostream &os, const state_t &gr);

  /*alignas(sizeof(T) * 16)*/
  static constexpr size_t num_elts = rank * Lb * Wb * Hb;
  T data[num_elts];
};

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
inline std::enable_if_t<rank == 3> state<T, rank, buff, L, W, H>::vectLap(state_t& dest) const{
  for(int A = 0; A < 3; ++A){
    for(u_int z = 0; z < H; ++z){
      for(u_int y = 0; y < W; ++y){
        for(u_int x = 0; x < L; ++x){
          dest.data[I(A, x, y, z)] = 6 * data[I(A, x, y, z)];
          for(int B = 0; B < 3; ++B){
            dest.data[I(A, x, y, z)] -= data[I_off(A, x, y, z, B, 2)] + data[I_off(A, x, y, z, B, -2)];
          }
        }
      }
    }
  }
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
inline std::enable_if_t<rank == 3> state<T, rank, buff, L, W, H>::dVfvf(state_t& dest) const{
  for(int A = 0; A < 3; ++A){
    for(u_int z = 0; z < H; ++z){
      for(u_int y = 0; y < W; ++y){
        for(u_int x = 0; x < L; ++x){
          dest.data[I(A, x, y, z)] = 0;
          for(int B = 0; B < 3; ++B){
            dest.data[I(A, x, y, z)] += data[I_off(B, x, y, z, B, 1)] * data[I_off(A, x, y, z, B, 1)];
            dest.data[I(A, x, y, z)] -= data[I_off(B, x, y, z, B, -1)] * data[I_off(A, x, y, z, B, -1)];
          }
        }
      }
    }
  }
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
inline void state<T, rank, buff, L, W, H>::fillRand(){
  std::random_device rand;
  std::default_random_engine generator(rand());
  std::uniform_real_distribution<> distribution(2.0, 15.0);
  for(int A = 0; A < rank; ++A){
    for(u_int z = 0; z < H; ++z){
      for(u_int y = 0; y < W; ++y){
        for(u_int x = 0; x < L; ++x){
          data[I(A, x, y, z)] = distribution(generator);
        }
      }
    }
  }
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
inline void state<T, rank, buff, L, W, H>::scale(const T& scalar, state_t& dest) const {
  for(int A = 0; A < rank; ++A){
    for(u_int z = 0; z < H; ++z){
      for(u_int y = 0; y < W; ++y){
        for(u_int x = 0; x < L; ++x){
          dest.data[I(A, x, y, z)] = scalar * data[I(A, x, y, z)];
        }
      }
    }
  }
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
inline void state<T, rank, buff, L, W, H>::sum(const state_t& arg, state_t& dest) const {
  for(int A = 0; A < rank; ++A){
    for(u_int z = 0; z < H; ++z){
      for(u_int y = 0; y < W; ++y){
        for(u_int x = 0; x < L; ++x){
          dest.data[I(A, x, y, z)] = data[I(A, x, y, z)] + arg.data[I(A, x, y, z)];
        }
      }
    }
  }
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
inline std::enable_if_t<rank == 3> state<T, rank, buff, L, W, H>::fillTG(){
  using namespace std;

  const T pi = acos(-1);
  for(u_int z = 0; z < H; ++z){
    T z_val = pi / H + 2 * pi * z / H;
    for(u_int y = 0; y < W; ++y){
      T y_val = pi / W + 2 * pi * y / W;
      for(u_int x = 0; x < L; ++x){
        T x_val = pi / L + 2 * pi * x / L;
        data[I(0, x, y, z)] = cos(x_val) * sin(y_val) * sin(z_val);
        data[I(1, x, y, z)] = - sin(x_val) * cos(y_val) * sin(z_val) / 2.0;
        data[I(2, x, y, z)] = - sin(x_val) * sin(y_val) * cos(z_val) / 2.0;
      }
    }
  }
}
