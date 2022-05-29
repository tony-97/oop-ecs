#include <memory>
#include <utility>
#include <vector>
#include <cstdint>
#include <limits>

#include <cassert>

namespace ECS
{

template<class T>
struct Slot_t
{
    std::size_t mDataIndex {  };
    std::size_t mEraseIndex {  };
    T mData {  };

    template<class... Args_t> constexpr
    Slot_t(Args_t&&... args)
        : mData { std::forward<Args_t>(args)... } {  }
};

template<class T>
struct ECSMap_t
{
    std::size_t mFreeIndex {  };
    std::size_t mLastIndex {  };
    std::vector<std::size_t> mIndexes  {  };
    std::vector<std::size_t> mEraseIndexs {  };
    std::vector<T> mData {  };

    struct Key_t
    {
        Key_t(Key_t&& other) : mIndex { other.mIndex }
        {
            other.mIndex = std::numeric_limits<std::size_t>::max();
        }

        Key_t& operator=(Key_t&& other)
        {
            mIndex = other.mIndex;
            other.mIndex = std::numeric_limits<std::size_t>::max();
        }
    private:
        friend ECSMap_t;

        Key_t(std::size_t index)
            : mIndex { index } {  };

        std::size_t mIndex {  };
    };

    template<class... Args_t> 
    [[nodiscard]] constexpr Key_t emplace_back(Args_t&&... args)
    {
        Key_t key { mFreeIndex };
        if (mFreeIndex == mLastIndex) {
            mIndexes.emplace_back(mData.size());
            mEraseIndexs.emplace_back(mData.size());
            mData.emplace_back(std::forward<Args_t>(args)...);
            ++mFreeIndex;
        } else {
            new (&mData[mLastIndex]) T { std::forward<Args_t>(args)... };
            mEraseIndexs[mLastIndex] = mFreeIndex; 
            mFreeIndex = mIndexes[mFreeIndex];
        }
        ++mLastIndex;

        return key;
    }

    void erase(Key_t&& slot)
    {
        assert(slot.mIndex != std::numeric_limits<std::size_t>::max());

        --mLastIndex;
        mData[mIndexes[slot.mIndex]].~T();
        mData[mIndexes[slot.mIndex]] = std::move(mData[mLastIndex]);
        mEraseIndexs[mIndexes[slot.mIndex]] = mEraseIndexs[mLastIndex];
        // update the slot
        mIndexes[mEraseIndexs[mLastIndex]] = mIndexes[slot.mIndex];
        // update the index list
        mIndexes[slot.mIndex] = mFreeIndex;
        mFreeIndex = slot.mIndex;
    }

          T& operator[](const Key_t& slot)       { return mData[mIndexes[slot.mIndex]]; }
    const T& operator[](const Key_t& slot) const { return mData[mIndexes[slot.mIndex]]; }
};

} // namespace ECS
