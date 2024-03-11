// Copyright 2023 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DTHttpServerStruct.generated.h"

UENUM()
enum class EDTHttpServerVerbs : uint8
{
	GET,
	POST,
	PUT,
	PATCH,
	DELETE,
};

USTRUCT(BlueprintType, meta=(DisplayName="DT Http Server Params", HasNativeBreak = "DTHttpServer.DTHttpServerBPLib.BreakParams"))
struct FDTHttpServerParams
{
	GENERATED_BODY()
	TMap<FString, FString> Params;
};

USTRUCT(BlueprintType, meta=(DisplayName="DT Http Server Headers", HasNativeBreak = "DTHttpServer.DTHttpServerBPLib.BreakHeaders"))
struct FDTHttpServerHeaders
{
	GENERATED_BODY()
	TMap<FString, FString> Headers;
};

UCLASS(NotBlueprintable, NotBlueprintType, meta=(DisplayName="DT Http Server BP Library"))
class DTHTTPSERVER_API UDTHttpServerBPLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Break DT Http Server Params
	UFUNCTION(BlueprintPure, meta = (DisplayName="Break Http Server Params"), Category = "DT Http Server|Params")
	static void BreakParams(const FDTHttpServerParams& HttpServerParams, TMap<FString, FString>& Params);

	// Find DT Http Server Params
	UFUNCTION(BlueprintPure, meta = (DisplayName="Find Http Server Params"), Category = "DT Http Server|Params")
	static void FindParam(const FDTHttpServerParams& HttpServerParams, const FString & Key, FString & Param );
	
	// Break DT Http Server Headers
	UFUNCTION(BlueprintPure, meta = (DisplayName="Break Http Server Headers"), Category = "DT Http Server|Headers")
	static void BreakHeaders(const FDTHttpServerHeaders& HttpServerHeaders, TMap<FString, FString>& Headers);

	// Find DT Http Server Headers
	UFUNCTION(BlueprintPure, meta = (DisplayName="Find Http Server Headers"), Category = "DT Http Server|Headers")
	static void FindHeader(const FDTHttpServerHeaders& HttpServerHeaders, const FString & Key, FString & Header );
};
