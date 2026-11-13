#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "FWMSessionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWMOnSessionResult, bool, bSuccessful, FString, Message);

/**
 * Single entry point for cooperative sessions.
 *
 * Covers creating, finding, joining and leaving sessions, always reading the
 * player limit from the project settings. No call is made without checking
 * that the online subsystem is available, and every result is reported
 * through a delegate so the UI can react without querying internal state.
 */
UCLASS()
class FINALWEEKMULTIPLAYER_API UFWMSessionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** Creates a session with the configured player limit. */
	UFUNCTION(BlueprintCallable, Category = "Final Week Multiplayer")
	void HostSession();

	/** Searches for available sessions. */
	UFUNCTION(BlueprintCallable, Category = "Final Week Multiplayer")
	void FindSessions(int32 MaxResults = 20);

	/** Joins the session at the given index of the last search. */
	UFUNCTION(BlueprintCallable, Category = "Final Week Multiplayer")
	bool JoinFoundSession(int32 Index);

	/** Leaves the current session, if any. */
	UFUNCTION(BlueprintCallable, Category = "Final Week Multiplayer")
	void LeaveSession();

	UFUNCTION(BlueprintPure, Category = "Final Week Multiplayer")
	bool IsInSession() const;

	UFUNCTION(BlueprintPure, Category = "Final Week Multiplayer")
	int32 GetCurrentPlayers() const;

	UFUNCTION(BlueprintPure, Category = "Final Week Multiplayer")
	int32 GetMaxPlayers() const;

	UFUNCTION(BlueprintPure, Category = "Final Week Multiplayer")
	int32 GetFoundSessionCount() const;

	UPROPERTY(BlueprintAssignable, Category = "Final Week Multiplayer")
	FWMOnSessionResult OnSessionCreated;

	UPROPERTY(BlueprintAssignable, Category = "Final Week Multiplayer")
	FWMOnSessionResult OnSessionJoined;

	UPROPERTY(BlueprintAssignable, Category = "Final Week Multiplayer")
	FWMOnSessionResult OnSessionLeft;

	UPROPERTY(BlueprintAssignable, Category = "Final Week Multiplayer")
	FWMOnSessionResult OnSessionsFound;

private:
	void CreateSessionNow();

	void HandleCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void HandleFindSessionsComplete(bool bWasSuccessful);
	void HandleJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SearchHandle;
	TArray<FOnlineSessionSearchResult> FoundSessions;

	bool bCreateAfterDestroy = false;

	FDelegateHandle CreateDelegateHandle;
	FDelegateHandle DestroyDelegateHandle;
	FDelegateHandle FindDelegateHandle;
	FDelegateHandle JoinDelegateHandle;
};
