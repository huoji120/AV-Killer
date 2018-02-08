#pragma once

#include "windefs.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

BOOL ScmCreateService(
  _Out_ PHANDLE ServiceHandle,
  _In_ LPCWSTR ServiceName,
  _In_opt_ LPCWSTR DisplayName,
  _In_ LPCWSTR BinPath,
  _In_ ACCESS_MASK DesiredAccess,
  _In_ ULONG ServiceType,
  _In_ ULONG StartType,
  _In_ ULONG ErrorControl
);

BOOL ScmDeleteService(
  _In_ HANDLE ServiceHandle
);

BOOL ScmOpenServiceHandle(
  _Out_ PHANDLE ServiceHandle,
  _In_ LPCWSTR ServiceName,
  _In_ ACCESS_MASK DesiredAccess
);

BOOL ScmCloseServiceHandle(
  _In_ HANDLE ServiceHandle
);

BOOL ScmStartService(
  _In_ HANDLE ServiceHandle
);

BOOL ScmPauseService(
  _In_ HANDLE ServiceHandle
);

BOOL ScmResumeService(
  _In_ HANDLE ServiceHandle
);

BOOL ScmStopService(
  _In_ HANDLE ServiceHandle
);

#if 0
{
#endif
#ifdef __cplusplus
}
#endif