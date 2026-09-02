// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.

#undef CreateDirectory

#include "Online/CustomFileDownloader.h"
#include "OpenDriveToMap.h"
#include "Generation/OpenDriveFileGenerationParameters.h"
#include "HttpModule.h"
#include "Http.h"
#include "Misc/FileHelper.h"
#include "CarlaTools.h"
#include "GenerationPathsHelper.h"

#if defined(WITH_OSM2ODR) && __has_include(<OSM2ODR.h>)
  #define HAS_OSM2ODR
  #include <OSM2ODR.h>
#endif

void UCustomFileDownloader::ConvertOSMInOpenDrive(
    FString FilePath,
    float Lat_0, float Lon_0,
    const FOpenDriveFileGenerationParameters& OpenDriveGenParams)
{
#ifdef HAS_OSM2ODR
  IPlatformFile &FileManager = FPlatformFileManager::Get().GetPlatformFile();

  FString FileContent;
  // Always first check if the file that you want to manipulate exist.
  if (FileManager.FileExists(*FilePath))
  {
    // We use the LoadFileToString to load the file into
    if (!FFileHelper::LoadFileToString(FileContent, *FilePath, FFileHelper::EHashOptions::None))
    {
      UE_LOG(LogCarlaTools, Warning, TEXT("FileManipulation: Did not load text from \"%s\""), *FilePath);
    }
  }
  else
  {
    UE_LOG(LogCarlaTools, Warning, TEXT("File: \"%s\" does not exist"), *FilePath);
    return;
  }

  std::string OpenDriveFile;

  osm2odr::OSM2ODRSettings Settings;
  Settings.default_sidewalk_width = OpenDriveGenParams.DefaultSidewalkWidth;
  Settings.default_lane_width = OpenDriveGenParams.DefaultLaneWidth;
  Settings.elevation_layer_height = OpenDriveGenParams.DefaultOSMLayerHeight;
  Settings.proj_string += " +lat_0=" + std::to_string(Lat_0) + " +lon_0=" + std::to_string(Lon_0);
  Settings.center_map = false;

  try
  {
    FTCHARToUTF8 FileContentUTF8(*FileContent, FileContent.Len());
    auto OsmFile = std::string(FileContentUTF8.Get(), FileContentUTF8.Length());
    OpenDriveFile = osm2odr::ConvertOSMToOpenDRIVE(OsmFile, Settings);
  }
  catch (std::runtime_error& re)
  {
    FString fs;
    fs = re.what();
    UE_LOG(LogCarlaTools, Error, TEXT("FileManipulation: osm2odr::ConvertOSMToOpenDRIVE failed: %s"), *fs);
  }
  
  FilePath.RemoveFromEnd(".osm", ESearchCase::Type::IgnoreCase);
  FilePath += ".xodr";

  if (FFileHelper::SaveStringToFile(FString(OpenDriveFile.c_str()), *FilePath))
  {
    UE_LOG(LogCarlaTools, Display, TEXT("FileManipulation: Successfully Written: \"%s\" to the text file"), *FilePath);
  }
  else
  {
    UE_LOG(LogCarlaTools, Error, TEXT("FileManipulation: Failed to write FString to file."));
  }
#else
  UE_LOG(LogCarlaTools, Error, TEXT(
      "ConvertOSMInOpenDrive: CARLA was built without OSM2ODR support; cannot convert \"%s\"."),
      *FilePath);
#endif
}

void UCustomFileDownloader::StartDownload()
{
  UE_LOG(LogCarlaTools, Log, TEXT("FHttpDownloader CREATED"));
  UE_LOG(LogCarlaTools, Log, TEXT("Map Name Is %s"), *ResultFileName );
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

static TAutoConsoleVariable<float> CVarCFDTimeout(
	TEXT("CarlaDT.CustomFileDownloader.Timeout"),
	-1.0F,
	TEXT("Sets the timeout for OSM downloads."));

void FHttpDownloader::Run(void)
{
  UE_LOG(LogCarlaTools, Log, TEXT("Starting download [%s] Url=[%s]"), *Verb, *Url);
  TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
  UE_LOG(LogCarlaTools, Log, TEXT("Map Name Is %s"), *Filename );
  Request->OnProcessRequestComplete().BindRaw(this, &FHttpDownloader::RequestComplete);
  Request->SetURL(Url);
  float Timeout = CVarCFDTimeout.GetValueOnAnyThread();
  if (Timeout > 0.0F)
    Request->SetTimeout(Timeout);
  Request->SetActivityTimeout(Timeout);
  Request->SetVerb(Verb);
  Request->ProcessRequest();
}

void FHttpDownloader::RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
  if (!HttpResponse.IsValid() )
  {
    UE_LOG(LogCarlaTools, Log, TEXT("Download failed. NULL response"));
  }
  else
  {
    // If we do not get success responses codes we do not do anything
    if (HttpResponse->GetResponseCode() < 200 || 300 <= HttpResponse->GetResponseCode())
    {
      UE_LOG(LogCarlaTools, Error, TEXT("Error during download [%s] Url=[%s] Response=[%d]"),
             *HttpRequest->GetVerb(),
             *HttpRequest->GetURL(),
             HttpResponse->GetResponseCode());
      return;
    }

    UE_LOG(LogCarlaTools, Log, TEXT("Completed download [%s] Url=[%s] Response=[%d]"),
           *HttpRequest->GetVerb(),
           *HttpRequest->GetURL(),
           HttpResponse->GetResponseCode());

    FString CurrentFile = UGenerationPathsHelper::GetRawMapDirectoryPath(Filename) + "OpenDrive/";
    UE_LOG(LogCarlaTools, Log, TEXT("FHttpDownloader::RequestComplete CurrentFile %s."), *CurrentFile);

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
      UE_LOG(LogCarlaTools, Log, TEXT("FileManipulation: Successfully Written "));
      DelegateToCall.ExecuteIfBound();
    }
    else
    {
      UE_LOG(LogCarlaTools, Log, TEXT("FileManipulation: Failed to write FString to file."));
      UE_LOG(LogCarlaTools, Log, TEXT("FileManipulation: CurrentFile %s."), *CurrentFile);
    }
  }
  delete this;
}
