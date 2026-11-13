#include "FWMSessionSubsystem.h"

#include "FWMSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "OnlineSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogFWMSession, Log, All);

namespace
{
	const FName SessionKeyName(TEXT("FWM_SESSION_NAME"));
	const FName SessionKeyVersion(TEXT("FWM_VERSION"));
	const FName SessionKeyPlayers(TEXT("FWM_MAX_PLAYERS"));

	FString DescribeJoinResult(EOnJoinSessionCompleteResult::Type Result)
	{
		switch (Result)
		{
		case EOnJoinSessionCompleteResult::Success:
			return TEXT("joined the session");
		case EOnJoinSessionCompleteResult::SessionIsFull:
			return TEXT("the session is full");
		case EOnJoinSessionCompleteResult::SessionDoesNotExist:
			return TEXT("the session no longer exists");
		case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
			return TEXT("could not retrieve the session address");
		case EOnJoinSessionCompleteResult::AlreadyInSession:
			return TEXT("already in a session");
		case EOnJoinSessionCompleteResult::UnknownError:
		default:
			return TEXT("unknown error while joining the session");
		}
	}
}

void UFWMSessionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem == nullptr)
	{
		UE_LOG(LogFWMSession, Warning, TEXT("No online subsystem available."));
		return;
	}

	SessionInterface = Subsystem->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogFWMSession, Warning, TEXT("Session interface unavailable."));
		return;
	}

	CreateDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(
		FOnCreateSessionCompleteDelegate::CreateUObject(
			this, &UFWMSessionSubsystem::HandleCreateSessionComplete));

	DestroyDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
		FOnDestroySessionCompleteDelegate::CreateUObject(
			this, &UFWMSessionSubsystem::HandleDestroySessionComplete));

	FindDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(
		FOnFindSessionsCompleteDelegate::CreateUObject(
			this, &UFWMSessionSubsystem::HandleFindSessionsComplete));

	JoinDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(
		FOnJoinSessionCompleteDelegate::CreateUObject(
			this, &UFWMSessionSubsystem::HandleJoinSessionComplete));

	UE_LOG(LogFWMSession, Log, TEXT("Session subsystem initialized (%s)."),
		*Subsystem->GetSubsystemName().ToString());
}

void UFWMSessionSubsystem::Deinitialize()
{
	if (SessionInterface.IsValid())
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateDelegateHandle);
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroyDelegateHandle);
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindDelegateHandle);
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinDelegateHandle);

		if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
		{
			SessionInterface->DestroySession(NAME_GameSession);
		}
	}

	SessionInterface.Reset();
	SearchHandle.Reset();
	FoundSessions.Reset();

	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// Creation
// ---------------------------------------------------------------------------

void UFWMSessionSubsystem::HostSession()
{
	if (!SessionInterface.IsValid())
	{
		OnSessionCreated.Broadcast(false, TEXT("online subsystem unavailable"));
		return;
	}

	// An existing session must be destroyed before creating another one;
	// otherwise the engine rejects the request.
	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		bCreateAfterDestroy = true;
		SessionInterface->DestroySession(NAME_GameSession);
		return;
	}

	CreateSessionNow();
}

void UFWMSessionSubsystem::CreateSessionNow()
{
	const UFWMSettings* Settings = GetDefault<UFWMSettings>();
	check(Settings != nullptr);

	const int32 MaxPlayers = Settings->GetEffectiveMaxPlayers();

	FOnlineSessionSettings SessionSettings;
	SessionSettings.NumPublicConnections = MaxPlayers;
	SessionSettings.NumPrivateConnections = 0;
	SessionSettings.bIsLANMatch = Settings->bUseLANMatch;
	SessionSettings.bShouldAdvertise = Settings->bAdvertiseSession;
	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bAllowJoinViaPresence = !Settings->bUseLANMatch;
	SessionSettings.bUseLobbiesIfAvailable = !Settings->bUseLANMatch;
	SessionSettings.bUsesPresence = !Settings->bUseLANMatch;
	SessionSettings.bAllowInvites = true;

	SessionSettings.Set(SessionKeyName, Settings->SessionName, EOnlineDataAdvertisementType::ViaOnlineService);
	SessionSettings.Set(SessionKeyVersion, FString(TEXT("1")), EOnlineDataAdvertisementType::ViaOnlineService);
	// Also stored as searchable data so the server list shows the real
	// limit without opening the session.
	SessionSettings.Set(SessionKeyPlayers, MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineService);

	if (!Settings->MapName.IsEmpty())
	{
		SessionSettings.Set(SETTING_MAPNAME, Settings->MapName, EOnlineDataAdvertisementType::ViaOnlineService);
	}

	UE_LOG(LogFWMSession, Log, TEXT("Creating session '%s' for %d players."),
		*Settings->SessionName, MaxPlayers);

	if (!SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings))
	{
		OnSessionCreated.Broadcast(false, TEXT("could not start creating the session"));
	}
}

void UFWMSessionSubsystem::HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogFWMSession, Log, TEXT("Create session %s: %s"),
		*SessionName.ToString(), bWasSuccessful ? TEXT("ok") : TEXT("failed"));

	OnSessionCreated.Broadcast(
		bWasSuccessful,
		bWasSuccessful ? TEXT("session created") : TEXT("failed to create the session"));
}

// ---------------------------------------------------------------------------
// Search and join
// ---------------------------------------------------------------------------

