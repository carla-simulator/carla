// Copyright (c) 2026 Computer Vision Center (CVC) at the Universitat Autonoma
// de Barcelona (UAB).
//
// This work is licensed under the terms of the MIT license.
// For a copy, see <https://opensource.org/licenses/MIT>.

#include "CameraModelUtil.h"

#include <util/ue-header-guard-begin.h>
#include "Engine/TextureRenderTargetCube.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GlobalShader.h"
#include "ShaderParameterUtils.h"
#include "PipelineStateCache.h"
#include "RHIStaticStates.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include <util/ue-header-guard-end.h>

#include <cmath>



enum WideAngleLensShaderFlags : uint32
{
    WAL_SHADER_FLAGS_EQUIRECTANGULAR = 1U << 0,
    WAL_SHADER_FLAGS_FOV_MASK = 1U << 1
};


// @TODO: Actually find out what is the current GPU's subgroup size (warp size).
static const auto SubgroupSize = 32U;
static const auto WorkPerInvocation = static_cast<uint32>(64U / sizeof(FVector4f));
static const auto KannalaBrandtSolverIterations = 32U;

// static constexpr auto DegToRad = PI / 180.0F;
// static constexpr auto RadToDeg = 180.0F / PI;

static constexpr std::pair<const TCHAR*, uint32> CommonShaderDefinitions[] =
{
    std::make_pair(TEXT("THREAD_GROUP_SIZE_X"), SubgroupSize),
    std::make_pair(TEXT("THREAD_GROUP_SIZE_Y"), 1U),
    std::make_pair(TEXT("THREAD_GROUP_SIZE_Z"), 1U),
    std::make_pair(TEXT("WORK_PER_INVOCATION"), WorkPerInvocation),
    std::make_pair(TEXT("KANNALA_BRANDT_SOLVER_ITERATIONS"), KannalaBrandtSolverIterations),
};



template <ECameraModel>
struct FWideAngleLensShaderBase
{
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutImage)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeFront)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeBack)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeRight)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeLeft)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeTop)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeBottom)
        SHADER_PARAMETER_SAMPLER(SamplerState, FaceSampler)
        SHADER_PARAMETER(float, YFOVAngle)
        SHADER_PARAMETER(FVector4f, CameraParams)
        SHADER_PARAMETER(uint32, Flags)
        SHADER_PARAMETER(float, LongitudeOffset)
        SHADER_PARAMETER(float, FOVFadeSize)
    END_SHADER_PARAMETER_STRUCT()
};

template <>
struct FWideAngleLensShaderBase<ECameraModel::KannalaBrandt>
{
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutImage)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeFront)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeBack)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeRight)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeLeft)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeTop)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, CubeBottom)
        SHADER_PARAMETER_SAMPLER(SamplerState, FaceSampler)
        SHADER_PARAMETER(float, YFOVAngle)
        SHADER_PARAMETER(FVector4f, CameraParams)
        SHADER_PARAMETER(uint32, Flags)
        SHADER_PARAMETER(float, LongitudeOffset)
        SHADER_PARAMETER(float, FOVFadeSize)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float>, Coefficients)
    END_SHADER_PARAMETER_STRUCT()
};



template <ECameraModel>
struct FToPerspectiveShaderBase
{
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Destination)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, Source)
        SHADER_PARAMETER_SAMPLER(SamplerState, SourceSampler)
        SHADER_PARAMETER(FVector4f, DestinationCameraParams)
        SHADER_PARAMETER(FVector4f, SourceCameraParams)
    END_SHADER_PARAMETER_STRUCT()
};

template <>
struct FToPerspectiveShaderBase<ECameraModel::KannalaBrandt>
{
    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, Destination)
        SHADER_PARAMETER_RDG_TEXTURE_SRV(Texture2D, Source)
        SHADER_PARAMETER_SAMPLER(SamplerState, SourceSampler)
        SHADER_PARAMETER(FVector4f, DestinationCameraParams)
        SHADER_PARAMETER(FVector4f, SourceCameraParams)
        SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<float>, Coefficients)
    END_SHADER_PARAMETER_STRUCT()
};



// Depending on which version of clang we're using, templates may not work with IMPLEMENT_SHADER.
// Macro fallback:

#define DECLARE_WIDE_ANGLE_LENS_SHADER(NAME, CAMERA_MODEL)                                                      \
    class NAME :                                                                                                \
        public FWideAngleLensShaderBase<CAMERA_MODEL>,                                                          \
        public FGlobalShader                                                                                    \
    {                                                                                                           \
    public:                                                                                                     \
                                                                                                                \
        DECLARE_GLOBAL_SHADER(NAME);                                                                            \
        SHADER_USE_PARAMETER_STRUCT(NAME, FGlobalShader);                                                       \
                                                                                                                \
        static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)              \
        {                                                                                                       \
            return true;                                                                                        \
        }                                                                                                       \
                                                                                                                \
        static void ModifyCompilationEnvironment(                                                               \
            const FGlobalShaderPermutationParameters& Parameters,                                               \
            FShaderCompilerEnvironment& OutEnvironment)                                                         \
        {                                                                                                       \
            for (auto Definition : CommonShaderDefinitions)                                                     \
                OutEnvironment.SetDefine(Definition.first, Definition.second);                                  \
            OutEnvironment.SetDefine(TEXT("CAMERA_TYPE"), static_cast<uint32>(CAMERA_MODEL));                   \
            FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);                            \
        }                                                                                                       \
    }



