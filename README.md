# Lattice Hydrodynamics

## Building the project

Navigate to a new build directory and run:
`cmake path/to/source && make`.

## Design Notes

### Preamble

First and foremost, this code is written in the style of a _modern C++ library_,
and is heavily influenced by the style of `xtensor`, which it has as a dependency.
C++ library implementation is a highly niche skill and consists of many facets
that are have no analogues in other programming languages, or in different use-cases.

The principles of good design in implementing a C++ library are:
* __Be opaque to the user__: The user should be able to use the library without
ever having to understand its internals. The interface forwarded to the user
should be dictated by requiring only the inputs that are logically necessary
for a theoretical specification of the functionality.
* __Be customizable__: Offer the user a wide range of behaviors, and generalize
everything that may be done at no performance cost (e.g. for a numeric library,
the user should be able to change the floating point precision).
* __Avoid bloat__: The user should pay, in computation time and executable size,
only for the features which they use. Essentially, one strive for design in which
the library performs as though it was written with the same generality with which
it is used.
* __Be extensible__: Different axes of functionality should be orthogonal. Each
unit of functionality should use the others only by way of prescribed interfaces.
Hence, if the library is to be extended as to account for different behavior
along one such axis, this would require changing the code in a minimal number of
locations.
* __Be unmodifiable__: If a section of code means what it says, and one thinks
of a different meaning, then one can make the code mean a different thing by
modifying it. _This should be avoided!_

...__Q:__ But where is the university?

...__A:__ The meaning of the code, and thus the reason why it works, should be
in the gestalt of the design. This somewhat a logical necessity because C++
requires a lot of duck typing - _if it walks like a duck and quacks like a duck,
then it is a duck_ - i.e. there isn't a mechanism in C++17 to specify behavior
expectations on template parameters (the somewhat pretentiously named _concepts_
of C++20 provide a partial solution to this problem).
