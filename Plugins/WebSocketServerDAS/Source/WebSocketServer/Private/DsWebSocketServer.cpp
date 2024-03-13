// Copyright 2020-2022 MassSun. All Rights Reserved.


#include "DsWebSocketServer.h"
#include "Containers/Ticker.h"
#include "IPAddress.h"
#include "IWebSocketNetworkingModule.h"
#include "WebSocketNetworkingDelegates.h"
#include <string>
#include "Runtime/Core/Public/Misc/CString.h"


#if PLATFORM_WINDOWS
#include "Windows/AllowWindowsPlatformTypes.h"
#include "Windows/MinWindows.h"
#include "Windows/HideWindowsPlatformTypes.h"
#endif



void UDsWebSocketServer::Tick(float DeltaTime)
{
	WebSocketServerTick(DeltaTime);
}

TStatId UDsWebSocketServer::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UDsWebSocketServer, STATGROUP_Tickables);
}


UDsWebSocketServer::~UDsWebSocketServer()
{
	Stop();
}



bool UDsWebSocketServer::Start(int Port)
{
	FWebSocketClientConnectedCallBack CallBack;
	CallBack.BindUObject(this, &UDsWebSocketServer::OnWebSocketClientConnected);

	Server = FModuleManager::Get().LoadModuleChecked<IWebSocketNetworkingModule>(TEXT("WebSocketNetworking")).CreateServer();

	if (!Server || !Server->Init(Port, CallBack))
	{
		Server.Reset();
		return false;
	}

	return true;
}

void UDsWebSocketServer::Stop()
{
	if (IsRunning()) {
		Server.Reset();
	}
}

bool UDsWebSocketServer::WebSocketServerTick(float DeltaTime)
{
	if (IsRunning()) {
		Server->Tick();
		return true;
	}
	else {
		return false;
	}
}

void UDsWebSocketServer::Send(const FGuid& InTargetClientId, const TArray<uint8>& InUTF8Payload)
{
	if (FWebSocketConnection* Connection = Connections.FindByPredicate([&InTargetClientId](const FWebSocketConnection& InConnection)
		{ return InConnection.Id == InTargetClientId; }))
	{
		Connection->Socket->Send(InUTF8Payload.GetData(), InUTF8Payload.Num(), /*PrependSize=*/false);
	}
}

void UDsWebSocketServer::Send(const FString msg)
{
	FTCHARToUTF8 utf8Str(*msg);
	int32 utf8StrLen = utf8Str.Length();

	TArray<uint8> uint8Array;
	uint8Array.SetNum(utf8StrLen);
	memcpy(uint8Array.GetData(), utf8Str.Get(), utf8StrLen);

	for (auto& ws : Connections) {
		ws.Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
	}
}


void UDsWebSocketServer::SendBytesToClientId(const FString clientId, const TArray<uint8>& uint8Array)
{
	//for (auto& ws : Connections) {
	//	if (ws->getId() == clientId)
	//		ws->Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
	//}
	if (FWebSocketConnection* Connection = Connections.FindByPredicate([clientId](const FWebSocketConnection& InConnection)
		{ return InConnection.Id.ToString() == clientId; }))
	{
		Connection->Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
	}
}


void UDsWebSocketServer::SendToClientId(const FString clientId, const FString msg)
{
	FTCHARToUTF8 utf8Str(*msg);
	int32 utf8StrLen = utf8Str.Length();

	TArray<uint8> uint8Array;
	uint8Array.SetNum(utf8StrLen);
	memcpy(uint8Array.GetData(), utf8Str.Get(), utf8StrLen);

	//for (auto& ws : Connections) {
	//	if (ws->Id.ToString() == clientId)
	//		ws->Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
	//}
	if (FWebSocketConnection* Connection = Connections.FindByPredicate([clientId](const FWebSocketConnection& InConnection)
		{ return InConnection.Id.ToString() == clientId; }))
	{
		Connection->Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
	}

}


void UDsWebSocketServer::SendBytesToAllClients(const TArray<uint8>& uint8Array)
{

	for (auto& ws : Connections) {
		ws.Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
	}

}


void UDsWebSocketServer::SendToAllClients(const FString msg)
{
	FTCHARToUTF8 utf8Str(*msg);
	int32 utf8StrLen = utf8Str.Length();

	TArray<uint8> uint8Array;
	uint8Array.SetNum(utf8StrLen);
	memcpy(uint8Array.GetData(), utf8Str.Get(), utf8StrLen);

	for (auto& ws : Connections) {
		ws.Socket->Send(uint8Array.GetData(), uint8Array.Num(), /*PrependSize=*/false);
	}
}







