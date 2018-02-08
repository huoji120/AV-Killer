#include "scm.h"

#include <stdio.h>

BOOL ScmCreateService(
  _Out_ PHANDLE ServiceHandle,
  _In_ LPCWSTR ServiceName,
  _In_opt_ LPCWSTR DisplayName,
  _In_ LPCWSTR BinPath,
  _In_ ACCESS_MASK DesiredAccess,
  _In_ ULONG ServiceType,
  _In_ ULONG StartType,
  _In_ ULONG ErrorControl
)
{
  SC_HANDLE Scm;
  WCHAR QualifiedPath[MAX_PATH + 2];

  Scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_CREATE_SERVICE);

  if(!Scm)
    return FALSE;

  swprintf_s(QualifiedPath, MAX_PATH + 2, L"%ws", BinPath);

  *ServiceHandle = CreateServiceW(
    Scm,
    ServiceName, DisplayName,
    DesiredAccess,
    ServiceType, StartType,
    ErrorControl, QualifiedPath,
    NULL, NULL, NULL, NULL, NULL
  );

  CloseServiceHandle(Scm);

  return *ServiceHandle != NULL;
}

BOOL ScmDeleteService(
  _In_ HANDLE ServiceHandle
)
{
  return DeleteService(ServiceHandle);
}

BOOL ScmOpenServiceHandle(
  _Out_ PHANDLE ServiceHandle,
  _In_ LPCWSTR ServiceName,
  _In_ ACCESS_MASK DesiredAccess
)
{
  SC_HANDLE Scm;

  Scm = OpenSCManagerW(NULL, NULL, SC_MANAGER_CONNECT);

  if(!Scm)
    return FALSE;

  *ServiceHandle = OpenService(Scm, ServiceName, DesiredAccess);

  CloseServiceHandle(Scm);

  return *ServiceHandle != NULL;
}

BOOL ScmCloseServiceHandle(
  _In_ HANDLE ServiceHandle
)
{
  return CloseServiceHandle(ServiceHandle);
}

BOOL ScmStartService(
  _In_ HANDLE ServiceHandle
)
{
  return StartServiceW(ServiceHandle, 0, NULL);
}

BOOL ScmPauseService(
  _In_ HANDLE ServiceHandle
)
{
  SERVICE_STATUS ServiceStatus;

  return ControlService(ServiceHandle, SERVICE_CONTROL_PAUSE, &ServiceStatus);
}

BOOL ScmResumeService(
  _In_ HANDLE ServiceHandle
)
{
  SERVICE_STATUS ServiceStatus;

  return ControlService(ServiceHandle, SERVICE_CONTROL_CONTINUE, &ServiceStatus);
}

BOOL ScmStopService(
  _In_ HANDLE ServiceHandle
)
{
  SERVICE_STATUS ServiceStatus;

  return ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);
}