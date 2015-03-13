//  Copyright 2015 Google Inc. All Rights Reserved.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//  http ://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "stdafx.h"
#include "CommonUtils.h"
#include <strsafe.h>

void __stdcall my_puts(const char* str)
{
	fwrite(str, 1, strlen(str), stdout);
}

static console_output _pout = my_puts;

void DebugSetOutput(console_output pout)
{
	_pout = pout;
}

void DebugPrintf(const char* lpFormat, ...)
{
	CHAR buf[1024];
	va_list va;

	va_start(va, lpFormat);

	StringCbVPrintfA(buf, sizeof(buf), lpFormat, va);	

	_pout(buf);
}

std::wstring GetErrorMessage(DWORD dwError)
{
	LPWSTR pBuffer = NULL;

	DWORD dwSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS |
		FORMAT_MESSAGE_ALLOCATE_BUFFER, 0, dwError, 0, (LPWSTR)&pBuffer, 32 * 1024, nullptr);

	if (dwSize > 0)
	{
		std::wstring ret = pBuffer;

		LocalFree(pBuffer);

		return ret;
	}
	else
	{
		printf("Error getting message %d\n", GetLastError());
		WCHAR buf[64];
		StringCchPrintf(buf, _countof(buf), L"%d", dwError);
		return buf;
	}
}

std::wstring GetErrorMessage()
{
	return GetErrorMessage(GetLastError());
}


BOOL SetPrivilege(HANDLE hToken, LPCTSTR lpszPrivilege, BOOL bEnablePrivilege)
{
	TOKEN_PRIVILEGES tp;
	LUID luid;

	if (!LookupPrivilegeValue(NULL, lpszPrivilege, &luid))
	{
		return FALSE;
	}

	tp.PrivilegeCount = 1;
	tp.Privileges[0].Luid = luid;
	if (bEnablePrivilege)
	{
		tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
	}
	else
	{
		tp.Privileges[0].Attributes = 0;
	}

	if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, (PDWORD)NULL))
	{
		return FALSE;
	}

	if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		return FALSE;
	}

	return TRUE;
}

typedef ULONG(NTAPI* _RtlNtStatusToDosError)(NTSTATUS status);

DWORD NtStatusToDosError(NTSTATUS status)
{
	_RtlNtStatusToDosError func = (_RtlNtStatusToDosError)GetProcAddressNT("RtlNtStatusToDosError");

	return func(status);
}