#include "helpers.hpp"
#include "tmpl/sequence.hpp"
#include "tmpl/type_list.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <tuple>
#include <utility>

namespace ECS {

template<class T, std::size_t Capacity> struct BufferArray_t : Uncopyable_t
{
  alignas(T) unsigned char mBuffer[Capacity * sizeof(T)]{};

  constexpr auto getMemoryAt(std::size_t index) const -> const T*
  {
    return reinterpret_cast<const T*>(&mBuffer[index * sizeof(T)]);
  }

  constexpr auto getMemoryAt(std::size_t index) -> auto* { return ECS::SameAsConstMemFunc(*this, &BufferArray_t::getMemoryAt, index); }
};

template<class EntitySig_t, class... Cmps_t> struct Chunk_t : Uncopyable_t
{
  using ComponentList_t = TMPL::TypeList_t<Cmps_t...>;

  std::tuple<BufferArray_t<Cmps_t, 3>...> mElements{};
  std::size_t                             mSize{};

  template<class... ArgsCmps_t> constexpr auto emplace_back(ArgsCmps_t&&... cmps)
  {
    emplace_back_impl<TMPL::Sequence::Size_v<ComponentList_t>>(std::forward<ArgsCmps_t>(cmps)...);
    ++mSize;
  }

private:
  template<std::size_t Index, class Cmp_t, class... ArgsCmps_t>
  constexpr auto emplace_back_impl(Cmp_t&& cmp, ArgsCmps_t&&... cmps)
  {
    constexpr auto CmpIndex_v{ TMPL::Sequence::Size_v<ComponentList_t> - Index };

    auto* element{ std::get<CmpIndex_v>(mElements).getMemoryAt(mSize) };
    std::construct_at(element, std::forward<Cmp_t>(cmp));
    if constexpr (Index != 0) {
      emplace_back_impl<Index - 1>(std::forward<ArgsCmps_t>(cmps)...);
    }
  }
};

} // namespace ECS