include (Util)

set (
  REQUIRED_BOOST_LIBRARIES
  assert
  algorithm
  python
  iterator
  asio
)
  
carla_string_option (
  CARLA_BOOST_VERSION
  "Target boost version."
  1.84.0
)

carla_string_option (
  CARLA_BOOST_TAG
  "Target boost git tag."
  boost-${CARLA_BOOST_VERSION}
)

carla_declare_git_dependency (
    boost
    https://github.com/boostorg/boost.git
    ${CARLA_BOOST_TAG}
    GIT_SUBMODULES
      tools/cmake
      libs/python
      libs/asio
      libs/array
      libs/assert
      libs/bind
      libs/concept_check
      libs/config
      libs/core
      libs/exception
      libs/function
      libs/iterator
      libs/mpl
      libs/range
      libs/regex
      libs/static_assert
      libs/throw_exception
      libs/tuple
      libs/type_traits
      libs/unordered
)
