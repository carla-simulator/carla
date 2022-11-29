// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Online/CustomFileDownloader.h"
#include "HttpModule.h"
#include "Http.h"
#include "Misc/FileHelper.h"


void UCustomFileDownloader::StartDownload()
{
    UE_LOG(LogCarla, Log, TEXT("FHttpDownloader CREATED"));
    FHttpDownloader* Test = new FHttpDownloader("GET", Url, ResultFileName);

    Test->Run();
}

FHttpDownloader::FHttpDownloader(const FString& InVerb, const FString& InUrl, const FString& InFilename)
	: Verb(InVerb)
	, Url(InUrl)
    , Filename( InFilename )
{
	
}

void FHttpDownloader::Run(void)
{
  UE_LOG(LogCarla, Log, TEXT("Starting test [%s] Url=[%s]"), 	*Verb, *Url);
  TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
  Request->OnProcessRequestComplete().BindRaw(this, &FHttpTest::RequestComplete);
  Request->SetURL(Url);
  Request->SetVerb(Verb);
  Request->ProcessRequest();	
}

void FHttpDownloader::RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
  if (!HttpResponse.IsValid())
  {
      UE_LOG(LogCarla, Log, TEXT("Test failed. NULL response"));
  }
  else
  {
    UE_LOG(LogCarla, Log, TEXT("Completed test [%s] Url=[%s] Response=[%d]"), 
      *HttpRequest->GetVerb(), 
      *HttpRequest->GetURL(), 
       HttpResponse->GetResponseCode());
  
    HttpRequest->OnProcessRequestComplete().Unbind();
  
    FString CurrentFile = FPaths::ProjectConfigDir();
    CurrentFile.Append(Filename);
  
    // We will use this FileManager to deal with the file.
    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
    FString StringToWrite = HttpResponse->GetContentAsString();
  
    // We use the LoadFileToString to load the file into
    if( FFileHelper::SaveStringToFile(StringToWrite,*CurrentFile) )
    {
      UE_LOG(LogCarla, Warning, TEXT("FileManipulation: Sucsesfuly Written "));
    }
    else
    {
      UE_LOG(LogCarla, Warning, TEXT("FileManipulation: Failed to write FString to file."));
    }
  }
  
  delete this;
}
