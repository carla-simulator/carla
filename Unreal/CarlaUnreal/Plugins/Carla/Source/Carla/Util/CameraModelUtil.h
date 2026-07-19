// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include <util/ue-header-guard-begin.h>
#include "CoreMinimal.h"
#include "RHIDefinitions.h"
#include "Containers/ArrayView.h"
#include <util/ue-header-guard-end.h>

#include "CameraModelUtil.generated.h"



class FRDGBuilder;
class FRDGTexture;
class FRHISamplerState;
class UTextureRenderTarget2D;



UENUM(BlueprintType)
enum class ECameraModel : uint8
{
    Perspective,
    Stereographic,
    Equidistant,
    Equisolid,
    Orthographic,
    KannalaBrandt,
    BrownConrady,
    LUT1D,
    MaxEnum UMETA(Hidden),
    Default = Perspective UMETA(Hidden)
};



// Shared lens-model descriptor: single source of truth for lens forward/inverse
// mapping math, mirrored by the GPU core in Engine/Shaders/Private/LensModels.ush.
//
// Coordinate convention for all CameraModelUtil forward/inverse ray functions
// below (camera space, matches the GPU core, NOT Unreal's native +X-forward
// camera convention -- callers must convert):
//     +X = right, +Y = down, +Z = forward.
//
// Coeffs packing:
//     KannalaBrandt : Coeffs[0..3] = k1..k4
//     BrownConrady  : Coeffs[0..2] = k1,k2,k3 (radial), Coeffs[3..4] = p1,p2 (tangential), OpenCV convention
//     LUT1D         : Coeffs unused; LUT holds the samples instead
//
// LUT1D: LUT[i] is R(Theta) uniformly sampled for Theta in [0, ThetaMax],
// i = 0 .. LUT.Num() - 1, assumed monotonically non-decreasing.
struct FLensModelDescriptor
{
    ECameraModel Model = ECameraModel::Perspective;
    float FocalX = 1.f, FocalY = 1.f;   // normalized focal lengths
    float CenterX = 0.5f, CenterY = 0.5f; // normalized principal point
    TArray<float> Coeffs;   // KB: k1..k4; BC: k1,k2,k3,p1,p2
    TArray<float> LUT;      // LUT1D samples theta->r
    float ThetaMax = PI/2;  // max half-FOV angle (radians)
    float CAScaleR = 1.f, CAScaleB = 1.f; // per-channel radial scale for chromatic aberration
};



namespace CameraModelUtil
{
    struct FDistortCubemapToImageOptions
    {
        TArrayView<const float> KannalaBrandtCoefficients;
        float YFOVAngle;
        float YFocalLength;
        float LongitudeOffset;
        float FOVFadeSize;
        ECameraModel CameraModel;
        bool bRenderEquirectangular : 1;
        bool bFOVMaskEnable : 1;
        bool bRenderPerspective : 1;
    };



    float ComputeAngle(
        ECameraModel CameraModel,
        float Distance,
        TArrayView<const float> Coefficients);

    float ComputeDistance(
        ECameraModel CameraModel,
        float Angle,
        int32 ImageHeight,
        TArrayView<const float> Coefficients);



    namespace KannalaBrandt
    {
        float ComputeCameraPolynomial(
            float Theta,
            TArrayView<const float> Coefficients);

        float ComputeCameraPolynomialDerivative(
            float Theta,
            TArrayView<const float> Coefficients);
    }



    // Which color channel a lens-model query is for; only affects models with
    // chromatic-aberration support (FLensModelDescriptor::CAScaleR/CAScaleB).
    enum class ELensColorChannel : uint8
    {
        Green, // no CA scale applied
        Red,
        Blue
    };

    // Forward map: camera-space direction (need not be normalized; +X right,
    // +Y down, +Z forward -- see FLensModelDescriptor) -> normalized
    // image-plane point (Descriptor.CenterX/Y +/- Descriptor.FocalX/Y * extent).
    FVector2f ComputeLensImagePoint(
        const FLensModelDescriptor& Descriptor,
        FVector Direction,
        ELensColorChannel Channel = ELensColorChannel::Green);

    // Inverse map: normalized image-plane point -> camera-space unit ray
    // direction (+X right, +Y down, +Z forward). Uses Newton-Raphson for
    // KannalaBrandt and BrownConrady, closed form for the other radial models,
    // and binary-search interpolation for LUT1D.
    FVector ComputeLensRayDirection(
        const FLensModelDescriptor& Descriptor,
        FVector2f ImagePoint,
        ELensColorChannel Channel = ELensColorChannel::Green);

    // Convention adapters between the lens core's camera space (+X right,
    // +Y down, +Z forward -- matches OpenCV, see FLensModelDescriptor) and
    // Unreal's native camera space (+X forward, +Y right, +Z up). Pure axis
    // relabeling: an isometry, and each is the other's exact inverse. Not a
    // "rotation" in the proper sense -- the lens core's convention is
    // right-handed and UE's is left-handed, so this necessarily flips
    // handedness rather than preserving it.
    FVector LensDirToUE(FVector LensDir);
    FVector UEDirToLens(FVector UEDir);

    namespace BrownConrady
    {
        // Normalized pinhole coords (Xn = X/Z, Yn = Y/Z) -> distorted coords.
        // Coefficients: k1,k2,k3 (radial), p1,p2 (tangential), OpenCV convention.
        FVector2f Distort(
            float Xn,
            float Yn,
            TArrayView<const float> Coefficients);

        // Inverse of Distort() via 2D Newton-Raphson with an analytic Jacobian.
        FVector2f Undistort(
            float Xd,
            float Yd,
            TArrayView<const float> Coefficients,
            int32 Iterations = 10);
    }

    namespace LUT1D
    {
        // Theta -> R, linear interpolation over uniformly spaced samples.
        float SampleForward(
            float Theta,
            TArrayView<const float> Samples,
            float ThetaMax);

        // R -> Theta, binary search + linear interpolation. Assumes Samples is
        // monotonically non-decreasing.
        float SampleInverse(
            float Distance,
            TArrayView<const float> Samples,
            float ThetaMax);
    }



	void DistortCubemapToImage(
        FRDGBuilder& GraphBuilder,
        FRDGTexture* Destination,
        FRDGTexture** CubeTextures, // CubeTextures[6]
        FRHISamplerState* Sampler,
        const FDistortCubemapToImageOptions& Options);

    void DistortCubemapToImage(
        FRDGBuilder& GraphBuilder,
        UTextureRenderTarget2D* Destination,
        UTextureRenderTarget2D** CubeRenderTargets, // CubeRenderTargets[6]
        FRHISamplerState* Sampler,
        const FDistortCubemapToImageOptions& Options);

    FRHISamplerState* GetSampler(ESamplerFilter Filter);

} // CameraModelUtil
