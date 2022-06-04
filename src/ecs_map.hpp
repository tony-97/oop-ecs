#pragma once

#include <vector>
#include <limits>
#include <iterator>

#include <cassert>

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
        //Key_t(Key_t&& other) : mIndex { other.mIndex }
        //{
        //    other.mIndex = std::numeric_limits<std::size_t>::max();
        //}
        //
        //Key_t& operator=(Key_t&& other)
        //{
        //    mIndex = other.mIndex;
        //    other.mIndex = std::numeric_limits<std::size_t>::max();
        //}
    //private:
        friend ECSMap_t;

        Key_t(size_type index) : mIndex { index } {  };

        size_type GetIndex() const { return mIndex; }
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

        operator iterator_t<const Value_t> () const
        {
            return { mSlot };
        }

        constexpr reference operator*() const
        {
            return mSlot->mValue;
        }

        constexpr pointer operator->() const
        {
            return &mSlot->mValue;
        }

        constexpr iterator_t& operator++()
        {
            ++mSlot;
            return *this;
        }

        constexpr iterator_t operator++(int)
        {
            iterator_t tmp { *this };
            ++(*this);
            return tmp;
        }

        constexpr iterator_t& operator--()
        {
            --mSlot;
            return *this;
        }

        constexpr iterator_t operator--(int)
        {
            iterator_t tmp { *this };
            --(*this);
            return tmp;
        }

        constexpr iterator_t& operator+=(std::size_t n)
        {
            mSlot += n;
            return *this;
        }

        constexpr friend iterator_t operator+(iterator_t it, std::size_t n)
        {
            it += n;
            return it;
        }

        constexpr iterator_t& operator-=(std::size_t n)
        {
            mSlot -= n;
            return *this;
        }

        constexpr friend iterator_t operator-(iterator_t it, std::size_t n)
        {
            it -= n;
            return it;
        }

        constexpr difference_type operator-(const iterator_t& it)
        {
            return mSlot - it.mSlot;
        }

        constexpr friend bool operator==(const iterator_t& lhs, const iterator_t& rhs)
        {
            return lhs.mSlot == rhs.mSlot;
        }

        constexpr friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs)
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

    template<class... Args_t> 
    [[nodiscard]] constexpr Key_t emplace_back(Args_t&&... args)
    {
        Key_t key { mFreeIndex };
        if (mFreeIndex == mLastIndex) {
            auto& slot { mData.emplace_back(std::forward<Args_t>(args)...) };
            slot.mIndex = slot.mEraseIndex = mFreeIndex;
            ++mFreeIndex;
        } else {
            new (&mData[mLastIndex].mValue) T { std::forward<Args_t>(args)... };
            mData[mLastIndex].mEraseIndex = mFreeIndex;
            mData[mFreeIndex].mIndex = mLastIndex;
            mFreeIndex = mData[mFreeIndex].mIndex;
        }
        ++mLastIndex;

        return key;
    }

    constexpr void erase(const Key_t& slot)
    {
        assert(slot.mIndex != std::numeric_limits<size_type>::max());

        --mLastIndex;
        mData[mData[slot.mIndex].mIndex].mValue.~T();
        mData[mData[slot.mIndex].mIndex].mValue = std::move(mData[mLastIndex].mValue);
        mData[mData[slot.mIndex].mIndex].mEraseIndex = mData[mLastIndex].mEraseIndex;
        // update the slot
        mData[mData[mLastIndex].mEraseIndex].mIndex = mData[slot.mIndex].mIndex;
        // update the index list
        mData[slot.mIndex].mIndex = mFreeIndex;
        mFreeIndex = slot.mIndex;
    }

    constexpr size_type size() const { return mLastIndex; }

    constexpr void erase(const_iterator it)
    {
        erase(get_key(it));
    }

    constexpr Key_t get_key(const_iterator it) const
    {
        return { mData[get_index(it)].mEraseIndex };
    }

    constexpr Key_t get_key(size_type pos) const
    {
        return { mData[pos].mEraseIndex };
    }

    constexpr reference operator[](const Key_t& slot)
    {
        assert(slot.mIndex != std::numeric_limits<size_type>::max());
        return mData[mData[slot.mIndex].mIndex].mValue;
    }

    constexpr const_reference operator[](const Key_t& slot) const
    {
        assert(slot.mIndex != std::numeric_limits<size_type>::max());
        return mData[mData[slot.mIndex].mIndex].mValue;
    }

    constexpr reference operator[](size_type pos)
    {
        return mData[pos].mValue;
    }

    constexpr const_reference operator[](size_type pos) const
    {
        return mData[pos].mValue;
    }

    iterator               begin()         { return { mData.data() }; }
    const_iterator         begin()   const { return { mData.data() }; }
    reverse_iterator       rbegin()        { return { std::make_reverse_iterator(end()) }; }
    const_reverse_iterator rbegin()  const { return { std::make_reverse_iterator(end()) }; }
    const_reverse_iterator crbegin() const { return { rbegin() }; }

    iterator               end()         { return { mData.data() + mLastIndex }; }
    const_iterator         end()   const { return { mData.data() + mLastIndex }; }
    reverse_iterator       rend()        { return { std::make_reverse_iterator(begin()) }; }
    const_reverse_iterator rend()  const { return { std::make_reverse_iterator(begin()) }; }
    const_reverse_iterator crend() const { return { rend() }; }

private:

    constexpr auto get_index(const_iterator it) const
    {
        return static_cast<size_type>(std::distance(begin(), it));
    }

    size_type mFreeIndex {  };
    size_type mLastIndex {  };
    std::vector<Slot_t> mData {  };
};

} // namespace ECS
