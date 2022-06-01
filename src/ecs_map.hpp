#pragma once

#include <type_traits>
#include <vector>
#include <cstdint>
#include <limits>
#include <iterator>

#include <cassert>

namespace ECS
{

template<class T>
struct Slot_t
{
    std::size_t mDataIndex {  };
    std::size_t mEraseIndex {  };
    T mValue {  };

    template<class... Args_t> constexpr
    Slot_t(Args_t&&... args)
        : mValue { std::forward<Args_t>(args)... } {  }
};

template<class T>
struct ECSMap_t
{
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

        Key_t(std::size_t index)
            : mIndex { index } {  };

        std::size_t mIndex {  };
    };

    template<bool IsConst>
    struct iterator_t
    {
        template<class U>
        using Constness_t = std::conditional_t<IsConst, std::add_const_t<U>, U>;

        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = T;
        using pointer           = Constness_t<value_type>*;
        using reference         = Constness_t<value_type>&;

        constexpr explicit iterator_t(Slot_t<T>* slot) : mSlot { slot } {  }

        constexpr reference operator*() const
        {
            return mSlot->mData;
        }

        constexpr pointer operator->() const
        {
            return &mSlot->mData;
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
            return (mSlot - it.mSlot) / sizeof(Slot_t<T>);
        }

        constexpr friend bool operator==(const iterator_t& lhs, const iterator_t& rhs)
        {
            return lhs.mSlot != rhs.mSlot;
        }

        constexpr friend bool operator!=(const iterator_t& lhs, const iterator_t& rhs)
        {
            return !(lhs == rhs);
        }

    private:
        Slot_t<T>* mSlot {  };
    };

    using iterator               = iterator_t<false>;
    using const_iterator         = iterator_t<true>;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using value_type = T;
    using size_type  = typename std::vector<T>::size_type;
    using difference_type = std::ptrdiff_t;
    using reference = value_type&;
    using const_reference = const value_type&;
    using pointer = value_type*;
    using const_pointer = const pointer;

    template<class... Args_t> 
    [[nodiscard]] constexpr Key_t emplace_back(Args_t&&... args)
    {
        Key_t key { mFreeIndex };
        if (mFreeIndex == mLastIndex) {
            auto& slot { mData.emplace_back(std::forward<Args_t>(args)...) };
            slot.mIndex = slot.mEraseIndex = mLastIndex;
            ++mFreeIndex;
        } else {
            new (&mData[mLastIndex].mValue) T { std::forward<Args_t>(args)... };
            mFreeIndex = mData[mFreeIndex].mIndex;
            mData[mFreeIndex].mIndex = mLastIndex;
            mData[mLastIndex].mEraseIndex = mFreeIndex;
        }
        ++mLastIndex;

        return key;
    }

    constexpr void erase(Key_t&& slot)
    {
        assert(slot.mIndex != std::numeric_limits<std::size_t>::max());

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

    constexpr void erase(const_iterator it)
    {
        auto idx { std::distance(begin(), it) };
        erase(Key_t(mData[idx].mEraseIndex));
    }

    constexpr T& operator[](const Key_t& slot)
    {
        assert(slot.mIndex != std::numeric_limits<std::size_t>::max());
        return mData[mData[slot.mIndex].mIndex].mValue;
    }

    const T& operator[](const Key_t& slot) const
    {
        assert(slot.mIndex != std::numeric_limits<std::size_t>::max());
        return mData[mData[slot.mIndex].mIndex].mValue;
    }

    iterator               begin()         { return { mData.data() }; }
    const_iterator         begin()   const { return { mData.data() }; }
    reverse_iterator       rbegin()        { return { &mData.back() }; }
    const_reverse_iterator rbegin()  const { return { &mData.back() }; }
    const_reverse_iterator crbegin() const { return { &mData.back() }; }

    iterator               end()         { return { mData.data() + mLastIndex }; }
    const_iterator         end()   const { return { mData.data() + mLastIndex }; }
    reverse_iterator       rend()        { return { --begin() }; }
    const_reverse_iterator rend()  const { return { --begin() }; }
    const_reverse_iterator crend() const { return { --begin() }; }

private:
    std::size_t mFreeIndex {  };
    std::size_t mLastIndex {  };
    std::vector<Slot_t<T>> mData {  };
};

} // namespace ECS
