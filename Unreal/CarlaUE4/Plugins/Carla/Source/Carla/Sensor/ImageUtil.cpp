#include "Carla/Sensor/ImageUtil.h"
#include "Runtime/RHI/Public/RHISurfaceDataConversion.h"
#include <thread>



namespace ImageUtil
{
    TArray<FColor> ExtractTexturePixelsFromReadback(
		FRHIGPUTextureReadback* Readback,
        FIntPoint SourceExtent,
        FIntPoint DestinationExtent,
		EPixelFormat Format)
    {
        TArray<FColor> Result;
        check(Readback->IsReady());
        
        auto FormatInfo = GPixelFormats[Format];
        auto BlockSizeX = FormatInfo.BlockSizeX;
        auto BlockSizeY = FormatInfo.BlockSizeY;
        auto BlockBytes = FormatInfo.BlockBytes;
        auto OutPixelCount = DestinationExtent.X * DestinationExtent.Y;
	    Result.AddUninitialized(OutPixelCount);
        
        void* MappedPtr;
        int32 SourcePitch;
        Readback->LockTexture(FRHICommandListExecutor::GetImmediateCommandList(), MappedPtr, SourcePitch);
		check(MappedPtr != nullptr);
        auto Data = (uint8*)MappedPtr;
		SourcePitch *= BlockBytes;

        switch (Format)
        {
	        case PF_FloatRGBA:
	        	for (int32 Y = 0; Y < DestinationExtent.Y; Y++)
	        	{
	        		auto SrcPtr = (FFloat16Color*)(Data + Y * SourcePitch);
	        		auto DestPtr = Result.GetData() + Y * DestinationExtent.X;
	        		for (int32 X = 0; X < DestinationExtent.X; X++)
	        		{
	        			*DestPtr = FLinearColor(
							(float)SrcPtr->R,
							(float)SrcPtr->G,
							(float)SrcPtr->B,
							(float)SrcPtr->A).ToFColor(true);
	        			++SrcPtr;
	        			++DestPtr;
	        		}
	        	}
	        	break;
            case PF_R16F:
				ConvertRawR16DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData());
	        	break;
	        case PF_R8G8B8A8:
				ConvertRawR8G8B8A8DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData());
				break;
	        case PF_B8G8R8A8:
				ConvertRawB8G8R8A8DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData());
				break;
	        case PF_A2B10G10R10:
				ConvertRawR10G10B10A2DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData());
	        	break;
	        case PF_R16G16B16A16_UNORM:
	        case PF_R16G16B16A16_SNORM:
				ConvertRawR16G16B16A16FDataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData(), false);
	        	break;
	        case PF_FloatR11G11B10:
				ConvertRawR11G11B10DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData(), false);
	        	break;
	        case PF_R32G32B32A32_UINT:
				ConvertRawR32G32B32A32DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData(), false);
	        	break;
	        case PF_X24_G8:
				ConvertRawR24G8DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData(), FReadSurfaceDataFlags());
	        	break;
	        case PF_R32_FLOAT:
				ConvertRawR32DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData(), FReadSurfaceDataFlags());
	        	break;
	        case PF_R16G16B16A16_UINT:
	        case PF_R16G16B16A16_SINT:
				ConvertRawR16G16B16A16DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData());
	        	break;
			case PF_R16G16_UINT:
				ConvertRawR16G16DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData());
	        	break;
            case PF_G8:
                UE_LOG(LogCarla, Warning, TEXT("PING"));
				ConvertRawR8DataToFColor(DestinationExtent.X, DestinationExtent.Y, Data, SourcePitch, Result.GetData());
	        	break;
	        default:
                UE_LOG(LogCarla, Warning, TEXT("Unsupported format %llu"), (unsigned long long)Format);
                check(false);
                break;
        }
		Readback->Unlock();

        return Result;
    }
}