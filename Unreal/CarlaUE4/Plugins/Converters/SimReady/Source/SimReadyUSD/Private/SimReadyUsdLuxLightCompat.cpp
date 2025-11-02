// SPDX-FileCopyrightText: Copyright (c) 2022-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUsdLuxLightCompat.h"

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatLightAttrs = 
{
#if PXR_VERSION >= 2102
    pxr::TfToken("intensity"),
    pxr::TfToken("exposure"),
    pxr::TfToken("diffuse"),
    pxr::TfToken("specular"),
    pxr::TfToken("normalize"),
    pxr::TfToken("color"),
    pxr::TfToken("enableColorTemperature"),
    pxr::TfToken("colorTemperature")
#else
    pxr::TfToken("inputs:intensity"),
    pxr::TfToken("inputs:exposure"),
    pxr::TfToken("inputs:diffuse"),
    pxr::TfToken("inputs:specular"),
    pxr::TfToken("inputs:normalize"),
    pxr::TfToken("inputs:color"),
    pxr::TfToken("inputs:enableColorTemperature"),
    pxr::TfToken("inputs:colorTemperature")
#endif
};

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatCylinderLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("length"),
    pxr::TfToken("radius")
#else
    pxr::TfToken("inputs:length"),
    pxr::TfToken("inputs:radius")
#endif
};
const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatDiskLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("radius")
#else
    pxr::TfToken("inputs:radius")
#endif
};

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatDistantLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("angle"),
    pxr::TfToken("intensity")
#else
    pxr::TfToken("inputs:angle"),
    pxr::TfToken("inputs:intensity")

#endif
};

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatDomeLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("texture:file"),
    pxr::TfToken("texture:format")
#else
    pxr::TfToken("inputs:texture:file"),
    pxr::TfToken("inputs:texture:format")
#endif
};

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatRectLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("width"),
    pxr::TfToken("height"),
    pxr::TfToken("texture:file")
#else
    pxr::TfToken("inputs:width"),
    pxr::TfToken("inputs:height"),
    pxr::TfToken("inputs:texture:file")
#endif
};

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatShapingAPIAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("shaping:focus"),
    pxr::TfToken("shaping:focusTint"),
    pxr::TfToken("shaping:cone:angle"),
    pxr::TfToken("shaping:cone:softness"),
    pxr::TfToken("shaping:ies:file"),
    pxr::TfToken("shaping:ies:angleScale"),
    pxr::TfToken("shaping:ies:normalize")
#else
    pxr::TfToken("inputs:shaping:focus"),
    pxr::TfToken("inputs:shaping:focusTint"),
    pxr::TfToken("inputs:shaping:cone:angle"),
    pxr::TfToken("inputs:shaping:cone:softness"),
    pxr::TfToken("inputs:shaping:ies:file"),
    pxr::TfToken("inputs:shaping:ies:angleScale"),
    pxr::TfToken("inputs:shaping:ies:normalize")
#endif
};

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CompatSphereLightAttrs
{
#if PXR_VERSION >= 2102
    pxr::TfToken("radius")
#else
    pxr::TfToken("inputs:radius")
#endif
};

// Handle 21.11 changes for the definition of "is a prim a light?"
bool SimReadyUsdLuxLightCompat::PrimIsALight(const pxr::UsdPrim& Prim)
{
#if PXR_VERSION >= 2111
    return Prim.HasAPI<pxr::UsdLuxLightAPI>();
#else
    return Prim.IsA<pxr::UsdLuxLight>();
#endif // PXR_VERSION >= 2111

}