#define DECLARE_PERSPECTIVE_SHADER(NAME, CAMERA_MODEL)                                                          \
    class NAME :                                                                                                \
        public FToPerspectiveShaderBase<CAMERA_MODEL>,                                                          \
        public FGlobalShader                                                                                    \
    {                                                                                                           \
    public:                                                                                                     \
                                                                                                                \
        DECLARE_GLOBAL_SHADER(NAME);                                                                            \
        SHADER_USE_PARAMETER_STRUCT(NAME, FGlobalShader);                                                       \
                                                                                                                \
        static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)              \
        {                                                                                                       \
            return true;                                                                                        \
        }                                                                                                       \
                                                                                                                \
        static void ModifyCompilationEnvironment(                                                               \
            const FGlobalShaderPermutationParameters& Parameters,                                               \
            FShaderCompilerEnvironment& OutEnvironment)                                                         \
        {                                                                                                       \
            for (auto Definition : CommonShaderDefinitions)                                                     \
                OutEnvironment.SetDefine(Definition.first, Definition.second);                                  \
            OutEnvironment.SetDefine(TEXT("CAMERA_TYPE"), static_cast<uint32>(CAMERA_MODEL));                   \
            FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);                            \
        }                                                                                                       \
    }

DECLARE_WIDE_ANGLE_LENS_SHADER(FWideAngleLensShader_Perspective, ECameraModel::Perspective);
DECLARE_WIDE_ANGLE_LENS_SHADER(FWideAngleLensShader_Stereographic, ECameraModel::Stereographic);
DECLARE_WIDE_ANGLE_LENS_SHADER(FWideAngleLensShader_Equidistance, ECameraModel::Equidistant);
DECLARE_WIDE_ANGLE_LENS_SHADER(FWideAngleLensShader_Equisolid, ECameraModel::Equisolid);
DECLARE_WIDE_ANGLE_LENS_SHADER(FWideAngleLensShader_Orthogonal, ECameraModel::Orthographic);
DECLARE_WIDE_ANGLE_LENS_SHADER(FWideAngleLensShader_Custom, ECameraModel::KannalaBrandt);

DECLARE_PERSPECTIVE_SHADER(FToPerspectiveShader_Perspective, ECameraModel::Perspective);
DECLARE_PERSPECTIVE_SHADER(FToPerspectiveShader_Stereographic, ECameraModel::Stereographic);
DECLARE_PERSPECTIVE_SHADER(FToPerspectiveShader_Equidistance, ECameraModel::Equidistant);
DECLARE_PERSPECTIVE_SHADER(FToPerspectiveShader_Equisolid, ECameraModel::Equisolid);
DECLARE_PERSPECTIVE_SHADER(FToPerspectiveShader_Orthogonal, ECameraModel::Orthographic);
DECLARE_PERSPECTIVE_SHADER(FToPerspectiveShader_Custom, ECameraModel::KannalaBrandt);



template <typename FShaderType>
static auto CreateDistortionParameters(
    FRDGBuilder& GraphBuilder,
    FRDGTextureRef Destination,
    FRDGTextureRef CubeTextures[6],
    FRHISamplerState* Sampler,
    float YFOVAngle,
    float FocalDistance,
    FIntPoint Size,
    float LongitudeOffset,
    float FOVFadeSize,
    bool RenderEquirectangular,
    bool FovMaskEnable)
{
    const auto Center = Size / 2;
    auto Parameters = GraphBuilder.AllocParameters<typename FShaderType::FParameters>();
    Parameters->OutImage = GraphBuilder.CreateUAV(Destination);
    Parameters->CubeFront = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(CubeTextures[0]));
    Parameters->CubeBack = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(CubeTextures[1]));
    Parameters->CubeRight = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(CubeTextures[2]));
    Parameters->CubeLeft = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(CubeTextures[3]));
    Parameters->CubeTop = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(CubeTextures[4]));
    Parameters->CubeBottom = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(CubeTextures[5]));
    Parameters->FaceSampler = Sampler;
    Parameters->CameraParams = FVector4f(FocalDistance, FocalDistance, Center.X, Center.Y);
    Parameters->YFOVAngle = YFOVAngle;
    Parameters->LongitudeOffset = LongitudeOffset;
    Parameters->FOVFadeSize = FOVFadeSize;
    Parameters->Flags = 0;
    if (RenderEquirectangular)
        Parameters->Flags |= WAL_SHADER_FLAGS_EQUIRECTANGULAR;
    if (FovMaskEnable)
        Parameters->Flags |= WAL_SHADER_FLAGS_FOV_MASK;
    return Parameters;
}

