#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include "dllmain.h"

#include "detours.h"

using namespace std;

char* GetFullPath(char *subPath){
	DWORD* TheGame_addr = (DWORD*)(*(DWORD*)0x696CC0);
	typedef char* (__thiscall * GetFullPath)(DWORD* _this, char *subPath);
	return GetFullPath(0x5DC1B0)(TheGame_addr, subPath);
}

DWORD* LoadAndInitGameModule(char *modulePath, char *b3dName, char *resName){
	DWORD* TheGame_addr = (DWORD*)(*(DWORD*)0x696CC0);
	typedef DWORD* (__thiscall * GetFullPath)(DWORD* _this, char *modulePath, char *b3dName, char *resName);
	return GetFullPath(0x5DAE10)(TheGame_addr, modulePath, b3dName, resName);
}

DWORD *__cdecl LoadGameResource(const char *path, const char *name){
    char *modulePath = new char[];

	char *b3dName = new char[];
	char *resName = new char[];
    char *m_sPath = new char[];

	DWORD MemState = *(DWORD*)0x6CED90; //видимо, параметр качества текстур
	DWORD* result; //указатель на загруженный игровой модуль

    strcpy(m_sPath, path);

    strcpy(b3dName, name);
    strcat(b3dName, ".b3d");

	strcpy(resName, name);

	//это определение названия архива с текстурами
	//*.res - самое высокое качество, *2.res - самое низкое (*3.res для trucks)
	switch (MemState){
	case 0:
		//если trucks, то *3.res
		if(!strcmp(resName, "trucks")){
			strcat(resName, "3.res");
		} else {
			strcat(resName, "2.res");
		}
		break;
	case 1:
		//если trucks, то *2.res
		if(!strcmp(resName, "trucks")){
			strcat(resName, "2.res");
		} else {
			strcat(resName, "1.res");
		}
		break;
	case 2:
		strcat(resName, "1.res");
		break;
	case 3:
		//если trucks, то *1.res
		if(!strcmp(resName, "trucks")){
			strcat(resName, "1.res");
		} else {
			strcat(resName, ".res");
		}
		break;
	case 4:
		strcat(resName, ".res");
		break;
	default:
		strcat(resName, ".res");
		break;
	}


	cout << "LoadGameRes(" << path << ", " << name << ")" << ": b3dName=" << b3dName << ", resName=" << resName << endl;

	modulePath = GetFullPath(m_sPath); //иногда возвращает неправильный путь?
	result = LoadAndInitGameModule(modulePath, b3dName, resName);

	return result;
}

void LoadAdditionalResources(){
    char file_path[256];
    char file_name[256];
    char section[32];

    int iter = 1;

    while(true){
        sprintf(section, "FILE_%d", iter);
        GetPrivateProfileStringA(section, "path", "none", file_path, 256, ".\\ModuleLoader.ini");
        GetPrivateProfileStringA(section, "name", "none", file_name, 256, ".\\ModuleLoader.ini");

        iter++;

        if (!strcmp(file_path, "none") || !strcmp(file_name, "none")){
            break;
        }

        LoadGameResource(file_path, file_name);
    }
}

DWORD *__cdecl LoadGameResources(const char *path, const char *name){
	//в стандартной игре сначала загружается модуль common,
	//затем cabines и следом trucks

	if (!strcmp(name, "cabines")){ //подгрузка модулей из ini
		LoadAdditionalResources(); //сразу перед cabines
	}

	return LoadGameResource(path, name);
}

void DisplayConsole(){
	AllocConsole();
	freopen("conin$","r", stdin);
	freopen("conout$","w",stdout);
	freopen("conout$","w",stderr);
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	cout << "Mod debug window started" << endl;
}

void AttachHooks(){
    cout << "Try to attach hooks" << endl;
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DWORD addr_LoadGameResource = 0x510720;

    DetourAttach(&(LPVOID&)addr_LoadGameResource, &LoadGameResources);
    DetourTransactionCommit();
    cout << "Hooks attached!" << endl;
}

bool consoleEnabled;

void ReadConfig(){
	char buffer[32];

	GetPrivateProfileStringA("common", "DisplayConsole", "off", buffer, 32, ".\\ModuleLoader.ini");

	if (!strcmp(buffer, "on")){
		consoleEnabled = true;
	}
}
	
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{

	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			ReadConfig();

			if (consoleEnabled){
				DisplayConsole();
			}

			AttachHooks();

			break;
		}
		case DLL_PROCESS_DETACH:
		{
			break;
		}
		case DLL_THREAD_ATTACH:
		{
			break;
		}
		case DLL_THREAD_DETACH:
		{
			break;
		}
	}

	/* Return TRUE on success, FALSE on failure */
	return TRUE;
}