// Universal attribute retrieval for UsdLuxLight attributes updated with the "inputs:" connectible schema change
pxr::UsdAttribute SimReadyUsdLuxLightCompat::GetLightAttr(const pxr::UsdPrim& Light, const pxr::UsdAttribute& DefaultAttr, bool bPreferNewSchema)
{
    // * Light has only "intensity" authored --- return "intensity"
    // * Light has only "inputs:intensity" authored --- return "inputs:intensity"
    // * Light has both "inputs:intensity" and "intensity" authored --- depends on bPreferNewSchema

    pxr::TfToken DefaultAttrName = DefaultAttr.GetName();
    // assume CompatAttr is the one with "inputs:" for now
    pxr::UsdAttribute CompatAttr = Light.GetAttribute(CompatName(DefaultAttrName));
    if (!CompatAttr)
    {
        return DefaultAttr;
    }
    else if (CompatAttr.IsAuthored() && !DefaultAttr.IsAuthored())
    {
        return CompatAttr;
    }

    // In 20.08, the CompatAttr is the new schema
    // In 21.02, the CompatAttr is the old schema
#if PXR_VERSION >= 2102
    return (bPreferNewSchema) ? DefaultAttr : CompatAttr;
#else
    return (bPreferNewSchema) ? CompatAttr : DefaultAttr;
#endif
}


const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(pxr::TfTokenVector DefaultTokenVec, const pxr::TfTokenVector& CompatTokenVec)
{
    DefaultTokenVec.insert(DefaultTokenVec.end(), CompatTokenVec.begin(), CompatTokenVec.end());
    return DefaultTokenVec;
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::LightGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
#if PXR_VERSION >= 2111
        pxr::UsdLuxLightAPI::GetSchemaAttributeNames(false),
#else
        pxr::UsdLuxLight::GetSchemaAttributeNames(false),
#endif //PXR_VERSION >= 2111
        SimReadyUsdLuxLightCompat::CompatLightAttrs);
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::CylinderLightGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxCylinderLight::GetSchemaAttributeNames(false), 
        SimReadyUsdLuxLightCompat::CompatCylinderLightAttrs);
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::DiskLightGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxDiskLight::GetSchemaAttributeNames(false),
        SimReadyUsdLuxLightCompat::CompatDiskLightAttrs);
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::DistantLightGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxDistantLight::GetSchemaAttributeNames(false),
        SimReadyUsdLuxLightCompat::CompatDistantLightAttrs);
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::DomeLightGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxDomeLight::GetSchemaAttributeNames(false),
        SimReadyUsdLuxLightCompat::CompatDomeLightAttrs);
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::RectLightGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxRectLight::GetSchemaAttributeNames(false),
        SimReadyUsdLuxLightCompat::CompatRectLightAttrs);
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::ShapingAPIGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxShapingAPI::GetSchemaAttributeNames(false),
        SimReadyUsdLuxLightCompat::CompatShapingAPIAttrs);
}

const pxr::TfTokenVector SimReadyUsdLuxLightCompat::SphereLightGetSchemaAttributeNames()
{
    return SimReadyUsdLuxLightCompat::CombineWithCompatAttrNames(
        pxr::UsdLuxSphereLight::GetSchemaAttributeNames(false),
        SimReadyUsdLuxLightCompat::CompatSphereLightAttrs);
}


// UsdLuxDistantLight
void SimReadyUsdLuxLightCompat::CreateAngleAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    auto DistantLight = pxr::UsdLuxDistantLight(Light);
    if (DistantLight)
    {
        pxr::UsdAttribute Attr = SetNewAttr(DistantLight.CreateAngleAttr(), Value);
        Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
    }
}

// UsdLuxDomeLight
void SimReadyUsdLuxLightCompat::CreateTextureFileAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr;
    if (Light.IsA<pxr::UsdLuxDomeLight>())
    {
        Attr = SetNewAttr(pxr::UsdLuxDomeLight(Light).CreateTextureFileAttr(), Value);
    }
    else if (Light.IsA<pxr::UsdLuxRectLight>())
    {
        Attr = SetNewAttr(pxr::UsdLuxRectLight(Light).CreateTextureFileAttr(), Value);
    }

    if (Attr)
    {
        Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
    }
}

void SimReadyUsdLuxLightCompat::CreateTextureFormatAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    auto DomeLight = pxr::UsdLuxDomeLight(Light);
    if (DomeLight)
    {
        pxr::UsdAttribute Attr = SetNewAttr(DomeLight.CreateTextureFormatAttr(), Value);
        Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
    }
}


