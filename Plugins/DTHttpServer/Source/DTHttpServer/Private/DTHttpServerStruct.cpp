// Copyright 2023 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#include "DTHttpServerStruct.h"

void UDTHttpServerBPLib::BreakParams(const FDTHttpServerParams& HttpServerParams, TMap<FString, FString>& Params)
{
	Params = HttpServerParams.Params;
}

void UDTHttpServerBPLib::FindParam(const FDTHttpServerParams& HttpServerParams, const FString& Key, FString& Param)
{
	Param.Empty();
	if ( const FString * pParam = HttpServerParams.Params.Find(Key) )
	{
		Param = *pParam;
	}
}

void UDTHttpServerBPLib::BreakHeaders(const FDTHttpServerHeaders& HttpServerHeaders, TMap<FString, FString>& Headers)
{
	Headers = HttpServerHeaders.Headers;
}

void UDTHttpServerBPLib::FindHeader(const FDTHttpServerHeaders& HttpServerHeaders, const FString& Key, FString& Header)
{
	Header.Empty();
	if ( const FString * pHeader = HttpServerHeaders.Headers.Find(Key) )
	{
		Header = *pHeader;
	}
}