template <typename FShaderType>
static void ApplyDistortion(
    FRDGBuilder& GraphBuilder,
    FRDGTextureRef Destination,
    FRDGTextureRef CubeTextures[6],
    FRHISamplerState* Sampler,
    float YFOVAngle,
    float FocalDistance,
    FIntPoint Size,
    float LongitudeOffset,
    float FOVFadeSize,
    bool RenderEquirectangular,
    bool FovMaskEnable)
{
    auto Parameters = CreateDistortionParameters<FShaderType>(
        GraphBuilder,
        Destination,
        CubeTextures,
        Sampler,
        YFOVAngle,
        FocalDistance,
        Size,
        LongitudeOffset,
        FOVFadeSize,
        RenderEquirectangular,
        FovMaskEnable);

    GraphBuilder.AddPass(
        RDG_EVENT_NAME("WideAngleLens-Dispatch"),
        Parameters,
        ERDGPassFlags::Compute,
        [Parameters, Size](FRHICommandListImmediate& RHICmdList)
        {
            TShaderMapRef<FShaderType> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            check(ComputeShader.IsValid());

            FComputeShaderUtils::Dispatch(
                RHICmdList,
                ComputeShader,
                *Parameters,
                FComputeShaderUtils::GetGroupCount(
                    FIntVector(Size.X, Size.Y, 1),
                    FIntVector(SubgroupSize, 1, 1)));
        });
}

static void ApplyDistortion(
    FRDGBuilder& GraphBuilder,
    FRDGTextureRef Destination,
    FRDGTextureRef CubeTextures[6],
    FRHISamplerState* Sampler,
    float YFOVAngle,
    float FocalDistance,
    FIntPoint Size,
    float LongitudeOffset,
    float FOVFadeSize,
    bool RenderEquirectangular,
    bool FovMaskEnable,
    TArrayView<const float> KannalaBrandtCoefficients)
{
    using FShaderType = FWideAngleLensShader_Custom;

    auto Parameters = CreateDistortionParameters<FShaderType>(
        GraphBuilder,
        Destination,
        CubeTextures,
        Sampler,
        YFOVAngle,
        FocalDistance,
        Size,
        LongitudeOffset,
        FOVFadeSize,
        RenderEquirectangular,
        FovMaskEnable);

    auto CoefficientBuffer = CreateStructuredBuffer(
        GraphBuilder,
        TEXT("CoefficientBuffer"),
        sizeof(float),
        KannalaBrandtCoefficients.Num(),
        KannalaBrandtCoefficients.GetData(),
        KannalaBrandtCoefficients.Num() * sizeof(float),
        ERDGInitialDataFlags::None);

    Parameters->Coefficients = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(CoefficientBuffer, PF_R32_FLOAT));

    GraphBuilder.AddPass(
        RDG_EVENT_NAME("WideAngleLens-Dispatch"),
        Parameters,
        ERDGPassFlags::Compute,
        [Parameters, Size](FRHICommandListImmediate& RHICmdList)
        {
            TShaderMapRef<FShaderType> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            check(ComputeShader.IsValid());

            FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *Parameters,
                FComputeShaderUtils::GetGroupCount(
                    FIntVector(Size.X, Size.Y, 1),
                    FIntVector(SubgroupSize, 1, 1)));
        });
}

template <typename FShaderType>
static void ToPerspective(
    FRDGBuilder& GraphBuilder,
    FRDGTextureRef Destination,
    FRDGTextureRef Source,
    FRHISamplerState* Sampler,
    float YFOVAngle,
    float DestinationFocalDistance,
    float SourceFocalDistance,
    FIntPoint Size)
{
    const auto Center = Size / 2;

    auto Parameters = GraphBuilder.AllocParameters<typename FShaderType::FParameters>();
    Parameters->Destination = GraphBuilder.CreateUAV(Destination);
    Parameters->Source = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(Source));
    Parameters->SourceSampler = Sampler;
    Parameters->SourceCameraParams = FVector4f(SourceFocalDistance, SourceFocalDistance, Center.X, Center.Y);
    Parameters->DestinationCameraParams = FVector4f(DestinationFocalDistance, DestinationFocalDistance, Center.X, Center.Y);

    GraphBuilder.AddPass(
        RDG_EVENT_NAME("ToPerspective-Dispatch"),
        Parameters,
        ERDGPassFlags::Compute,
        [Parameters, Size](FRHICommandListImmediate& RHICmdList)
        {
            TShaderMapRef<FShaderType> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            check(ComputeShader.IsValid());

            FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *Parameters,
                FComputeShaderUtils::GetGroupCount(
                    FIntVector(Size.X, Size.Y, 1),
                    FIntVector(SubgroupSize * WorkPerInvocation, 1, 1)));
        });
}

