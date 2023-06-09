#include "memory.h"

//Credits: learn_more, stevemk14ebr
size_t findPattern(const PBYTE rangeStart, size_t len, const char* pattern)
{
	size_t l = strlen(pattern);
	PBYTE patt_base = static_cast<PBYTE>(malloc(l >> 1));
	PBYTE msk_base = static_cast<PBYTE>(malloc(l >> 1));
	PBYTE pat = patt_base;
	PBYTE msk = msk_base;
	if (pat && msk)
	{
		l = 0;
		while (*pattern)
		{
			if (*pattern == ' ')
				pattern++;
			if (!*pattern)
				break;
			if (*(PBYTE)pattern == (BYTE)'\?')
			{
				*pat++ = 0;
				*msk++ = '?';
				pattern += ((*(PWORD)pattern == (WORD)'\?\?') ? 2 : 1);
			}
			else
			{
				*pat++ = getByte(pattern);
				*msk++ = 'x';
				pattern += 2;
			}
			l++;
		}
		*msk = 0;
		pat = patt_base;
		msk = msk_base;
		for (size_t n = 0; n < (len - l); ++n)
		{
			if (isMatch(rangeStart + n, patt_base, msk_base))
			{
				free(patt_base);
				free(msk_base);
				return n;
			}
		}
		free(patt_base);
		free(msk_base);
	}
	return -1;
}

uint64_t Memory::get_proc_baseaddr()
{
	return proc.baseaddr;
}

process_status Memory::get_proc_status()
{
	return status;
}

void Memory::check_proc()
{
	/*if (status == process_status::FOUND_READY)
	{*/
		//short c;
		//VMMDLL_MemRead(hVmm, dwPID, proc.baseaddr, (PBYTE)& c, sizeof(c));
		////Read<short>(hVmm, proc.baseaddr, c);
		//printf("c: %x\n", c);

		//if (c != 0x5A4D)
		//{
		//	status = process_status::FOUND_NO_ACCESS;
		//	close_proc();
		//}
	//}
}

void Memory::open_proc(const char* name)
{
	if (!hVmm)
	{
		printf("Attempting to initiate your FPGA device directly..\n");
		LPSTR args[] = { (LPSTR)"-device", (LPSTR)"fpga", (LPSTR)"-v" };
		hVmm = VMMDLL_Initialize(3, args);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // set text color back to normal
		printf("Attempting to grab the process id of 'r5apex.exe' on target system..\n");
		conn = false;
	}

	if (!hVmm == false && !conn)
	{
		bool bResult = false;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE); // set text color to intense purple
		printf("[INFO] ");
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // set text color back to normal
		printf("Attempting to grab the process id of 'r5apex.exe' on target system..\n");
		// try to get the process id of the 'r5apex.exe' process
		bResult = VMMDLL_PidGetFromName(hVmm, (LPSTR)name, &dwPID);
		if (dwPID == 0) { printf("Failed to get process id of '%s' on target system\n",name); return; }
		PVMMDLL_MAP_MODULEENTRY pModuleEntryExplorer;
		bResult = VMMDLL_Map_GetModuleFromNameU(hVmm, dwPID, (LPSTR)name, &pModuleEntryExplorer);
		if (pModuleEntryExplorer== NULL) { printf("Failed to get module entry of '%s' on target system\n",name); return; }
		proc.baseaddr = pModuleEntryExplorer->vaBase;
		proc.hProcess = dwPID;
		status == process_status::FOUND_READY;
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE); // set text color back to normal
		printf("PID: %i & BASE_ADDR: 0x%I64X FOUND\n",dwPID,proc.baseaddr);
		conn = true;
	}
	else
	{
		printf("Can't create connector\n");
		exit(0);
	}
}

void Memory::close_proc()
{
	VMMDLL_Close(hVmm);
}

uint64_t Memory::ScanPointer(VMM_HANDLE x, uint64_t ptr_address, const uint32_t offsets[], int level)
{
	if (!ptr_address)
		return 0;

	uint64_t lvl = ptr_address;

	for (int i = 0; i < level; i++)
	{
		if (!Read<uint64_t>(hVmm, lvl, lvl) || !lvl)
			return 0;
		lvl += offsets[i];
	}

	return lvl;
}