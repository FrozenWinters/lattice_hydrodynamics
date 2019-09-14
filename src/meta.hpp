#include <utility>

// There are no fitting comments to be made on this section of code,
// apart from that it is unadulterated template meta-programming wankery.

namespace meta{
  template<class... AS>
  using list = std::tuple<AS...>;

  using empty = list<>;

  template <class... TS>
  struct concatenate;

  template <class... TS>
  using concatenate_t = typename concatenate<TS...>::type;

  template <class TA>
  struct concatenate<TA> {
    using type = TA;
  };

  template <class... AS, class... BS, class... TS>
  struct concatenate<list<AS...>, list<BS...>, TS...> {
    using type = concatenate_t<list<AS..., BS...>, TS...>;
  };

  template <class... TS>
  struct flatten;

  template <class... TS>
  using flatten_t = typename flatten<TS...>::type;

  template <>
  struct flatten<> {
    using type = empty;
  };

  template <class... AS, class... TS>
  struct flatten<list<AS...>, TS...> {
    using type = concatenate_t<flatten_t<AS>..., flatten_t<TS...>>;
  };

  template <class A, class... TS>
  struct flatten<A, TS...> {
    using type = concatenate_t<list<A>, flatten_t<TS...>>;
  };

  template <class... TS>
  struct cartesian_product;

  template <class... TS>
  using cartesian_product_t = typename cartesian_product<TS...>::type;

  template <class TA>
  struct cartesian_product<TA> {
    using type = TA;
  };

  template <class... TS>
  struct cartesian_product<empty, TS...> {
    using type = empty;
  };

  template <class A, class... AS, class... BS, class... TS>
  struct cartesian_product<list<A, AS...>, list<BS...>, TS...> {
    using type =
      cartesian_product_t<
        concatenate_t<
          list<flatten_t<A, BS>...>,
          cartesian_product_t<list<AS...>, list<BS...>>
        >,
        TS...
      >;
  };

  template <class IA>
  struct sequence_to_list;

  template <class IA>
  using sequence_to_list_t = typename sequence_to_list<IA>::type;

  template <int... AS>
  struct sequence_to_list<std::integer_sequence<int, AS...>> {
    using type = list<std::integral_constant<int, AS>...>;
  };

  template <class TA>
  struct list_to_sequence;

  template <class TA>
  using list_to_sequence_t = typename list_to_sequence<TA>::type;

  template <int... AS>
  struct list_to_sequence<list<std::integral_constant<int, AS>...>> {
    using type = std::integer_sequence<int, AS...>;
  };

  template <class TA>
  struct decode;

  template <class TA>
  using decode_t = typename decode<TA>::type;

  template <class... TS>
  struct decode<list<TS...>>{
    using type = list<list_to_sequence_t<TS>...>;
  };

  template <class... IS>
  struct cartesian{
    using type = decode_t<cartesian_product_t<sequence_to_list_t<IS>...>>;
  };

  template <class... IS>
  using cartesian_t = typename cartesian<IS...>::type;

  using A = std::integer_sequence<int, 0, 2>;
  using B = std::integer_sequence<int, 5, 7>;

  using result = cartesian_t<A, B>;

}