static void ToPerspective(
    FRDGBuilder& GraphBuilder,
    FRDGTextureRef Destination,
    FRDGTextureRef Source,
    FRHISamplerState* Sampler,
    float YFOVAngle,
    float DestinationFocalDistance,
    float SourceFocalDistance,
    FIntPoint Size,
    TArrayView<const float> KannalaBrandtCoefficients)
{
    using FShaderType = FToPerspectiveShader_Custom;

    const auto Center = Size / 2;

    auto CoefficientBuffer = CreateStructuredBuffer(
        GraphBuilder,
        TEXT("CoefficientBuffer"),
        sizeof(float),
        KannalaBrandtCoefficients.Num(),
        KannalaBrandtCoefficients.GetData(),
        KannalaBrandtCoefficients.Num() * sizeof(float),
        ERDGInitialDataFlags::None);

    auto Parameters = GraphBuilder.AllocParameters<typename FShaderType::FParameters>();
    Parameters->Destination = GraphBuilder.CreateUAV(Destination);
    Parameters->Source = GraphBuilder.CreateSRV(FRDGTextureSRVDesc::Create(Source));
    Parameters->SourceSampler = Sampler;
    Parameters->SourceCameraParams = FVector4f(SourceFocalDistance, SourceFocalDistance, Center.X, Center.Y);
    Parameters->DestinationCameraParams = FVector4f(DestinationFocalDistance, DestinationFocalDistance, Center.X, Center.Y);
    Parameters->Coefficients = GraphBuilder.CreateSRV(FRDGBufferSRVDesc(CoefficientBuffer, PF_R32_FLOAT));

    GraphBuilder.AddPass(
        RDG_EVENT_NAME("ToPerspective-Dispatch"),
        Parameters,
        ERDGPassFlags::Compute,
        [Parameters, Size](FRHICommandListImmediate& RHICmdList)
        {
            TShaderMapRef<FShaderType> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));
            check(ComputeShader.IsValid());

            FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *Parameters,
                FComputeShaderUtils::GetGroupCount(
                    FIntVector(Size.X, Size.Y, 1),
                    FIntVector(SubgroupSize * WorkPerInvocation, 1, 1)));
        });
}



namespace CameraModelUtil
{
    namespace KannalaBrandt
    {
        float ComputeCameraPolynomial(
            float Theta,
            TArrayView<const float> Coefficients)
        {
            auto Result = 1.0F;
            auto Theta2 = Theta * Theta;
            auto ThetaN = 1.0F;

            for (auto K : Coefficients)
            {
                ThetaN *= Theta2;
                Result += K * ThetaN;
            }

            return Result * Theta;
        }

        float ComputeCameraPolynomialDerivative(
            float Theta,
            TArrayView<const float> Coefficients)
        {
            auto Result = 1.0F;
            auto Theta2 = Theta * Theta;
            auto ThetaN = Theta2;
            auto A = 3.0F;

            for (auto K : Coefficients)
            {
                Result += A * K * ThetaN;
                A += 2.0F;
                ThetaN *= Theta2;
            }

            return Result;
        }

    }

    float ComputeAngle(
        ECameraModel CameraModel,
        float Distance,
        TArrayView<const float> Coefficients)
    {
        switch (CameraModel)
        {
        case ECameraModel::Perspective:
            return std::atan(Distance);
        case ECameraModel::Stereographic:
            return std::atan(Distance * 0.5F) * 2;
        case ECameraModel::Equidistant:
            return Distance;
        case ECameraModel::Equisolid:
            return std::asin(FMath::Clamp(Distance * 0.5F, -1.0F, 1.0F)) * 2;
        case ECameraModel::Orthographic:
            return std::asin(FMath::Clamp(Distance, -1.0F, 1.0F));
        case ECameraModel::KannalaBrandt:
        {
            float Theta = Distance;
            for (uint8 i = 0; i != KannalaBrandtSolverIterations; ++i)
            {
                float N = Distance - KannalaBrandt::ComputeCameraPolynomial(Theta, Coefficients);
                float D = -KannalaBrandt::ComputeCameraPolynomialDerivative(Theta, Coefficients);
                Theta -= N / D;
            }
            return Theta;
        }
        default:
            check(false);
            return 0.0F;
        }
    }

    float ComputeDistance(
        ECameraModel CameraModel,
        float Angle,
        int32 ImageHeight,
        TArrayView<const float> Coefficients)
    {
        const auto H = static_cast<float>(ImageHeight);
        const auto R = H * 0.5F;
        float F = 0.0F;

        Angle *= 0.5F;
        switch (CameraModel)
        {
        case ECameraModel::Perspective:
            F = R / std::tan(Angle);
            break;
        case ECameraModel::Stereographic:
            F = R / (std::tan(Angle * 0.5F) * 2);
            break;
        case ECameraModel::Equidistant:
            F = R / Angle;
            break;
        case ECameraModel::Equisolid:
            F = R / (std::sin(Angle * 0.5F) * 2);
            break;
        case ECameraModel::Orthographic:
            F = R / std::sin(Angle);
            break;
        case ECameraModel::KannalaBrandt:
            F = R / KannalaBrandt::ComputeCameraPolynomial(Angle, Coefficients);
            break;
        default:
            check(false);
            break;
        }
        return F;
    }

