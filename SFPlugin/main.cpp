#include <windows.h>
#include <string>
#include <assert.h>
#include <process.h>

#include "SAMPFUNCS_API.h"
#include "game_api\game_api.h"

SAMPFUNCS *SF = new SAMPFUNCS();

char routePath[MAX_PATH];
FILE *routeFile = NULL;
bool recordEnabled = false;

bool CALLBACK outcomingData(stRakNetHookParams *params)
{
	if (recordEnabled) {
		if (params->packetId == PacketEnumeration::ID_PLAYER_SYNC) {
			stOnFootData data;
			memset(&data, 0, sizeof(stOnFootData));
			byte packet;

			params->bitStream->ResetReadPointer();
			params->bitStream->Read(packet);
			params->bitStream->Read((PCHAR)&data, sizeof(stOnFootData));
			params->bitStream->ResetReadPointer();

			if (routeFile != NULL) {
				fwrite((char *)&data, sizeof(stOnFootData), 1, routeFile);
				fflush(routeFile);
			}
		}
	}
	return true;
};

void CALLBACK startRecord(std::string param) {
	if (routeFile == NULL && recordEnabled == false) {
		routeFile = fopen(routePath, "wb");
		recordEnabled = true;

		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 0), "Запись маршрута {FF0000}включена");
	} else {
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 0, 0), "Запись уже запущена!");
	}
}

void CALLBACK stopRecord(std::string param) {
	if (routeFile != NULL && recordEnabled == true) {
		fclose(routeFile);
		routeFile = NULL;
		recordEnabled = false;

		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 0), "Запись маршрута {FF0000}отключена");
	} else {
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 0, 0), "Сначала начните запись!");
	}
}

void CALLBACK mainloop() {
	static bool init = false;
	if (!init) {
		if (GAME == nullptr)
			return;
		if (GAME->GetSystemState() != eSystemState::GS_PLAYING_GAME)
			return;
		if (!SF->getSAMP()->IsInitialized())
			return;

		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 0), "Плагин записи маршрута RakBot загружен");
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 0), "Команды:");
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 0), "{FF0000}  /rr {FFFF00}- запись маршрута");
		SF->getSAMP()->getChat()->AddChatMessage(D3DCOLOR_XRGB(255, 255, 0), "{FF0000}  /rs {FFFF00}- остановка записи");

		SF->getSAMP()->registerChatCommand("rr", startRecord);
		SF->getSAMP()->registerChatCommand("rs", stopRecord);
		SF->getRakNet()->registerRakNetCallback(RakNetScriptHookType::RAKHOOK_TYPE_OUTCOMING_PACKET, outcomingData);

		init = true;
	}
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReasonForCall, LPVOID lpReserved) {
	switch (dwReasonForCall) {
		case DLL_PROCESS_ATTACH:
			GetModuleFileName(hModule, routePath, sizeof(routePath));
			*strrchr(routePath, '\\') = 0;
			strcat(routePath, "\\RakRoute.route");

			SF->initPlugin(mainloop, hModule);
			break;

		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
	}
	return TRUE;
}
