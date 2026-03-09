// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Engine/Engine.h"

void UMyGameInstance::Init()
{
	Super::Init();

	//the default active online subsystem
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		SessionInterface = Subsystem->GetSessionInterface(); //ptr that holds the session-interface
		UE_LOG(LogTemp, Log, TEXT("OnlineSubsystem loaded: %s"), *Subsystem->GetSubsystemName().ToString());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("No OnlineSubsystem found"));
	}

	if (GEngine)
	{
		GEngine->OnNetworkFailure().AddUObject(
			this, &UMyGameInstance::HandleNetworkFailure);

		GEngine->OnTravelFailure().AddUObject(
			this, &UMyGameInstance::HandleTravelFailure);
	}
}

void UMyGameInstance::PlayGame()
{

	if (bConnectionInProgress)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Connection already in progress"));
		return;
	}

	FString Url = TEXT("https://3jmztmby8g.execute-api.us-east-1.amazonaws.com/prod-1/quick-play");

	TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
		FHttpModule::Get().CreateRequest();

	Request->SetURL(Url);
	Request->SetVerb(TEXT("POST"));
	Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	Request->SetContentAsString(TEXT("{}")); 

	Request->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
		{
			if (!bSuccess || !Resp.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("QuickPlay request failed"));
				return;
			}

			TSharedPtr<FJsonObject> Json;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(Resp->GetContentAsString());

			if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid QuickPlay JSON"));
				return;
			}

			FString Ip = Json->GetStringField(TEXT("ip"));
			int32 Port = Json->GetIntegerField(TEXT("port"));
			const FString PlayerSessionId =
				Json->GetStringField(TEXT("playerSessionId"));

			const FString TravelURL =
				FString::Printf(
					TEXT("%s:%d?PlayerSessionId=%s"),
					*Ip,
					Port,
					*PlayerSessionId
				);

			UE_LOG(LogTemp, Log, TEXT("QuickPlay joining %s"), *TravelURL);

			ConnectToServer(TravelURL);
		}
	);

	Request->ProcessRequest();
}

void UMyGameInstance::CreateGameSession()
{
	//test-1 ---> working

	/*GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("GameInstance: CreateGameSession"));

	FString url = TEXT("https://3jmztmby8g.execute-api.us-east-1.amazonaws.com/prod/ping");

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL(url);
	Request->SetVerb("GET");

	Request->OnProcessRequestComplete().BindLambda(
		[](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess) {

			if (!bSuccess || !Res.IsValid()) {
				UE_LOG(LogTemp, Error, TEXT("HTTP request failed"));
				return;
			}

			UE_LOG(LogTemp, Log, TEXT("Http response recieved: %s"), *Res->GetContentAsString());
		}
	);

	Request->ProcessRequest();*/

	//test - 2 ---> working
	/*GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Creating Game Session"));

	const FString CreateSessionURL =
		TEXT("https://3jmztmby8g.execute-api.us-east-1.amazonaws.com/prod-1/create-session");

	TSharedRef<IHttpRequest> CreateReq = FHttpModule::Get().CreateRequest();
	CreateReq->SetURL(CreateSessionURL);
	CreateReq->SetVerb("POST");

	CreateReq->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
		{
			if (!bSuccess || !Res.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("CreateSession HTTP failed"));
				return;
			}

			// Parse GameSessionId
			TSharedPtr<FJsonObject> Json;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(Res->GetContentAsString());

			if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid JSON from create-session"));
				return;
			}

			const FString GameSessionId = Json->GetStringField("gameSessionId");

			UE_LOG(LogTemp, Log, TEXT("GameSessionId: %s"), *GameSessionId);

			
			// JOIN SESSION
		
			const FString JoinURL =
				TEXT("https://3jmztmby8g.execute-api.us-east-1.amazonaws.com/prod-1/join-session");

			TSharedRef<IHttpRequest> JoinReq = FHttpModule::Get().CreateRequest();
			JoinReq->SetURL(JoinURL);
			JoinReq->SetVerb("POST");
			JoinReq->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

			TSharedPtr<FJsonObject> Body = MakeShared<FJsonObject>();
			Body->SetStringField("gameSessionId", GameSessionId);

			FString BodyStr;
			TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&BodyStr);
			FJsonSerializer::Serialize(Body.ToSharedRef(), Writer);

			JoinReq->SetContentAsString(BodyStr);

			JoinReq->OnProcessRequestComplete().BindLambda(
				[this](FHttpRequestPtr Req2, FHttpResponsePtr Res2, bool bSuccess2)
				{
					if (!bSuccess2 || !Res2.IsValid())
					{
						UE_LOG(LogTemp, Error, TEXT("JoinSession HTTP failed"));
						return;
					}

					TSharedPtr<FJsonObject> JoinJson;
					TSharedRef<TJsonReader<>> Reader2 =
						TJsonReaderFactory<>::Create(Res2->GetContentAsString());

					if (!FJsonSerializer::Deserialize(Reader2, JoinJson) || !JoinJson.IsValid())
					{
						UE_LOG(LogTemp, Error, TEXT("Invalid JSON from join-session"));
						return;
					}

					const FString Ip = JoinJson->GetStringField("ip");
					const int32 Port = JoinJson->GetIntegerField("port");
					const FString PlayerSessionId =
						JoinJson->GetStringField("playerSessionId");

					UE_LOG(LogTemp, Log, TEXT("Connecting to %s:%d"), *Ip, Port);

					if (APlayerController* PC = GetFirstLocalPlayerController())
					{
						const FString TravelURL =
							FString::Printf(TEXT("%s:%d?PlayerSessionId=%s"),
								*Ip, Port, *PlayerSessionId);

						PC->ClientTravel(TravelURL, TRAVEL_Absolute);
					}
				}
			);

			JoinReq->ProcessRequest();
		}
	);

	CreateReq->ProcessRequest();*/

if (bConnectionInProgress)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Connection already in progress"));
	return;
}

GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Creating Game Session"));

const FString CreateURL =
TEXT("https://3jmztmby8g.execute-api.us-east-1.amazonaws.com/prod-1/create-session");

TSharedRef<IHttpRequest> CreateReq = FHttpModule::Get().CreateRequest();
CreateReq->SetURL(CreateURL);
CreateReq->SetVerb("POST");

CreateReq->OnProcessRequestComplete().BindLambda(
	[this](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
	{
		if (!bSuccess || !Res.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Create-session failed"));
			return;
		}

		TSharedPtr<FJsonObject> Json;
		TSharedRef<TJsonReader<>> Reader =
			TJsonReaderFactory<>::Create(Res->GetContentAsString());

		if (!FJsonSerializer::Deserialize(Reader, Json) || !Json.IsValid())
		{
			UE_LOG(LogTemp, Error, TEXT("Invalid JSON from create-session"));
			return;
		}

		const FString SessionCode = Json->GetStringField("sessionCode");

		CurrentMatchCode = SessionCode;

		UE_LOG(LogTemp, Log, TEXT("SESSION CODE: %s"), *SessionCode);

		GEngine->AddOnScreenDebugMessage(
			-1, 30.f, FColor::Yellow,
			FString::Printf(TEXT("SESSION CODE: %s"), *SessionCode)
		);

		
		// join uing code
		
		const FString JoinURL =
			TEXT("https://3jmztmby8g.execute-api.us-east-1.amazonaws.com/prod-1/join-session");

		TSharedRef<IHttpRequest> JoinReq = FHttpModule::Get().CreateRequest();
		JoinReq->SetURL(JoinURL);
		JoinReq->SetVerb("POST");
		JoinReq->SetHeader(TEXT("Content-Type"), TEXT("application/json"));

		const FString Body =
			FString::Printf(TEXT("{\"code\":\"%s\"}"), *SessionCode);

		JoinReq->SetContentAsString(Body);

		JoinReq->OnProcessRequestComplete().BindLambda(
			[this](FHttpRequestPtr Req2, FHttpResponsePtr Res2, bool bSuccess2)
			{
				if (!bSuccess2 || !Res2.IsValid())
				{
					UE_LOG(LogTemp, Error, TEXT("Join-session failed"));
					return;
				}

				TSharedPtr<FJsonObject> JoinJson;
				TSharedRef<TJsonReader<>> Reader2 =
					TJsonReaderFactory<>::Create(Res2->GetContentAsString());

				if (!FJsonSerializer::Deserialize(Reader2, JoinJson) || !JoinJson.IsValid())
				{
					UE_LOG(LogTemp, Error, TEXT("Invalid JSON from join-session"));
					return;
				}

				if (!JoinJson->HasField("ip") ||
					!JoinJson->HasField("port") ||
					!JoinJson->HasField("playerSessionId"))
				{
					UE_LOG(LogTemp, Error, TEXT("Missing fields in join-session response"));
					return;
				}

				const FString Ip = JoinJson->GetStringField("ip");
				const int32 Port = JoinJson->GetIntegerField("port");
				const FString PlayerSessionId =
					JoinJson->GetStringField("playerSessionId");

				const FString TravelURL =
					FString::Printf(TEXT("%s:%d?PlayerSessionId=%s"),
						*Ip, Port, *PlayerSessionId);

				UE_LOG(LogTemp, Log, TEXT("ClientTravel: %s"), *TravelURL);

				ConnectToServer(TravelURL);
			}
		);

		JoinReq->ProcessRequest();
	}
);

CreateReq->ProcessRequest();

}