bool UDsWebSocketServer::IsRunning() const
{
	return !!Server;
}

void UDsWebSocketServer::_DebugLog(FString msg, float delayTime, FColor color)
{
	if (ShowOnScreenDebugMessages)
		GEngine->AddOnScreenDebugMessage(-1, delayTime, color, msg);// 打印到屏幕
	UE_LOG(LogTemp, Log, TEXT("%s"), *msg);// 打印到outputlog
}


void UDsWebSocketServer::OnClientSocketError(INetworkingWebSocket* Socket)
{

}

void UDsWebSocketServer::OnWebSocketClientConnected(INetworkingWebSocket* Socket)
{
	_DebugLog("----OnWebSocketClientConnected ", 10, FColor::Red);
	if (ensureMsgf(Socket, TEXT("Socket was null while creating a new websocket connection.")))
	{
		FWebSocketConnection Connection = FWebSocketConnection{ Socket };

		FWebSocketPacketReceivedCallBack ReceiveCallBack;
		ReceiveCallBack.BindUObject(this, &UDsWebSocketServer::ReceivedRawPacket, Connection.Id);
		Socket->SetReceiveCallBack(ReceiveCallBack);

		FWebSocketInfoCallBack CloseCallback;
		CloseCallback.BindUObject(this, &UDsWebSocketServer::OnSocketClose, Socket);
		Socket->SetSocketClosedCallBack(CloseCallback);

		FWebSocketInfoCallBack ErrorCallBack;
		ErrorCallBack.BindUObject(this, &UDsWebSocketServer::OnClientSocketError, Socket);
		Socket->SetErrorCallBack(ErrorCallBack);


		Connections.Add(MoveTemp(Connection));

		//有新客户端连接 delegate
		WsClientOnConnected.Broadcast(Connection.Id.ToString());
	}
}

void UDsWebSocketServer::ReceivedRawPacket(void* Data, int32 Size, FGuid ClientId)
{
	TArrayView<uint8> dataArrayView = MakeArrayView(static_cast<uint8*>(Data), Size);
	const std::string cstr(reinterpret_cast<const char*>(
		dataArrayView.GetData()),
		dataArrayView.Num());
	//FString frameAsFString = UTF8_TO_TCHAR(cstr.c_str());
	//OnH5MsgCallback(frameAsFString);


	TArray<uint8> bytesArray;
	bytesArray.Append((uint8*)Data, Size);

	//收到数据 delegate
	WsClientOnRawMessage.Broadcast(bytesArray, Size, ClientId.ToString());
}


void UDsWebSocketServer::OnSocketClose(INetworkingWebSocket* Socket)
{
	int32 Index = Connections.IndexOfByPredicate([Socket](const FWebSocketConnection& Connection) { return Connection.Socket == Socket; });

	_DebugLog("----OnSocketClose " + FString::FromInt(Index), 10, FColor::Red);
	if (Index != INDEX_NONE)
	{
		//OnConnectionClosed().Broadcast(Connections[Index].Id);
		WsClientOnClosed.Broadcast(Connections[Index].Id.ToString());
		Connections.RemoveAtSwap(Index);
	}
}




int UDsWebSocketServer::getClientCount()
{

	return Connections.Num();
}


TArray<FString> UDsWebSocketServer::getClients()
{
	TArray<FString> result;
	for (int i = 0; i < Connections.Num(); i++)
	{
		result.Add(Connections[i].Id.ToString());
	}
	return result;
}

void UDsWebSocketServer::setClientNameById(FString clientid, FString name)
{
	if (FWebSocketConnection* Connection = Connections.FindByPredicate([clientid](const FWebSocketConnection& InConnection)
		{ return InConnection.Id.ToString() == clientid; }))
	{
		Connection->clientName = name;
	}
}

FString UDsWebSocketServer::getClientIdByName(FString name)
{
	if (FWebSocketConnection* Connection = Connections.FindByPredicate([name](const FWebSocketConnection& InConnection)
		{ return InConnection.clientName == name; }))
	{
		return Connection->Id.ToString();
	}
	return FString(TEXT(""));
}


