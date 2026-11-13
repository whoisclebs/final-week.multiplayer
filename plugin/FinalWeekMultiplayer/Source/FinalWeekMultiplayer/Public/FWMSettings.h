#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "FWMSettings.generated.h"

/** Player range accepted by the plugin. */
namespace FWMLimits
{
	constexpr int32 MinPlayers = 2;
	constexpr int32 MaxPlayers = 64;
	constexpr int32 DefaultPlayers = 8;
}

/**
 * Multiplayer settings, editable in Project Settings > Game > Final Week Multiplayer.
 *
 * MaxPlayers is clamped when edited and when used, so an invalid value
 * never reaches the online subsystem.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Final Week Multiplayer"))
class FINALWEEKMULTIPLAYER_API UFWMSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UFWMSettings();

	/** Maximum number of players per session. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session",
		meta = (ClampMin = "2", ClampMax = "64", UIMin = "2", UIMax = "32"))
	int32 MaxPlayers;

	/** Session name shown in the server list. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
	FString SessionName;

	/** Hosts on the local network instead of the online provider. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
	bool bUseLANMatch;

	/** Advertises the session so other players can join from the list. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
	bool bAdvertiseSession;

	/** Map loaded when the session is created. Empty keeps the current map. */
	UPROPERTY(Config, EditAnywhere, BlueprintReadOnly, Category = "Session")
	FString MapName;

	/** Returns MaxPlayers clamped to the valid range. */
	UFUNCTION(BlueprintPure, Category = "Final Week Multiplayer")
	int32 GetEffectiveMaxPlayers() const;

	/** Sets MaxPlayers, clamped to the valid range. */
	UFUNCTION(BlueprintCallable, Category = "Final Week Multiplayer")
	void SetMaxPlayers(int32 InMaxPlayers);

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
