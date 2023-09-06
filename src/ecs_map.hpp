#pragma once

#include <iterator>
#include <vector>

namespace ECS
{

template<class T>
struct ECSMap_t
{
    struct Slot_t;

    using value_type      = typename std::vector<Slot_t>::value_type;
    using size_type       = typename std::vector<Slot_t>::size_type;
    using difference_type = typename std::vector<Slot_t>::difference_type;
    using reference       = typename std::vector<Slot_t>::reference;
    using const_reference = typename std::vector<Slot_t>::const_reference;
    using pointer         = typename std::vector<Slot_t>::pointer;
    using const_pointer   = typename std::vector<Slot_t>::const_pointer;

    using iterator               = typename std::vector<Slot_t>::iterator;
    using const_iterator         = typename std::vector<Slot_t>::const_iterator;
    using reverse_iterator       = typename std::vector<Slot_t>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<Slot_t>::const_reverse_iterator;

    struct Key_t
    {
        using value_type = T;

        friend ECSMap_t;

        constexpr Key_t() {  };
        constexpr Key_t(size_type index) : mIndex { index } {  };

        constexpr auto GetIndex() const -> size_type { return mIndex; }

        constexpr operator size_type() { return mIndex; }
    private:
        size_type mIndex {  };
    };

    struct Slot_t
    {
        using value_type = T;
        friend ECSMap_t;
    private:
        T mValue {  };
        size_type mEraseIndex {  };
        size_type mIndex {  };
    public:
        template<class... Args_t> constexpr
        Slot_t(Args_t&&... args)
            : mValue { std::forward<Args_t>(args)... } {  }

              T& value()       { return mValue; }
        const T& value() const { return mValue; }

        Key_t key() const { return { mEraseIndex }; }
    };

    constexpr explicit ECSMap_t() = default;

    constexpr auto push_back(const T& value) -> void
    {
        emplace_back(std::move(value));
    }

    template<class... Args_t> [[nodiscard]] constexpr auto
    emplace_back(Args_t&&... args) -> reference
    {
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
        return mData[mLastIndex++];
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

    constexpr auto clear() -> void
    {
        mFreeIndex = mLastIndex = 0;
        mData.clear();
    }

    constexpr auto size() const -> size_type { return mLastIndex; }

    constexpr auto erase(const_iterator it) -> void
    {
        erase(it->key());
    }

    constexpr auto next_key() const -> Key_t
    {
        return { mFreeIndex };
    }

    constexpr auto get_key(size_type pos) const -> Key_t
    {
        return { mData[pos].mEraseIndex };
    }

    constexpr auto operator[](Key_t key) -> T&
    {
        return mData[mData[key.mIndex].mIndex].mValue;
    }

    constexpr auto operator[](Key_t key) const -> const T&
    {
        return mData[mData[key.mIndex].mIndex].mValue;
    }

    constexpr auto operator[](size_type pos) -> T&
    {
        return mData[pos].mValue;
    }

    constexpr auto operator[](size_type pos) const -> const T&
    {
        return mData[pos].mValue;
    }

    constexpr auto begin()         -> iterator               { return mData.begin();   }
    constexpr auto begin()   const -> const_iterator         { return mData.begin();   }
    constexpr auto cbegin()  const -> const_iterator         { return mData.cbegin();  }
    constexpr auto rbegin()        -> reverse_iterator       { return std::make_reverse_iterator(end()); }
    constexpr auto rbegin()  const -> const_reverse_iterator { return std::make_reverse_iterator(end()); }
    constexpr auto crbegin() const -> const_reverse_iterator { return std::make_reverse_iterator(end()); }

    constexpr auto end()         -> iterator               { return begin() + mLastIndex; }
    constexpr auto end()   const -> const_iterator         { return begin() + mLastIndex; }
    constexpr auto cend()  const -> const_iterator         { return begin() + mLastIndex; }
    constexpr auto rend()        -> reverse_iterator       { return std::make_reverse_iterator(begin());  }
    constexpr auto rend()  const -> const_reverse_iterator { return std::make_reverse_iterator(begin());  }
    constexpr auto crend() const -> const_reverse_iterator { return std::make_reverse_iterator(begin()); }

private:
    size_type mFreeIndex {  };
    size_type mLastIndex {  };
    std::vector<Slot_t> mData {  };
};

} // namespace ECS
