// SPDX-FileCopyrightText: Copyright (c) 2019-2025 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: MIT
//

#include "SimReadyUsdStageCtrl.h"
#include "USDCustomLayerData.h"
#include "SimReadyPathHelper.h"
#include "GenericPlatform/GenericPlatformFile.h"
#include "HAL/PlatformFilemanager.h"
#include "Misc/PathViews.h"
#pragma warning(push)
#pragma warning(disable: 4244 4305 4003)
#include <pxr/usd/sdf/attributeSpec.h>
#include <pxr/usd/sdf/copyUtils.h>
#include <pxr/usd/usdGeom/imageable.h>
#pragma warning(pop)

static const pxr::TfToken customLayerData("customLayerData");

static std::string NormalizePath(const std::string& path)
{
    static auto replaceAll = [](std::string str, const std::string& from, const std::string& to)
    {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos)
        {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
        }
        return str;
    };

    std::string finalPath = path;
    // FIXME: Need a better way to normalize path.
    finalPath = replaceAll(finalPath, "%3C", "<");
    finalPath = replaceAll(finalPath, "%3E", ">");
    finalPath = replaceAll(finalPath, "%20", " ");
    finalPath = replaceAll(finalPath, "%5C", "/");
    std::replace(finalPath.begin(), finalPath.end(), '\\', '/');

    return finalPath;
}

static std::string ComputeAbsolutePath(const pxr::SdfLayerRefPtr& rootLayer, const std::string& path)
{
    if (pxr::SdfLayer::IsAnonymousLayerIdentifier(path) || rootLayer->IsAnonymous())
    {
        return path;
    }
    else
    {
        // Compute the path through the resolver
        const std::string& absolutePath = rootLayer->ComputeAbsolutePath(path);
        return NormalizePath(absolutePath);
    }
}

static bool AreEqualPaths(const std::string& A, const std::string& B)
{
    FString StrPathA = NormalizePath(A).c_str();
    FString StrPathB = NormalizePath(B).c_str();

    // for comparing C:/ and c:/
#if PLATFORM_WINDOWS
    return StrPathA.Equals(StrPathB, ESearchCase::IgnoreCase);
#else
    return StrPathA.Equals(StrPathB);
#endif
}

std::vector<std::string> LayerTraverse(const pxr::SdfLayerRefPtr& layer, const std::string& identifier)
{
    std::vector<std::string> layerList;
    layerList.push_back(identifier);

    if (layer)
    {
        for (auto subLayerPath : layer->GetSubLayerPaths())
        {
            auto sublayerIdentifier = layer->ComputeAbsolutePath(subLayerPath);
            auto subLayer = pxr::SdfLayer::FindOrOpen(sublayerIdentifier);
            auto sublayers = LayerTraverse(subLayer, sublayerIdentifier);
            layerList.insert(std::end(layerList), std::begin(sublayers), std::end(sublayers));
        }
    }

    return layerList;
}

static bool fileExists(const char* fileUrl)
{
    return FPaths::FileExists(TCHAR_TO_UTF8(fileUrl));

}

// copyable static methods start here

bool SimReadyUsdStageCtrl::RemovePrimSpecInEditLayer(const pxr::UsdPrim& prim)
{
    auto layer = prim.GetStage()->GetEditTarget().GetLayer();
    auto primSpec = layer->GetPrimAtPath(prim.GetPath());
    if (primSpec)
    {
        auto parentSpec = primSpec->GetRealNameParent();
        if (parentSpec)
        {
            parentSpec->RemoveNameChild(primSpec);
            return true;
        }
    }
    return false;
}

pxr::SdfLayerHandle SimReadyUsdStageCtrl::HasOverriddenInStrongerLayer(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath, bool bOnlyActivePrim)
{
    auto LayerStack = Stage->GetLayerStack();

    for (auto Layer : LayerStack)
    {
        if (Layer == Stage->GetEditTarget().GetLayer())
        {
            break;
        }

        auto PrimSpec = Layer->GetPrimAtPath(PrimPath);

        if (!PrimSpec)
        {
            continue;
        }

        if (!bOnlyActivePrim || (PrimSpec->HasActive() && PrimSpec->GetActive()))
        {
            return Layer;
        }
    }

    return nullptr;
}

int SimReadyUsdStageCtrl::FindSubLayerIndex(const pxr::SdfLayerHandle& rootLayer, const std::string& path)
{
    auto paths = rootLayer->GetSubLayerPaths();

    auto identifier = rootLayer->ComputeAbsolutePath(path);

    for (int i = 0; i < paths.size(); ++i)
    {
        if (AreEqualPaths(rootLayer->ComputeAbsolutePath(paths[i]), identifier))
        {
            return i;
        }
    }

    return -1;
}

