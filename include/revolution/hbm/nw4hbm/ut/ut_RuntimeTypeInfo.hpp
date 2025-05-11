#ifndef RVL_SDK_HBM_NW4HBM_UT_RUNTIME_TYPE_INFO_HPP
#define RVL_SDK_HBM_NW4HBM_UT_RUNTIME_TYPE_INFO_HPP

/*******************************************************************************
 * headers
 */

#include "revolution/types.h" // nullptr

/*******************************************************************************
 * class
 */

namespace nw4hbm { namespace ut
{
	namespace detail
	{
		// [SGLEA4]/GormitiDebug.elf:.debug_info::0x478206
		class RuntimeTypeInfo
		{
		// methods
		public:
			// cdtors
			RuntimeTypeInfo(const RuntimeTypeInfo *parent):
				mParentTypeInfo(parent)
			{}

			// methods
			bool IsDerivedFrom(const RuntimeTypeInfo *typeInfo) const
			{
				const RuntimeTypeInfo *self;
				for (self = this; self; self = self->mParentTypeInfo)
				{
					if (self == typeInfo)
						return true;
				}

				return false;
			}

		// members
		private:
			const RuntimeTypeInfo	*mParentTypeInfo;	// size 0x04, offset 0x00
		}; // size 0x04

		template <class T>
		const RuntimeTypeInfo *GetTypeInfoFromPtr_(T *)
		{
			return &T::typeInfo;
		}
	} // namespace detail

	template <typename U, class T>
	U DynamicCast(T *obj)
	{
		const detail::RuntimeTypeInfo *typeInfoU =
			detail::GetTypeInfoFromPtr_(static_cast<U>(nullptr));

		if (obj->GetRuntimeTypeInfo()->IsDerivedFrom(typeInfoU))
			return reinterpret_cast<U>(obj);

		return nullptr;
	}
}} // namespace nw4hbm::ut

#endif // RVL_SDK_HBM_NW4HBM_UT_RUNTIME_TYPE_INFO_HPP
