#pragma once

#include <vector>
#include <iterator>

#include "helpers.hpp"

namespace ECS
{

template<class T>
struct ECSMap_t
{
    using value_type      = typename std::vector<T>::value_type;
    using size_type       = typename std::vector<T>::size_type;
    using difference_type = typename std::vector<T>::difference_type;
    using reference       = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;
    using pointer         = typename std::vector<T>::pointer;
    using const_pointer   = typename std::vector<T>::const_pointer;

    struct Slot_t
    {
        using value_type = T;

        size_type mIndex {  };
        size_type mEraseIndex {  };
        T mValue {  };

        template<class... Args_t> constexpr
        Slot_t(Args_t&&... args)
            : mValue { std::forward<Args_t>(args)... } {  }
    };

    struct Key_t
    {
        using value_type = T;

        friend ECSMap_t;

        constexpr Key_t(size_type index) : mIndex { index } {  };

        constexpr auto GetIndex() const -> size_type { return mIndex; }
    private:
        size_type mIndex {  };
    };

    template<class Value_t>
    struct iterator_t
    {
        using slot_ptr = AddConstIf_t<Value_t, Slot_t>*;

        using iterator_category = std::random_access_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = Value_t;
        using pointer           = value_type*;
        using reference         = value_type&;

        constexpr iterator_t(slot_ptr slot) : mSlot { slot } {  }

        constexpr operator iterator_t<const Value_t> () const
        {
            return { mSlot };
        }

        constexpr auto operator*() const -> reference
        {
            return mSlot->mValue;
        }

        constexpr auto operator->() const -> pointer
        {
            return &mSlot->mValue;
        }

        constexpr auto operator++() -> iterator_t&
        {
            ++mSlot;
            return *this;
        }

        constexpr auto operator++(int) -> iterator_t 
        {
            iterator_t tmp { *this };
            ++(*this);
            return tmp;
        }

        constexpr auto operator--() -> iterator_t&
        {
            --mSlot;
            return *this;
        }

        constexpr auto operator--(int) -> iterator_t
        {
            iterator_t tmp { *this };
            --(*this);
            return tmp;
        }

        constexpr auto operator+=(std::size_t n) -> iterator_t&
        {
            mSlot += n;
            return *this;
        }

        constexpr auto friend operator+(iterator_t it, std::size_t n) -> iterator_t
        {
            it += n;
            return it;
        }

        constexpr auto operator-=(std::size_t n) -> iterator_t&
        {
            mSlot -= n;
            return *this;
        }

        constexpr auto friend operator-(iterator_t it, std::size_t n) -> iterator_t
        {
            it -= n;
            return it;
        }

        constexpr auto operator-(const iterator_t it) -> difference_type
        {
            return mSlot - it.mSlot;
        }

        constexpr auto friend operator==(const iterator_t lhs, const iterator_t rhs) -> bool
        {
            return lhs.mSlot == rhs.mSlot;
        }

        constexpr auto friend operator!=(const iterator_t lhs, const iterator_t rhs) -> bool
        {
            return !(lhs == rhs);
        }

    private:
        slot_ptr mSlot {  };
    };

    using iterator               = iterator_t<T>;
    using const_iterator         = iterator_t<const T>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    constexpr explicit ECSMap_t() = default;

    template<class... Args_t> [[nodiscard]] constexpr auto
    emplace_back(Args_t&&... args) -> Key_t
    {
        Key_t key { mFreeIndex };
        if (mFreeIndex == mLastIndex && mLastIndex == mData.size()) {
            auto& slot { mData.emplace_back(std::forward<Args_t>(args)...) };
            slot.mIndex = slot.mEraseIndex = mFreeIndex;
            ++mFreeIndex;
        } else {
            new (&mData[mLastIndex].mValue) T { std::forward<Args_t>(args)... };
            mData[mLastIndex].mEraseIndex = mFreeIndex;
            auto next_free_index { mData[mFreeIndex].mIndex };
            mData[mFreeIndex].mIndex = mLastIndex;
            mFreeIndex = next_free_index;
        }
        ++mLastIndex;

        return key;
    }

    constexpr auto erase(Key_t key) -> void
    {
        --mLastIndex;
        if (mData[key.mIndex].mIndex != mLastIndex) {
            mData[mData[key.mIndex].mIndex].mValue = std::move(mData[mLastIndex].mValue);
        }
        mData[mData[key.mIndex].mIndex].mEraseIndex = mData[mLastIndex].mEraseIndex;
        // update the key
        mData[mData[mLastIndex].mEraseIndex].mIndex = mData[key.mIndex].mIndex;
        // update the index list
        mData[key.mIndex].mIndex = mFreeIndex;
        mFreeIndex = key.mIndex;
    }

    constexpr auto size() -> size_type const { return mLastIndex; }

    constexpr auto erase(const_iterator it) -> void
    {
        erase(get_key(it));
    }

    constexpr auto get_key(const_iterator it) const -> Key_t
    {
        return { mData[get_index(it)].mEraseIndex };
    }

    constexpr auto get_key(size_type pos) const -> Key_t
    {
        return { mData[pos].mEraseIndex };
    }

    constexpr auto operator[](Key_t key) -> reference
    {
        return mData[mData[key.mIndex].mIndex].mValue;
    }

    constexpr auto operator[](Key_t key) const -> const_reference
    {
        return mData[mData[key.mIndex].mIndex].mValue;
    }

    constexpr auto operator[](size_type pos) -> reference
    {
        return mData[pos].mValue;
    }

    constexpr auto operator[](size_type pos) const -> const_reference
    {
        return mData[pos].mValue;
    }

    constexpr auto begin()         -> iterator               { return { mData.data() }; }
    constexpr auto begin()   const -> const_iterator         { return { mData.data() }; }
    constexpr auto rbegin()        -> reverse_iterator       { return { std::make_reverse_iterator(end()) }; }
    constexpr auto rbegin()  const -> const_reverse_iterator { return { std::make_reverse_iterator(end()) }; }
    constexpr auto crbegin() const -> const_reverse_iterator { return { rbegin() }; }

    constexpr auto end()         -> iterator               { return { mData.data() + mLastIndex }; }
    constexpr auto end()   const -> const_iterator         { return { mData.data() + mLastIndex }; }
    constexpr auto rend()        -> reverse_iterator       { return { std::make_reverse_iterator(begin()) }; }
    constexpr auto rend()  const -> const_reverse_iterator { return { std::make_reverse_iterator(begin()) }; }
    constexpr auto crend() const -> const_reverse_iterator { return { rend() }; }

private:

    constexpr auto get_index(const_iterator it) const -> auto
    {
        return static_cast<size_type>(std::distance(begin(), it));
    }

    size_type mFreeIndex {  };
    size_type mLastIndex {  };
    std::vector<Slot_t> mData {  };
};

} // namespace ECS
