#include <random>
#include <iostream>

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
class state;

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
std::ostream& operator<<(std::ostream& os, const state<T, rank, buff, L, W, H>& gr){
  using S = state<T, rank, buff, L, W, H>;

  for(u_int z = 0; z < H; ++z){
    os << "[ ";
    for(u_int y = 0; y < W; ++y){
      os << std::endl << "    [ ";
      for(u_int x = 0; x < L; ++x){
        if constexpr (rank == 1){
          os << gr.data[S::I(0, x, y, z)];
        } else{
          os << "< ";
          for(int i = 0; i < rank; ++i){
            os << gr.data[S::I(0, x, y, z)];
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

  return os;
}

template<typename T, size_t rank, size_t buff, size_t L, size_t W, size_t H>
class state{
public:
  using state_t = state<T, rank, buff, L, W, H>;

  //void computeN(state_t& dest) const;

  void fillRand();

private:
  static constexpr size_t Lb = L + 2 * buff;
  static constexpr size_t Wb = W + 2 * buff;
  static constexpr size_t Hb = H + 2 * buff;

  static inline int I(int A, int x, int y, int z) {
    const size_t xb = x + buff;
    const size_t yb = y + buff;
    const size_t zb = z + buff;
    return A * Lb * Wb * Hb + xb + yb * W + zb * W * H;
  }

  static inline int I_off(int A, int x, int y, int z, int B, int sgn) {
    switch(B)
    {
      case 0:
        return I(A, x + sgn, y, z);
      case 1:
        return I(A, x, y + sgn, z);
      default:
        return I(A, x, y, z + sgn);
    }
  }

  friend std::ostream & operator<<<T, rank, buff, L, W, H>(std::ostream &os, const state_t &gr);

  /*alignas(sizeof(T) * 16)*/
  static constexpr size_t num_elts = rank * Lb * Wb * Hb;
  T data[num_elts];
};


/*template<typename T, size_t L, size_t W, size_t H, size_t VecSize>
inline void state<T, L, W, H, VecSize>::computeN(state_t& dest) const{
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
}*/

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
