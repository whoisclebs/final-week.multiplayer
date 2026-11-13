#include "FWMSettings.h"

UFWMSettings::UFWMSettings()
	: MaxPlayers(FWMLimits::DefaultPlayers)
	, SessionName(TEXT("Final Week"))
	, bUseLANMatch(false)
	, bAdvertiseSession(true)
	, MapName()
{
}

int32 UFWMSettings::GetEffectiveMaxPlayers() const
{
	return FMath::Clamp(MaxPlayers, FWMLimits::MinPlayers, FWMLimits::MaxPlayers);
}

void UFWMSettings::SetMaxPlayers(int32 InMaxPlayers)
{
	MaxPlayers = FMath::Clamp(InMaxPlayers, FWMLimits::MinPlayers, FWMLimits::MaxPlayers);
}

#if WITH_EDITOR
void UFWMSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	MaxPlayers = GetEffectiveMaxPlayers();
}
#endif