// UsdLuxSphereLight
void SimReadyUsdLuxLightCompat::CreateRadiusAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    auto SphereLight = pxr::UsdLuxSphereLight(Light);
    if (SphereLight)
    {
        pxr::UsdAttribute Attr = SetNewAttr(SphereLight.CreateRadiusAttr(), Value);
        Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
    }
}

// UsdLuxShapingAPI
void SimReadyUsdLuxLightCompat::CreateShapingConeAngleAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxShapingAPI::Apply(Light).CreateShapingConeAngleAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void SimReadyUsdLuxLightCompat::CreateShapingConeSoftnessAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    pxr::UsdAttribute Attr = SetNewAttr(pxr::UsdLuxShapingAPI::Apply(Light).CreateShapingConeSoftnessAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

// UsdLuxRectLight
void SimReadyUsdLuxLightCompat::CreateWidthAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    auto RectLight = pxr::UsdLuxRectLight(Light);
    if (RectLight)
    {
        pxr::UsdAttribute Attr = SetNewAttr(RectLight.CreateWidthAttr(), Value);
        Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
    }
}

void SimReadyUsdLuxLightCompat::CreateHeightAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
    auto RectLight = pxr::UsdLuxRectLight(Light);
    if (RectLight)
    {
        pxr::UsdAttribute Attr = SetNewAttr(RectLight.CreateHeightAttr(), Value);
        Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
    }
}

// UsdLuxLightAPI
void SimReadyUsdLuxLightCompat::CreateEnableColorTemperatureAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateEnableColorTemperatureAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void SimReadyUsdLuxLightCompat::CreateColorTemperatureAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateColorTemperatureAttr(), Value);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value);
}

void SimReadyUsdLuxLightCompat::CreateColorAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value, pxr::UsdTimeCode Time)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateColorAttr(), Value, Time);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value, Time);
}

void SimReadyUsdLuxLightCompat::CreateIntensityAttr(const pxr::UsdPrim& Light, const pxr::VtValue& Value, pxr::UsdTimeCode Time)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111
    pxr::UsdAttribute Attr = SetNewAttr(LightAPI.CreateIntensityAttr(), Value, Time);
    Light.CreateAttribute(CompatName(Attr), Attr.GetTypeName(), false).Set(Value, Time);
}

void SimReadyUsdLuxLightCompat::CreateExtentAttr(const pxr::UsdPrim& Light, pxr::UsdTimeCode Time)
{
#if PXR_VERSION >= 2111
    // The core lights UsdLuxCylinderLight, UsdLuxDiskLight, UsdLuxRectLight, UsdLuxSphereLight, and UsdLuxPortalLight
    // all inherit from from UsdLuxBoundableLightBase, are now Boundable, and have extents/bbox computations.
    pxr::UsdGeomBoundable Boundable(Light);
    // NOTE: not all the lights are boundable Light 
    if (Boundable)
    {
        pxr::VtArray<pxr::GfVec3f> Extent;
        pxr::UsdGeomBoundable::ComputeExtentFromPlugins(Boundable, pxr::UsdTimeCode::Default(), &Extent);
        Boundable.CreateExtentAttr().Set(Extent);
    }
#else
    if (Light.IsA<pxr::UsdLuxRectLight>() || Light.IsA<pxr::UsdLuxSphereLight>()
        //|| Light.IsA<pxr::UsdLuxCylinderLight>() || Light.IsA<pxr::UsdLuxDiskLight>()
        //|| Light.IsA<pxr::UsdLuxPortalLight>()
        )
    {
        pxr::VtArray<pxr::GfVec3f> Extent;
        if (Light.IsA<pxr::UsdLuxRectLight>())
        {
            pxr::UsdLuxRectLight RectLight = pxr::UsdLuxRectLight(Light);
            auto WidthAttr = RectLight.GetWidthAttr();
            auto HeightAttr = RectLight.GetHeightAttr();
            float Width;
            float Height;
            if (WidthAttr && WidthAttr.Get(&Width) && HeightAttr && HeightAttr.Get(&Height))
            {
                Extent.push_back(pxr::GfVec3f(-Width * 0.5f, -Height * 0.5f, 0));
                Extent.push_back(pxr::GfVec3f(Width * 0.5f, Height * 0.5f, 0));
            }
        }
        else if (Light.IsA<pxr::UsdLuxSphereLight>())
        {
            pxr::UsdLuxSphereLight SphereLight = pxr::UsdLuxSphereLight(Light);
            auto RadiusAttr = SphereLight.GetRadiusAttr();
            float Radius;
            if (RadiusAttr && RadiusAttr.Get(&Radius))
            {
                Extent.push_back(pxr::GfVec3f(-Radius));
                Extent.push_back(pxr::GfVec3f(Radius));
            }
        }

        Light.CreateAttribute(pxr::UsdGeomTokens->extent, pxr::SdfValueTypeNames->Float3Array, false).Set(Extent);
    }
#endif //PXR_VERSION >= 2111
}