    void DistortCubemapToImage(
        FRDGBuilder& GraphBuilder,
        FRDGTexture* Destination,
        FRDGTexture** CubeTextures,
        FRHISamplerState* Sampler,
        const FDistortCubemapToImageOptions& Options)
    {
        auto& DestinationDesc = Destination->Desc;
        auto SizeVector = DestinationDesc.GetSize();
        auto Size = FIntPoint(SizeVector.X, SizeVector.Y);

        auto DistortedTexture =
            GraphBuilder.CreateTexture(
                FRDGTextureDesc::Create2D(
                    Size,
                    DestinationDesc.Format,
                    DestinationDesc.ClearValue,
                    TexCreate_ShaderResource | TexCreate_UAV),
                TEXT("DistortedTexture"));

        switch (Options.CameraModel)
        {
        case ECameraModel::Perspective:
            ApplyDistortion<FWideAngleLensShader_Perspective>(
                GraphBuilder,
                DistortedTexture,
                CubeTextures,
                Sampler,
                Options.YFOVAngle,
                Options.YFocalLength,
                Size,
                Options.LongitudeOffset,
                Options.FOVFadeSize,
                Options.bRenderEquirectangular,
                Options.bFOVMaskEnable);
            break;
        case ECameraModel::Stereographic:
            ApplyDistortion<FWideAngleLensShader_Stereographic>(
                GraphBuilder,
                DistortedTexture,
                CubeTextures,
                Sampler,
                Options.YFOVAngle,
                Options.YFocalLength,
                Size,
                Options.LongitudeOffset,
                Options.FOVFadeSize,
                Options.bRenderEquirectangular,
                Options.bFOVMaskEnable);
            break;
        case ECameraModel::Equidistant:
            ApplyDistortion<FWideAngleLensShader_Equidistance>(
                GraphBuilder,
                DistortedTexture,
                CubeTextures,
                Sampler,
                Options.YFOVAngle,
                Options.YFocalLength,
                Size,
                Options.LongitudeOffset,
                Options.FOVFadeSize,
                Options.bRenderEquirectangular,
                Options.bFOVMaskEnable);
            break;
        case ECameraModel::Equisolid:
            ApplyDistortion<FWideAngleLensShader_Equisolid>(
                GraphBuilder,
                DistortedTexture,
                CubeTextures,
                Sampler,
                Options.YFOVAngle,
                Options.YFocalLength,
                Size,
                Options.LongitudeOffset,
                Options.FOVFadeSize,
                Options.bRenderEquirectangular,
                Options.bFOVMaskEnable);
            break;
        case ECameraModel::Orthographic:
            ApplyDistortion<FWideAngleLensShader_Orthogonal>(
                GraphBuilder,
                DistortedTexture,
                CubeTextures,
                Sampler,
                Options.YFOVAngle,
                Options.YFocalLength,
                Size,
                Options.LongitudeOffset,
                Options.FOVFadeSize,
                Options.bRenderEquirectangular,
                Options.bFOVMaskEnable);
            break;
        case ECameraModel::KannalaBrandt:
            ApplyDistortion(
                GraphBuilder,
                DistortedTexture,
                CubeTextures,
                Sampler,
                Options.YFOVAngle,
                Options.YFocalLength,
                Size,
                Options.LongitudeOffset,
                Options.FOVFadeSize,
                Options.bRenderEquirectangular,
                Options.bFOVMaskEnable,
                Options.KannalaBrandtCoefficients);
            break;
        default:
            check(false);
        }

        if (Options.bRenderPerspective && !Options.bRenderEquirectangular)
        {
            auto PerspectiveTexture =
                GraphBuilder.CreateTexture(
                    FRDGTextureDesc::Create2D(
                        Size,
                        DestinationDesc.Format,
                        FClearValueBinding::Black,
                        TexCreate_ShaderResource | TexCreate_UAV),
                    TEXT("PerspectiveTexture"));

            auto FocalDistancePerspective = ComputeDistance(
                ECameraModel::Perspective,
                Options.YFOVAngle,
                Size.Y,
                Options.KannalaBrandtCoefficients);

            switch (Options.CameraModel)
            {
            case ECameraModel::Perspective:
                ToPerspective<FToPerspectiveShader_Perspective>(
                    GraphBuilder,
                    PerspectiveTexture,
                    DistortedTexture,
                    Sampler,
                    Options.YFOVAngle,
                    Options.YFocalLength,
                    FocalDistancePerspective,
                    Size);
                break;
            case ECameraModel::Stereographic:
                ToPerspective<FToPerspectiveShader_Stereographic>(
                    GraphBuilder,
                    PerspectiveTexture,
                    DistortedTexture,
                    Sampler,
                    Options.YFOVAngle,
                    Options.YFocalLength,
                    FocalDistancePerspective,
                    Size);
                break;
            case ECameraModel::Equidistant:
                ToPerspective<FToPerspectiveShader_Equidistance>(
                    GraphBuilder,
                    PerspectiveTexture,
                    DistortedTexture,
                    Sampler,
                    Options.YFOVAngle,
                    Options.YFocalLength,
                    FocalDistancePerspective,
                    Size);
                break;
            case ECameraModel::Equisolid:
                ToPerspective<FToPerspectiveShader_Equisolid>(
                    GraphBuilder,
                    PerspectiveTexture,
                    DistortedTexture,
                    Sampler,
                    Options.YFOVAngle,
                    Options.YFocalLength,
                    FocalDistancePerspective,
                    Size);
                break;
            case ECameraModel::Orthographic:
                ToPerspective<FToPerspectiveShader_Orthogonal>(
                    GraphBuilder,
                    PerspectiveTexture,
                    DistortedTexture,
                    Sampler,
                    Options.YFOVAngle,
                    Options.YFocalLength,
                    FocalDistancePerspective,
                    Size);
                break;
            case ECameraModel::KannalaBrandt:
                ToPerspective(
                    GraphBuilder,
                    PerspectiveTexture,
                    DistortedTexture,
                    Sampler,
                    Options.YFOVAngle,
                    Options.YFocalLength,
                    FocalDistancePerspective,
                    Size,
                    Options.KannalaBrandtCoefficients);
                break;
            default:
                check(false);
            }

            AddCopyTexturePass(
                GraphBuilder,
                PerspectiveTexture,
                Destination);
        }
        else
        {
            AddCopyTexturePass(
                GraphBuilder,
                DistortedTexture,
                Destination);
        }
    }

