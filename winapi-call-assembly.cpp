#include "stdafx.h"
#include "windows.h"
#include <conio.h>

int main()
{
	DWORD ntdllBase;
	DWORD exportDir;
	DWORD indexNumber;
	DWORD apiAddress;
	char apiName[] = "DbgBreakPoint";

	__asm
	{
		mov ebx, fs:[0x30]						// eax = peb
		mov ebx, [ebx + 0xc]					// eax = peb->ldr
		mov ebx, [ebx + 0x1c]					// eax = peb->ldr.ininitializationordermodulelist.flink
		mov ebx, [ebx + 0x8]					// eax = image base of ntdll
		mov ntdllBase, ebx

		add ebx, [ebx + 0x3C]					// PE header
		mov ebx, [ebx + 0x78]					// export dir RVA
		add ebx, ntdllBase						// export dir VA
		mov exportDir, ebx

		mov esi, exportDir
		mov esi, [esi + 0x20]					// AddressOfNames VA
		add esi, ntdllBase						// AddressOfNames VA

		xor ebx, ebx

		startLoop:
			inc ebx
			lodsd
			add eax, ntdllBase					// store pointer to function name in eax
			push esi							// push the current value
			mov esi, eax						// add to esi for comparison
			lea edi, apiName					// load the name from the variable
			mov ecx, 10							// load number of operations? just make ecx not zero
			repe cmpsb							// compare strings
			pop esi								// retrieve previous value
			jne startLoop						

		dec ebx
		mov indexNumber, ebx					// save the index number
		mov eax, exportDir
		mov eax, [eax + 0x24]					// AddressOfOrdinals RVA
		add eax, ntdllBase						// AddressOfOrdinals VA
		
		movzx eax, [ebx * 2 + eax]				// store ordinal of the function
		mov ebx, exportDir
		mov ebx, [ebx + 0x1C]					// AddressOfFunctions RVA
		add ebx, ntdllBase						// AddressOfFunctions VA
		mov ebx, [eax * 4 + ebx]				
		add ebx, ntdllBase						// function VA
		mov eax, ebx							// store the address of the function in eax
		mov apiAddress, eax						// save the address of the function
	}

	// Test with GetProcAddress
	DWORD dwApiAddress;
	HMODULE WINAPI hInstLib = LoadLibrary(TEXT("ntdll.dll"));
	if (hInstLib != NULL)
		dwApiAddress = (DWORD)GetProcAddress(hInstLib, apiName);

	printf("\n[*] %s was found at 0x%08x (index %d) using shell code\n", apiName, apiAddress, indexNumber);
	printf("\n[*] %s was found at 0x%08x by GetProcAddress\n\n", apiName, dwApiAddress);
	printf("Hey, that's pretty good");
	_getch();

	return 0;
}

