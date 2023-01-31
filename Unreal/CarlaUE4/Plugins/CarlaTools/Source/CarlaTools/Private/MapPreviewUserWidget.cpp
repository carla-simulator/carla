// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "MapPreviewUserWidget.h"

#include "Sockets.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Common/TcpSocketBuilder.h"
#include "SocketSubsystem.h"

void UMapPreviewUserWidget::ConnectToSocket()
{
    //FSocket* Socket = FSocket::CreateTCPConnection(nullptr, TEXT("OSMRendererSocket"));
    FSocket* Socket = FTcpSocketBuilder(TEXT("OSMRendererSocket")).AsReusable();
    FIPv4Address RemoteAddress;
    FIPv4Address::Parse(FIPv4Address::InternalLoopback.ToString(), RemoteAddress);
    TSharedRef<FInternetAddr> RemoteAddr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
    RemoteAddr->SetIp(RemoteAddress.Value);
    RemoteAddr->SetPort(5000);

    // Connect to the remote server
    bool Connected = Socket->Connect(*RemoteAddr);
    if (!Connected)
    {
        UE_LOG(LogTemp, Error, TEXT("Error connecting to remote server"));
        return;
    }

    // Send a message
    FString Message = "-C /home/aollero/Downloads/libosmcout/libosmscout-master/maps/madrid_downtown/madrid_big /home/aollero/Downloads/libosmcout/libosmscout-master/stylesheets/standard.oss 512";
    //uint8* SendBuffer = (uint8*)TCHAR_TO_UTF8(*Message);
    std::string MessageStr = std::string(TCHAR_TO_UTF8(*Message));
    int32 BytesSent = 0;
    bool bSent = Socket->Send((uint8*)MessageStr.c_str(), MessageStr.size(), BytesSent);
    if (!bSent)
    {
        UE_LOG(LogTemp, Error, TEXT("Error sending message"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Sent %d bytes"), BytesSent);
    }

    int32 BytesReceived = 0;
    uint8* ReceivedData = 0;
    uint32 ReceivedDataSize = 0;
    bool bRecv = Socket->Recv(ReceivedData, ReceivedDataSize, BytesReceived);
    if (!bRecv)
    {
        UE_LOG(LogTemp, Error, TEXT("Error receiving message"));
    }
    else
    {
        //std::string ReceivedStr = std::string((char*)ReceivedData);
        FString Str = UTF8_TO_TCHAR(ReceivedData);
        UE_LOG(LogTemp, Log, TEXT("Received %d bytes. [%d]"), BytesReceived, Str.Len());
    }

    // Close the socket
    Socket->Close();
}
