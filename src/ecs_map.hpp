#include "helpers.hpp"
#include <vector>
#include <cstdint>
#include <limits>

namespace ECS
{

template<class T>
struct Slot_t
{
    std::size_t mDataIndex {  };
    std::size_t mEraseIndex {  };
    std::uint8_t mData[sizeof(T)] {  };

    template<class... Args_t> constexpr
    Slot_t(Args_t&&... args)
    {
        new (mData) T { std::forward<Args_t>(args)... };
    }
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

        std::size_t mIndex {  };
    };

    template<class... Args_t> constexpr
    Key_t emplace_back(Args_t&&... args)
    {
        Key_t key {  };
        if (mFreeIndex == mData.size()) {
            key.mIndex = mIndexes.emplace_back(mFreeIndex);
            mData.emplace_back(std::forward<Args_t>(args)...);
            ++mFreeIndex;
        } else {

        }

        return key;
    }

    void erase(Key_t&& slot)
    {
    }

          T& operator[](const Key_t& slot)       { return mData[mIndexes[slot.mIndex]]; }
    const T& operator[](const Key_t& slot) const { return mData[mIndexes[slot.mIndex]]; }
};

} // namespace ECS
