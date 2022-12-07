// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/IHttpRequest.h"
#include "CustomFileDownloader.generated.h"
/**
 * 
 */
UCLASS(Blueprintable)
class CARLA_API UCustomFileDownloader : public UObject
{
  GENERATED_BODY()
public:	
  UFUNCTION(BlueprintCallable)
  void StartDownload();
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FString ResultFileName;
  UPROPERTY( EditAnywhere, BlueprintReadWrite, Category="Settings" )
  FString Url;
private:
  void RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded);

  FString Payload;
};

class FHttpDownloader
{
public:
  /**
   *
   *
   * @param Verb - verb to use for request (GET,POST,DELETE,etc)
   * @param Url - url address to connect to
   */
  FHttpDownloader( const FString& InVerb, const FString& InUrl, const FString& InFilename );  

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
};
