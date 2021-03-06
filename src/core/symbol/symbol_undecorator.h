#pragma once
#include <stdlib.h>
#include <QString>
// undname.cxx
// Not declared in a Wine header.The function is either undocumented, or missing from Wine.
// Implemented in "dlls/msvcrt/undname.c".https://source.winehq.org/source/dlls/msvcrt/undname.c
// Debug channel "msvcrt".

enum
{
	X_UNDNAME_COMPLETE = 0x0, //Enables full undecoration.
	X_UNDNAME_NO_LEADING_UNDERSCORES = 0x1, //Removes leading underscores from Microsoft extended keywords.
	X_UNDNAME_NO_MS_KEYWORDS = 0x2, //Disables expansion of Microsoft extended keywords.
	X_UNDNAME_NO_FUNCTION_RETURNS = 0x4, //Disables expansion of return type for primary declaration.
	X_UNDNAME_NO_ALLOCATION_MODEL = 0x8, //Disables expansion of the declaration model.
	X_UNDNAME_NO_ALLOCATION_LANGUAGE = 0x10, //Disables expansion of the declaration language specifier.
	X_UNDNAME_NO_MS_THISTYPE = 0x20, //NYI Disable expansion of MS keywords on the 'this' type for primary declaration.
	X_UNDNAME_NO_CV_THISTYPE = 0x40, //NYI Disable expansion of CV modifiers on the 'this' type for primary declaration/
	X_UNDNAME_NO_THISTYPE = 0x60, //Disables all modifiers on the this type.
	X_UNDNAME_NO_ACCESS_SPECIFIERS = 0x80, //Disables expansion of access specifiers for members.
	X_UNDNAME_NO_THROW_SIGNATURES = 0x100, //Disables expansion of "throw-signatures" for functions and pointers to functions.
	X_UNDNAME_NO_MEMBER_TYPE = 0x200, //Disables expansion of static or virtual members.
	X_UNDNAME_NO_RETURN_UDT_MODEL = 0x400, //Disables expansion of the Microsoft model for UDT returns.
	X_UNDNAME_32_BIT_DECODE = 0x800, //Undecorates 32-bit decorated names.
	X_UNDNAME_NAME_ONLY = 0x1000, //Gets only the name for primary declaration; returns just [scope::]name. Expands template params.
	X_UNDNAME_TYPE_ONLY = 0x2000, //Input is just a type encoding; composes an abstract declarator.
	X_UNDNAME_HAVE_PARAMETERS = 0x4000, //The real template parameters are available.
	X_UNDNAME_NO_ECSU = 0x8000, //Suppresses enum/class/struct/union.
	X_UNDNAME_NO_IDENT_CHAR_CHECK = 0x10000, //Suppresses check for valid identifier characters.
	X_UNDNAME_NO_PTR64 = 0x20000, //Does not include ptr64 in output.
};


typedef char* pchar_t;
typedef const char* pcchar_t;
typedef char* (*GetParameter_t)(long n);
using Alloc_t = decltype(malloc);
using Free_t = decltype(free);

extern "C" pchar_t __cdecl __unDNameEx(_Out_opt_z_cap_(maxStringLength) pchar_t outputString,
	pcchar_t name,
	int maxStringLength,    // Note, COMMA is leading following optional arguments
	Alloc_t pAlloc,
	Free_t pFree,
	GetParameter_t pGetParameter,
	unsigned long disableFlags
);

#define MAX_DECORATE_NAME_SIZE 512

template<size_t MaxSize = 512>
inline bool UndecorateName(const QString& decoratedName, QString& undecoratedName)
{
	//TODO: undocumented hack to have some kind of performance while undecorating names
	auto mymalloc = [](size_t size) { return emalloc(size, "symbolundecorator::undecoratedName"); };
	auto myfree = [](void* ptr) { return efree(ptr, "symbolundecorator::undecoratedName"); };

	char szUndecoratedName[MAX_DECORATE_NAME_SIZE] = { 0 };
	 
	//undecoratedName.resize(max(MaxSize, decoratedName.length() * 2));
	if (!__unDNameEx(szUndecoratedName,
		decoratedName.toStdString().c_str(),
		MAX_DECORATE_NAME_SIZE,
		mymalloc,
		myfree,
		nullptr,
		X_UNDNAME_COMPLETE))
	{
		undecoratedName.clear();
		return false;
	}
	else
	{
		undecoratedName = QString::fromLocal8Bit(szUndecoratedName);
		if (decoratedName == undecoratedName)
			undecoratedName = ""; //https://stackoverflow.com/a/18299315
		return true;
	}
}