    void DistortCubemapToImage(
        FRDGBuilder& GraphBuilder,
        UTextureRenderTarget2D* Destination,
        UTextureRenderTarget2D** CubeRenderTargets,
        FRHISamplerState* Sampler,
        const FDistortCubemapToImageOptions& Options)
    {
        FTextureRHIRef TextureRHIs[6] = {};

        for (uint8 i = 0; i != 6; ++i)
        {
            TextureRHIs[i] = CubeRenderTargets[i]
                ->GetRenderTargetResource()
                ->GetTextureRenderTarget2DResource()
                ->GetTextureRHI();
        }

        auto DestinationRHI = Destination
            ->GetRenderTargetResource()
            ->GetTextureRenderTarget2DResource()
            ->GetTextureRHI();

        TRefCountPtr<IPooledRenderTarget> PRTs[] =
        {
            CreateRenderTarget(TextureRHIs[0], TEXT("CubeTextures[0]")),
            CreateRenderTarget(TextureRHIs[1], TEXT("CubeTextures[1]")),
            CreateRenderTarget(TextureRHIs[2], TEXT("CubeTextures[2]")),
            CreateRenderTarget(TextureRHIs[3], TEXT("CubeTextures[3]")),
            CreateRenderTarget(TextureRHIs[4], TEXT("CubeTextures[4]")),
            CreateRenderTarget(TextureRHIs[5], TEXT("CubeTextures[5]"))
        };

        auto CaptureRenderTargetTexture = GraphBuilder.RegisterExternalTexture(
            CreateRenderTarget(DestinationRHI, TEXT("CaptureRenderTargetTexture")));

        FRDGTextureRef CubeTextures[6] = { };

        for (uint8 i = 0; i != 6; ++i)
        {
            CubeTextures[i] = GraphBuilder.RegisterExternalTexture(PRTs[i]);
        }

        DistortCubemapToImage(
            GraphBuilder,
            CaptureRenderTargetTexture,
            CubeTextures,
            Sampler,
            Options);
    }

    FRHISamplerState* GetSampler(ESamplerFilter Filter)
    {
        switch (Filter)
        {

            case SF_Point:
                return TStaticSamplerState<	SF_Point>::GetRHI();
            case SF_Bilinear:
                return TStaticSamplerState<	SF_Bilinear>::GetRHI();
            case SF_Trilinear:
                return TStaticSamplerState<	SF_Trilinear>::GetRHI();
            case SF_AnisotropicPoint:
                return TStaticSamplerState<	SF_AnisotropicPoint>::GetRHI();
            case SF_AnisotropicLinear:
                return TStaticSamplerState<	SF_AnisotropicLinear>::GetRHI();
            default:
                check(false);
                return nullptr;
        }
    }



    // -----------------------------------------------------------------------
    // FLensModelDescriptor forward/inverse core.
    // -----------------------------------------------------------------------

    namespace BrownConrady
    {
        // Evaluates Distort() and its Jacobian in one pass, so Undistort()'s
        // Newton-Raphson loop only pays for the distortion math once per iteration.
        static void EvalWithJacobian(
            float Xn,
            float Yn,
            TArrayView<const float> C,
            float& OutXd,
            float& OutYd,
            float OutJ[4]) // row-major [dXd/dXn, dXd/dYn, dYd/dXn, dYd/dYn]
        {
            const float K1 = C.Num() > 0 ? C[0] : 0.0F;
            const float K2 = C.Num() > 1 ? C[1] : 0.0F;
            const float K3 = C.Num() > 2 ? C[2] : 0.0F;
            const float P1 = C.Num() > 3 ? C[3] : 0.0F;
            const float P2 = C.Num() > 4 ? C[4] : 0.0F;

            const float R2 = Xn * Xn + Yn * Yn;
            const float R4 = R2 * R2;
            const float Radial = 1.0F + K1 * R2 + K2 * R4 + K3 * R4 * R2;
            const float DRadialDR2 = K1 + 2.0F * K2 * R2 + 3.0F * K3 * R4;

            OutXd = Xn * Radial + 2.0F * P1 * Xn * Yn + P2 * (R2 + 2.0F * Xn * Xn);
            OutYd = Yn * Radial + P1 * (R2 + 2.0F * Yn * Yn) + 2.0F * P2 * Xn * Yn;

            const float DRadialDXn = 2.0F * Xn * DRadialDR2;
            const float DRadialDYn = 2.0F * Yn * DRadialDR2;

            OutJ[0] = Radial + Xn * DRadialDXn + 2.0F * P1 * Yn + 6.0F * P2 * Xn; // dXd/dXn
            OutJ[1] = Xn * DRadialDYn + 2.0F * P1 * Xn + 2.0F * P2 * Yn;          // dXd/dYn
            OutJ[2] = Yn * DRadialDXn + 2.0F * P1 * Xn + 2.0F * P2 * Yn;          // dYd/dXn
            OutJ[3] = Radial + Yn * DRadialDYn + 6.0F * P1 * Yn + 2.0F * P2 * Xn; // dYd/dYn
        }

