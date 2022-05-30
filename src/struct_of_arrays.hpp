#pragma once

#include <variant>
#include <array>

#include <sequence.hpp>

#include "type_aliases.hpp"

namespace ECS
{

template<template<class...>class Container_t, class... Ts>
struct SoA_t
{
public:
    template<class Type_t> using value_type             = typename Container_t<Type_t>::value_type;
    template<class Type_t> using size_type              = typename Container_t<Type_t>::size_type;
    template<class Type_t> using difference_type        = typename Container_t<Type_t>::size_type;
    template<class Type_t> using reference              = value_type<Type_t>&;
    template<class Type_t> using const_reference        = const value_type<Type_t>&;
    template<class Type_t> using pointer                = typename Container_t<Type_t>::pointer;
    template<class Type_t> using const_pointer          = typename Container_t<Type_t>::const_pointer;
    template<class Type_t> using iterator               = typename Container_t<Type_t>::iterator;
    template<class Type_t> using const_iterator         = typename Container_t<Type_t>::const_iterator;
    template<class Type_t> using reverse_iterator       = typename Container_t<Type_t>::reverse_iterator;
    template<class Type_t> using const_reverse_iterator = typename Container_t<Type_t>::const_reverse_iterator;

    using TypeFields = std::tuple<Container_t<Ts>...>;

    constexpr explicit SoA_t() : mTypeTable { Container_t<Ts>{  }... }
    {
        CheckIfTypesAreUnique();
    }

    template<class T> constexpr reference<T>&       operator[](std::size_t index) { return GetRequiredContainer<T>()[index]; }
    template<class T> constexpr const_reference<T>& operator[](std::size_t index) const { return GetRequiredContainer<T>()[index]; }

    template<class T> constexpr reference<T>&       at(std::size_t index)       { return GetRequiredContainer<T>().at(index); }
    template<class T> constexpr const_reference<T>& at(std::size_t index) const { return GetRequiredContainer<T>().at(index); }

    template<class T> constexpr reference<T>&       front()       { return GetRequiredContainer<T>().front(); }
    template<class T> constexpr const_reference<T>& front() const { return GetRequiredContainer<T>().front(); }

    template<class T> constexpr reference<T>&       back()       { return GetRequiredContainer<T>().back(); }
    template<class T> constexpr const_reference<T>& back() const { return GetRequiredContainer<T>().back(); }

    template<class T> constexpr pointer<T>       data()       { return GetRequiredContainer<T>().data(); }
    template<class T> constexpr const_pointer<T> data() const { return GetRequiredContainer<T>().data(); }

    template<class T> constexpr bool         empty   () const { return GetRequiredContainer<T>().empty(); }
    template<class T> constexpr size_type<T> size    () const { return GetRequiredContainer<T>().size(); }
    template<class T> constexpr size_type<T> max_size() const { return GetRequiredContainer<T>().max_size(); }
    template<class T> constexpr size_type<T> capacity() const { return GetRequiredContainer<T>().capacity(); }

    template<class T> constexpr void reserve(size_type<T> new_size) { GetRequiredContainer<T>().reserve(new_size); }
    template<class T> constexpr void shrink_to_fit               () {  GetRequiredContainer<T>().shrink_to_fit(); }

    template<class T> constexpr void clear() {  GetRequiredContainer<T>().clear(); }

    template<class T, class... Args_t> constexpr auto emplace(const_iterator<T> pos, Args_t&&... args)
    -> iterator<T> { return GetRequiredContainer<T>().emplace(pos, std::forward<Args_t>(args)...); }

    template<class T> constexpr iterator<T> erase                          (const_iterator<T> pos) { return GetRequiredContainer<T>().erase(pos); }
    template<class T> constexpr iterator<T> erase(const_iterator<T> first, const_iterator<T> last) { return GetRequiredContainer<T>().erase(first, last); }

    template<class T> constexpr void push_back(const T& value) { GetRequiredContainer<T>().push_back(value); }
    template<class T> constexpr void push_back     (T&& value) { GetRequiredContainer<T>().push_back(value); }

    template<class T, class... Args_t> constexpr auto emplace_back(Args_t&&... args )
    -> reference<T> { return GetRequiredContainer<T>().emplace_back(std::forward<Args_t>(args)...); }

    template<class T> constexpr void pop_back() { GetRequiredContainer<T>().pop_back(); }

    template<class T> constexpr void resize                            (size_type<T> count) { GetRequiredContainer<T>().resize(count); }
    template<class T> constexpr void resize(size_type<T> count, const value_type<T>& value) { GetRequiredContainer<T>().resize(count, value); }

    template<class T> constexpr static size_type<T> get_type_index() { return TMPL::IndexOf_v<T, Ts...>; }

    template<class T> constexpr const_iterator<T> begin () const { return GetRequiredContainer<T>().begin(); }
    template<class T> constexpr const_iterator<T> cbegin() const { return GetRequiredContainer<T>().cbegin(); }
    template<class T> constexpr iterator<T>       begin ()       { return GetRequiredContainer<T>().begin(); }

    template<class T> constexpr const_reverse_iterator<T> rbegin () const { return GetRequiredContainer<T>().rbegin(); }
    template<class T> constexpr const_reverse_iterator<T> crbegin() const { return GetRequiredContainer<T>().crbegin(); }
    template<class T> constexpr reverse_iterator<T>       rbegin ()       { return GetRequiredContainer<T>().rbegin(); }

    template<class T> constexpr const_iterator<T> end () const { return GetRequiredContainer<T>().end(); }
    template<class T> constexpr const_iterator<T> cend() const { return GetRequiredContainer<T>().cend(); }
    template<class T> constexpr iterator<T>       end ()       { return GetRequiredContainer<T>().end(); }

    template<class T> constexpr const_reverse_iterator<T> rend () const { return GetRequiredContainer<T>().rend(); }
    template<class T> constexpr const_reverse_iterator<T> crend() const { return GetRequiredContainer<T>().crend(); }
    template<class T> constexpr reverse_iterator<T>       rend ()       { return GetRequiredContainer<T>().rend(); }

private:

    template<class RequiredType_t> constexpr auto
    GetRequiredContainer() const -> const Container_t<RequiredType_t>&
    {
        CheckIfTypeExists<RequiredType_t>();
        return std::get<Container_t<RequiredType_t>>(mTypeTable);
    }

    template<class RequiredType_t> constexpr auto
    GetRequiredContainer() -> Container_t<RequiredType_t>&
    {
        return SameAsConstMemFunc(this, &SoA_t::GetRequiredContainer<RequiredType_t>);
    }

    constexpr static void CheckIfTypesAreUnique()
    {
        static_assert(TMPL::AreUnique_v<Ts...>, "Types must be unique.");
    }

    template<class Type_t>
    constexpr static void CheckIfTypeExists()
    {
        static_assert(TMPL::IsOneOf_v<Type_t, Ts...>,
                      "The requiered type does not exists in this instance.");
    }

    TypeFields mTypeTable {  };
};

} // namespace ECS
