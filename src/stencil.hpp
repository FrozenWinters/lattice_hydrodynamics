
// This file consists of meta utilities for generating stincils,
// all the wile working within the cpp type system.
// These implementation have large overlap with typelists, e.g. Loki/Boost.MPL
// Supporting Intel v18 was a major constraint.
// There are no additioanl fitting comments to be made on this,
// apart from that it is unadulterated template meta-programming wankery.

#ifndef STENCIL_HPP_
#define STENCIL_HPP_

#include <utility>

namespace meta{
  template<class... AS>
  using list = std::tuple<AS...>;

  using empty = list<>;

  template <class... TS>
  struct append;

  template <class... TS>
  using append_t = typename append<TS...>::type;

  template <class TA>
  struct append<TA> {
    using type = TA;
  };

  template <class... AS, class... BS, class... TS>
  struct append<list<AS...>, list<BS...>, TS...> {
    using type = append_t<list<AS..., BS...>, TS...>;
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
    using type = append_t<flatten_t<AS>..., flatten_t<TS...>>;
  };

  template <class A, class... TS>
  struct flatten<A, TS...> {
    using type = append_t<list<A>, flatten_t<TS...>>;
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
        append_t<
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
  struct decode<list<TS...>> {
    using type = list<list_to_sequence_t<TS>...>;
  };

  template <class... IS>
  struct cartesian {
    using type = decode_t<cartesian_product_t<sequence_to_list_t<IS>...>>;
  };

  template <class... IS>
  using cartesian_t = typename cartesian<IS...>::type;

  // Conjunction for Intel 18 support
  template<bool... vals>
  struct conjunction;

  template<bool... vals>
  using conjunction_t = typename conjunction<vals...>::type;

  template<>
  struct conjunction<> {
    using type = std::true_type;
  };

  template<bool val, bool... vals>
  struct conjunction<val, vals...> {
    using type = std::conditional_t<val, conjunction_t<vals...>, std::false_type>;
  };

  template<class TS>
  struct filter_zero;

  template<class TS>
  using filter_zero_t = typename filter_zero<TS>::type;

  template<>
  struct filter_zero<empty> {
    using type = empty;
  };

  template<int... AS, class... IS>
  struct filter_zero<list<std::integer_sequence<int, AS...>, IS...>> {
    using type =
      std::conditional_t<
        conjunction_t<(!AS)...>::value, //Condition
        filter_zero_t<list<IS...>>, //True type, exluded
        append_t<list<std::integer_sequence<int, AS...>>, filter_zero_t<list<IS...>>>
      >;
  };

  template<template<class...> class type, class param, class IS>
  struct repeat_param_impl;

  template<template<class...> class type, class param, size_t... AS>
  struct repeat_param_impl<type, pram, std::index_sequence<AS...>>
    : public type<std::conditional_t<AS != AS + 1, param, void>...> {};

  template<template<class...> class type, class param, size_t times>
  struct repeat_param
    : public repeat_param_impl<type, param, std::make_index_sequence<times>> {};

  using dim1_stencil = std::integer_sequence<int, -1, 0, 1>;

  template<size_t dim>
  using neighbour_stencil = repeat_param<cartesian_t, dim1_stencil, dim>;

  template<size_t dim>
  using strict_neighbour_stencil = filter_zero_t<neighbour_stencil<dim>>;

}

#endif
