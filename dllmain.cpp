#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cmath>
#include "dllmain.h"

#include "detours.h"

using namespace std;

char file_path[256];
char file_name[256];
char section[32];

char base_install[256];
char path[256];

int file_index = 1;

bool consoleEnabled;
bool loggingEnabled;

void WriteDebugLog(const char *text){
	FILE *file = fopen(".\\ModuleLoader.log", "a");
	if (file != NULL){
		fputs(text, file);
		fputs("\n", file);
		fclose(file);
	}
}

void ReadConfigs(){
	char buffer[32];

	GetPrivateProfileStringA("common", "DisplayConsole", "off", buffer, 32, ".\\ModuleLoader.ini");
	if (!strcmp(buffer, "on")){
		consoleEnabled = true;
	}

	GetPrivateProfileStringA("common", "WriteLog", "off", buffer, 32, ".\\ModuleLoader.ini");
	if (!strcmp(buffer, "on")){
		loggingEnabled = true;
		remove(".\\ModuleLoader.log");
	}

	GetPrivateProfileStringA("INSTALL", "base", ".", base_install, 256, ".\\truck.ini");
}

//05DC1B0
char* GetFullPath(char *subPath) {
	memset(path, 0, 256);

	if (strcmp(base_install, ".\\")){
		strcpy(path, base_install);
	}

	strcat(path, subPath);
	return path;
}

DWORD* LoadAndInitGameModule(char *modulePath, char *b3dName, char *resName){
	DWORD* TheGame_addr = (DWORD*)(*(DWORD*)0x696CC0);
	typedef DWORD* (__thiscall * LoadAndInitGameModule)(DWORD* _this, char *modulePath, char *b3dName, char *resName);
	return LoadAndInitGameModule(0x5DAE10)(TheGame_addr, modulePath, b3dName, resName);
}

DWORD *__cdecl LoadGameModule(const char *path, const char *name){
    char modulePath[255];

	char b3dName[128];
	char resName[128];

	DWORD MemState = *(DWORD*)0x6CED90; //видимо, параметр качества текстур
	DWORD* result; //указатель на загруженный игровой модуль

    //strcpy(m_sPath, path);

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


	//modulePath = GetFullPath((char *)path); //иногда возвращает неправильный путь?

	strcpy(modulePath, GetFullPath((char *)path));

	if (loggingEnabled){
		char buffer[255];
		sprintf(buffer, "%d. Loading %s, %s ->\nmodulePath=%s, b3dName=%s, resName=%s\n", file_index, path, name, modulePath, b3dName, resName);
		WriteDebugLog(buffer);
	}

	cout << file_index << ". Loading """ << path << """, """ << name << """" << endl;
	file_index++;

	result = LoadAndInitGameModule(modulePath, b3dName, resName);

	return result;
}

void LoadAdditionalModules(){
	int iter = 1;

    while(true){
        sprintf(section, "FILE_%d", iter);
        GetPrivateProfileStringA(section, "path", "none", file_path, 256, ".\\ModuleLoader.ini");
        GetPrivateProfileStringA(section, "name", "none", file_name, 256, ".\\ModuleLoader.ini");

        iter++;

        if (!strcmp(file_path, "none") || !strcmp(file_name, "none")){
            break;
        }

        LoadGameModule(file_path, file_name);
    }
}

DWORD *__cdecl LoadGameResources(const char *path, const char *name){
	DWORD *result;

	//в стандартной игре сначала загружается модуль common,
	//затем cabines и следом trucks

	if (!strcmp(name, "common") && file_index != 1){ //сброс вывода на экран,
		file_index = 1;                              //если загрузка начата заново
		cout << "\n\n\n";
	}

	result = LoadGameModule(path, name);

	if (!strcmp(name, "trucks")){  //подгрузка модулей из ini
		LoadAdditionalModules();   //сразу после trucks
	}

	return result;
}

void DisplayConsole(){
	AllocConsole();
	freopen("conin$","r", stdin);
	freopen("conout$","w",stdout);
	freopen("conout$","w",stderr);
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	//cout << "Mod debug window started" << endl;
}

void AttachHooks(){
    //cout << "Try to attach hooks" << endl;
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());

    DWORD addr_LoadGameResource = 0x510720;

    DetourAttach(&(LPVOID&)addr_LoadGameResource, &LoadGameResources);
    DetourTransactionCommit();
    //cout << "Hooks attached!" << endl;
	cout << "ModuleLoader v1.02 (16.03.2023) started." << endl;
}
	
BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{

	switch(fdwReason)
	{
		case DLL_PROCESS_ATTACH:
		{
			ReadConfigs();

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
