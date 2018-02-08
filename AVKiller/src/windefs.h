#pragma once

#define WIN32_LEAN_AND_MEAN
#define NO_STRICT
#define NOMINMAX
#include <ntstatus.h>
#define WIN32_NO_STATUS
#include <Windows.h>
#include <winternl.h>

#ifndef _WIN64
#error "x64 only please"
#endif

typedef struct _EPROCESS *PEPROCESS;
typedef struct _HANDLE_TABLE_ENTRY_INFO *PHANDLE_TABLE_ENTRY_INFO;
typedef PVOID EX_PUSH_LOCK;
typedef struct _HANDLE_TABLE* PHANDLE_TABLE;

typedef struct _HANDLE_TABLE_ENTRY
{
  //This struct is incomplete, but we dont really care about the other fields
  ULONGLONG Value;
  ULONGLONG GrantedAccess : 25;
} HANDLE_TABLE_ENTRY, *PHANDLE_TABLE_ENTRY;

typedef struct _HANDLE_TABLE
{
  CHAR fill[100];
} HANDLE_TABLE, *PHANDLE_TABLE;