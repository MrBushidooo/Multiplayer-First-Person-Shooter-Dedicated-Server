// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameMode.h"
#include "GameLiftServerSDK.h"
#include "Kismet/GameplayStatics.h"
#include "GulagCharacter.h"
#include "MyGameState.h"
#include "MyPlayerState.h"
#include "MyPlayerController.h"
#include <UObject/FastReferenceCollector.h>

DEFINE_LOG_CATEGORY(GameServerLog);

void AMyGameMode::InitGameLift()
{
#if WITH_GAMELIFT
    UE_LOG(GameServerLog, Log, TEXT("Calling InitGameLift..."));

    // Getting the module first.
    FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

    //Define the server parameters for a GameLift Anywhere fleet. These are not needed for a GameLift managed EC2 fleet.
    FServerParameters ServerParametersForAnywhere;

    bool bIsAnywhereActive = false;
    if (FParse::Param(FCommandLine::Get(), TEXT("glAnywhere")))
    {
        bIsAnywhereActive = true;
    }

    if (bIsAnywhereActive)
    {
        UE_LOG(GameServerLog, Log, TEXT("Configuring server parameters for Anywhere..."));

        // If GameLift Anywhere is enabled, parse command line arguments and pass them in the ServerParameters object.
        FString glAnywhereWebSocketUrl = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereWebSocketUrl="), glAnywhereWebSocketUrl))
        {
            ServerParametersForAnywhere.m_webSocketUrl = TCHAR_TO_UTF8(*glAnywhereWebSocketUrl);
        }

        FString glAnywhereFleetId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereFleetId="), glAnywhereFleetId))
        {
            ServerParametersForAnywhere.m_fleetId = TCHAR_TO_UTF8(*glAnywhereFleetId);
        }

        FString glAnywhereProcessId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereProcessId="), glAnywhereProcessId))
        {
            ServerParametersForAnywhere.m_processId = TCHAR_TO_UTF8(*glAnywhereProcessId);
        }
        else
        {
            // If no ProcessId is passed as a command line argument, generate a randomized unique string.
            FString TimeString = FString::FromInt(std::time(nullptr));
            FString ProcessId = "ProcessId_" + TimeString;
            ServerParametersForAnywhere.m_processId = TCHAR_TO_UTF8(*ProcessId);
        }

        FString glAnywhereHostId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereHostId="), glAnywhereHostId))
        {
            ServerParametersForAnywhere.m_hostId = TCHAR_TO_UTF8(*glAnywhereHostId);
        }

        FString glAnywhereAuthToken = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAuthToken="), glAnywhereAuthToken))
        {
            ServerParametersForAnywhere.m_authToken = TCHAR_TO_UTF8(*glAnywhereAuthToken);
        }

        /*FString glAnywhereAwsRegion = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAwsRegion="), glAnywhereAwsRegion))
        {
            ServerParametersForAnywhere.m_awsRegion = TCHAR_TO_UTF8(*glAnywhereAwsRegion);
        }

        FString glAnywhereAccessKey = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAccessKey="), glAnywhereAccessKey))
        {
            ServerParametersForAnywhere.m_accessKey = TCHAR_TO_UTF8(*glAnywhereAccessKey);
        }

        FString glAnywhereSecretKey = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereSecretKey="), glAnywhereSecretKey))
        {
            ServerParametersForAnywhere.m_secretKey = TCHAR_TO_UTF8(*glAnywhereSecretKey);
        }

        FString glAnywhereSessionToken = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereSessionToken="), glAnywhereSessionToken))
        {
            ServerParametersForAnywhere.m_sessionToken = TCHAR_TO_UTF8(*glAnywhereSessionToken);
        }*/

        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_YELLOW);
        UE_LOG(GameServerLog, Log, TEXT(">>>> WebSocket URL: %s"), *ServerParametersForAnywhere.m_webSocketUrl);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Fleet ID: %s"), *ServerParametersForAnywhere.m_fleetId);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Process ID: %s"), *ServerParametersForAnywhere.m_processId);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Host ID (Compute Name): %s"), *ServerParametersForAnywhere.m_hostId);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Auth Token: %s"), *ServerParametersForAnywhere.m_authToken);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Aws Region: %s"), *ServerParametersForAnywhere.m_awsRegion);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Access Key: %s"), *ServerParametersForAnywhere.m_accessKey);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Secret Key: %s"), *ServerParametersForAnywhere.m_secretKey);
        UE_LOG(GameServerLog, Log, TEXT(">>>> Session Token: %s"), *ServerParametersForAnywhere.m_sessionToken);
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }

    UE_LOG(GameServerLog, Log, TEXT("Initializing the GameLift Server..."));

    //InitSDK will establish a local connection with GameLift's agent to enable further communication.
    FGameLiftGenericOutcome InitSdkOutcome = GameLiftSdkModule->InitSDK(ServerParametersForAnywhere);
    if (InitSdkOutcome.IsSuccess())
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_GREEN);
        UE_LOG(GameServerLog, Log, TEXT("GameLift InitSDK succeeded!"));
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }
    else
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_RED);
        UE_LOG(GameServerLog, Log, TEXT("ERROR: InitSDK failed : ("));
        FGameLiftError GameLiftError = InitSdkOutcome.GetError();
        UE_LOG(GameServerLog, Log, TEXT("ERROR: %s"), *GameLiftError.m_errorMessage);
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
        return;
    }

    ProcessParameters = MakeShared<FProcessParameters>();

    //When a game session is created, Amazon GameLift Servers sends an activation request to the game server and passes along the game session object containing game properties and other settings.
    //Here is where a game server should take action based on the game session object.
    //Once the game server is ready to receive incoming player connections, it should invoke GameLiftServerAPI.ActivateGameSession()
    ProcessParameters->OnStartGameSession.BindLambda([=](Aws::GameLift::Server::Model::GameSession InGameSession)
        {
            FString GameSessionId = FString(InGameSession.GetGameSessionId());
            UE_LOG(GameServerLog, Log, TEXT("GameSession Initializing: %s"), *GameSessionId);
            GameLiftSdkModule->ActivateGameSession();
        });

    //OnProcessTerminate callback. Amazon GameLift Servers will invoke this callback before shutting down an instance hosting this game server.
    //It gives this game server a chance to save its state, communicate with services, etc., before being shut down.
    //In this case, we simply tell Amazon GameLift Servers we are indeed going to shutdown.
    ProcessParameters->OnTerminate.BindLambda([=]()
        {
            UE_LOG(GameServerLog, Log, TEXT("Game Server Process is terminating"));
            // First call ProcessEnding()
            FGameLiftGenericOutcome processEndingOutcome = GameLiftSdkModule->ProcessEnding();
            // Then call Destroy() to free the SDK from memory
            FGameLiftGenericOutcome destroyOutcome = GameLiftSdkModule->Destroy();
            // Exit the process with success or failure
            if (processEndingOutcome.IsSuccess() && destroyOutcome.IsSuccess()) {
                UE_LOG(GameServerLog, Log, TEXT("Server process ending successfully"));
            }
            else {
                if (!processEndingOutcome.IsSuccess()) {
                    const FGameLiftError& error = processEndingOutcome.GetError();
                    UE_LOG(GameServerLog, Error, TEXT("ProcessEnding() failed. Error: %s"),
                        error.m_errorMessage.IsEmpty() ? TEXT("Unknown error") : *error.m_errorMessage);
                }
                if (!destroyOutcome.IsSuccess()) {
                    const FGameLiftError& error = destroyOutcome.GetError();
                    UE_LOG(GameServerLog, Error, TEXT("Destroy() failed. Error: %s"),
                        error.m_errorMessage.IsEmpty() ? TEXT("Unknown error") : *error.m_errorMessage);
                }
            }
        });

    //This is the HealthCheck callback.
    //Amazon GameLift Servers will invoke this callback every 60 seconds or so.
    //Here, a game server might want to check the health of dependencies and such.
    //Simply return true if healthy, false otherwise.
    //The game server has 60 seconds to respond with its health status. Amazon GameLift Servers will default to 'false' if the game server doesn't respond in time.
    //In this case, we're always healthy!
    ProcessParameters->OnHealthCheck.BindLambda([]()
        {
            UE_LOG(GameServerLog, Log, TEXT("Performing Health Check"));
            return true;
        });

    //GameServer.exe -port=7777 LOG=server.mylog
    ProcessParameters->port = FURL::UrlConfig.DefaultPort;
    TArray<FString> CommandLineTokens;
    TArray<FString> CommandLineSwitches;

    FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);

    for (FString SwitchStr : CommandLineSwitches)
    {
        FString Key;
        FString Value;

        if (SwitchStr.Split("=", &Key, &Value))
        {
            if (Key.Equals("port"))
            {
                ProcessParameters->port = FCString::Atoi(*Value);
            }
        }
    }

    //Here, the game server tells Amazon GameLift Servers where to find game session log files.
    //At the end of a game session, Amazon GameLift Servers uploads everything in the specified 
    //location and stores it in the cloud for access later.
    TArray<FString> Logfiles;
    Logfiles.Add(TEXT("GameLiftUnrealApp/Saved/Logs/server.log"));
    ProcessParameters->logParameters = Logfiles;

    //The game server calls ProcessReady() to tell Amazon GameLift Servers it's ready to host game sessions.
    UE_LOG(GameServerLog, Log, TEXT("Calling Process Ready..."));
    FGameLiftGenericOutcome ProcessReadyOutcome = GameLiftSdkModule->ProcessReady(*ProcessParameters);

    if (ProcessReadyOutcome.IsSuccess())
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_GREEN);
        UE_LOG(GameServerLog, Log, TEXT("Process Ready!"));
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }
    else
    {
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_RED);
        UE_LOG(GameServerLog, Log, TEXT("ERROR: Process Ready Failed!"));
        FGameLiftError ProcessReadyError = ProcessReadyOutcome.GetError();
        UE_LOG(GameServerLog, Log, TEXT("ERROR: %s"), *ProcessReadyError.m_errorMessage);
        UE_LOG(GameServerLog, SetColor, TEXT("%s"), COLOR_NONE);
    }

    UE_LOG(GameServerLog, Log, TEXT("InitGameLift completed!"));
