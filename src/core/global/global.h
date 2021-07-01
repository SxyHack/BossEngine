#pragma once

// Windows Header Files:
#include <windows.h>

#include <imagehlp.h>
#include <wincrypt.h>
#include <psapi.h>
//#include <commdlg.h>
#include <shellapi.h>
#include <tlhelp32.h>


#include "be_assert.h"

#define MAX_FILE_PATH_SIZE 1024


//functions
#ifdef ENABLE_MEM_TRACE
void* emalloc(size_t size, const char* reason = "emalloc:???");
void* erealloc(void* ptr, size_t size, const char* reason = "erealloc:???");
void efree(void* ptr, const char* reason = "efree:???");
#else
void* emalloc(size_t size, const char* reason = nullptr);
void* erealloc(void* ptr, size_t size, const char* reason = nullptr);
void efree(void* ptr, const char* reason = nullptr);

#endif //ENABLE_MEM_TRACE