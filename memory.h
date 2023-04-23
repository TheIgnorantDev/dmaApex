#include <cstring>
#include <stdio.h>
#include <mutex>
#include "pcileech/vmmdll.h"
#include "pcileech/vmmdll.h"
#pragma comment(lib, "pcileech/vmm")  
#pragma warning(disable:4200) // ignore vmm header and lc header throwing warnings
#define INRANGE(x,a,b)		(x >= a && x <= b) 
#define getBits( x )		(INRANGE(x,'0','9') ? (x - '0') : ((x&(~0x20)) - 'A' + 0xa))
#define getByte( x )		(getBits(x[0]) << 4 | getBits(x[1]))

typedef uint8_t* PBYTE;
typedef uint8_t BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;
typedef WORD* PWORD;
inline bool conn = false;
inline DWORD dwPID = 0;
inline bool isMatch(const PBYTE addr, const PBYTE pat, const PBYTE msk)
{
	size_t n = 0;
	while (addr[n] == pat[n] || msk[n] == (BYTE)'?')
	{
		if (!msk[++n])
		{
			return true;
		}
	}
	return false;
}

size_t findPattern(const PBYTE rangeStart, size_t len, const char* pattern);

typedef struct Process
{
	DWORD hProcess = dwPID;
	uint64_t baseaddr = 0;
}Process;

enum class process_status : BYTE
{
	NOT_FOUND,
	FOUND_NO_ACCESS,
	FOUND_READY
};

class Memory
{
private:
	Process proc;
	process_status status = process_status::NOT_FOUND;
	std::mutex m;
public:
	~Memory() { }
	VMM_HANDLE hVmm = 0;
	uint64_t get_proc_baseaddr();

	process_status get_proc_status();

	void check_proc();

	void open_proc(const char* name);

	void close_proc();

	template<typename T>
	bool Read(VMM_HANDLE x, uint64_t address, T& out);

	template<typename T>
	bool ReadArray(VMM_HANDLE x, uint64_t address, T out[], size_t len);

	template<typename T>
	bool Write(VMM_HANDLE x, uint64_t address, const T& value);

	template<typename T>
	bool WriteArray(VMM_HANDLE x, uint64_t address, const T value[], size_t len);

	uint64_t ScanPointer(VMM_HANDLE x, uint64_t ptr_address, const uint32_t offsets[], int level);
};
inline DWORD       dwReadCache = NULL;
template<typename T>
inline bool Memory::Read(VMM_HANDLE x, uint64_t address, T& out)
{
	std::lock_guard<std::mutex> l(m);
	//printf("Read: %I64X\n", address);
	return VMMDLL_MemReadEx(x, dwPID, address, (PBYTE)&out, (DWORD)sizeof(out), &dwReadCache, VMMDLL_FLAG_NOCACHE);
}

template<typename T>
inline bool Memory::ReadArray(VMM_HANDLE x, uint64_t address, T out[], size_t len)
{
	std::lock_guard<std::mutex> l(m);
	return VMMDLL_MemReadEx(x, dwPID, address, (PBYTE)&out, sizeof(out) * len, &dwReadCache, VMMDLL_FLAG_NOCACHE) == 0;
}

template<typename T>
inline bool Memory::Write(VMM_HANDLE x, uint64_t address, const T& value)
{
	std::lock_guard<std::mutex> l(m);
	/*return mem && virt_write_raw(mem, address, (uint8_t*)&value, sizeof(T)) == 0;*/
	return VMMDLL_MemWrite(x, dwPID, address, (PBYTE)&value, sizeof(value));
}

template<typename T>
inline bool Memory::WriteArray(VMM_HANDLE x, uint64_t address, const T value[], size_t len)
{
	std::lock_guard<std::mutex> l(m);
	return VMMDLL_MemWrite(x, dwPID, address, (PBYTE)&value, sizeof(T) * len);
}