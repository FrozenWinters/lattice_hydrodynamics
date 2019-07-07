#include "xsimd/xsimd.hpp"
#include <random>
#include <iostream>

namespace xs = xsimd;

template<typename T, size_t L, size_t W, size_t H, size_t VecSize>
class grid;

template<typename T, size_t L, size_t W, size_t H, size_t VecSize>
std::ostream& operator<<(std::ostream& os, const grid<T, L, W, H, VecSize>& gr){
  using G = grid<T, L, W, H, VecSize>;

  for(u_int z = 0; z < H; ++z){
    os << "[ ";
    for(u_int y = 0; y < W; ++y){
      os << std::endl << "    [ ";
      for(u_int x = 0; x < L; ++x){
        os << "< " << gr.data[G::I(0, x, y, z)] << ", " << gr.data[G::I(1, x, y, z)]
          /*<< ", " << gr.data[G::I(2, x, y, z)]*/ << " > ";
      }
      os << "] ";
    }
    os << "]" << std::endl;
  }

  return os;
}

template<typename T, size_t L, size_t W, size_t H, size_t VecSize = 4>
class grid{
public:
  using grid_t = grid<T, L, W, H, VecSize>;
  using batch_t = xs::batch<T, VecSize>;

  void computeN(grid_t& dest) const;

  void fillRand();

private:
  static inline int I(int A, int x, int y, int z) {
    return A * L * W * H + ((x+ L) % L) + ((y+ W) % W) * W + ((z+ H) % H) * W * H;
  }

  static inline int I_off(int A, int x, int y, int z, int B, int sgn) {
    switch(B)
    {
      case 0:
        return I(A, x + sgn, y, z);
      case 1:
        return I(A, x, y + sgn, z);
      default:
        return I(A, x, y, z+sgn);
    }
  }

  friend std::ostream & operator<<<T, L, W, H, VecSize>(std::ostream &os, const grid_t &gr);

  alignas(sizeof(T) * VecSize) T data[3 * L * W * H];
};


template<typename T, size_t L, size_t W, size_t H, size_t VecSize>
inline void grid<T, L, W, H, VecSize>::computeN(grid_t& dest) const{
  for(int A = 0; A < 3; ++A){
    for(int B = 0; B < 3; ++B){
      for(u_int z = 0; z < H; ++z){
        for(u_int y = 0; y < W; ++y){
          if(B != 0){
            for(u_int x = 0; x < L; x += VecSize){
              batch_t row_a(&data[I_off(B, x, y, z, B, 1)], xs::aligned_mode());
              batch_t row_b(&data[I_off(A, x, y, z, B, 1)], xs::aligned_mode());
              batch_t row_c(&dest.data[I(A, x, y, z)], xs::aligned_mode());
              row_a *= row_b;
              row_c += row_a;
              row_a.load_aligned(&data[I_off(B, x, y, z, B, -1)]);
              row_b.load_aligned(&data[I_off(A, x, y, z, B, -1)]);
              row_a *= row_b;
              row_c -= row_a;
              row_c.store_aligned(&dest.data[I(A, x, y, z)]);
            }
          } else{
            u_int x = 0;
            for(; x < VecSize; ++x){
              int update = 0;
              update += data[I_off(B, x, y, z, B, 1)] * data[I_off(A, x, y, z, B, 1)];
              update -= data[I_off(B, x, y, z, B, -1)] * data[I_off(A, x, y, z, B, -1)];
              dest.data[I(A, x, y, z)] += update;
            }
            for(; x < L - VecSize; x += VecSize){
              batch_t row_a(&data[I_off(B, x, y, z, B, 1)]);
              batch_t row_b(&data[I_off(A, x, y, z, B, 1)]);
              batch_t row_c(&dest.data[I(A, x, y, z)], xs::aligned_mode());
              row_a *= row_b;
              row_c += row_a;
              row_a.load_unaligned(&data[I_off(B, x, y, z, B, -1)]);
              row_b.load_unaligned(&data[I_off(A, x, y, z, B, -1)]);
              row_a *= row_b;
              row_c -= row_a;
              row_c.store_aligned(&dest.data[I(A, x, y, z)]);
            }
            for(; x < L; ++x){
              int update = 0;
              update += data[I_off(B, x, y, z, B, 1)] * data[I_off(A, x, y, z, B, 1)];
              update -= data[I_off(B, x, y, z, B, -1)] * data[I_off(A, x, y, z, B, -1)];
              dest.data[I(A, x, y, z)] += update;
            }
          }
        }
      }
    }
  }
}

template<typename T, size_t L, size_t W, size_t H, size_t VecSize>
inline void grid<T, L, W, H, VecSize>::fillRand(){
  std::random_device rand;
  std::default_random_engine generator(rand());
  std::uniform_real_distribution<> distribution(2.0, 15.0);
  for(int A = 0; A < 3; ++A){
    for(u_int z = 0; z < H; ++z){
      for(u_int y = 0; y < W; ++y){
        for(u_int x = 0; x < L; ++x){
          data[I(A, x, y, z)] = distribution(generator);
        }
      }
    }
  }
}
