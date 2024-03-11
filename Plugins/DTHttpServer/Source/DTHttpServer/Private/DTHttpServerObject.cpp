// Copyright 2023 Dexter.Wan. All Rights Reserved. 
// EMail: 45141961@qq.com

#include "DTHttpServerObject.h"
#include "HttpServerModule.h"
#include "IHttpRouter.h"
#include "Core/Public/Misc/ConfigCacheIni.h"
#include "Sockets/Public/IPAddress.h"
#include <string>

// 开始销毁
void UDTHttpServerObject::BeginDestroy()
{
	if ( m_HttpRouter.IsValid() )
	{
		for ( const auto & HttpRouteHandle : m_HttpRouteHandles )
		{
			m_HttpRouter->UnbindRoute(HttpRouteHandle);
		}
		m_HttpRouter.Reset();
	}
	UObject::BeginDestroy();
}

// 创建路由
void UDTHttpServerObject::StartListen(int Port)
{
	// 设置所有端口都监听
	static const FString IniSectionName(TEXT("HTTPServer.Listeners"));
	GConfig->SetString(*IniSectionName, TEXT("DefaultBindAddress"), TEXT("any"), GEngineIni);
	
	// 监听路由
	m_HttpRouter = FHttpServerModule::Get().GetHttpRouter(Port);
	FHttpServerModule::Get().StartAllListeners();
}

TUniquePtr<FHttpServerResponse> UDTHttpServerObject::CreateHttpServerResponse() const
{
	// 创建返回数据
	TUniquePtr<FHttpServerResponse> Response = MakeUnique<FHttpServerResponse>();
	Response->Code = EHttpServerResponseCodes::Ok;

	// 设置返回类型
	Response->Headers.Add(TEXT("Content-Type"), { TEXT("application/json;charset=utf-8") });
	Response->Headers.Add(TEXT("Access-Control-Allow-Origin"), { TEXT("*") });
	Response->Headers.Add(TEXT("Access-Control-Allow-Methods"), { TEXT("GET,POST,PUT,PATCH,DELETE,OPTIONS") });
	Response->Headers.Add(TEXT("Access-Control-Allow-Headers"), { TEXT("Origin,X-Requested-With,Content-Type,Accept") });
	Response->Headers.Add(TEXT("Access-Control-Max-Age"), { TEXT("600") });
	Response->Headers.Add(TEXT("Access-Control-Allow-Credentials"), { TEXT("true") });

	return Response;
}

// 创建服务对象
void UDTHttpServerObject::CreateHttpServer(int Port, UDTHttpServerObject*& HttpServer)
{
	// 创建服务器并开启监听
	HttpServer = NewObject<UDTHttpServerObject>();
	HttpServer->StartListen(Port);
}

// 绑定Get消息
void UDTHttpServerObject::Bind(const FString& HttpPath, EDTHttpServerVerbs HttpVerbs, FHttpResponse HttpResponse)
{
	// 无效路由
	if ( !m_HttpRouter.IsValid() ) { return; }

	// 绑定配置消息
	FHttpRouteHandle HttpRouteOptions = m_HttpRouter->BindRoute(HttpPath, EHttpServerRequestVerbs::VERB_OPTIONS,
		[this](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete) {

			OnComplete(CreateHttpServerResponse());

			return true;
		});

	// 设置类型
	EHttpServerRequestVerbs Verb = EHttpServerRequestVerbs::VERB_NONE;
	switch ( HttpVerbs )
	{
	case EDTHttpServerVerbs::GET:
		Verb = EHttpServerRequestVerbs::VERB_GET;
		break;
	case EDTHttpServerVerbs::POST:
		Verb = EHttpServerRequestVerbs::VERB_POST;
		break;
	case EDTHttpServerVerbs::PUT:
		Verb = EHttpServerRequestVerbs::VERB_PUT;
		break;
	case EDTHttpServerVerbs::PATCH:
		Verb = EHttpServerRequestVerbs::VERB_PATCH;
		break;
	case EDTHttpServerVerbs::DELETE:
		Verb = EHttpServerRequestVerbs::VERB_DELETE;
		break;
	}
	
	// 绑定正常接口
	FHttpRouteHandle HttpRouteHandle = m_HttpRouter->BindRoute(HttpPath, Verb,
		[this, HttpResponse](const FHttpServerRequest& Request, const FHttpResultCallback& OnComplete)
		{
			// 创建返回对象
			TUniquePtr<FHttpServerResponse> Response = CreateHttpServerResponse();
			
			// 有效执行
			FString ResponseInfo;
			if ( HttpResponse.IsBound())
			{
				// 重新生成头
				FDTHttpServerHeaders DTHttpServerHeaders;
				for ( const auto & Header : Request.Headers )
				{
					DTHttpServerHeaders.Headers.Add(Header.Key, FString::Join(Header.Value, TEXT(",")));
				}

				// 重新生成参数
				FDTHttpServerParams DTHttpServerParams;
				DTHttpServerParams.Params = Request.QueryParams;

				// 获取数据
				FString Body;
				if ( Request.Body.Num() > 0 )
				{
					const std::string szBody((char*)Request.Body.GetData(), Request.Body.Num());
					Body = UTF8_TO_TCHAR(szBody.c_str());
				}

				// 执行回调函数
				ResponseInfo = HttpResponse.Execute(
					Request.RelativePath.GetPath(),
					DTHttpServerHeaders,
					DTHttpServerParams,
					Body);
			}

			// 赋值返回数据
			if ( !ResponseInfo.IsEmpty() )
			{
				std::string szInfo;
				szInfo = TCHAR_TO_UTF8(*ResponseInfo);
				Response->Body.Append((const uint8*)szInfo.c_str(), szInfo.length());
			}

			// 返回网页
			OnComplete(MoveTemp(Response));
			
			return true;
		});

	// 添加缓存
	m_HttpRouteHandles.Add(HttpRouteOptions);
	m_HttpRouteHandles.Add(HttpRouteHandle);
}
