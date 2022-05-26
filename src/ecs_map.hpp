#include <vector>
#include <cstdint>

template<class T>
struct Slot_t
{
    std::size_t mDataIndex {  };
    std::size_t mEraseIndex {  };
    std::uint8_t mData[sizeof(T)] {  };

    template<class... Args_t>
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
    std::vector<std::size_t> mFreeIndexs  {  };
    std::vector<std::size_t> mEraseIndexs {  };
    std::vector<T> mData {  };

    template<class... Args_t>
    void emplace(Args_t&&... args)
    {
        if (mFreeIndex == mData.size()) {
            mData.emplace_back(std::forward<Args_t>(args)...);
            ++mFreeIndex;
        } else {
            mFreeIndexs[mLastIndex];
        }
    }
};
