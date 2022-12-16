// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#include "Online/CustomFileDownloader.h"
#include "HttpModule.h"
#include "Http.h"
#include "Misc/FileHelper.h"

#include <OSM2ODR.h>


void UCustomFileDownloader::StartDownload()
{
  UE_LOG(LogCarla, Log, TEXT("FHttpDownloader CREATED"));
  FHttpDownloader* Test = new FHttpDownloader("GET", Url, ResultFileName);

  Test->Run();
}

void UCustomFileDownloader::ConvertOSMInOpenDrive(FString FilePath)
{
 IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();

  FString FileContent;
  // Always first check if the file that you want to manipulate exist.
  if (FileManager.FileExists(*FilePath))
  {
    // We use the LoadFileToString to load the file into
    if(FFileHelper::LoadFileToString(FileContent, *FilePath, FFileHelper::EHashOptions::None))
    {
      UE_LOG(LogCarla, Warning, TEXT("FileManipulation: Text From File: %s"), *FilePath);  
    }
    else
    {
      UE_LOG(LogCarla, Warning, TEXT("FileManipulation: Did not load text from file"));
    }
  }

  std::string OsmFile = std::string(TCHAR_TO_UTF8(*FileContent));
  std::string OpenDriveFile = osm2odr::ConvertOSMToOpenDRIVE(OsmFile);

  FString OpenDriveName = FPaths::GetCleanFilename(FilePath);
  OpenDriveName.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
  OpenDriveName += ".xodr";

  // We use the LoadFileToString to load the file into
  if( FFileHelper::SaveStringToFile(FString(OpenDriveFile.c_str()), *OpenDriveName) )
  {
    UE_LOG(LogCarla, Warning, TEXT("FileManipulation: Sucsesfuly Written: \"%s\" to the text file"), *OpenDriveName);
  }
  else
  {
    UE_LOG(LogCarla, Warning, TEXT("FileManipulation: Failed to write FString to file."));
  }
}

FHttpDownloader::FHttpDownloader(const FString& InVerb, const FString& InUrl, const FString& InFilename)
  : Verb(InVerb)
  , Url(InUrl)
  , Filename( InFilename )
{

}

void FHttpDownloader::Run(void)
{
  UE_LOG(LogCarla, Log, TEXT("Starting download [%s] Url=[%s]"), 	*Verb, *Url);
  TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
  Request->OnProcessRequestComplete().BindRaw(this, &FHttpDownloader::RequestComplete);
  Request->SetURL(Url);
  Request->SetVerb(Verb);
  Request->ProcessRequest();	
}

void FHttpDownloader::RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
  if (!HttpResponse.IsValid())
  {
    UE_LOG(LogCarla, Log, TEXT("Download failed. NULL response"));
  }
  else
  {
    UE_LOG(LogCarla, Log, TEXT("Completed download [%s] Url=[%s] Response=[%d]"), 
      *HttpRequest->GetVerb(), 
      *HttpRequest->GetURL(), 
        HttpResponse->GetResponseCode());

    HttpRequest->OnProcessRequestComplete().Unbind();

    FString CurrentFile = FPaths::ProjectContentDir() + "CustomMaps/" + Filename + "/";

    // We will use this FileManager to deal with the file.
    IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
    if( !FileManager.DirectoryExists(*CurrentFile) )
    {
      FileManager.CreateDirectory(*CurrentFile);
    }
    CurrentFile += Filename + ".osm";

    FString StringToWrite = HttpResponse->GetContentAsString();

    // We use the LoadFileToString to load the file into
    if( FFileHelper::SaveStringToFile(StringToWrite,*CurrentFile,  FFileHelper::EEncodingOptions::ForceUTF8) )
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
