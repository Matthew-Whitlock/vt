
#if !defined INCLUDED_TOPOS_INDEX_DENSE_DENSE_ARRAY_H
#define INCLUDED_TOPOS_INDEX_DENSE_DENSE_ARRAY_H

#include "config.h"
#include "utils/bits/bits_packer.h"
#include "serialization/traits/byte_copy_trait.h"

#include <array>
#include <type_traits>
#include <string>
#include <sstream>
#include <cstdint>
#include <functional>
#include <utility>

#if backend_check_enabled(detector)
  #include "topos/index/traits/traits.h"
#endif

namespace vt { namespace index {

using NumDimensionsType = int8_t;

template <typename IndexType, NumDimensionsType ndim = 1>
struct DenseIndexArray;

template <typename IndexType, NumDimensionsType ndim>
struct DenseIndexArray : serialization::ByteCopyTrait {
  using ThisIndexType = DenseIndexArray<IndexType, ndim>;
  using IndexSizeType = size_t;
  using ApplyType = std::function<void(ThisIndexType)>;
  using IsByteCopyable = serialization::ByteCopyTrait;

  struct dense_single_value_tag {};

  using DenseIndexArrayType = DenseIndexArray<IndexType, ndim>;
  using DenseArraySizeType = uint64_t;
  using DenseIndexType = IndexType;

  DenseIndexArray() = default;
  DenseIndexArray(DenseIndexArray const&) = default;
  DenseIndexArray& operator=(DenseIndexArray const&) = default;
  DenseIndexArray(DenseIndexArray&&) = default;

  template <typename... Idxs>
  explicit DenseIndexArray(Idxs&&... init);
  DenseIndexArray(dense_single_value_tag, IndexType const& init_value);

  IndexType& operator[](IndexType const& index);
  IndexType const& operator[](IndexType const& index) const;
  IndexType get(IndexType const& index) const;
  IndexSizeType packedSize() const;
  bool indexIsByteCopyable() const;
  DenseArraySizeType getSize() const;
  std::string toString() const;
  void foreach(ThisIndexType max, ApplyType fn) const;
  UniqueIndexBitType uniqueBits() const;

  static ThisIndexType uniqueBitsToIndex(UniqueIndexBitType const& bits);

  bool operator==(DenseIndexArrayType const& other) const;
  bool operator<(DenseIndexArrayType const& other) const;

  // operator + and - for accessing neighboring indices
  DenseIndexArrayType operator+(DenseIndexArrayType const& other) const;
  DenseIndexArrayType operator-(DenseIndexArrayType const& other) const;

  // special accessors (x,y,z) enabled depending on the number of dimensions
  template <
    typename T = void, typename = typename std::enable_if<ndim >= 1, T>::type
  >
  IndexType x() const;

  template <
    typename T = void, typename = typename std::enable_if<ndim >= 2, T>::type
  >
  IndexType y() const;

  template <
    typename T = void, typename = typename std::enable_if<ndim >= 3, T>::type
  >
  IndexType z() const;

private:
  std::array<IndexType, ndim> dims = {};
};

#if backend_check_enabled(detector)
  static_assert(
    vt::index::IndexTraits<DenseIndexArray<int, 10>>::is_index,
    "DenseIndexArray must follow the index concept"
  );
#endif

}}  // end namespace vt::index

#include "topos/index/dense/dense_array.impl.h"

#endif /*INCLUDED_TOPOS_INDEX_DENSE_DENSE_ARRAY_H*/