#endif
}


AMyGameMode::AMyGameMode()
{
}

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();

#if WITH_GAMELIFT
    InitGameLift();
#endif

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("MyGameMode::BeginPlay called"));
    UE_LOG(LogTemp, Log, TEXT("Dedicated server ready, waiting for players"));

    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS) return;

    GS->RoundNumber = 0;
    GS->bRoundInProgress = false;
    GS->bRoundEnding = false;
    GS->bMatchOver = false;

}

void AMyGameMode::PreLogin(
    const FString& Options,
    const FString& Address,
    const FUniqueNetIdRepl& UniqueId,
    FString& ErrorMessage
)
{
    Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

    //check if match is full
    const int32 CurrentPlayers = GetNumPlayers();
    if (CurrentPlayers >= RequiredPlayers)
    {
        ErrorMessage = TEXT("MATCH_FULL");
        UE_LOG(LogTemp, Warning, TEXT("Match full: %d/%d"), CurrentPlayers, RequiredPlayers);
        return;
    }

    //parse player session id
    FString PlayerSessionId =
        UGameplayStatics::ParseOption(Options, TEXT("PlayerSessionId"));

    if (PlayerSessionId.IsEmpty())
    {
        ErrorMessage = TEXT("Missing PlayerSessionId");
        UE_LOG(LogTemp, Error, TEXT("No PlayerSessionId provided"));
        return;
    }

    UE_LOG(LogTemp, Log, TEXT("PreLogin: Validating PlayerSessionId: %s"), *PlayerSessionId);

    //store temporarily for Postlogin
    PendingPlayerSessions.Add(UniqueId, PlayerSessionId);

    //check with gamelift
#if WITH_GAMELIFT
    auto Outcome = Aws::GameLift::Server::AcceptPlayerSession(
        TCHAR_TO_UTF8(*PlayerSessionId)
    );

    if (!Outcome.IsSuccess())
    {
        // Remove from pending if validation fails
        PendingPlayerSessions.Remove(UniqueId);

        FString FailureReason = FString(Outcome.GetError().GetErrorMessage());
        UE_LOG(LogTemp, Error, TEXT("AcceptPlayerSession failed: %s"), *FailureReason);
        ErrorMessage = TEXT("Invalid PlayerSessionId");
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("GameLift accepted player session"));
#else
    UE_LOG(LogTemp, Warning, TEXT("GameLift SDK not enabled - skipping validation"));
#endif
}

void AMyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);

    /*if (ConnectedPlayers >= RequiredPlayers)
    {
        NewPlayer->ClientMessage(TEXT("Match full"));
        NewPlayer->ClientTravel(TEXT("/Game/Maps/MainMenu"), TRAVEL_Absolute);
        return;
    }*/

    if (!HasAuthority() || !NewPlayer)
        return;

    // retrieve player session Id from temporary storage
    FString PlayerSessionId;
    if (NewPlayer->PlayerState)
    {
        FUniqueNetIdRepl UniqueId = NewPlayer->PlayerState->GetUniqueId();

        if (PendingPlayerSessions.Contains(UniqueId))
        {
            PlayerSessionId = PendingPlayerSessions[UniqueId];
            PendingPlayerSessions.Remove(UniqueId); // Clean up temporary storage

            //Store in permanent map for logout cleanup
            PlayerSessionMap.Add(NewPlayer, PlayerSessionId);
            UE_LOG(LogTemp, Log, TEXT("Stored PlayerSessionId: %s"), *PlayerSessionId);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No PlayerSessionId found in pending sessions"));
        }
    }

    //ConnectedPlayers++;
    const int32 PlayersNum = GetNumPlayers();

    UE_LOG(LogTemp, Log, TEXT("Player joined. Count = %d"), PlayersNum);

    // Spawn pawn once
    if (!NewPlayer->GetPawn())
    {
        RestartPlayer(NewPlayer);
    }

    // Force idle state
    NewPlayer->SetIgnoreMoveInput(true);
    NewPlayer->SetIgnoreLookInput(true);

    if (AGulagCharacter* Char = Cast<AGulagCharacter>(NewPlayer->GetPawn()))
    {
        Char->FreezeForRound();
        Char->Client_ShowCrosshair(false);
    }

    // Reset playerstate
    if (AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>())
    {
        PS->PlayerScore = 0;
    }

    // start match when all players are in 
    if (PlayersNum == RequiredPlayers && !bCountdownStarted)
    {
        bCountdownStarted = true;

        AMyGameState* GS = GetGameState<AMyGameState>();
        if (GS)
        {
            GS->CountdownStartTime = GetWorld()->GetTimeSeconds();
        }

        /*for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) 
        {

            if (AMyPlayerController* PC = Cast<AMyPlayerController>(It->Get()))
            {
				PC->Client_IdleToStartTransition();
                
                if (PC->MatchInfoWidget) {
					//PC->MatchInfoWidget->PlayStartScreenAnimation();
                    PC->MatchInfoWidget->ShowCountdownText(true);
                }
            }
            
        }*/
        UE_LOG(LogTemp, Warning, TEXT("All players connected - starting countdown!"));
        StartPreRoundCountdown();
    }
}

void AMyGameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

#if WITH_GAMELIFT
    // Remove player session from GameLift
    APlayerController* PC = Cast<APlayerController>(Exiting);
    if (PC && PlayerSessionMap.Contains(PC))
    {
        FString PlayerSessionId = PlayerSessionMap[PC];

        auto Outcome = Aws::GameLift::Server::RemovePlayerSession(
            TCHAR_TO_UTF8(*PlayerSessionId)
        );

        if (Outcome.IsSuccess())
        {
            UE_LOG(LogTemp, Log, TEXT("Removed player session from GameLift: %s"), *PlayerSessionId);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to remove player session from GameLift"));
        }

        PlayerSessionMap.Remove(PC);
    }
#endif

    if (GetNumPlayers() < RequiredPlayers)
    {
        AMyGameState* GS = GetGameState<AMyGameState>();
        if (GS && !GS->bMatchOver)
        {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Not enough players - ending match"));
            UE_LOG(LogTemp, Warning, TEXT("Not enough players - ending match"));
            GS->bMatchOver = true;
        }

        for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It) {

            AMyPlayerController* RemainingPC = Cast<AMyPlayerController>(It->Get());
            if (RemainingPC && RemainingPC!=Exiting) {
                RemainingPC->Client_ReturnToMainMenuLevel();
            }

        }

    }
    //ConnectedPlayers = FMath::Max(0, ConnectedPlayers - 1);

    //UE_LOG(LogTemp, Warning, TEXT("Player left. Count = %d"), ConnectedPlayers);
}


void AMyGameMode::PlayerDied(AController* KillerController, AController* VictimController)
{
    if (!HasAuthority())
        return;

	AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS || GS->bMatchOver) return;

    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("PlayerDied called"));
    //checks
    if (!KillerController || !VictimController) return;
    if (KillerController == VictimController) return; // suicide case

    if (GS->bRoundEnding)
        return;

    //get playerstates
    AMyPlayerState* KillerPS = KillerController->GetPlayerState<AMyPlayerState>();
    AMyPlayerState* VictimPS = VictimController->GetPlayerState<AMyPlayerState>();

    if (!KillerPS || !VictimPS) return;

    //give killer a point
    KillerPS->AddScore(1);
    
