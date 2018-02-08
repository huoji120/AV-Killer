#include "sup.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct _RTL_PROCESS_MODULE_INFORMATION
{
  HANDLE Section;
  PVOID MappedBase;
  PVOID ImageBase;
  ULONG ImageSize;
  ULONG Flags;
  USHORT LoadOrderIndex;
  USHORT InitOrderIndex;
  USHORT LoadCount;
  USHORT OffsetToFileName;
  UCHAR FullPathName[256];
} RTL_PROCESS_MODULE_INFORMATION, *PRTL_PROCESS_MODULE_INFORMATION;

typedef struct _RTL_PROCESS_MODULES
{
  ULONG NumberOfModules;
  RTL_PROCESS_MODULE_INFORMATION Modules[1];
} RTL_PROCESS_MODULES, *PRTL_PROCESS_MODULES;


BOOL SupFileExists(
  _In_ LPCWSTR FilePath
)
{
  NTSTATUS Status;
  HANDLE FileHandle;
  UNICODE_STRING NtFileName;
  OBJECT_ATTRIBUTES ObjAttr;
  IO_STATUS_BLOCK IoStatus;

  RtlInitUnicodeString(&NtFileName, (LPWSTR)FilePath);
  InitializeObjectAttributes(&ObjAttr, &NtFileName, 0, NULL, NULL);

  Status = NtCreateFile(
    &FileHandle,
    SYNCHRONIZE,
    &ObjAttr,
    &IoStatus,
    NULL,
    FILE_ATTRIBUTE_NORMAL,
    FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
    FILE_OPEN,
    FILE_NON_DIRECTORY_FILE,
    NULL, 0);

  if(!NT_SUCCESS(Status)) {
    return FALSE;
  }

  NtClose(FileHandle);

  return TRUE;
}

BOOL SupWriteResourceToDisk(
  _In_     LPCWSTR FilePath,
  _In_opt_ HMODULE Module,
  _In_     LPCWSTR ResourceName,
  _In_     LPCWSTR ResourceType
)
{
  HANDLE File;
  HRSRC ResInfo;
  PVOID ResData;
  ULONG ResSize;
  ULONG BytesWritten;

  ResInfo = FindResourceW(Module, ResourceName, ResourceType);

  if(!ResInfo)
    return FALSE;

  File = SupCreateFile(FilePath, GENERIC_WRITE | SYNCHRONIZE, 0, CREATE_ALWAYS);

  if(File == INVALID_HANDLE_VALUE)
    return FALSE;

  ResData = LoadResource(Module, ResInfo);
  ResData = LockResource(ResData);
  ResSize = SizeofResource(Module, ResInfo);

  BOOL Result = WriteFile(File, ResData, ResSize, &BytesWritten, NULL);

  CloseHandle(File);

  return Result;
}

HANDLE SupCreateFile(
  _In_ LPCWSTR FilePath,
  _In_ ACCESS_MASK DesiredAccess,
  _In_ ULONG ShareMode,
  _In_ ULONG CreateDisposition
)
{
  NTSTATUS Status;
  HANDLE FileHandle;
  UNICODE_STRING NtFileName;
  OBJECT_ATTRIBUTES ObjAttr;
  IO_STATUS_BLOCK IoStatus;

  RtlInitUnicodeString(&NtFileName, (LPWSTR)FilePath);
  InitializeObjectAttributes(&ObjAttr, &NtFileName, 0, NULL, NULL);

  Status = NtCreateFile(
    &FileHandle,
    DesiredAccess,
    &ObjAttr,
    &IoStatus,
    NULL,
    FILE_ATTRIBUTE_NORMAL,
    ShareMode,
    CreateDisposition,
    FILE_NON_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT,
    NULL, 0);

  if(!NT_SUCCESS(Status))
    FileHandle = INVALID_HANDLE_VALUE;

  return FileHandle;
}

BOOL SupLookupErrorMessage(
  _In_  ULONG ErrorCode,
  _Out_ LPSTR Buffer,
  _In_  ULONG BufferSize
)
{
  if(!FormatMessageA(
    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    ErrorCode,
    0,
    Buffer,
    BufferSize,
    NULL)) {
    return sprintf_s(Buffer, BufferSize, "%d", ErrorCode) > 0;
  }

  return TRUE;
}

LPVOID SupGetKernelBase(
  _Out_opt_ PSIZE_T KernelSize
)
{
  NTSTATUS status;
  PVOID buffer;
  ULONG bufferSize = 2048;

  buffer = malloc(bufferSize);

  status = NtQuerySystemInformation(
    11/*SystemModuleInformation*/,
    buffer,
    bufferSize,
    &bufferSize
  );

  if(status == STATUS_INFO_LENGTH_MISMATCH) {
    free(buffer);
    buffer = malloc(bufferSize);

    status = NtQuerySystemInformation(
      11/*SystemModuleInformation*/,
      buffer,
      bufferSize,
      &bufferSize
    );
  }

  if(!NT_SUCCESS(status))
    return NULL;

  if(KernelSize)
    *KernelSize = (SIZE_T)((PRTL_PROCESS_MODULES)buffer)->Modules[0].ImageSize;

  return ((PRTL_PROCESS_MODULES)buffer)->Modules[0].ImageBase;
}