bool SimReadyUsdStageCtrl::SetSubLayerName(const pxr::SdfLayerHandle& rootLayer, int index, const pxr::TfToken& name)
{
    // Remove old name information
    auto subLayerPaths = rootLayer->GetSubLayerPaths();
    if (index < 0 || index >= subLayerPaths.size())
        return false;

    auto nameDict = rootLayer->GetFieldDictValueByKey(pxr::SdfPath::AbsoluteRootPath(), customLayerData, pxr::TfToken("omni_layer:names")).GetWithDefault<pxr::VtDictionary>();

    auto identifier = rootLayer->ComputeAbsolutePath(subLayerPaths[index]);

    for (const auto& entry : nameDict)
    {
        if (!AreEqualPaths(rootLayer->ComputeAbsolutePath(entry.first), identifier))
        {
            continue;
        }

        nameDict.erase(entry.first);
        break;
    }

    // Set new name information
    std::string layerFilename = pxr::TfGetBaseName((std::string)subLayerPaths[index]);

    nameDict.SetValueAtPath(layerFilename, pxr::VtValue(name));

    rootLayer->SetFieldDictValueByKey(pxr::SdfPath::AbsoluteRootPath(), customLayerData, pxr::TfToken("omni_layer:names"), pxr::VtValue(nameDict));

    return true;
}