// check if match is over
    CheckWinCondition();
}

void AMyGameMode::CheckWinCondition()
{
    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS || GS->bMatchOver) return;

    if (GS->bRoundEnding)
        return;

    GS->bRoundEnding = true; //lock immediately

    for (APlayerState* PS : GameState->PlayerArray)
    {
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
        if (MyPS && MyPS->PlayerScore >= 2)         // best of 3
        {
            //EndRound();
            
            GS->bMatchOver = true;
            EndMatch(MyPS);    
            return;
        }
    }

    
    //GS->bRoundEnding = true;
    EndRound();
}


void AMyGameMode::StartPreRoundCountdown()
{
    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS) return;

    if (count > 0) {
        GS->RoundNumber++;
    }
    

	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

    GS->CountdownTime = 10; // replicated int

    GetWorld()->GetTimerManager().SetTimer(
        CountdownTimerHandle,
        this,
        &AMyGameMode::TickCountdown,
        1.0f,
        true
    );
}

void AMyGameMode::TickCountdown()
{
    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS) return;

   

    GS->CountdownTime--;
	GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Yellow, FString::Printf(TEXT("Round starts in: %d"), GS->CountdownTime));

    if (GS->CountdownTime <= 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
        StartRound();
    }
}

void AMyGameMode::StartRoundCountdown()
{
	AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS)return;

    GS->RoundCountdownTime = 60;

	GetWorld()->GetTimerManager().ClearTimer(RoundCountdownTimerHandle);

    GetWorld()->GetTimerManager().SetTimer(
        RoundCountdownTimerHandle,
        this,
        &AMyGameMode::RoundTickCountdown,
		1.0f,
        true //looping every 1sec
	);

}

void AMyGameMode::RoundTickCountdown()
{
    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS) return;

    GS->RoundCountdownTime--;

    if (GS->RoundCountdownTime <= 0) {

       GetWorld()->GetTimerManager().ClearTimer(RoundCountdownTimerHandle);
	   //EndRound(); 
       CheckClosestPlayer();
    }

}

void AMyGameMode::CheckClosestPlayer()
{
    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS || GS->bRoundEnding) return;

    //GS->bRoundEnding = true;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("CheckClosestPlayer called"));
    TArray<AGulagCharacter*> AlivePlayers;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        AGulagCharacter* Char = Cast<AGulagCharacter>(PC->GetPawn());
        if (Char && !Char->isDead)   
        {
            AlivePlayers.Add(Char);
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Adding players"));
        }
        else {
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Player not found in level"));
        }
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("Alive players count: %d"), AlivePlayers.Num()));
    }

    // if both players alive - check distnace check
    if (AlivePlayers.Num() == 2)
    {
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, TEXT("Both players alive - distance check"));
        AGulagCharacter* P1 = AlivePlayers[0];
        AGulagCharacter* P2 = AlivePlayers[1];

		FVector MapCenter(0.f, 0.f, 0.f);

        const float Dist1 = FVector::Dist(
            P1->GetActorLocation(),MapCenter
            //MapCenterPoint->GetActorLocation()
        );

        const float Dist2 = FVector::Dist(
            P2->GetActorLocation(),MapCenter
            //MapCenterPoint->GetActorLocation()
        );

        AGulagCharacter* WinnerChar = nullptr;
        if (Dist1 <= Dist2) {
			WinnerChar = P1;
        }
        else {
			WinnerChar = P2;
        }

        AController* WinnerController = WinnerChar->GetController();
        if (WinnerController)
        {
            AMyPlayerState* WinnerPS =
                WinnerController->GetPlayerState<AMyPlayerState>();

            if (WinnerPS)
            {
                WinnerPS->AddScore(1);   //add score to nearest player
            }
        }
    }

    //Only one alive - edge case
    else if (AlivePlayers.Num() == 1)
    {
        AController* WinnerController = AlivePlayers[0]->GetController();
        if (WinnerController)
        {
            AMyPlayerState* WinnerPS =
                WinnerController->GetPlayerState<AMyPlayerState>();

            if (WinnerPS)
            {
                WinnerPS->AddScore(1);
            }
        }
    }

	CheckWinCondition();

}

