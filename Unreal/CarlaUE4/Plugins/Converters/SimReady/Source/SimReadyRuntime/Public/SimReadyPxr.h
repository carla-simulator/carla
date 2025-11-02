// SPDX-FileCopyrightText: Copyright (c) 2020-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#pragma once

#if PLATFORM_LINUX

THIRD_PARTY_INCLUDES_START
#pragma warning(push)
#pragma warning(disable: 4193) /* #pragma warning(pop): no matching '#pragma warning(push)', the pop is in USDIncludesEnd.h */
#pragma warning(disable: 4582) /* Workaround union in pxr/usd/pcp/mapFunction.h */
#pragma warning(disable: 4583)
#pragma warning(disable: 4003) /* pxr/usd/sdf/fileFormat.h BOOST_PP_SEQ_DETAIL_IS_NOT_EMPTY during static analysis */
#pragma warning(disable: 5033) /* 'register' is no longer a supported storage class */
#pragma warning(disable: 6319)

#pragma push_macro("check")
#undef check // Boost function is named 'check' in boost\python\detail\convertible.hpp

#endif

#ifdef _WIN64
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/AllowWindowsPlatformAtomics.h"
#pragma warning(push)
#pragma warning(disable:4244) // = Conversion from double to float / int to float
#pragma warning(disable:4305) // argument truncation from double to float
#pragma warning(disable:4643) // fwd declarations in namespace std
#pragma warning(disable:4800) // int to bool
#pragma warning(disable:4996) // call to std::copy with parameters that may be unsafe
#pragma warning(disable:4668)
#pragma warning(disable:4946)
// https://answers.unrealengine.com/questions/607946/anonymous-union-with-none-trivial-type.html
#pragma warning(disable:4582) // PcpMapFunction::_Data::localPairs': constructor is not implicitly called
#pragma warning(disable:4583) // PcpMapFunction::_Data::localPairs': destructor is not implicitly called
#pragma push_macro("check")
#pragma push_macro("__MSVC_RUNTIME_CHECKS")
#undef check
#undef __MSVC_RUNTIME_CHECKS
#endif // _WIN64

#include <pxr/base/gf/matrix4f.h>
#include <pxr/base/gf/matrix3f.h>
#include <pxr/base/gf/matrix2f.h>
#include <pxr/base/gf/quaternion.h>
#include <pxr/base/gf/rotation.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/ar/asset.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/pcp/layerStack.h>
#include <pxr/usd/sdf/copyUtils.h>
#include <pxr/usd/sdf/proxyTypes.h>
#include <pxr/usd/sdf/types.h>
#include <pxr/usd/sdf/layerUtils.h>
#include <pxr/usd/sdf/attributeSpec.h>
#include <pxr/usd/sdf/relationshipSpec.h>
#include <pxr/usd/sdf/fileFormat.h>
#include <pxr/usd/sdf/textFileFormat.h>
#include <pxr/usd/ar/resolver.h>
#include <pxr/usd/kind/registry.h>
#include <pxr/usd/usd/editContext.h>
#include <pxr/usd/usd/primFlags.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/notice.h>
#include <pxr/usd/usd/modelAPI.h>
#include <pxr/usd/usd/relationship.h>
#include <pxr/usd/usd/payloads.h>
#include <pxr/usd/usd/primCompositionQuery.h>
#include <pxr/usd/usdGeom/metrics.h> 
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/points.h>
#include <pxr/usd/usdGeom/pointInstancer.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/scope.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/curves.h>
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/xformCommonAPI.h>
#include <pxr/usd/usdLux/cylinderLight.h>
#include <pxr/usd/usdLux/distantLight.h>
#include <pxr/usd/usdLux/rectLight.h>
#include <pxr/usd/usdLux/sphereLight.h>
#include <pxr/usd/usdLux/shapingAPI.h>
#include <pxr/usd/usdLux/shadowAPI.h>
#include <pxr/usd/usdLux/domeLight.h>
#include <pxr/usd/usdLux/diskLight.h>
#include <pxr/usd/usdShade/tokens.h>
#include <pxr/usd/usdSkel/animation.h>
#include <pxr/usd/usdSkel/bindingAPI.h>
#include <pxr/usd/usdSkel/cache.h>
#include <pxr/usd/usdSkel/root.h>
#include <pxr/usd/usdSkel/skeleton.h>
#include <pxr/usd/usdSkel/skeletonQuery.h>
#include <pxr/usd/usdSkel/tokens.h>
#include <pxr/usd/usdSkel/topology.h>
#include <pxr/usd/usdSkel/utils.h>
#include <pxr/usd/usdUtils/dependencies.h>
#include <pxr/usd/usdUtils/pipeline.h>
#include <pxr/usd/usdUtils/flattenLayerStack.h>
#include <pxr/usd/usdUtils/stitch.h>

#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdUI/nodeGraphNodeAPI.h>

#ifdef _WIN64
#pragma pop_macro("check")
#pragma pop_macro("__MSVC_RUNTIME_CHECKS")
#pragma warning(pop)
#include "Windows/HideWindowsPlatformAtomics.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif // _WIN64

#if PLATFORM_LINUX
#pragma warning(pop)
#pragma pop_macro("check")
THIRD_PARTY_INCLUDES_END
#endif

#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/utility/value_init.hpp>

#if PLATFORM_WINDOWS
// Fix C4506 caused by "VT_API Vt_DefaultValueHolder Vt_DefaultValueFactory<VT_TYPE(elem)>::Invoke();" in value.h
PXR_NAMESPACE_OPEN_SCOPE
#define _VT_DECLARE_ZERO_VALUE_FACTORY(r, unused, elem)                 \
template <>                                                             \
VT_API Vt_DefaultValueHolder Vt_DefaultValueFactory<VT_TYPE(elem)>::Invoke(){return Vt_DefaultValueHolder::Create<VT_TYPE(elem)>(boost::value_initialized<VT_TYPE(elem)>().data());}
BOOST_PP_SEQ_FOR_EACH(_VT_DECLARE_ZERO_VALUE_FACTORY,
    unused,
    VT_VEC_VALUE_TYPES
    VT_MATRIX_VALUE_TYPES
    VT_QUATERNION_VALUE_TYPES)
#undef _VT_DECLARE_ZERO_VALUE_FACTORY
PXR_NAMESPACE_CLOSE_SCOPE
#endif 

// For TMap
inline uint32 GetTypeHash(const pxr::SdfPath& Path)
{
    return Path.GetHash();
}

namespace UE4GetTypeHashExists_Private
{
    // For TMap
    inline uint32 GetTypeHash(const pxr::SdfPath& Path)
    {
        return ::GetTypeHash(Path);
    }
}
