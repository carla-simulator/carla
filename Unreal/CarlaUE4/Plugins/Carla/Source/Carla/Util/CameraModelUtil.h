#pragma once
#include "CoreMinimal.h"
#include "RHIDefinitions.h"
#include "Containers/ArrayView.h"
#include "CameraModelUtil.generated.h"



class FRDGBuilder;
class FRDGTexture;
class FRHISamplerState;
class UTextureRenderTarget2D;
struct FIntPoint;



UENUM()
enum class ECameraModel : uint8
{
    Perspective,
    Stereographic,
    Equidistant,
    Equisolid,
    Orthographic,
    KannalaBrandt,
    MaxEnum,
    Default = Perspective
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
