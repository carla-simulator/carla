// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "MapPreviewUserWidget.h"

#if PLATFORM_WINDOWS
  #include "AllowWindowsPlatformTypes.h"
#endif

#include "GenericPlatform/GenericPlatformMath.h"
#include "GenericPlatform/GenericPlatformFile.h"

#include "Sockets.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Common/TcpSocketBuilder.h"
#include "SocketSubsystem.h"
#include "SocketTypes.h"

#if PLATFORM_WINDOWS
  #include "HideWindowsPlatformTypes.h"
#endif

#include "Engine/Texture2D.h"
#include "Containers/ResourceArray.h"
#include "Containers/UnrealString.h"
#include "Rendering/Texture2DResource.h"
#include "RHI.h"
#include "RHICommandList.h"
#include "RenderingThread.h"
#include "Misc/Timespan.h"
#include "Containers/UnrealString.h"
#include "Misc/Paths.h"


namespace Asio = boost::asio;
using AsioStreamBuf = boost::asio::streambuf;
using AsioTCP = boost::asio::ip::tcp;
using AsioSocket = boost::asio::ip::tcp::socket;
using AsioAcceptor = boost::asio::ip::tcp::acceptor;
using AsioEndpoint = boost::asio::ip::tcp::endpoint;

void UMapPreviewUserWidget::CreateTexture()
{
  if(!MapTexture)
  {
    MapTexture = UTexture2D::CreateTransient(512,512,EPixelFormat::PF_R8G8B8A8,"MapTextureRendered");
    MapTexture->UpdateResource();
  }
}

void UMapPreviewUserWidget::ConnectToSocket(FString DatabasePath, FString StylesheetPath, int Size)
{
  const unsigned int PORT = 5000;
  SocketPtr = std::make_unique<AsioSocket>(io_service);
  SocketPtr->connect(AsioEndpoint(AsioTCP::v4(), PORT));
  if(!SocketPtr->is_open())
  {
    UE_LOG(LogTemp, Error, TEXT("Error connecting to remote server"));
    return;
  }

  // Send a message
  FString Message = "-C " + DatabasePath + " " + StylesheetPath + " " + FString::FromInt(Size);
  if( !SendStr(Message) ){
    return;
  }
  UE_LOG(LogTemp, Log, TEXT("Configuration Completed"));
}

void UMapPreviewUserWidget::RenderMap(FString Latitude, FString Longitude, FString Zoom)
{
  FString Message = "-R " + Latitude + " " + Longitude + " " + Zoom;
  if( !SendStr(Message) ){
    UE_LOG(LogTemp, Log, TEXT("Send Str failed"));
    return;
  }

  TArray<uint8_t> ReceivedData;
  uint32 ReceivedDataSize = 0;

  {
    SocketPtr->wait(boost::asio::ip::tcp::socket::wait_read);
    while (SocketPtr->available())
    {
      AsioStreamBuf Buffer;
      std::size_t BytesReceived =
        Asio::read(*SocketPtr, Buffer, Asio::transfer_at_least(2));
      TArray<uint8_t> ThisReceivedData;
      const char* DataPtr = Asio::buffer_cast<const char*>(Buffer.data());
      for (std::size_t i = 0; i < Buffer.size(); ++i)
      {
        ThisReceivedData.Add(DataPtr[i]);
      }
      ReceivedData.Append(ThisReceivedData);
    }
    UE_LOG(LogTemp, Log, TEXT("Size of Data: %d"), ReceivedData.Num());

    // TODO: Move to function
    if(ReceivedData.Num() > 0)
    {
      ENQUEUE_RENDER_COMMAND(UpdateDynamicTextureCode)
      (
        [NewData=ReceivedData, Texture=MapTexture](auto &InRHICmdList) mutable
        {
          UE_LOG(LogTemp, Log, TEXT("RHI: Updating texture"));
          FUpdateTextureRegion2D Region;
          Region.SrcX = 0;
          Region.SrcY = 0;
          Region.DestX = 0;
          Region.DestY = 0;
          Region.Width = Texture->GetSizeX();
          Region.Height = Texture->GetSizeY();

          FTexture2DResource* Resource = (FTexture2DResource*)Texture->Resource;
          RHIUpdateTexture2D(Resource->GetTexture2DRHI(), 0, Region, Region.Width * sizeof(uint8_t) * 4, &NewData[0]);
        }
      );
    }
  }
}

FString UMapPreviewUserWidget::RecvCornersLatLonCoords()
{
  if( !SendStr("-L") ){
    UE_LOG(LogTemp, Error, TEXT("Error sending message: num bytes mismatch"));
    return FString();
  }

  AsioStreamBuf Buffer;
  std::size_t BytesReceived =
      Asio::read(*SocketPtr, Buffer, Asio::transfer_at_least(2));
  std::string BytesStr = Asio::buffer_cast<const char*>(Buffer.data());

  FString CoordStr = FString(BytesStr.size(), UTF8_TO_TCHAR(BytesStr.c_str()));
  UE_LOG(LogTemp, Log, TEXT("Received Coords %s"), *CoordStr);
  return CoordStr;
}

void UMapPreviewUserWidget::Shutdown()
{
  // Close the socket
  SocketPtr->close();
}

void UMapPreviewUserWidget::OpenServer()
{
  // Todo: automatically spawn the osm renderer process
}

void UMapPreviewUserWidget::CloseServer()
{
  if( !SendStr("-X") ){
    UE_LOG(LogTemp, Error, TEXT("Error sending message"));
    return;
  }
}

bool UMapPreviewUserWidget::SendStr(FString Msg)
{
  if(!SocketPtr)
  {
    UE_LOG(LogTemp, Error, TEXT("Error. No socket."));
    return false;
  }

  std::string MessageStr = std::string(TCHAR_TO_UTF8(*Msg));
  std::size_t BytesSent = 0;
  try
  {
    BytesSent = Asio::write(*SocketPtr, Asio::buffer(MessageStr));
  }
  catch (const boost::system::system_error& e)
  {
    FString ErrorMessage = e.what();
    UE_LOG(LogTemp, Error, TEXT("Error sending message: %s"), *ErrorMessage);
  }
  if (BytesSent != MessageStr.size())
  {
    UE_LOG(LogTemp, Error, TEXT("Error sending message: num bytes mismatch"));
    return false;
  }
  else
  {
    UE_LOG(LogTemp, Log, TEXT("Sent %d bytes"), BytesSent);
    return true;
  }
}

void UMapPreviewUserWidget::UpdateLatLonCoordProperties()
{
  FString CoordStr = RecvCornersLatLonCoords();
  if(CoordStr.Len() == 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Error during update of lat lon coord properties. Check osm server connection or use OSMURL to generate map") );
    return;
  }
  UE_LOG(LogTemp, Log, TEXT("Received laton [%s] with size %d"), *CoordStr, CoordStr.Len());

  TArray<FString> CoordsArray;
  CoordStr.ParseIntoArray(CoordsArray, TEXT("&"), true);

  ensure(CoordsArray.Num() == 4);

  TopRightLat = FCString::Atof(*CoordsArray[0]);
  TopRightLon = FCString::Atof(*CoordsArray[1]);
  BottomLeftLat = FCString::Atof(*CoordsArray[2]);
  BottomLeftLon = FCString::Atof(*CoordsArray[3]);
}
