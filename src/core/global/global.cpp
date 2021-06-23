#include "global.h"
#include <objbase.h>
#include <shlobj.h>
#include <psapi.h>
#include <QString>

#pragma comment(lib, "Shlwapi.lib")

/**
\brief Number of allocated buffers by emalloc(). This should be 0 when x64dbg ends.
*/
static int g_emalloc_count = 0;


/**
\brief Allocates a new buffer.
\param size The size of the buffer to allocate (in bytes).
\param reason The reason for allocation (can be used for memory allocation tracking).
\return Always returns a valid pointer to the buffer you requested. Will quit the application on errors.
*/
void* emalloc(size_t size, const char* reason)
{
	ASSERT_NONZERO(size);
#ifdef ENABLE_MEM_TRACE
	unsigned char* a = (unsigned char*)GlobalAlloc(GMEM_FIXED, size + sizeof(void*));
#else
	unsigned char* a = (unsigned char*)GlobalAlloc(GMEM_FIXED, size);
#endif //ENABLE_MEM_TRACE
	if (!a)
	{
		wchar_t sizeString[25];
		swprintf_s(sizeString, L"%p bytes", (void*)size);
		MessageBoxW(0, L"Could not allocate memory (minidump will be created)", sizeString, MB_ICONERROR);
		__debugbreak();
		ExitProcess(1);
	}
	g_emalloc_count++;
#ifdef ENABLE_MEM_TRACE
	EnterCriticalSection(&criticalSection);
	memset(a, 0, size + sizeof(void*));
	FILE* file = fopen(alloctrace, "a+");
	fprintf(file, "DBG%.5d:  alloc:%p:%p:%s:%p\n", emalloc_count, a, _ReturnAddress(), reason, size);
	fclose(file);
	alloctracemap[_ReturnAddress()]++;
	*(void**)a = _ReturnAddress();
	LeaveCriticalSection(&criticalSection);
	return a + sizeof(void*);
#else
	memset(a, 0, size);
	return a;
#endif //ENABLE_MEM_TRACE
}

/**
\brief Reallocates a buffer allocated with emalloc().
\param [in] Pointer to memory previously allocated with emalloc(). When NULL a new buffer will be allocated by emalloc().
\param size The new memory size.
\param reason The reason for allocation (can be used for memory allocation tracking).
\return Always returns a valid pointer to the buffer you requested. Will quit the application on errors.
*/
void* erealloc(void* ptr, size_t size, const char* reason)
{
	ASSERT_NONZERO(size);
	// Free the memory if the pointer was set (as per documentation).
	if (ptr) efree(ptr, reason);

	return emalloc(size, reason);
}

/**
\brief Free memory previously allocated with emalloc().
\param [in] Pointer to the memory to free.
\param reason The reason for freeing, should be the same as the reason for allocating.
*/
void efree(void* ptr, const char* reason)
{
	g_emalloc_count--;
#ifdef ENABLE_MEM_TRACE
	EnterCriticalSection(&criticalSection);
	char* ptr2 = (char*)ptr - sizeof(void*);
	FILE* file = fopen(alloctrace, "a+");
	fprintf(file, "DBG%.5d:   free:%p:%p:%s\n", emalloc_count, ptr, *(void**)ptr2, reason);
	fclose(file);
	if (alloctracemap.find(*(void**)ptr2) != alloctracemap.end())
	{
		if (--alloctracemap.at(*(void**)ptr2) < 0)
		{
			String str = StringUtils::sprintf("address %p, reason %s", *(void**)ptr2, reason);
			MessageBoxA(0, str.c_str(), "Freed memory more than once", MB_OK);
			__debugbreak();
		}
	}
	else
	{
		String str = StringUtils::sprintf("address %p, reason %s", *(void**)ptr2, reason);
		MessageBoxA(0, str.c_str(), "Trying to free const memory", MB_OK);
		__debugbreak();
	}
	LeaveCriticalSection(&criticalSection);
	GlobalFree(ptr2);
#else
	GlobalFree(ptr);
#endif //ENABLE_MEM_TRACE
}