/*
FWebSocketConnection* AWebSocketServer::getClient(FString clientid)
{
	FWebSocketConnection* connection = nullptr;


	if (FWebSocketConnection* Connection = Connections.FindByPredicate([&clientid](const FWebSocketConnection& InConnection)
		{ return InConnection.Id.toString() == clientid; }))
	{
		return Connection;
	}

	return nullptr;

//	for (int i = 0; i < Connections.Num(); i++)
//	{
//		if (Connections[i]->getId() == clientid)
//		{
//			connection = Connections[i];
//			break;
//		}
//	}
//	return connection;

}

*/

/**
* Convert FString to  UTF8 bytes
*/
TArray<uint8> UDsWebSocketServer::FStringToUTF8Bytes(FString Message)
{
	const TCHAR* tchar = Message.GetCharArray().GetData();
	int iLength = WideCharToMultiByte(CP_UTF8, 0, tchar, -1, NULL, 0, NULL, NULL);
	char* _utfchar = new char[iLength];
	//memset(_utfchar, 0, iLength);
	WideCharToMultiByte(CP_UTF8, 0, tchar, -1, _utfchar, iLength, NULL, NULL);

	TArray<uint8> utf8Array;
	utf8Array.Append((uint8*)_utfchar, iLength);

	delete[]_utfchar;
	return utf8Array;
}

/**
* Convert FString to  ANSI bytes
*/
TArray<uint8> UDsWebSocketServer::FStringToANSIBytes(FString Message)
{
	int iLength;

	const TCHAR* tchar = Message.GetCharArray().GetData();
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	char* _char = new char[iLength];
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);

	TArray<uint8> bytesArray;
	bytesArray.Append((uint8*)_char, iLength);

	delete[]_char;
	return bytesArray;
}



/**
* Convert ANSI bytes to FString
*/
FString UDsWebSocketServer::ANSIBytesToFString(const TArray<uint8>& data)
{
	int32 num = data.Num();
	char* pchar = new char[num];
	memcpy(pchar, data.GetData(), num);

	int32 iLength = MultiByteToWideChar(CP_ACP, 0, pchar, num, NULL, 0);
	TCHAR* tchar2 = new TCHAR[iLength + 1];
	memset(tchar2, 0, sizeof(TCHAR) * (iLength + 1));
	MultiByteToWideChar(CP_ACP, 0, pchar, num, tchar2, iLength);

	FString fstring = FString(tchar2);
	delete[]pchar;
	delete[]tchar2;
	return fstring;
}

/**
* Convert UTF8 bytes to FString
*/
FString UDsWebSocketServer::UTF8BytesToFString(const TArray<uint8>& data)
{
	int32 num = data.Num();
	char* _utfchar = new char[num];
	memcpy(_utfchar, data.GetData(), num);
	int iLength = MultiByteToWideChar(CP_UTF8, 0, _utfchar, num, NULL, 0);
	TCHAR* tchar3 = new TCHAR[iLength + 1];
	memset(tchar3, 0, sizeof(TCHAR) * (iLength + 1));
	MultiByteToWideChar(CP_UTF8, 0, _utfchar, num, tchar3, iLength);
	FString utf8str = FString(tchar3);
	delete[]_utfchar;
	delete[]tchar3;
	return utf8str;
}


/**
* Convert FString to TCHAR Bytes
*/
TArray<uint8> UDsWebSocketServer::FStringToTCHARBytes(FString Message)
{
	TArray<uint8> ubytes;
	TArray<TCHAR> tchars = Message.GetCharArray();
	int32 ilen1 = Message.Len();
	for (int32 k = 0; k < ilen1; k++)
	{
		WORD word = tchars[k];
		uint8 high = HIBYTE(word);
		uint8 low = LOBYTE(word);
		ubytes.Add(high);
		ubytes.Add(low);
	}
	return ubytes;
}

/**
* Convert TCHAR bytes to FString
*/
FString UDsWebSocketServer::TCHARBytesToFString(const TArray<uint8>& ubytes)
{
	TArray<TCHAR> tchars;
	for (int32 k1 = 0; k1 < ubytes.Num(); k1 += 2)
	{
		uint8 high = ubytes[k1];
		uint8 low = ubytes[k1 + 1];
		WORD word = MAKEWORD(low, high);
		TCHAR tc = word;
		tchars.Add(tc);
	}

	return FString(tchars);
}