/*void AMyGameMode::StartRound()
{
    if (!HasAuthority()) return;

    // Reset round status on GameState
    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS || GS->bMatchOver) return;

    GS->bRoundInProgress = true;
    GS->RoundNumber++;
    GS->bRoundEnding = false;

    // Get both player controllers
    TArray<AController*> PlayerControllers;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (PC)
        {
            PlayerControllers.Add(PC);
        }
    }

    // Safety check: must have at least 2 players
    if (PlayerControllers.Num() < 2)
    {
        UE_LOG(LogTemp, Error, TEXT("Not enough players to start round!"));
        return;
    }

    // Respawn both players
    for (AController* Controller : PlayerControllers)
    {
        if (!Controller) continue;

        // This destroys old pawn + spawns a new one at PlayerStart
        RequestRespawn(Controller);

        // Now reset the character state
        if (AGulagCharacter* Char = Cast<AGulagCharacter>(Controller->GetPawn()))
        {
            Char->ResetCharacterForNewRound();
        }

        // Enable input again at start of round
        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            PC->SetIgnoreMoveInput(false);
            PC->SetIgnoreLookInput(false);
        }
    }
}*/

void AMyGameMode::StartRound()
{
    if (!HasAuthority()) return;

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("StartRound() called"));

    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS || GS->bMatchOver) return;

    GS->bRoundInProgress = true;
    if (count == 0) {
        GS->RoundNumber++;
    }
    //
    GS->bRoundEnding = false;

    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);

        if (AGulagCharacter* Char = Cast<AGulagCharacter>(PC->GetPawn())) {
            Char->UnfreezeForRound();
			Char->Client_ShowCrosshair(true);
        }
    }

    count++;
    StartRoundCountdown();
}



void AMyGameMode::EndRound()
{
    AMyGameState * GS = GetGameState<AMyGameState>();
    if (!GS || GS->bMatchOver) return;

    GS->bRoundInProgress = false;
    GS->bRoundEnding = true;
    //GS->RoundNumber++;

    GetWorld()->GetTimerManager().ClearTimer(RoundCountdownTimerHandle);
    
    //StartPreRoundCountdown();
    GetWorld()->GetTimerManager().SetTimer(
		StartNextRoundTimerHandle, 
        this, 
        &AMyGameMode::RespawnNextRound,
        3.0f, false);
}

void AMyGameMode::RespawnNextRound()
{
    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS)return;

    GS->RoundCountdownTime = 60;
    // Respawn everyone immediately 
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        AController* Controller = It->Get();
        if (!Controller) continue;

        RequestRespawn(Controller);

        if (APlayerController* PC = Cast<APlayerController>(Controller))
        {
            PC->SetIgnoreMoveInput(true);
            PC->SetIgnoreLookInput(true);

            if (AGulagCharacter* Char = Cast<AGulagCharacter>(PC->GetPawn())) {
                Char->FreezeForRound();
            }

        }
    }

    StartPreRoundCountdown();
}

void AMyGameMode::RequestRespawn(AController* Controller)
{
    if (!HasAuthority() || !Controller) return;

    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS || GS->bMatchOver) return;
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Cyan, TEXT("RequestRespawn called"));

    APawn* Pawn = Controller->GetPawn();
    if (Pawn)
    {
        Controller->UnPossess();
        Pawn->Destroy();
    }

    //Only respawn if controller has no pawn
    if (Controller->GetPawn() == nullptr)
    {
        RestartPlayer(Controller);
    }
   
}



void AMyGameMode::EndMatch(AMyPlayerState* WinnerPlayerState)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, TEXT("=== MATCH OVER ==="));
    if (!HasAuthority() || !WinnerPlayerState) return;

    AMyGameState* GS = GetGameState<AMyGameState>();
    if (!GS) return;

    // Mark match over 
    GS->bMatchOver = true;
    GS->WinnerPlayer = WinnerPlayerState;

    //Disable gameplay for all players
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        APlayerController* PC = It->Get();
        if (!PC) continue;

        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);

        //Tell each player to show correct ui
        AMyPlayerState* PS = PC->GetPlayerState<AMyPlayerState>();

        const bool bIsWinner = (PS == WinnerPlayerState);

        //rpc to client via your subclass
        if (AMyPlayerController* MPC = Cast<AMyPlayerController>(PC))
        {
            MPC->Client_HandleMatchEnd(bIsWinner);
        }
        else
        {
            PC->ClientMessage(bIsWinner ? TEXT("YOU WON") : TEXT("YOU LOST"));
        }
    }
    GetWorld()->GetTimerManager().ClearTimer(RoundCountdownTimerHandle);
    
}
