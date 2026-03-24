// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#undef CreateDirectory

#include "Online/CustomFileDownloader.h"
#include "OpenDriveToMap.h"
#include "HttpModule.h"
#include "Http.h"
#include "Misc/FileHelper.h"

#include <OSM2ODR.h>

void UCustomFileDownloader::ConvertOSMInOpenDrive(FString FilePath, float Lat_0, float Lon_0)
{
  IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();

  FString FileContent;
  // Always first check if the file that you want to manipulate exist.
  if (FileManager.FileExists(*FilePath))
  {
    // We use the LoadFileToString to load the file into
    if (FFileHelper::LoadFileToString(FileContent, *FilePath, FFileHelper::EHashOptions::None))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Text From File: %s"), *FilePath);
    }
    else
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Did not load text from file"));
    }
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("File: %s does not exist"), *FilePath);
    return;
  }
  std::string OsmFile = std::string(TCHAR_TO_UTF8(*FileContent));

  osm2odr::OSM2ODRSettings Settings;
  Settings.proj_string += " +lat_0=" + std::to_string(Lat_0) + " +lon_0=" + std::to_string(Lon_0);
  Settings.center_map = false;
  std::string OpenDriveFile = osm2odr::ConvertOSMToOpenDRIVE(OsmFile, Settings);

  FilePath.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
  FilePath += ".xodr";

  // We use the LoadFileToString to load the file into
  if (FFileHelper::SaveStringToFile(FString(OpenDriveFile.c_str()), *FilePath))
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Sucsesfuly Written: \"%s\" to the text file"), *FilePath);
  }
  else
  {
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Failed to write FString to file."));
  }
}

void UCustomFileDownloader::StartDownload()
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("FHttpDownloader CREATED"));
  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Map Name Is %s"), *ResultFileName );
  FHttpDownloader *Download = new FHttpDownloader("GET", Url, ResultFileName, DownloadDelegate);
  Download->Run();
}

FHttpDownloader::FHttpDownloader(const FString &InVerb, const FString &InUrl, const FString &InFilename, FDownloadComplete &Delegate)
    : Verb(InVerb), Url(InUrl), Filename(InFilename), DelegateToCall(Delegate)
{
}

FHttpDownloader::FHttpDownloader()
{

}

void FHttpDownloader::Run(void)
{
  UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Starting download [%s] Url=[%s]"), *Verb, *Url);
  TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
  UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("Map Name Is %s"), *Filename );
  Request->OnProcessRequestComplete().BindRaw(this, &FHttpDownloader::RequestComplete);
  Request->SetURL(Url);
  Request->SetVerb(Verb);
  Request->ProcessRequest();
}

void FHttpDownloader::RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
  if (!HttpResponse.IsValid() )
  {
    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Download failed. NULL response"));
  }
  else
  {
    // If we do not get success responses codes we do not do anything
    if (HttpResponse->GetResponseCode() < 200 || 300 <= HttpResponse->GetResponseCode())
    {
      UE_LOG(LogCarlaToolsMapGenerator, Error, TEXT("Error during download [%s] Url=[%s] Response=[%d]"),
             *HttpRequest->GetVerb(),
             *HttpRequest->GetURL(),
             HttpResponse->GetResponseCode());
      return;
    }

    UE_LOG(LogCarlaToolsMapGenerator, Log, TEXT("Completed download [%s] Url=[%s] Response=[%d]"),
           *HttpRequest->GetVerb(),
           *HttpRequest->GetURL(),
           HttpResponse->GetResponseCode());

    FString CurrentFile = FPaths::ProjectContentDir() + "CustomMaps/" + (Filename) + "/OpenDrive/";
    UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FHttpDownloader::RequestComplete CurrentFile %s."), *CurrentFile);

    // We will use this FileManager to deal with the file.
    IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();
    if (!FileManager.DirectoryExists(*CurrentFile))
    {
      FileManager.CreateDirectory(*CurrentFile);
    }
    CurrentFile += Filename + ".osm";

    FString StringToWrite = HttpResponse->GetContentAsString();

    // We use the LoadFileToString to load the file into
    if (FFileHelper::SaveStringToFile(StringToWrite, *CurrentFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Sucsesfuly Written "));
      DelegateToCall.ExecuteIfBound();
    }
    else
    {
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: Failed to write FString to file."));
      UE_LOG(LogCarlaToolsMapGenerator, Warning, TEXT("FileManipulation: CurrentFile %s."), *CurrentFile);
    }
  }
  delete this;
}