        FVector2f Distort(
            float Xn,
            float Yn,
            TArrayView<const float> Coefficients)
        {
            float Xd, Yd, J[4];
            EvalWithJacobian(Xn, Yn, Coefficients, Xd, Yd, J);
            return FVector2f(Xd, Yd);
        }

        FVector2f Undistort(
            float Xd,
            float Yd,
            TArrayView<const float> Coefficients,
            int32 Iterations)
        {
            float Xn = Xd, Yn = Yd; // initial guess: undistorted ~ distorted for small distortion
            for (int32 i = 0; i != Iterations; ++i)
            {
                float Fx, Fy, J[4];
                EvalWithJacobian(Xn, Yn, Coefficients, Fx, Fy, J);
                Fx -= Xd;
                Fy -= Yd;

                const float Det = J[0] * J[3] - J[1] * J[2];
                if (FMath::Abs(Det) < 1e-12F)
                    break;

                const float InvDet = 1.0F / Det;
                const float DXn = InvDet * (J[3] * Fx - J[1] * Fy);
                const float DYn = InvDet * (-J[2] * Fx + J[0] * Fy);

                Xn -= DXn;
                Yn -= DYn;
            }
            return FVector2f(Xn, Yn);
        }
    } // BrownConrady

    namespace LUT1D
    {
        float SampleForward(
            float Theta,
            TArrayView<const float> Samples,
            float ThetaMax)
        {
            const int32 Count = Samples.Num();
            if (Count == 0)
                return 0.0F;
            if (Count == 1)
                return Samples[0];

            const float Clamped = FMath::Clamp(Theta, 0.0F, ThetaMax);
            const float T = ThetaMax > 0.0F ? (Clamped / ThetaMax) * static_cast<float>(Count - 1) : 0.0F;
            const int32 I0 = FMath::Clamp(static_cast<int32>(FMath::FloorToFloat(T)), 0, Count - 1);
            const int32 I1 = FMath::Min(I0 + 1, Count - 1);
            const float Frac = T - static_cast<float>(I0);
            return FMath::Lerp(Samples[I0], Samples[I1], Frac);
        }

        float SampleInverse(
            float Distance,
            TArrayView<const float> Samples,
            float ThetaMax)
        {
            const int32 Count = Samples.Num();
            if (Count < 2)
                return 0.0F;
            if (Distance <= Samples[0])
                return 0.0F;
            if (Distance >= Samples[Count - 1])
                return ThetaMax;

            // Samples is assumed monotonically non-decreasing.
            int32 Lo = 0;
            int32 Hi = Count - 1;
            while (Hi - Lo > 1)
            {
                const int32 Mid = (Lo + Hi) / 2;
                if (Samples[Mid] <= Distance)
                    Lo = Mid;
                else
                    Hi = Mid;
            }

            const float R0 = Samples[Lo];
            const float R1 = Samples[Hi];
            const float Frac = R1 > R0 ? (Distance - R0) / (R1 - R0) : 0.0F;
            const float IndexF = static_cast<float>(Lo) + Frac;
            return (IndexF / static_cast<float>(Count - 1)) * ThetaMax;
        }
    } // LUT1D

    // R(Theta) for every model except BrownConrady, which is not radially symmetric.
    static float RadialForward(
        ECameraModel Model,
        float Theta,
        TArrayView<const float> Coeffs,
        TArrayView<const float> LUT,
        float ThetaMax)
    {
        switch (Model)
        {
        case ECameraModel::Perspective:
            return std::tan(Theta);
        case ECameraModel::Stereographic:
            return std::tan(Theta * 0.5F) * 2.0F;
        case ECameraModel::Equidistant:
            return Theta;
        case ECameraModel::Equisolid:
            return std::sin(Theta * 0.5F) * 2.0F;
        case ECameraModel::Orthographic:
            return std::sin(Theta);
        case ECameraModel::KannalaBrandt:
            return KannalaBrandt::ComputeCameraPolynomial(Theta, Coeffs);
        case ECameraModel::LUT1D:
            return LUT1D::SampleForward(Theta, LUT, ThetaMax);
        default:
            check(false);
            return 0.0F;
        }
    }

    // Theta(R) for every model except BrownConrady.
    static float RadialInverse(
        ECameraModel Model,
        float Distance,
        TArrayView<const float> Coeffs,
        TArrayView<const float> LUT,
        float ThetaMax)
    {
        switch (Model)
        {
        case ECameraModel::Perspective:
            return std::atan(Distance);
        case ECameraModel::Stereographic:
            return std::atan(Distance * 0.5F) * 2.0F;
        case ECameraModel::Equidistant:
            return Distance;
        case ECameraModel::Equisolid:
            return std::asin(FMath::Clamp(Distance * 0.5F, -1.0F, 1.0F)) * 2.0F;
        case ECameraModel::Orthographic:
            return std::asin(FMath::Clamp(Distance, -1.0F, 1.0F));
        case ECameraModel::KannalaBrandt:
        {
            float Theta = Distance;
            for (uint8 i = 0; i != KannalaBrandtSolverIterations; ++i)
            {
                float N = Distance - KannalaBrandt::ComputeCameraPolynomial(Theta, Coeffs);
                float D = -KannalaBrandt::ComputeCameraPolynomialDerivative(Theta, Coeffs);
                Theta -= N / D;
            }
            return Theta;
        }
        case ECameraModel::LUT1D:
            return LUT1D::SampleInverse(Distance, LUT, ThetaMax);
        default:
            check(false);
            return 0.0F;
        }
    }

