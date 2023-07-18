// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "CustomFileDownloader.generated.h"
/**
 *
 */

DECLARE_DELEGATE(FDownloadComplete)


struct FHttpDownloader
{
public:
  FHttpDownloader();

  /**
   *
   * @param Verb - verb to use for request (GET,POST,DELETE,etc)
   * @param Url - url address to connect to
   */
  FHttpDownloader( const FString& InVerb, const FString& InUrl, const FString& InFilename, FDownloadComplete& Delegate  );

  // Kick off the Http request  and wait for delegate to be called
  void Run(void);

  /**
   * Delegate called when the request completes
   *
   * @param HttpRequest - object that started/processed the request
   * @param HttpResponse - optional response object if request completed
   * @param bSucceeded - true if Url connection was made and response was received
   */
  void RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

private:
  FString Verb;
  FString Url;
  FString Filename;
  FDownloadComplete DelegateToCall;
};

UCLASS(Blueprintable)
class CARLATOOLS_API UCustomFileDownloader : public UObject
{
  GENERATED_BODY()
public:
  UFUNCTION(BlueprintCallable)
  void StartDownload();
  UFUNCTION(BlueprintCallable)
  void ConvertOSMInOpenDrive(FString FilePath, float Lat_0 = 0, float Lon_0 = 0);

  FString ResultFileName;

  FString Url;

  FDownloadComplete DownloadDelegate;
private:
  void RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

  FString Payload;
};