void UFWMSessionSubsystem::FindSessions(int32 MaxResults)
{
	if (!SessionInterface.IsValid())
	{
		OnSessionsFound.Broadcast(false, TEXT("online subsystem unavailable"));
		return;
	}

	const UFWMSettings* Settings = GetDefault<UFWMSettings>();
	check(Settings != nullptr);

	SearchHandle = MakeShared<FOnlineSessionSearch>();
	SearchHandle->MaxSearchResults = FMath::Clamp(MaxResults, 1, 200);
	SearchHandle->bIsLanQuery = Settings->bUseLANMatch;

	if (!Settings->bUseLANMatch)
	{
		SearchHandle->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
	}

	UE_LOG(LogFWMSession, Log, TEXT("Searching for up to %d sessions (LAN: %s)."),
		MaxResults, Settings->bUseLANMatch ? TEXT("yes") : TEXT("no"));

	if (!SessionInterface->FindSessions(0, SearchHandle.ToSharedRef()))
	{
		OnSessionsFound.Broadcast(false, TEXT("could not start the search"));
	}
}

void UFWMSessionSubsystem::HandleFindSessionsComplete(bool bWasSuccessful)
{
	FoundSessions.Reset();

	if (bWasSuccessful && SearchHandle.IsValid())
	{
		FoundSessions = SearchHandle->SearchResults;
	}

	const int32 Count = FoundSessions.Num();
	UE_LOG(LogFWMSession, Log, TEXT("Search finished: %d session(s)."), Count);

	OnSessionsFound.Broadcast(
		bWasSuccessful,
		FString::Printf(TEXT("%d session(s) found"), Count));
}

bool UFWMSessionSubsystem::JoinFoundSession(int32 Index)
{
	if (!SessionInterface.IsValid())
	{
		OnSessionJoined.Broadcast(false, TEXT("online subsystem unavailable"));
		return false;
	}

	if (!FoundSessions.IsValidIndex(Index))
	{
		OnSessionJoined.Broadcast(false, TEXT("invalid session index"));
		return false;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		OnSessionJoined.Broadcast(false, TEXT("already in a session"));
		return false;
	}

	if (!SessionInterface->JoinSession(0, NAME_GameSession, FoundSessions[Index]))
	{
		OnSessionJoined.Broadcast(false, TEXT("could not start joining the session"));
		return false;
	}

	return true;
}

void UFWMSessionSubsystem::HandleJoinSessionComplete(
	FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const bool bSuccess = Result == EOnJoinSessionCompleteResult::Success;
	const FString Message = DescribeJoinResult(Result);

	UE_LOG(LogFWMSession, Log, TEXT("Join session %s: %s"),
		*SessionName.ToString(), *Message);

	OnSessionJoined.Broadcast(bSuccess, Message);

	if (!bSuccess || !SessionInterface.IsValid())
	{
		return;
	}

	FString ConnectString;
	if (!SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		OnSessionJoined.Broadcast(false, TEXT("session address unavailable"));
		return;
	}

	UGameInstance* GameInstance = GetGameInstance();
	APlayerController* Controller = GameInstance ? GameInstance->GetFirstLocalPlayerController() : nullptr;
	if (Controller == nullptr)
	{
		OnSessionJoined.Broadcast(false, TEXT("no local player to connect"));
		return;
	}

	Controller->ClientTravel(ConnectString, TRAVEL_Absolute);
}

// ---------------------------------------------------------------------------
// Leave and queries
// ---------------------------------------------------------------------------

void UFWMSessionSubsystem::LeaveSession()
{
	if (!SessionInterface.IsValid())
	{
		OnSessionLeft.Broadcast(false, TEXT("online subsystem unavailable"));
		return;
	}

	if (SessionInterface->GetNamedSession(NAME_GameSession) == nullptr)
	{
		OnSessionLeft.Broadcast(true, TEXT("no active session"));
		return;
	}

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		OnSessionLeft.Broadcast(false, TEXT("could not leave the session"));
	}
}

void UFWMSessionSubsystem::HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogFWMSession, Log, TEXT("Destroy session %s: %s"),
		*SessionName.ToString(), bWasSuccessful ? TEXT("ok") : TEXT("failed"));

	if (bCreateAfterDestroy)
	{
		bCreateAfterDestroy = false;
		if (bWasSuccessful)
		{
			CreateSessionNow();
			return;
		}
		OnSessionCreated.Broadcast(false, TEXT("could not destroy the previous session"));
		return;
	}

	OnSessionLeft.Broadcast(
		bWasSuccessful,
		bWasSuccessful ? TEXT("left the session") : TEXT("failed to leave the session"));
}

bool UFWMSessionSubsystem::IsInSession() const
{
	return SessionInterface.IsValid()
		&& SessionInterface->GetNamedSession(NAME_GameSession) != nullptr;
}

int32 UFWMSessionSubsystem::GetCurrentPlayers() const
{
	if (!SessionInterface.IsValid())
	{
		return 0;
	}

	const FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (Session == nullptr)
	{
		return 0;
	}

	return FMath::Max(0, Session->NumPublicConnections - Session->NumOpenPublicConnections);
}

int32 UFWMSessionSubsystem::GetMaxPlayers() const
{
	const UFWMSettings* Settings = GetDefault<UFWMSettings>();
	return Settings ? Settings->GetEffectiveMaxPlayers() : FWMLimits::DefaultPlayers;
}

int32 UFWMSessionSubsystem::GetFoundSessionCount() const
{
	return FoundSessions.Num();
}
