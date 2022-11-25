// Copyright (c) 2017 Computer Vision Center (CVC) at the Universitat Autonoma de Barcelona (UAB). This work is licensed under the terms of the MIT license. For a copy, see <https://opensource.org/licenses/MIT>.


#include "Online/CustomFileDownloader.h"
#include "HttpModule.h"
#include "Http.h"


void UCustomFileDownloader::StartDownload()
{
    UE_LOG(LogCarla, Log, TEXT("FHttpTest CREATED"));
    FHttpTest* Test = new FHttpTest("GET", Url);

    Test->Run();
}

FHttpTest::FHttpTest(const FString& InVerb, const FString& InUrl)
	: Verb(InVerb)
	, Url(InUrl)
{
	
}

void FHttpTest::Run(void)
{
	UE_LOG(LogCarla, Log, TEXT("Starting test [%s] Url=[%s]"), 
		*Verb, *Url);

	TSharedPtr<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindRaw(this, &FHttpTest::RequestComplete);
    Request->SetURL(Url);
    Request->SetVerb(Verb);
    Request->ProcessRequest();
	
}

void FHttpTest::RequestComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSucceeded)
{
	
    if (!HttpResponse.IsValid())
	{
		UE_LOG(LogCarla, Log, TEXT("Test failed. NULL response"));
	}
	else
	{
		UE_LOG(LogCarla, Log, TEXT("Completed test [%s] Url=[%s] Response=[%d] [%s]"), 
			*HttpRequest->GetVerb(), 
			*HttpRequest->GetURL(), 
			HttpResponse->GetResponseCode(), 
			*HttpResponse->GetContentAsString());
	}
	
    HttpRequest->OnProcessRequestComplete().Unbind();
    delete this;
}
