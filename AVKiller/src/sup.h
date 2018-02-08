#pragma once

#include "windefs.h"

#ifdef __cplusplus
extern "C" {
#endif
#if 0
}
#endif

/**
 *  \brief Check if the specified file exists.
 *  
 *  \param[in] FilePath     NT Path to the file.
 *
 *  \returns If the file exists, the return value is nonzero.
 *           If the file could not be found, the return value is 0.
 *           To get extended error information, call \c GetLastError.
 */
BOOL SupFileExists(
  _In_ LPCWSTR FilePath
);

/**
 *  \brief Write a resource file to disk.
 *
 *  \param[in] FilePath     NT Path to the file.
 *  \param[in] Module       The module that contains the resource. 
 *                          Use \c NULL to specify the main process module.
 *  \param[in] ResourceName The resource name.
 *  \param[in] ResourceType The resource type.
 *
 *  \returns If the function succeeds, the return value is nonzero.
 *           If the function fails, the return value is 0.
 *           To get extended error information, call \c GetLastError.
 */
BOOL SupWriteResourceToDisk(
  _In_     LPCWSTR FilePath,
  _In_opt_ HMODULE Module,
  _In_     LPCWSTR ResourceName,
  _In_     LPCWSTR ResourceType
);

/**
 *  \brief Creates or opens a file.
 *  
 *  A wrapper around the Win32 \c NtCreateFile API.
 *  For more information see https://msdn.microsoft.com/en-us/library/bb432380(v=vs.85).aspx
 *
 *  \param[in] FilePath          NT Path to the file.
 *  \param[in] DesiredAccess     Desired access rights.
 *  \param[in] ShareMode         The requested sharing mode of the file.
 *  \param[in] CreateDisposition An action to take on a file or device that exists or does not exist.
 *
 *  \returns If the function succeeds, the return value is an open handle to the specified file.
 *           If the function fails, the return value is INVALID_HANDLE_VALUE. 
 *           To get extended error information, call GetLastError.
 */
HANDLE SupCreateFile(
  _In_ LPCWSTR FilePath,
  _In_ ACCESS_MASK DesiredAccess,
  _In_ ULONG ShareMode,
  _In_ ULONG CreateDisposition
);

/**
 *  \brief Translate a Win32 error code to its respective message.
 *  
 *  \param[in]  ErrorCode   The Win32 error code.
 *  \param[out] Buffer      Buffer to store the message.
 *  \param[in]  BufferSize  Max buffer size.
 *
 *  \remarks If the function cannot find the message for the provided error 
 *           the error code is converted to text and stored on the buffer
 *           instead of the message.
 *
 *  \returns If the function succeeds, the return value is nonzero.
 *           If the function fails, the return value is 0.
 */
BOOL SupLookupErrorMessage(
  _In_  ULONG ErrorCode,
  _Out_ LPSTR Buffer,
  _In_  ULONG BufferSize
);

/**
 *  \brief Retrieves the kernel base for the current session.
 *  
 *  \param[out] KernelSize The size of the kernel image.
 *
 *  \returns If the function succeeds, the return value is the base address for the ntos kernel.
 *           If the function fails, the return value is 0.
 *           To get extended error information, call \c GetLastError.
 */
LPVOID SupGetKernelBase(
  _Out_opt_ PSIZE_T KernelSize
);

#if 0
{
#endif
#ifdef __cplusplus
}
#endif