pxr::UsdCollectionAPI SimReadyUsdLuxLightCompat::GetLightLinkCollectionAPI(const pxr::UsdPrim& Light)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111

    return LightAPI.GetLightLinkCollectionAPI();
}

pxr::UsdCollectionAPI SimReadyUsdLuxLightCompat::GetShadowLinkCollectionAPI(const pxr::UsdPrim& Light)
{
#if PXR_VERSION >= 2111
    pxr::UsdLuxLightAPI LightAPI(Light);
#else
    pxr::UsdLuxLight LightAPI(Light);
#endif //PXR_VERSION >= 2111

    return LightAPI.GetShadowLinkCollectionAPI();
}

/////////////////////////////////////////////////////////
// Internal Helper Methods
/////////////////////////////////////////////////////////

// A simple utility for erasing a substring from a string
void SimReadyUsdLuxLightCompat::EraseSubStr(std::string& MainStr, const std::string& ToErase)
{
    // Search for the substring in string
    size_t Pos = MainStr.find(ToErase);
    if (Pos != std::string::npos)
    {
        // If found then erase it from string
        MainStr.erase(Pos, ToErase.length());
    }
}

// If version < 21.02, prepend "inputs:" to attribute name 
// If verseion >= 21.02, remove "inputs:" from attribute name
pxr::TfToken SimReadyUsdLuxLightCompat::CompatName(const pxr::TfToken& OriginalAttrName)
{
#if PXR_VERSION >= 2102
    // Convert "inputs:intensity" to "intensity"
    std::string AttrName(OriginalAttrName);
    EraseSubStr(AttrName, SimReadyUsdLuxLightCompat::LuxInputsStr);
#else
    // Convert "intensity" to "inputs:intensity"
    std::string AttrName(SimReadyUsdLuxLightCompat::LuxInputsStr);
    AttrName += OriginalAttrName;
#endif
    return pxr::TfToken(AttrName);
    //bool bHasInputsPrefix = pxr::TfStringStartsWith(DefaultAttrName, SimReadyUsdLuxLightCompat::LuxInputsStr);
}

pxr::TfToken SimReadyUsdLuxLightCompat::CompatName(const pxr::UsdAttribute& Attr)
{
    // Convert "intensity" to "inputs:intensity"
    return CompatName(Attr.GetName());
}

// These allow us to create, set, and assign an attribute in one line
inline pxr::UsdAttribute SimReadyUsdLuxLightCompat::SetNewAttr(pxr::UsdAttribute NewAttr, const pxr::VtValue& Value, pxr::UsdTimeCode& Time)
{
    NewAttr.Set(Value, Time);
    return NewAttr;
}
inline pxr::UsdAttribute SimReadyUsdLuxLightCompat::SetNewAttr(pxr::UsdAttribute NewAttr, const  pxr::VtValue& Value)
{
    NewAttr.Set(Value);
    return NewAttr;
}
