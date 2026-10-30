#pragma once

enum NetMessages
{
	NET_NOP = 0,
	NET_Disconnect = 1,
	NET_File = 2,
	NET_Tick = 3,
	NET_StringCmd = 4,
	NET_SetConVar = 5,
	NET_SignOnState = 6
};

enum SvcMessages
{
	SVC_Print = 7,
	SVC_ServerInfo = 8,
	SVC_SendTable = 9,
	SVC_ClassInfo = 10,
	SVC_SetPause = 11,
	SVC_CreateStringTable = 12,
	SVC_UpdateStringTable = 13,
	SVC_VoiceInit = 14,
	SVC_VoiceData = 15,

	SVC_Sounds = 17,
	SVC_SetView = 18,
	SVC_FixAngle = 19,
	SVC_CrosshairAngle = 20,
	SVC_BSPDecal = 21,

	SVC_UserMessage = 23,
	SVC_EntityMessage = 24,
	SVC_GameEvent = 25,
	SVC_PacketEntities = 26,
	SVC_TempEntities = 27,
	SVC_Prefetch = 28,
	SVC_Menu = 29,
	SVC_GameEventList = 30,
	SVC_GetCvarValue = 31,
	NumMessageTypes
};

enum class UserMessage
{
	Geiger = 0,
	Train,
	HudText,
	SayText,
	SayText2,
	TextMsg,
	HudMsg,
	ResetHUD,
	GameTitle,
	ItemPickup,
	ShowMenu,
	Shake,
	Fade,
	VGUIMenu,
	CloseCaption,
	SendAudio,
	RawAudio,
	VoiceMask,
	RequestState,
	BarTime,
	Damage,
	RadioText,
	HintText,
	ReloadEffect,
	PlayerAnimEvent,
	AmmoDenied,
	UpdateRadar,
	KillCam,
	SPHapWeapEvent,
	HapDmg,
	HapPunch,
	HapSetDrag,
	HapSetConst,
	HapMeleeContact
};