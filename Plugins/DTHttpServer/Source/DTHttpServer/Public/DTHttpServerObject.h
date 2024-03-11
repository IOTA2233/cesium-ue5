// Copyright 2023 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "DTHttpServerStruct.h"
#include "UObject/Object.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "DTHttpServerObject.generated.h"

UCLASS(Blueprintable, BlueprintType, meta=(DisplayName="DT Http Server"))
class DTHTTPSERVER_API UDTHttpServerObject : public UObject
{
	GENERATED_BODY()
	
	DECLARE_DYNAMIC_DELEGATE_RetVal_FourParams(FString, FHttpResponse, const FString &, RelativePath, const FDTHttpServerHeaders &, Headers, const FDTHttpServerParams &, QueryParams, const FString &, Body );
	
protected:
	TArray<FHttpRouteHandle>					m_HttpRouteHandles;
	TSharedPtr<IHttpRouter>						m_HttpRouter;

public:
	// 开始销毁
	virtual void BeginDestroy() override;

protected:
	// 开始监听
	void StartListen(int Port);
	// 返回跨域查询头
	TUniquePtr<FHttpServerResponse> CreateHttpServerResponse() const;
	
public:
	// Per-port-binding access to an http router
	// Param "Port" : Listening port, range 1-65535
	UFUNCTION(BlueprintCallable, meta = ( Port=8001 ), Category="DT Http Server")
	static void CreateHttpServer(int Port, UDTHttpServerObject *& HttpServer);
	
	// Binds the caller-supplied Uri to the caller-supplied handler
	// Param "Http Path" : The respective http path to bind
	// Param "Http Verbs" : The respective HTTP verbs to bind
	// Param "Http Response" : The caller-defined closure to execute when the binding is invoked
	UFUNCTION(BlueprintCallable, Category="DT Http Server")
	void Bind(const FString& HttpPath, EDTHttpServerVerbs HttpVerbs, FHttpResponse HttpResponse);
	
};
