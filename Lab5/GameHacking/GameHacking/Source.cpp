#include <Windows.h>
#include<TlHelp32.h>
#include <iostream>
#include <tchar.h> // _tcscmp
#include <vector>
 
 
DWORD GetModuleBaseAddress(TCHAR* lpszModuleName, DWORD pID) {
    DWORD dwModuleBaseAddress = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID); // make snapshot of all modules within process
    MODULEENTRY32 ModuleEntry32 = { 0 };
    ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
 
    if (Module32First(hSnapshot, &ModuleEntry32)) //store first Module in ModuleEntry32
    {
        do {
            if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) // if Found Module matches Module we look for -> done!
            {
                dwModuleBaseAddress = (DWORD)ModuleEntry32.modBaseAddr;
                break;
            }
        } while (Module32Next(hSnapshot, &ModuleEntry32)); // go through Module entries in Snapshot and store in ModuleEntry32
 
 
    }
    CloseHandle(hSnapshot);
    return dwModuleBaseAddress;
}
 
 
int main() {
 
    HWND hGameWindow = FindWindow(NULL, L"Age of Empires Expansion");
    if (hGameWindow == NULL) {
        std::cout << "Start the game!" << std::endl;
        return 0;
    }
    DWORD pID = NULL; // ID of our Game
    GetWindowThreadProcessId(hGameWindow, &pID);
    HANDLE processHandle = NULL;
    processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pID);
    if (processHandle == INVALID_HANDLE_VALUE || processHandle == NULL) { // error handling
        std::cout << "Failed to open process" << std::endl;
        return 0;
    }
 
    TCHAR gameName[13];
    wcscpy_s(gameName, 13,L"EMPIRESX.EXE");
    
    DWORD gameBaseAddress = GetModuleBaseAddress(gameName,pID);

    std::cout << "debugginfo: gameBaseAddress = " << gameBaseAddress << std::endl;
    
    DWORD offsetGameToBaseAdress = 0x003C4B18;
    std::vector<DWORD> pointsOffsets{ 0x3c, 0x100, 0x50, 0x0 };


    DWORD baseAddress = NULL;
    
    //Get value at gamebase+offset -> store it in baseAddress
    ReadProcessMemory(processHandle, (LPVOID)(gameBaseAddress+ offsetGameToBaseAdress), &baseAddress, sizeof(baseAddress), NULL);
    std::cout << "debugginfo: baseaddress = " << std::hex << baseAddress << std::endl;
    
    DWORD pointsAddress = baseAddress; //the Adress we need -> change now while going through offsets
    for (int i = 0; i < pointsOffsets.size() - 1; i++) // -1 because we dont want the value at the last offset
    {
        ReadProcessMemory(processHandle, (LPVOID)(pointsAddress + pointsOffsets.at(i)), &pointsAddress, sizeof(pointsAddress), NULL);
        std::cout << "debugginfo: Value at offset = " << std::hex << pointsAddress << std::endl;
    }
    pointsAddress += pointsOffsets.at(pointsOffsets.size() - 1); //Add Last offset -> done!!
    float currentPoint = 0;

    std::cout << sizeof(currentPoint) << std::endl;
    ReadProcessMemory(processHandle, (LPVOID)(pointsAddress), &currentPoint, sizeof(currentPoint), NULL);
    std::cout << "The last address is:" << pointsAddress << std::endl;
    std::cout << "Current value is:" << currentPoint << std::endl;
    //"UI"
    std::cout << "Age of Empires Hack" << std::endl;

    std::cout << "How many points you want?" << std::endl;
    float newPoints = 0;
    std::cin >> newPoints;
    WriteProcessMemory(processHandle, (LPVOID)(pointsAddress), &newPoints, 4, 0);
 
}