    static float SelectCAScale(const FLensModelDescriptor& Descriptor, ELensColorChannel Channel)
    {
        switch (Channel)
        {
        case ELensColorChannel::Red:
            return Descriptor.CAScaleR;
        case ELensColorChannel::Blue:
            return Descriptor.CAScaleB;
        default:
            return 1.0F;
        }
    }

    FVector2f ComputeLensImagePoint(
        const FLensModelDescriptor& Descriptor,
        FVector Direction,
        ELensColorChannel Channel)
    {
        Direction = Direction.GetSafeNormal();
        if (Direction.IsNearlyZero())
            Direction = FVector(0, 0, 1);

        const float CAScale = SelectCAScale(Descriptor, Channel);

        float Xn, Yn;

        if (Descriptor.Model == ECameraModel::BrownConrady)
        {
            const float Z = FMath::Max(static_cast<float>(Direction.Z), KINDA_SMALL_NUMBER);
            const float PinholeX = static_cast<float>(Direction.X) / Z;
            const float PinholeY = static_cast<float>(Direction.Y) / Z;
            const FVector2f Distorted = BrownConrady::Distort(PinholeX, PinholeY, Descriptor.Coeffs);
            Xn = Distorted.X;
            Yn = Distorted.Y;
        }
        else
        {
            const float Theta = std::acos(FMath::Clamp(static_cast<float>(Direction.Z), -1.0F, 1.0F));
            const float Phi = std::atan2(static_cast<float>(Direction.Y), static_cast<float>(Direction.X));
            const float R = RadialForward(Descriptor.Model, Theta, Descriptor.Coeffs, Descriptor.LUT, Descriptor.ThetaMax);
            Xn = R * std::cos(Phi);
            Yn = R * std::sin(Phi);
        }

        Xn *= CAScale;
        Yn *= CAScale;

        return FVector2f(
            Descriptor.CenterX + Descriptor.FocalX * Xn,
            Descriptor.CenterY + Descriptor.FocalY * Yn);
    }

    FVector ComputeLensRayDirection(
        const FLensModelDescriptor& Descriptor,
        FVector2f ImagePoint,
        ELensColorChannel Channel)
    {
        const float CAScale = SelectCAScale(Descriptor, Channel);
        const float SafeCAScale = FMath::IsNearlyZero(CAScale) ? 1.0F : CAScale;

        const float Xn = (ImagePoint.X - Descriptor.CenterX) / (Descriptor.FocalX * SafeCAScale);
        const float Yn = (ImagePoint.Y - Descriptor.CenterY) / (Descriptor.FocalY * SafeCAScale);

        if (Descriptor.Model == ECameraModel::BrownConrady)
        {
            const FVector2f Pinhole = BrownConrady::Undistort(Xn, Yn, Descriptor.Coeffs);
            return FVector(Pinhole.X, Pinhole.Y, 1.0F).GetSafeNormal();
        }

        const float R = std::sqrt(Xn * Xn + Yn * Yn);
        const float Phi = std::atan2(Yn, Xn);
        const float Theta = RadialInverse(Descriptor.Model, R, Descriptor.Coeffs, Descriptor.LUT, Descriptor.ThetaMax);
        const float SinTheta = std::sin(Theta);
        return FVector(
            SinTheta * std::cos(Phi),
            SinTheta * std::sin(Phi),
            std::cos(Theta));
    }

} // CameraModelUtil



IMPLEMENT_SHADER_TYPE(,
    FWideAngleLensShader_Perspective,
    TEXT("/Plugin/Carla/WideAngleLens.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FWideAngleLensShader_Stereographic,
    TEXT("/Plugin/Carla/WideAngleLens.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FWideAngleLensShader_Equidistance,
    TEXT("/Plugin/Carla/WideAngleLens.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FWideAngleLensShader_Equisolid,
    TEXT("/Plugin/Carla/WideAngleLens.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FWideAngleLensShader_Orthogonal,
    TEXT("/Plugin/Carla/WideAngleLens.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FWideAngleLensShader_Custom,
    TEXT("/Plugin/Carla/WideAngleLens.usf"),
    TEXT("MainCS"),
    SF_Compute);



IMPLEMENT_SHADER_TYPE(,
    FToPerspectiveShader_Perspective,
    TEXT("/Plugin/Carla/ToPerspective.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FToPerspectiveShader_Stereographic,
    TEXT("/Plugin/Carla/ToPerspective.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FToPerspectiveShader_Equidistance,
    TEXT("/Plugin/Carla/ToPerspective.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FToPerspectiveShader_Equisolid,
    TEXT("/Plugin/Carla/ToPerspective.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FToPerspectiveShader_Orthogonal,
    TEXT("/Plugin/Carla/ToPerspective.usf"),
    TEXT("MainCS"),
    SF_Compute);

IMPLEMENT_SHADER_TYPE(,
    FToPerspectiveShader_Custom,
    TEXT("/Plugin/Carla/ToPerspective.usf"),
    TEXT("MainCS"),
    SF_Compute);
