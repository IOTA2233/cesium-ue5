//Copyright 2020-2022 MassSun. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WebSocketServerActor.h"

#include "Engine.h"
#include "WebSocketNetDriver.h"
#include "UObject/StrongObjectPtr.h"
#include "INetworkingWebSocket.h"
#include "IWebSocketServer.h"
#include "Modules/ModuleManager.h"
#include "Tickable.h"


#include "DsWebSocketServer.generated.h"


UCLASS(BlueprintType, Blueprintable)
class WEBSOCKETSERVER_API UDsWebSocketServer :public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:

	~UDsWebSocketServer();

	//Get a UDsWebSocketServer Object
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		static UDsWebSocketServer* getWebSocketServer()
	{
		UDsWebSocketServer* server = NewObject<UDsWebSocketServer>();
		return server;
	}


	virtual void Tick(float DeltaTime) override;
	TStatId GetStatId() const override;
	virtual ETickableTickType GetTickableTickType() const override
	{
		return ETickableTickType::Always;
	}
	virtual bool IsTickable() const override
	{
		return true;
	}
	virtual bool IsTickableInEditor() const override
	{
		return false;
	}
	virtual bool IsTickableWhenPaused() const override
	{
		return false;
	}



public:
	//whether show debug message on screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WebSocketServer")
		bool ShowOnScreenDebugMessages = false;

public:

	// Open WebSocket Server
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		bool Start(int Port);

	// Close WebSocket Server £¨The automatic call this func when BeginDestroy£©
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		void Stop();

	// It is automatically called in actor tick to maintain the connection of websocket
	bool WebSocketServerTick(float DeltaTime);

	// Send message by client ID
	void Send(const FGuid& InTargetClientId, const TArray<uint8>& InUTF8Payload);

	//Send message to all clients
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		void SendToAllClients(const FString msg);

	//Send byte message to all client
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		void SendBytesToAllClients(const TArray<uint8>& uint8Array);

	// Send Message by client ID
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		void SendToClientId(const FString clientId, const FString msg);

	// Send Byte Message by client ID
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		void SendBytesToClientId(const FString clientId, const TArray<uint8>& uint8Array);


	//send message to all web client
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		void Send(const FString msg);

	// Returns whether the server is currently running
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		bool IsRunning() const;


	//Client connected delegate
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, Category = "WebSocketServer")
		FWebSocketClientOnConnectedDelegate WsClientOnConnected;

	//Client closed delegate
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, Category = "WebSocketServer")
		FWebSocketClientOnClosedDelegate WsClientOnClosed;

	//Received client row message
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, Category = "WebSocketServer")
		FWebSocketClientOnRawMessageDelegate WsClientOnRawMessage;

	//Client error delegate
	UPROPERTY(BlueprintAssignable, VisibleAnywhere, Category = "WebSocketServer")
		FWebSocketClientOnErrorDelegate WsClientOnError;


	//Get client count
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		int getClientCount();

	//Get All Client Id
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		TArray<FString> getClients();

	//get clientid by clientname
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		FString getClientIdByName(FString name);

	//set a name for client
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		void setClientNameById(FString clientid, FString name);

	//convert FString to utf8 bytes
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		TArray<uint8> FStringToUTF8Bytes(FString Message);

	//Convert FString to ansi bytes
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		TArray<uint8> FStringToANSIBytes(FString Message);

	//Convert ansi byte to FString
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		FString ANSIBytesToFString(const TArray<uint8>& data);

	//Convert utf8 bytes to FString
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		FString UTF8BytesToFString(const TArray<uint8>& data);

	//Convert FString to TChar bytes
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		TArray<uint8> FStringToTCHARBytes(FString Message);

	//Convert TChar bytes to FString
	UFUNCTION(BlueprintCallable, Category = "WebSocketServer")
		FString TCHARBytesToFString(const TArray<uint8>& ubytes);


	//Pring message
	void _DebugLog(FString msg, float delayTime, FColor color);


private:
	// Handles a new client connecting
	void OnWebSocketClientConnected(INetworkingWebSocket* Socket);

	// Handles sending the received packet to the message router.
	void ReceivedRawPacket(void* Data, int32 Size, FGuid ClientId);

	// Handles a client close
	void OnSocketClose(INetworkingWebSocket* Socket);

	void OnClientSocketError(INetworkingWebSocket* Socket);


private:
	/** Holds a web socket connection to a client. */
	class FWebSocketConnection
	{
	public:

		explicit FWebSocketConnection(INetworkingWebSocket* InSocket)
			: Socket(InSocket)
			, Id(FGuid::NewGuid())
		{
		}

		FWebSocketConnection(FWebSocketConnection&& WebSocketConnection)
			: Id(WebSocketConnection.Id)
		{
			Socket = WebSocketConnection.Socket;
			WebSocketConnection.Socket = nullptr;
		}

		~FWebSocketConnection()
		{
			if (Socket)
			{

				delete Socket;
				Socket = nullptr;
			}
		}

		FWebSocketConnection(const FWebSocketConnection&) = delete;
		FWebSocketConnection& operator=(const FWebSocketConnection&) = delete;
		FWebSocketConnection& operator=(FWebSocketConnection&&) = delete;

		/** Underlying WebSocket. */
		INetworkingWebSocket* Socket = nullptr;

		/** Generated ID for this client. */
		FGuid Id;
		FString  clientName;
	};



private:
	/** Holds the LibWebSocket wrapper. */
	TUniquePtr<IWebSocketServer> Server;

	/** Holds all active connections. */
	TArray<FWebSocketConnection> Connections;

};