void UMyGameInstance::JoinGameSession(const FString& SessionCode)
{
	if (bConnectionInProgress)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Connection already in progress"));
		return;
	}

	TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();

	Request->SetURL("https://3jmztmby8g.execute-api.us-east-1.amazonaws.com/prod-1/join-session");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");

	const FString Body = FString::Printf(
		TEXT("{\"code\":\"%s\"}"), *SessionCode
	);

	Request->SetContentAsString(Body);

	Request->OnProcessRequestComplete().BindLambda(
		[this](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bSuccess)
		{
			if (!bSuccess || !Res.IsValid())
			{
				UE_LOG(LogTemp, Error, TEXT("Join session HTTP failed"));
				return;
			}

			TSharedPtr<FJsonObject> Json;
			TSharedRef<TJsonReader<>> Reader =
				TJsonReaderFactory<>::Create(Res->GetContentAsString());

			if (!FJsonSerializer::Deserialize(Reader, Json))
			{
				UE_LOG(LogTemp, Error, TEXT("Invalid JSON"));
				return;
			}

			if (!Json->HasField("ip") ||
				!Json->HasField("port") ||
				!Json->HasField("playerSessionId"))
			{
				UE_LOG(LogTemp, Error, TEXT("Missing fields in join-session response"));
				return;
			}

			const FString IP = Json->GetStringField("ip");
			const int32 Port = Json->GetIntegerField("port");
			const FString PlayerSessionId = Json->GetStringField("playerSessionId");

			const FString TravelURL =
				FString::Printf(TEXT("%s:%d?PlayerSessionId=%s"),
					*IP, Port, *PlayerSessionId);

			ConnectToServer(TravelURL);
		}
	);

	Request->ProcessRequest();
}

void UMyGameInstance::ConnectToServer(const FString& ServerURL)
{
	if (bConnectionInProgress)
	{
		UE_LOG(LogTemp, Warning, TEXT("Connection already in progress"));
		return;
	}

	bConnectionInProgress = true;

	UE_LOG(LogTemp, Log, TEXT("Connecting to server: %s"), *ServerURL);

	UWorld* World = GetWorld();
	if (!World)
	{
		UE_LOG(LogTemp, Error, TEXT("No valid world in ConnectToServer"));
		return;
	}

	// Timeout
	World->GetTimerManager().SetTimer(
		ConnectionTimeoutHandle,
		this,
		&UMyGameInstance::OnConnectionTimeout,
		ConnectionTimeoutSeconds,
		false
	);

	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		PC->ClientTravel(ServerURL, TRAVEL_Absolute);
	}
}

void UMyGameInstance::OnConnectionTimeout()
{
	UE_LOG(LogTemp, Error, TEXT("Connection timed out"));

	CleanupFailedConnection(TEXT("Timeout"));
}

void UMyGameInstance::HandleNetworkFailure(
	UWorld* World,
	UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType,
	const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Network failure: %s"), *ErrorString);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
		FString::Printf(TEXT("Network failure: %s"), *ErrorString));

	CleanupFailedConnection(ErrorString);
}

void UMyGameInstance::HandleTravelFailure(
	UWorld* World,
	ETravelFailure::Type FailureType,
	const FString& ErrorString)
{
	UE_LOG(LogTemp, Error, TEXT("Travel failure: %s"), *ErrorString);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
		FString::Printf(TEXT("Travel failure: %s"), *ErrorString));

	CleanupFailedConnection(ErrorString);
}

void UMyGameInstance::CleanupFailedConnection(const FString& Reason)
{
	if (!bConnectionInProgress)
	{
		return;
	}

	UE_LOG(LogTemp, Error, TEXT("Connection failed: %s"), *Reason);

	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(ConnectionTimeoutHandle);
	}

	bConnectionInProgress = false;

}

void UMyGameInstance::OnSuccessfulConnection()
{
	if (!bConnectionInProgress) return;

	GetWorld()->GetTimerManager().ClearTimer(ConnectionTimeoutHandle);
	bConnectionInProgress = false;

	UE_LOG(LogTemp, Log, TEXT("Connection successful"));
}
