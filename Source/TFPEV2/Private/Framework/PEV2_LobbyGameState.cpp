// Copyright PEV 2 - Gun Master Arena. All Rights Reserved.

#include "Framework/PEV2_LobbyGameState.h"
#include "Net/UnrealNetwork.h"

APEV2_LobbyGameState::APEV2_LobbyGameState()
{
	ConnectedPlayerCount = 1;
	RoomCode = TEXT("");
}

void APEV2_LobbyGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APEV2_LobbyGameState, ConnectedPlayerCount);
	DOREPLIFETIME(APEV2_LobbyGameState, RoomCode);
}

void APEV2_LobbyGameState::SetConnectedPlayerCount(int32 NewCount)
{
	if (HasAuthority())
	{
		ConnectedPlayerCount = NewCount;
		OnLobbyPlayerCountChanged.Broadcast(ConnectedPlayerCount);
	}
}

void APEV2_LobbyGameState::OnRep_ConnectedPlayerCount()
{
	OnLobbyPlayerCountChanged.Broadcast(ConnectedPlayerCount);
}

void APEV2_LobbyGameState::SetRoomCode(const FString& NewCode)
{
	if (HasAuthority())
	{
		RoomCode = NewCode;
		OnLobbyRoomCodeChanged.Broadcast(RoomCode);
	}
}

void APEV2_LobbyGameState::OnRep_RoomCode()
{
	OnLobbyRoomCodeChanged.Broadcast(RoomCode);
}
