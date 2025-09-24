#ifndef RVL_SDK_HBM_NW4HBM_UT_RUNTIME_TYPE_INFO_HPP
#define RVL_SDK_HBM_NW4HBM_UT_RUNTIME_TYPE_INFO_HPP

#include "revolution/types.h" // nullptr

namespace nw4hbm {
namespace ut {
#define NW4R_UT_RUNTIME_TYPEINFO                                                                      \
    virtual const nw4hbm::ut::detail::RuntimeTypeInfo* GetRuntimeTypeInfo() const { return &typeInfo; } \
    static const nw4hbm::ut::detail::RuntimeTypeInfo typeInfo

#define NW4R_UT_GET_RUNTIME_TYPEINFO(T) const nw4hbm::ut::detail::RuntimeTypeInfo T::typeInfo(nullptr);

#define NW4R_UT_GET_DERIVED_RUNTIME_TYPEINFO(T, D) const nw4hbm::ut::detail::RuntimeTypeInfo T::typeInfo(&D::typeInfo);

namespace detail {
struct RuntimeTypeInfo {
    explicit RuntimeTypeInfo(const RuntimeTypeInfo* base) : mParentTypeInfo(base) {}

    bool IsDerivedFrom(const RuntimeTypeInfo* base) const {
        for (const RuntimeTypeInfo* it = this; it != NULL; it = it->mParentTypeInfo) {
            if (it == base) {
                return true;
            }
        }
        return false;
    }
    const RuntimeTypeInfo* mParentTypeInfo; // 0x00
};

template <typename T> inline const RuntimeTypeInfo* GetTypeInfoFromPtr_(T* pPtr) { return &pPtr->typeInfo; }
} // namespace detail

template <typename TDerived, typename TBase> inline TDerived DynamicCast(TBase* pPtr) {
    const detail::RuntimeTypeInfo* pDerivedTypeInfo = detail::GetTypeInfoFromPtr_(static_cast<TDerived>(NULL));
    if (pPtr->GetRuntimeTypeInfo()->IsDerivedFrom(pDerivedTypeInfo)) {
        return static_cast<TDerived>(pPtr);
    }
    return NULL;
}
} // namespace ut
} // namespace nw4hbm

#endif // RVL_SDK_HBM_NW4HBM_UT_RUNTIME_TYPE_INFO_HPP