pxr::SdfLayerRefPtr SimReadyUsdStageCtrl::CreateSubLayer(const pxr::SdfLayerHandle& rootLayer, int index)
{
    // Find position
    auto subLayerPaths = rootLayer->GetSubLayerPaths();
    if (index > subLayerPaths.size() || index < -1)
        return nullptr;

    pxr::SdfLayerRefPtr subLayer;
    std::string subLayerPath;

    if (rootLayer->IsAnonymous())
    {
        // Create anonymouse layer
        subLayer = pxr::SdfLayer::CreateAnonymous();
        subLayerPath = subLayer->GetIdentifier();
    }
    else
    {
        // Compose sub layer path
        std::string filename = pxr::TfStringGetBeforeSuffix(pxr::TfGetBaseName(rootLayer->GetIdentifier()));
        std::stringstream strStream;
        strStream << subLayerPaths.size() + 1;
        filename += "_" + strStream.str();

        std::srand((unsigned int)std::time(NULL));
        auto randomString = [](int length)
        {
            static const char alphanum[] =
                "0123456789"
                "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                "abcdefghijklmnopqrstuvwxyz";

            std::string result(length, ' ');
            for (int i = 0; i < length; ++i)
            {
                result[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
            }

            return result;
        };

        for (;;)
        {
            // Random string would be the same if two calls happen at the same time. We need to generate again if it happens.
            auto newFilename = filename + "_" + randomString(6) + ".sublayer.usd";

            auto identify = rootLayer->ComputeAbsolutePath("./" + newFilename);
            auto itr = std::find_if(subLayerPaths.begin(), subLayerPaths.end(),
                [&](const std::string& path)
                {
                    return AreEqualPaths(rootLayer->ComputeAbsolutePath(path), identify);
                }
            );

            if (itr == subLayerPaths.end())
            {
                subLayerPath = "./" + newFilename;
                break;
            }
        }

        // Create sub layer
        auto newPath = rootLayer->ComputeAbsolutePath(subLayerPath);

        subLayer = pxr::SdfLayer::FindOrOpen(newPath);
        if (subLayer)
            subLayer->Clear();
        else
            subLayer = pxr::SdfLayer::CreateNew(newPath);
    }

    // Add sub layer
    subLayerPaths.Insert(index, subLayerPath);

    return subLayer;
}


// the delete will only happen in the current target, and follows:
// 1. If the prim spec is a def, it will remove the prim spec.
// 2. If the prim spec is a over, it will only deactivate this prim.
// 3. If the prim spec is not existed, it will create over prim and deactivate it.
// 4. If there is an overridden in a stronger layer, it will report errors.
bool SimReadyUsdStageCtrl::RemovePrim(const pxr::UsdPrim& prim)
{
    if (!prim)
    {
        return false;
    }

    pxr::SdfChangeBlock changeBlock;
    auto layer = prim.GetStage()->GetEditTarget().GetLayer();
    auto primPath = prim.GetPath();
    if (CanRemovePrim(prim))
    {
        return RemovePrimSpecInEditLayer(prim);
    }
    else
    {
        if (HasOverriddenInStrongerLayer(prim.GetStage(), primPath, true))
        {
            // TODO: message?
            return false;
        }

        auto primSpec = layer->GetPrimAtPath(prim.GetPath());

        if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
        {
            // remove define at first
            if (RemovePrimSpecInEditLayer(prim))
            {
                // create empty over
                primSpec = pxr::SdfCreatePrimInLayer(layer, primPath);
            }
            else
            {
                return false;
            }
        }
        else
        {
            if (!primSpec)
            {
                // create empty over if there isn't one
                primSpec = pxr::SdfCreatePrimInLayer(layer, primPath);
            }
        }

        // deactive over
        if (primSpec)
        {
            primSpec->SetActive(false);
            return true;
        }
    }

    return false;
}

bool SimReadyUsdStageCtrl::RestorePrim(const pxr::SdfLayerRefPtr& AnonymousLayer, const std::string& LayerIdentifier, const pxr::SdfPath& PrimPath)
{
    pxr::SdfChangeBlock ChangeBlock;

    auto Layer = pxr::SdfLayer::FindOrOpen(LayerIdentifier);
    if (!Layer)
    {
        // Layer has been deleted
        return false;
    }

    auto PreviousPrimSpec = AnonymousLayer->GetPrimAtPath(PrimPath);
    auto CurrentPrimSpec = Layer->GetPrimAtPath(PrimPath);

    // there's an over, remove it
    if (!PreviousPrimSpec && CurrentPrimSpec)
    {
        auto parentSpec = CurrentPrimSpec->GetRealNameParent();
        if (parentSpec)
        {
            parentSpec->RemoveNameChild(CurrentPrimSpec);

            return true;
        }
    }
    // def is removed, recover it
    else if (PreviousPrimSpec)
    {
        if (!CurrentPrimSpec)
        {
            pxr::SdfCopySpec(AnonymousLayer, PrimPath, Layer, PrimPath);
        }
        else
        {
            CurrentPrimSpec->SetActive(true);
        }

        return true;
    }

    return false;
}

bool SimReadyUsdStageCtrl::CopyPrim(const pxr::UsdPrim& prim, pxr::SdfPath dstPath)
{
    auto primPath = prim.GetPath();
    bool bFromReferenceOrPayload = false;
    pxr::SdfLayerHandle introducingLayer;
    pxr::SdfPath introducingPrimPath;
    pxr::SdfPrimSpecHandle primSpecInDefLayer;

    GetIntroducingLayer(prim, introducingLayer, introducingPrimPath);

    for (auto primSpec : prim.GetPrimStack())
    {
        if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
        {
            if (introducingLayer != primSpec->GetLayer() && introducingPrimPath != primPath)
            {
                bFromReferenceOrPayload = true;
                primSpecInDefLayer = primSpec;
                break;
            }
        }
    }

    pxr::SdfChangeBlock changeBlock;
    std::vector<std::string> LayersHasPrimSpec;
    for (auto oldPrimSpec : prim.GetPrimStack())
    {
        if (!oldPrimSpec)
        {
            continue;
        }
        auto layer = oldPrimSpec->GetLayer();
        auto destLayer = prim.GetStage()->GetEditTarget().GetLayer();
        if (oldPrimSpec->HasReferences() || oldPrimSpec->HasPayloads() || oldPrimSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
        {
            pxr::SdfLayerRefPtr srcLayer;
            pxr::SdfPath srcPrimPath;
            if (layer != destLayer)
            {
                auto tempLayer = pxr::SdfLayer::CreateAnonymous();
                pxr::SdfCreatePrimInLayer(tempLayer, primPath);
                pxr::SdfCopySpec(layer, oldPrimSpec->GetPath(), tempLayer, primPath);
                ResolvePaths(layer->GetIdentifier(), tempLayer->GetIdentifier(), false);
                ResolvePaths(destLayer->GetIdentifier(), tempLayer->GetIdentifier(), true, true);

                if (!prim.GetStage()->HasLocalLayer(layer) && bFromReferenceOrPayload)
                {
                    auto Prefixes = oldPrimSpec->GetPath().GetPrefixes();
                    if (Prefixes.size() > 0)
                    {
                        ResolvePrimPathReferences(tempLayer, pxr::SdfPath(Prefixes[0]), pxr::SdfPath(introducingPrimPath));
                    }
                }

                while (LayersHasPrimSpec.size() > 0)
                {
                    auto layerHasPrim = LayersHasPrimSpec[LayersHasPrimSpec.size() - 1];
                    if (layerHasPrim != destLayer->GetIdentifier())
                    {
                        MergePrimSpec(tempLayer->GetIdentifier(), layerHasPrim, primPath.GetString(), false);
                    }
                    LayersHasPrimSpec.pop_back();
                }

                if (destLayer->GetPrimAtPath(primPath))
                {
                    MergePrimSpec(tempLayer->GetIdentifier(), destLayer->GetIdentifier(), primPath.GetString(), false);
                }

                srcLayer = tempLayer;
                srcPrimPath = primPath;
            }
            else
            {
                srcLayer = layer;
                srcPrimPath = oldPrimSpec->GetPath();
            }

            pxr::SdfCreatePrimInLayer(destLayer, dstPath);
            pxr::SdfCopySpec(srcLayer, srcPrimPath, destLayer, dstPath);
            break;
        }
        else
        {
            LayersHasPrimSpec.push_back(layer->GetIdentifier());
        }
    }

    return true;
}

bool SimReadyUsdStageCtrl::RenamePrim(const pxr::UsdPrim& prim, const pxr::TfToken& newName)
{
    if (!prim)
        return false;

    if (prim.IsPseudoRoot())
        return false;

    if (prim.GetPath().GetName() == newName)
        return false;

    auto dstPath = prim.GetPath().GetParentPath().AppendElementToken(newName);

    if (StitchPrimSpecs(prim, dstPath))
    {
        return RemovePrim(prim);
    }

    return false;
}

bool SimReadyUsdStageCtrl::IsAncestralPrim(const pxr::UsdPrim& Prim)
{
    if (Prim)
    {
        auto PrimIndex = Prim.GetPrimIndex();
        // walk the nodes, strong-to-weak
        for (auto Node : PrimIndex.GetNodeRange())
        {
            if (Node.IsDueToAncestor())
            {
                return true;
            }
        }
    }

    return false;
}

bool SimReadyUsdStageCtrl::IsAncestorGprim(const pxr::UsdStageRefPtr& Stage, const pxr::SdfPath& PrimPath)
{
    auto ParentPath = PrimPath;
    while (ParentPath != pxr::SdfPath::AbsoluteRootPath())
    {
        auto Prim = Stage->GetPrimAtPath(ParentPath);
        if (Prim && Prim.IsA<pxr::UsdGeomGprim>())
        {
            return true;
        }
        ParentPath = ParentPath.GetParentPath();
    }

    return false;
}


bool SimReadyUsdStageCtrl::CanRemovePrim(const pxr::UsdPrim& prim)
{
    if (IsAncestralPrim(prim))
    {
        return false;
    }

    auto editLayer = prim.GetStage()->GetEditTarget().GetLayer();

    bool bHasDelta = false;
    for (auto layer : prim.GetStage()->GetLayerStack())
    {
        auto primSpec = layer->GetPrimAtPath(prim.GetPath());
        if (!primSpec)
        {
            continue;
        }

        if (layer != editLayer && !bHasDelta && !layer->IsAnonymous())
        {
            bHasDelta = true;
        }
    }

    return !bHasDelta;
}

bool SimReadyUsdStageCtrl::GetIntroducingLayer(const pxr::UsdPrim& prim, pxr::SdfLayerHandle& outLayer, pxr::SdfPath& outPath)
{
    outLayer = nullptr;
    outPath = pxr::SdfPath();
    auto primStack = prim.GetPrimStack();

    for (auto primSpec : primStack)
    {
        if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
        {
            outLayer = primSpec->GetLayer();
            outPath = primSpec->GetPath();
            break;
        }
    }

    if (outLayer.IsInvalid())
    {
        return false;
    }

    auto query = pxr::UsdPrimCompositionQuery(prim);
    auto filter = pxr::UsdPrimCompositionQuery::Filter();
    filter.arcTypeFilter = pxr::UsdPrimCompositionQuery::ArcTypeFilter::ReferenceOrPayload;
    filter.arcIntroducedFilter = pxr::UsdPrimCompositionQuery::ArcIntroducedFilter::IntroducedInRootLayerStack;
    query.SetFilter(filter);
    auto arcs = query.GetCompositionArcs();
    for (auto arc : arcs)
    {
        auto layer = arc.GetIntroducingLayer();
        // Reference
        if (arc.GetArcType() == pxr::PcpArcTypeReference)
        {
            pxr::SdfReferenceEditorProxy proxy;
            pxr::SdfReference ref;

            if (arc.GetIntroducingListEditor(&proxy, &ref))
            {
                auto assetPath = layer->ComputeAbsolutePath(ref.GetAssetPath());
                if (AreEqualPaths(outLayer->GetIdentifier(), assetPath))
                {
                    outLayer = layer;
                    outPath = arc.GetIntroducingPrimPath();
                    break;
                }
            }
        }
        else if (arc.GetArcType() == pxr::PcpArcTypePayload)
        {
            pxr::SdfPayloadEditorProxy proxy;
            pxr::SdfPayload payload;

            if (arc.GetIntroducingListEditor(&proxy, &payload))
            {
                auto assetPath = layer->ComputeAbsolutePath(payload.GetAssetPath());
                if (AreEqualPaths(outLayer->GetIdentifier(), assetPath))
                {
                    outLayer = layer;
                    outPath = arc.GetIntroducingPrimPath();
                    break;
                }
            }
        }
    }

    return true;
}

void SimReadyUsdStageCtrl::ResolvePaths(const std::string& srcLayerIdentifier,
    const std::string& targetLayerIdentifier,
    bool storeRelativePath,
    bool relativeToSrcLayer,
    bool copySublayerLayerOffsets)
{
    auto srcLayer = pxr::SdfLayer::Find(srcLayerIdentifier);
    auto dstLayer = pxr::SdfLayer::Find(targetLayerIdentifier);
    if (!srcLayer || !dstLayer)
    {
        return;
    }

    ResolvePathsInternal(srcLayer, dstLayer, storeRelativePath, relativeToSrcLayer, copySublayerLayerOffsets);
}

bool SimReadyUsdStageCtrl::MergePrimSpec(const std::string& dstLayerIdentifier,
    const std::string& srcLayerIdentifier,
    const std::string& primSpecPath,
    bool isDstStrongerThanSrc,
    const std::string& targetPrimPath)
{
    auto dstLayer = pxr::SdfLayer::Find(dstLayerIdentifier);
    auto srcLayer = pxr::SdfLayer::Find(srcLayerIdentifier);
    if (!dstLayer || !srcLayer)
    {
        return false;
    }

    pxr::SdfPath sdfPrimPath;
    pxr::SdfPath sdfTargetPrimPath;
    if (primSpecPath.empty())
    {
        sdfPrimPath = pxr::SdfPath::EmptyPath();
    }
    else
    {
        sdfPrimPath = pxr::SdfPath(primSpecPath);
    }

    if (targetPrimPath.empty())
    {
        sdfTargetPrimPath = pxr::SdfPath::EmptyPath();
    }
    else
    {
        sdfTargetPrimPath = pxr::SdfPath(targetPrimPath);
    }

    return MergePrimSpecInternal(
        dstLayer, srcLayer, sdfPrimPath, isDstStrongerThanSrc, sdfTargetPrimPath);
}

std::vector<std::string> SimReadyUsdStageCtrl::GetAllSublayers(const pxr::UsdStageRefPtr& stage, bool includeSessionLayer)
{
    std::vector<std::string> allLayers;

    if (includeSessionLayer)
    {
        allLayers = LayerTraverse(stage->GetSessionLayer(), stage->GetSessionLayer()->GetIdentifier());
    }

    std::vector<std::string> layersInRoot = LayerTraverse(stage->GetRootLayer(), stage->GetRootLayer()->GetIdentifier());
    allLayers.insert(std::end(allLayers), std::begin(layersInRoot), std::end(layersInRoot));

    return allLayers;
}

void SimReadyUsdStageCtrl::ResolvePrimPathReferences(const pxr::SdfLayerRefPtr& layer, const pxr::SdfPath& oldPath, const pxr::SdfPath& newPath)
{
    static auto updatePrimPathRef = [](const pxr::SdfPrimSpecHandle& primSpec, const pxr::SdfPath& oldPath, const pxr::SdfPath& newPath)
    {
        auto modifyItemEditsCallback = [&oldPath, &newPath](const pxr::SdfPath& path)
        {
            return path.ReplacePrefix(oldPath, newPath);
        };

        auto modifyItemReferencesCallback = [&oldPath, &newPath](const pxr::SdfReference& reference)
        {
            pxr::SdfPath primPath;
            if (reference.GetAssetPath().empty())
            {
                primPath = reference.GetPrimPath().ReplacePrefix(oldPath, newPath);
            }
            else
            {
                primPath = reference.GetPrimPath();
            }

            return pxr::SdfReference(
                reference.GetAssetPath(),
                primPath,
                reference.GetLayerOffset(),
                reference.GetCustomData()
            );
        };

        // Update relationships
        for (const auto& relationship : primSpec->GetRelationships())
        {
            relationship->GetTargetPathList().ModifyItemEdits(modifyItemEditsCallback);
        }

        // Update connections
        for (const auto& attribute : primSpec->GetAttributes())
        {
            attribute->GetConnectionPathList().ModifyItemEdits(modifyItemEditsCallback);
        }

        primSpec->GetReferenceList().ModifyItemEdits(modifyItemReferencesCallback);
    };

    auto onPrimSpecPath = [&layer, &oldPath, &newPath](const pxr::SdfPath& primPath)
    {
        if (primPath.IsPropertyPath() || primPath == pxr::SdfPath::AbsoluteRootPath())
        {
            return;
        }

        auto primSpec = layer->GetPrimAtPath(primPath);
        if (primSpec)
        {
            updatePrimPathRef(primSpec, oldPath, newPath);
        }
    };

    layer->Traverse(pxr::SdfPath::AbsoluteRootPath(), onPrimSpecPath);
}

bool SimReadyUsdStageCtrl::StitchPrimSpecs(const pxr::UsdPrim& prim, pxr::SdfPath dstPath)
{
    auto primPath = prim.GetPath();
    auto editLayer = prim.GetStage()->GetEditTarget().GetLayer();

    bool bFromReferenceOrPayload = false;
    pxr::SdfLayerHandle introducingLayer;
    pxr::SdfPath introducingPrimPath;
    pxr::SdfPrimSpecHandle primSpecInDefLayer;

    GetIntroducingLayer(prim, introducingLayer, introducingPrimPath);
    for (auto primSpec : prim.GetPrimStack())
    {
        if (primSpec && primSpec->GetSpecifier() == pxr::SdfSpecifier::SdfSpecifierDef)
        {
            if (introducingLayer != primSpec->GetLayer() && introducingPrimPath != primPath)
            {
                bFromReferenceOrPayload = true;
                primSpecInDefLayer = primSpec;
                break;
            }
        }
    }

    auto Sublayers = GetAllSublayers(prim.GetStage(), true);

    pxr::SdfChangeBlock changeBlock;

    auto tempLayer = pxr::SdfLayer::CreateAnonymous();
    pxr::SdfCreatePrimInLayer(tempLayer, primPath);
    for (auto subLayerIdentifier : Sublayers)
    {
        pxr::SdfLayerRefPtr subLayer;
        if (bFromReferenceOrPayload && introducingLayer && introducingLayer->GetIdentifier() == subLayerIdentifier)
        {
            subLayer = pxr::SdfLayer::CreateAnonymous();
            pxr::SdfCreatePrimInLayer(subLayer, primPath);
            pxr::SdfCopySpec(primSpecInDefLayer->GetLayer(), primSpecInDefLayer->GetPath(), subLayer, primPath);
        }
        else
        {
            subLayer = pxr::SdfLayer::FindOrOpen(subLayerIdentifier);
        }

        if (subLayer)
        {
            auto srcPrimSpec = subLayer->GetPrimAtPath(primPath);
            if (srcPrimSpec)
            {
                MergePrimSpec(tempLayer->GetIdentifier(), subLayer->GetIdentifier(), primPath.GetString(), true);
            }
        }
    }

    pxr::SdfCreatePrimInLayer(editLayer, dstPath);
    ResolvePaths(editLayer->GetIdentifier(), tempLayer->GetIdentifier(), true, true);
    pxr::SdfCopySpec(tempLayer, primPath, editLayer, dstPath);

    return true;
}

bool SimReadyUsdStageCtrl::IsLayerExist(const std::string& layerIdentifier)
{
    auto layer = pxr::SdfLayer::FindOrOpen(layerIdentifier);
    return layer != nullptr;
}

void SimReadyUsdStageCtrl::ResolvePathsInternal(const pxr::SdfLayerRefPtr& srcLayer,
    pxr::SdfLayerRefPtr dstLayer,
    bool storeRelativePath,
    bool relativeToSrcLayer,
    bool copyLayerOffsets)
{
    using PathConvertFn = std::function<std::string(const std::string& path)>;
    PathConvertFn makePathAbsolute = [&srcLayer, &dstLayer](const std::string& path)
    {
        if (path.empty())
        {
            return path;
        }

        std::string externRefPathFull;
        if (!srcLayer->IsAnonymous())
        {
            externRefPathFull = ComputeAbsolutePath(srcLayer, path);
        }
        else
        {
            externRefPathFull = ComputeAbsolutePath(dstLayer, path);
        }

        if (pxr::ArGetResolver().IsSearchPath(path) && !fileExists(externRefPathFull.c_str()))
        {
            return path;
        }

        if (externRefPathFull.empty())
        {
            // If it failed to compute the absolute path, just returning the original one.
            return path;
        }
        else
        {
            return externRefPathFull;
        }
    };

    PathConvertFn makePathRelative = [&srcLayer, &dstLayer, relativeToSrcLayer](const std::string& path)
    {
        if (path.empty())
        {
            return path;
        }

        std::string relativePath = ComputeAbsolutePath(srcLayer, path);
        // FIXME: Resolver will firstly find MDL in the same dir as USD
        // for material reference, then Core Library. Currently, this is
        // used to check the existence of the path and see if it's necessary
        // to resolve path of mdl references.
        if (pxr::ArGetResolver().IsSearchPath(path) && !fileExists(relativePath.c_str()))
        {
            return path;
        }

        if (relativePath.empty() || pxr::SdfLayer::IsAnonymousLayerIdentifier(relativePath))
        {
            return path;
        }
        else
        {
            // Remove old omni: prefix
            if (relativePath.size() >= 5 && relativePath.substr(0, 5) == "omni:")
            {
                relativePath = relativePath.substr(5);
            }
            if (relativeToSrcLayer)
            {
                relativePath = TCHAR_TO_UTF8(*FSimReadyPathHelper::ComputeRelativePath(relativePath.c_str(), srcLayer->GetIdentifier().c_str()));
            }
            else
            {
                relativePath = TCHAR_TO_UTF8(*FSimReadyPathHelper::ComputeRelativePath(relativePath.c_str(), dstLayer->GetIdentifier().c_str()));
            }
            relativePath = NormalizePath(relativePath);

            // If relative path cannot be computed, it returns absolute path to avoid
            // reference issue. For example, if src and dst are not in the same domain.
            return relativePath;
        }
    };

    // Save offsets and scales.
    const auto& layerOffsets = srcLayer->GetSubLayerOffsets();

    PathConvertFn convertPath = storeRelativePath ? makePathRelative : makePathAbsolute;
    pxr::UsdUtilsModifyAssetPaths(
        dstLayer, [&convertPath](const std::string& assetPath) { return convertPath(assetPath); });

    // Copy sublayer offsets
    if (copyLayerOffsets)
    {
        for (size_t i = 0; i < layerOffsets.size(); i++)
        {
            const auto& layerOffset = layerOffsets[i];
            dstLayer->SetSubLayerOffset(layerOffset, static_cast<int>(i));
        }
    }

    // Resolve paths saved in customdata.
    pxr::VtDictionary valueMap;
    pxr::VtDictionary rootLayerCustomData = dstLayer->GetCustomLayerData();
    const auto& customDataValue = rootLayerCustomData.GetValueAtPath(FUSDCustomLayerData::MutenessCustomKey);
    if (customDataValue && !customDataValue->IsEmpty())
    {
        valueMap = customDataValue->Get<pxr::VtDictionary>();
    }

    pxr::VtDictionary newValueMap;
    for (const auto& valuePair : valueMap)
    {
        const std::string& absolutePath = srcLayer->ComputeAbsolutePath(valuePair.first);
        const std::string& relativePath = convertPath(absolutePath);
        newValueMap[relativePath] = valuePair.second;
    }

    rootLayerCustomData.SetValueAtPath(FUSDCustomLayerData::MutenessCustomKey, pxr::VtValue(newValueMap));
    dstLayer->SetCustomLayerData(rootLayerCustomData);
}

bool SimReadyUsdStageCtrl::MergePrimSpecInternal(pxr::SdfLayerRefPtr dstLayer,
    const pxr::SdfLayerRefPtr& srcLayer,
    const pxr::SdfPath& primSpecPath,
    bool isDstStrongerThanSrc,
    const pxr::SdfPath& targetPrimPath)
{
    if (dstLayer == srcLayer)
    {
        // If target path is not the same as original path, it means a duplicate.
        if (!targetPrimPath.IsEmpty() && primSpecPath != targetPrimPath)
        {
            pxr::SdfCopySpec(srcLayer, primSpecPath, dstLayer, targetPrimPath);
            return true;
        }

        return false;
    }

    if (!srcLayer->HasSpec(primSpecPath) && !dstLayer->HasSpec(primSpecPath))
    {
        return false;
    }

    auto originalStrongLayer = isDstStrongerThanSrc ? dstLayer : srcLayer;
    auto originalWeakLayer = isDstStrongerThanSrc ? srcLayer : dstLayer;
    auto targetLayer = dstLayer;

    // srcLayer is weak and dst is strong
    auto shouldCopyValueFn = [targetLayer](const pxr::TfToken& field, const pxr::SdfPath& path,
        const pxr::SdfLayerHandle& strongLayer, bool fieldInStrong,
        const pxr::SdfLayerHandle& weakLayer, bool fieldInWeak,
        pxr::VtValue* valueToCopy)
    {
        pxr::UsdUtilsStitchValueStatus status = pxr::UsdUtilsStitchValueStatus::UseDefaultValue;
        bool handleSublayers = false;
        if (field == pxr::SdfFieldKeys->SubLayers)
        {
            handleSublayers = true;
            status = pxr::UsdUtilsStitchValueStatus::UseSuppliedValue;
        }
        else if (fieldInWeak && fieldInStrong && field == pxr::SdfFieldKeys->Specifier)
        {
            const auto& sObj = weakLayer->GetObjectAtPath(path);
            const auto& dObj = strongLayer->GetObjectAtPath(path);
            const auto& sSpec = sObj->GetField(field).Get<pxr::SdfSpecifier>();
            const auto& dSpec = dObj->GetField(field).Get<pxr::SdfSpecifier>();

            // if either is not an over, we want the new specifier to be whatever that is.
            if (sSpec != pxr::SdfSpecifier::SdfSpecifierOver && dSpec != pxr::SdfSpecifier::SdfSpecifierOver)
            {

            }
            if (sSpec != pxr::SdfSpecifier::SdfSpecifierOver)
            {
                *valueToCopy = pxr::VtValue(sSpec);
                status = pxr::UsdUtilsStitchValueStatus::UseSuppliedValue;
            }
            else if (dSpec != pxr::SdfSpecifier::SdfSpecifierOver)
            {
                *valueToCopy = pxr::VtValue(dSpec);
                status = pxr::UsdUtilsStitchValueStatus::UseSuppliedValue;
            }
        }

        if (handleSublayers)
        {
            // Merge sublayers list between src and dst.
            pxr::SdfSubLayerProxy weakSublayerProxy = weakLayer->GetSubLayerPaths();
            pxr::SdfSubLayerProxy strongSublayerProxy = strongLayer->GetSubLayerPaths();
            std::vector<std::string> mergedSublayerList;
            std::unordered_set<std::string> uniqueSublayers;

            for (size_t i = 0; i < strongSublayerProxy.size(); i++)
            {
                std::string sublayer = strongSublayerProxy[i];
                if (uniqueSublayers.find(sublayer) == uniqueSublayers.end())
                {
                    mergedSublayerList.push_back(sublayer);
                    uniqueSublayers.insert(sublayer);
                }
            }

            for (size_t i = 0; i < weakSublayerProxy.size(); i++)
            {
                std::string sublayer = weakSublayerProxy[i];
                if (uniqueSublayers.find(sublayer) == uniqueSublayers.end())
                {
                    mergedSublayerList.push_back(sublayer);
                    uniqueSublayers.insert(sublayer);
                }
            }

            *valueToCopy = pxr::VtValue::Take(mergedSublayerList);
        }

        return status;
    };

    if (!srcLayer->HasSpec(primSpecPath))
    {
        return true;
    }

    auto tempStrongLayer = pxr::SdfLayer::CreateAnonymous();
    auto tempWeakLayer = pxr::SdfLayer::CreateAnonymous();
    auto tempPath = primSpecPath.IsAbsoluteRootPath() ? pxr::SdfPath::AbsoluteRootPath() : pxr::SdfPath::AbsoluteRootPath().AppendElementToken(primSpecPath.GetNameToken());
    pxr::SdfCreatePrimInLayer(tempStrongLayer, tempPath);
    pxr::SdfCreatePrimInLayer(tempWeakLayer, tempPath);
    if (originalStrongLayer->GetPrimAtPath(primSpecPath))
    {
        pxr::SdfCopySpec(originalStrongLayer, primSpecPath, tempStrongLayer, tempPath);
        ResolvePathsInternal(originalStrongLayer, tempStrongLayer, false);
    }
    if (originalWeakLayer->GetPrimAtPath(primSpecPath))
    {
        pxr::SdfCopySpec(originalWeakLayer, primSpecPath, tempWeakLayer, tempPath);
        ResolvePathsInternal(originalWeakLayer, tempWeakLayer, false);
    }
    pxr::UsdUtilsStitchLayers(tempStrongLayer, tempWeakLayer, shouldCopyValueFn);
    ResolvePathsInternal(targetLayer, tempStrongLayer, true, true);

    pxr::SdfCreatePrimInLayer(targetLayer, primSpecPath);
    pxr::SdfPath newPrimPath;
    if (targetPrimPath != pxr::SdfPath::EmptyPath())
    {
        newPrimPath = targetPrimPath;
    }
    else
    {
        newPrimPath = primSpecPath;
    }
    pxr::SdfCopySpec(tempStrongLayer, tempPath, targetLayer, newPrimPath);

    return true;
}


void SimReadyUsdStageCtrl::DebugLayerToFile(const pxr::SdfLayerHandle& Layer, const FString& FileName)
{
    std::string Export;
    Layer->ExportToString(&Export);
    IFileHandle* pFile = FPlatformFileManager::Get().GetPlatformFile().OpenWrite(*FileName);
    pFile->Write((const uint8*)Export.c_str(), Export.size());
    delete pFile;
}

