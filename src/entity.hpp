#pragma once

#include <sequence.hpp>

#include <array>
#include <type_traits>

#include "type_aliases.hpp"
#include "helpers.hpp"

namespace ECS
{

template<class Signature_t>
struct Entity_t final : Uncopyable_t
{
public:

    using Components_t  = typename Signature_t::type;
    using CompnentIDs_t = TMPL::Sequence::ConvertTo_t<std::tuple<>, Components_t>;

    //template<class... IDs_t,
    //         std::enable_if_t<std::conjunction_v<std::is_same<std::size_t,
    //                                                          IDs_t>...>, bool> = true>
    template<class... IDs_t>
    constexpr explicit Entity_t(IDs_t&&... ids) : mComponentIDs { ids... } {  }

    constexpr explicit Entity_t(Entity_t&& other) : mComponentIDs { std::move(other.mComponentIDs) } {  }

private:

    std::array<std::size_t, TMPL::Sequence::Size_v<Components_t>> mComponentIDs {  };
};

} // namespace ECS
