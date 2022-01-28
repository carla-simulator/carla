// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "MediaOutput.h"

#include "Carla/Sensor/NetMediaCapture.h"

#include "Engine/RendererSettings.h"
#include "ImageWriteBlueprintLibrary.h"

#include "NetMediaOutput.generated.h"


/** Texture format supported by UNetMediaOutput. */
UENUM()
enum class ENetMediaOutputPixelFormat
{
	B8G8R8A8					UMETA(DisplayName = "8bit RGBA"),
	FloatRGBA					UMETA(DisplayName = "Float RGBA"),
};


/**
 * Output information for a file media capture.
 * @note	'Frame Buffer Pixel Format' must be set to at least 8 bits of alpha to enabled the Key.
 * @note	'Enable alpha channel support in post-processing' must be set to 'Allow through tonemapper' to enabled the Key.
 */
UCLASS(BlueprintType)
class CARLA_API UNetMediaOutput : public UMediaOutput
{
	GENERATED_BODY()

public:
	UNetMediaOutput();

public:
	/** Options on how to save the images. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="File")
	FImageWriteOptions WriteOptions;

	/** The file path for the images. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="File", meta=(RelativePath))
	FDirectoryPath FilePath;

	/** The base file name of the images. The frame number will be append to the base file name. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="File", meta=(RelativePath))
	FString BaseFileName;

	/** Use the default back buffer size or specify a specific size to capture. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Media", meta = (InlineEditConditionToggle))
	bool bOverrideDesiredSize;

	/** Use the default back buffer size or specify a specific size to capture. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Media", meta=(EditCondition="bOverrideDesiredSize"))
	FIntPoint DesiredSize;

	/** Use the default back buffer pixel format or specify a specific the pixel format to capture. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Media", meta = (InlineEditConditionToggle))
	bool bOverridePixelFormat;

	/** Use the default back buffer pixel format or specify a specific the pixel format to capture. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Media", meta=(EditCondition="bOverridePixelFormat"))
	ENetMediaOutputPixelFormat DesiredPixelFormat;

 	UNetMediaCapture *GetMediaCapture()
	{
		return MediaCapture;
	}

	//~ UMediaOutput interface
public:
	virtual bool Validate(FString& FailureReason) const override;
	virtual FIntPoint GetRequestedSize() const override;
	virtual EPixelFormat GetRequestedPixelFormat() const override;
	virtual EMediaCaptureConversionOperation GetConversionOperation(EMediaCaptureSourceType InSourceType) const override;

protected:
	virtual UMediaCapture* CreateMediaCaptureImpl() override;

private:
	UNetMediaCapture *MediaCapture = nullptr;

	//~ End UMediaOutput interface
};
