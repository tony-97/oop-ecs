#pragma once

#include <tmpl/sequence.hpp>

namespace ECS
{

template<template<class...>class Container_t, class... Ts>
struct SoA_t : Container_t<Ts>...
{
public:
    template<class Type_t> using value_type             = typename Container_t<Type_t>::value_type;
    template<class Type_t> using size_type              = typename Container_t<Type_t>::size_type;
    template<class Type_t> using difference_type        = typename Container_t<Type_t>::difference_type;
    template<class Type_t> using reference              = typename Container_t<Type_t>::reference;
    template<class Type_t> using const_reference        = typename Container_t<Type_t>::const_reference;
    template<class Type_t> using pointer                = typename Container_t<Type_t>::pointer;
    template<class Type_t> using const_pointer          = typename Container_t<Type_t>::const_pointer;
    template<class Type_t> using iterator               = typename Container_t<Type_t>::iterator;
    template<class Type_t> using const_iterator         = typename Container_t<Type_t>::const_iterator;
    template<class Type_t> using reverse_iterator       = typename Container_t<Type_t>::reverse_iterator;
    template<class Type_t> using const_reverse_iterator = typename Container_t<Type_t>::const_reverse_iterator;

    constexpr explicit SoA_t() : Container_t<Ts>{  }...
    {
        CheckIfTypesAreUnique();
    }

    template<class T, class U> constexpr auto operator[](U u)       -> reference<T>       { return GetRequiredContainer<T>()[u]; }
    template<class T, class U> constexpr auto operator[](U u) const -> const_reference<T> { return GetRequiredContainer<T>()[u]; }

    template<class T, class U> constexpr auto at(U u)       -> reference<T>       { return GetRequiredContainer<T>().at(u); }
    template<class T, class U> constexpr auto at(U u) const -> const_reference<T> { return GetRequiredContainer<T>().at(u); }

    template<class T> constexpr auto front()       -> reference<T>       { return GetRequiredContainer<T>().front(); }
    template<class T> constexpr auto front() const -> const_reference<T> { return GetRequiredContainer<T>().front(); }

    template<class T> constexpr auto back()       -> reference<T>       { return GetRequiredContainer<T>().back(); }
    template<class T> constexpr auto back() const -> const_reference<T> { return GetRequiredContainer<T>().back(); }

    template<class T> constexpr auto data()       -> pointer<T>       { return GetRequiredContainer<T>().data(); }
    template<class T> constexpr auto data() const -> const_pointer<T> { return GetRequiredContainer<T>().data(); }

    template<class T> constexpr auto empty   () const -> bool         { return GetRequiredContainer<T>().empty();    }
    template<class T> constexpr auto size    () const -> size_type<T> { return GetRequiredContainer<T>().size();     }
    template<class T> constexpr auto max_size() const -> size_type<T> { return GetRequiredContainer<T>().max_size(); }
    template<class T> constexpr auto capacity() const -> size_type<T> { return GetRequiredContainer<T>().capacity(); }

    template<class T> constexpr auto reserve(size_type<T> new_size) -> void { GetRequiredContainer<T>().reserve(new_size); }
    template<class T> constexpr auto shrink_to_fit               () -> void { GetRequiredContainer<T>().shrink_to_fit();   }

    template<class T> constexpr auto clear() -> void {  GetRequiredContainer<T>().clear(); }

    template<class T, class... Args_t> constexpr auto
    emplace(const_iterator<T> pos, Args_t&&... args) -> iterator<T>
    { return GetRequiredContainer<T>().emplace(pos, std::forward<Args_t>(args)...); }

    template<class T> constexpr auto erase                          (const_iterator<T> pos) -> iterator<T> { return GetRequiredContainer<T>().erase(pos); }
    template<class T> constexpr auto erase(const_iterator<T> first, const_iterator<T> last) -> iterator<T> { return GetRequiredContainer<T>().erase(first, last); }

    template<class T, class U> constexpr auto erase(U u) -> auto { return GetRequiredContainer<T>().erase(u); }

    template<class T> constexpr auto push_back(const T& value) -> void { GetRequiredContainer<T>().push_back(value); }
    template<class T> constexpr auto push_back     (T&& value) -> void { GetRequiredContainer<T>().push_back(value); }

    template<class T, class... Args_t> constexpr auto emplace_back(Args_t&&... args ) -> decltype(auto)
    { return GetRequiredContainer<T>().emplace_back(std::forward<Args_t>(args)...); }

    template<class T> constexpr auto pop_back() -> void { GetRequiredContainer<T>().pop_back(); }

    template<class T> constexpr auto resize                            (size_type<T> count) -> void { GetRequiredContainer<T>().resize(count); }
    template<class T> constexpr auto resize(size_type<T> count, const value_type<T>& value) -> void { GetRequiredContainer<T>().resize(count, value); }

    template<class T> constexpr auto static get_container_index() -> size_type<T> { return TMPL::IndexOf_v<T, Ts...>; }

    template<class T> constexpr auto begin () const -> const_iterator<T> { return GetRequiredContainer<T>().begin(); }
    template<class T> constexpr auto cbegin() const -> const_iterator<T> { return GetRequiredContainer<T>().cbegin(); }
    template<class T> constexpr auto begin ()       -> iterator<T>       { return GetRequiredContainer<T>().begin(); }

    template<class T> constexpr auto rbegin () const -> const_reverse_iterator<T> { return GetRequiredContainer<T>().rbegin();  }
    template<class T> constexpr auto crbegin() const -> const_reverse_iterator<T> { return GetRequiredContainer<T>().crbegin(); }
    template<class T> constexpr auto rbegin ()       -> reverse_iterator<T>       { return GetRequiredContainer<T>().rbegin();  }

    template<class T> constexpr auto end () const -> const_iterator<T> { return GetRequiredContainer<T>().end();  }
    template<class T> constexpr auto cend() const -> const_iterator<T> { return GetRequiredContainer<T>().cend(); }
    template<class T> constexpr auto end ()       -> iterator<T>       { return GetRequiredContainer<T>().end();  }

    template<class T> constexpr auto rend () const -> const_reverse_iterator<T> { return GetRequiredContainer<T>().rend(); }
    template<class T> constexpr auto crend() const -> const_reverse_iterator<T> { return GetRequiredContainer<T>().crend(); }
    template<class T> constexpr auto rend ()       -> reverse_iterator<T>       { return GetRequiredContainer<T>().rend(); }

protected:

    template<class RequiredType_t> constexpr auto
    GetRequiredContainer() const -> const Container_t<RequiredType_t>&
    {
        CheckIfTypeExists<RequiredType_t>();
        return static_cast<const Container_t<RequiredType_t>&>(*this);
    }

    template<class RequiredType_t> constexpr auto
    GetRequiredContainer() -> Container_t<RequiredType_t>&
    {
        return SameAsConstMemFunc(this, &SoA_t::GetRequiredContainer<RequiredType_t>);
    }

    constexpr static auto CheckIfTypesAreUnique() -> void
    {
        static_assert(TMPL::AreUnique_v<Ts...>, "Types must be unique.");
    }

    template<class Type_t>
    constexpr static auto CheckIfTypeExists() -> void
    {
        static_assert(TMPL::IsOneOf_v<Type_t, Ts...>,
                      "The requiered type does not exists in this instance.");
    }
};

} // namespace ECS
