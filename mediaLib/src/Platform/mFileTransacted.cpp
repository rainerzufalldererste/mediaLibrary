#include "mFileTransacted.h"

#include "mFile.h"

#include <ktmw32.h>
#pragma comment (lib, "KtmW32.lib")

#include <aclapi.h>
#pragma comment(lib, "Advapi32.lib")

#if (NTDDI_VERSION >= NTDDI_WIN8)
#include "pathcch.h"

#pragma comment(lib, "Pathcch.lib")
#endif

#ifdef GIT_BUILD // Define __M_FILE__
  #ifdef __M_FILE__
    #undef __M_FILE__
  #endif
  #define __M_FILE__ "EoRawu1Wp6eVM6KH7CNc9TvnZd4za5XTxyCP+G50dVZurPiESRG8hgCOjZ5xkOvnjHxD9o/junjzIdkW"
#endif

//////////////////////////////////////////////////////////////////////////

struct mFileTransaction
{
  HANDLE transactionHandle;
  bool hasBeenTransacted;
};

mFUNCTION(mFileTransaction_Destroy_Internal, IN_OUT mFileTransaction *pTransaction);

mFUNCTION(mFileTransaction_CreateDirectoryRecursive_Internal, mPtr<mFileTransaction> &transaction, const wchar_t *directoryPath);

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mFileTransaction_Create, OUT mPtr<mFileTransaction> *pTransaction, IN mAllocator *pAllocator)
{
  mFUNCTION_SETUP();

  mERROR_IF(pTransaction == nullptr, mR_ArgumentNull);

#ifdef mPLATFORM_WINDOWS
  HANDLE transactionHandle = CreateTransaction(nullptr, 0, 0, 0, 0, 0, L"");

  if (transactionHandle == INVALID_HANDLE_VALUE)
  {
    const DWORD error = GetLastError();
    mUnused(error);

    mRETURN_RESULT(mR_InternalError);
  }

  mDEFER_ON_ERROR(CloseHandle(transactionHandle));

  mERROR_CHECK((mSharedPointer_Allocate<mFileTransaction>(pTransaction, pAllocator, [](mFileTransaction *pData) { mFileTransaction_Destroy_Internal(pData); }, 1)));

  (*pTransaction)->transactionHandle = transactionHandle;
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_Destroy, IN_OUT mPtr<mFileTransaction> *pTransaction)
{
  return mSharedPointer_Destroy(pTransaction);
}

mFUNCTION(mFileTransaction_Perform, mPtr<mFileTransaction> &transaction)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);

  if (0 == CommitTransaction(transaction->transactionHandle))
  {
    const DWORD error = GetLastError();
    mUnused(error);

    mRETURN_RESULT(mR_InternalError);
  }
  else
  {
    transaction->hasBeenTransacted = true;
  }

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mFileTransaction_CreateDirectory, mPtr<mFileTransaction> &transaction, const mString &directoryPath)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);
  mERROR_IF(directoryPath.hasFailed || directoryPath.bytes <= 1, mR_InvalidParameter);

  mString path;
  mERROR_CHECK(mFile_FailOnInvalidDirectoryPath(directoryPath, &path));

  wchar_t wDirectoryName[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(path, wDirectoryName, mARRAYSIZE(wDirectoryName)));

  mERROR_CHECK(mFileTransaction_CreateDirectoryRecursive_Internal(transaction, wDirectoryName));

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_WriteFile, mPtr<mFileTransaction> &transaction, const mString &filename, const uint8_t *pData, const size_t bytes)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr || pData == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);
  mERROR_IF(filename.hasFailed || filename.bytes <= 1, mR_InvalidParameter);

  wchar_t wFilename[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(filename, wFilename, mARRAYSIZE(wFilename)));

  HANDLE fileHandle = CreateFileTransactedW(wFilename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
  DWORD error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.

  if (fileHandle == INVALID_HANDLE_VALUE && error == ERROR_PATH_NOT_FOUND)
  {
    wchar_t parentDirectory[MAX_PATH];
    mERROR_CHECK(mStringCopy(parentDirectory, mARRAYSIZE(parentDirectory), wFilename, MAX_PATH));

#if (NTDDI_VERSION >= NTDDI_WIN8)
    HRESULT hr = S_OK;

    // Requires `pathcch.h` && `Pathcch.lib`.
    mERROR_IF(FAILED(hr = PathCchRemoveFileSpec(parentDirectory, mARRAYSIZE(parentDirectory))), mR_InternalError);
    mERROR_IF(hr == S_FALSE, mR_InvalidParameter);
#else
    // deprecated since windows 8.
    mERROR_IF(PathRemoveFileSpecW(parentDirectory), mR_InvalidParameter);
#endif

    mERROR_CHECK(mFileTransaction_CreateDirectoryRecursive_Internal(transaction, parentDirectory));
    
    fileHandle = CreateFileTransactedW(wFilename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
    error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.
  }

  mERROR_IF(fileHandle == INVALID_HANDLE_VALUE, mR_InternalError);
  mDEFER(CloseHandle(fileHandle));

  size_t bytesRemaining = bytes;
  size_t offset = 0;

  while (bytesRemaining > 0)
  {
    const DWORD bytesToWrite = (DWORD)mMin((size_t)MAXDWORD, bytesRemaining);
    DWORD bytesWritten = 0;

    if (0 == WriteFile(fileHandle, pData + offset, bytesToWrite, &bytesWritten, nullptr))
    {
      error = GetLastError();

      mRETURN_RESULT(mR_IOFailure);
    }

    mERROR_IF(bytesWritten == 0, mR_IOFailure);

    offset += bytesWritten;
    bytesRemaining -= bytesWritten;
  }

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_DeleteFile, mPtr<mFileTransaction> &transaction, const mString &filename)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);
  mERROR_IF(filename.hasFailed || filename.bytes <= 1, mR_InvalidParameter);

  wchar_t wFilename[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(filename, wFilename, mARRAYSIZE(wFilename)));

  if (0 == DeleteFileTransactedW(wFilename, transaction->transactionHandle))
  {
    const DWORD error = GetLastError();

    mERROR_IF(error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND, mR_ResourceNotFound);
    mRETURN_RESULT(mR_InternalError);
  }

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_CopyFile, mPtr<mFileTransaction> &transaction, const mString &target, const mString &source, const bool replaceIfExistent)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);
  mERROR_IF(target.hasFailed || source.hasFailed || target.bytes <= 1 || source.bytes <= 1, mR_InvalidParameter);

  wchar_t wSource[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(source, wSource, mARRAYSIZE(wSource)));

  wchar_t wTarget[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(target, wTarget, mARRAYSIZE(wSource)));

  if (0 == CopyFileTransactedW(wSource, wTarget, nullptr, nullptr, nullptr, replaceIfExistent ? 0 : COPY_FILE_FAIL_IF_EXISTS, transaction->transactionHandle))
  {
    DWORD error = GetLastError();

    mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

    if (error == ERROR_PATH_NOT_FOUND)
    {
      HANDLE fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
      error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.

      mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

      if (fileHandle == INVALID_HANDLE_VALUE && error == ERROR_PATH_NOT_FOUND)
      {
        wchar_t parentDirectory[MAX_PATH];
        mERROR_CHECK(mStringCopy(parentDirectory, mARRAYSIZE(parentDirectory), wTarget, MAX_PATH));

#if (NTDDI_VERSION >= NTDDI_WIN8)
        HRESULT hr = S_OK;

        // Requires `pathcch.h` && `Pathcch.lib`.
        mERROR_IF(FAILED(hr = PathCchRemoveFileSpec(parentDirectory, mARRAYSIZE(parentDirectory))), mR_InternalError);
        mERROR_IF(hr == S_FALSE, mR_InvalidParameter);
#else
        // deprecated since windows 8.
        mERROR_IF(PathRemoveFileSpecW(parentDirectory), mR_InvalidParameter);
#endif

        mERROR_CHECK(mFileTransaction_CreateDirectoryRecursive_Internal(transaction, parentDirectory));

        fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
        error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.
      }

      mERROR_IF(fileHandle == INVALID_HANDLE_VALUE, mR_InternalError);
      CloseHandle(fileHandle);

      if (0 == CopyFileTransactedW(wSource, wTarget, nullptr, nullptr, nullptr, 0, transaction->transactionHandle))
      {
        error = GetLastError();

        mRETURN_RESULT(mR_ResourceNotFound);
      }
      else
      {
        mRETURN_SUCCESS();
      }
    }

    mRETURN_RESULT(mR_IOFailure);
  }

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_MoveFile, mPtr<mFileTransaction> &transaction, const mString &target, const mString &source, const bool replaceIfExistent)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);
  mERROR_IF(target.hasFailed || source.hasFailed || target.bytes <= 1 || source.bytes <= 1, mR_InvalidParameter);

  wchar_t wSource[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(source, wSource, mARRAYSIZE(wSource)));

  wchar_t wTarget[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(target, wTarget, mARRAYSIZE(wSource)));

  if (0 == MoveFileTransactedW(wSource, wTarget, nullptr, nullptr, MOVEFILE_COPY_ALLOWED | (replaceIfExistent ? MOVEFILE_REPLACE_EXISTING : 0), transaction->transactionHandle))
  {
    DWORD error = GetLastError();

    mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

    if (error == ERROR_PATH_NOT_FOUND)
    {
      HANDLE fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
      error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.

      mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

      if (fileHandle == INVALID_HANDLE_VALUE && error == ERROR_PATH_NOT_FOUND)
      {
        wchar_t parentDirectory[MAX_PATH];
        mERROR_CHECK(mStringCopy(parentDirectory, mARRAYSIZE(parentDirectory), wTarget, MAX_PATH));

#if (NTDDI_VERSION >= NTDDI_WIN8)
        HRESULT hr = S_OK;

        // Requires `pathcch.h` && `Pathcch.lib`.
        mERROR_IF(FAILED(hr = PathCchRemoveFileSpec(parentDirectory, mARRAYSIZE(parentDirectory))), mR_InternalError);
        mERROR_IF(hr == S_FALSE, mR_InvalidParameter);
#else
        // deprecated since windows 8.
        mERROR_IF(PathRemoveFileSpecW(parentDirectory), mR_InvalidParameter);
#endif

        mERROR_CHECK(mFileTransaction_CreateDirectoryRecursive_Internal(transaction, parentDirectory));

        fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
        error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.
      }

      mERROR_IF(fileHandle == INVALID_HANDLE_VALUE, mR_InternalError);
      CloseHandle(fileHandle);

      if (0 == MoveFileTransactedW(wSource, wTarget, nullptr, nullptr, MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING, transaction->transactionHandle))
      {
        error = GetLastError();

        mRETURN_RESULT(mR_ResourceNotFound);
      }
      else
      {
        mRETURN_SUCCESS();
      }
    }

    mRETURN_RESULT(mR_IOFailure);
  }

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_WriteRegistryKey, mPtr<mFileTransaction> &transaction, const mString &keyUrl, const mString &value, OUT OPTIONAL bool *pNewlyCreated /* = nullptr */)
{
  mFUNCTION_SETUP();

  mString subAddress;
  subAddress.pAllocator = &mDefaultTempAllocator;

  const char classesRoot[] = "HKEY_CLASSES_ROOT\\";
  const char currentConfig[] = "HKEY_CURRENT_CONFIG\\";
  const char currentUser[] = "HKEY_CURRENT_USER\\";
  const char localMachine[] = "HKEY_LOCAL_MACHINE\\";
  const char users[] = "HKEY_USERS\\";

  HKEY parentKey = nullptr;

  if (keyUrl.StartsWith(classesRoot))
  {
    parentKey = HKEY_CLASSES_ROOT;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(classesRoot) - 1));
  }
  else if (keyUrl.StartsWith(currentConfig))
  {
    parentKey = HKEY_CURRENT_CONFIG;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(currentConfig) - 1));
  }
  else if (keyUrl.StartsWith(currentUser))
  {
    parentKey = HKEY_CURRENT_USER;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(currentUser) - 1));
  }
  else if (keyUrl.StartsWith(localMachine))
  {
    parentKey = HKEY_LOCAL_MACHINE;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(localMachine) - 1));
  }
  else if (keyUrl.StartsWith(users))
  {
    parentKey = HKEY_USERS;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(users) - 1));
  }
  else
  {
    mRETURN_RESULT(mR_InvalidParameter);
  }

  mString pathString;
  pathString.pAllocator = &mDefaultTempAllocator;

  mString valueName;
  valueName.pAllocator = &mDefaultTempAllocator;

  size_t index = (size_t)-1;
  size_t lastSlashIndex = (size_t)-1;
  const mchar_t slashCodePoint = mToChar<2>("\\");

  for (const auto &&_char : subAddress)
  {
    ++index;

    if (_char.codePoint == slashCodePoint)
      lastSlashIndex = index;
  }

  if (lastSlashIndex == (size_t)-1)
  {
    mERROR_CHECK(mString_Create(&pathString, subAddress, &mDefaultTempAllocator));
    mERROR_CHECK(mString_Create(&valueName, "", 1, &mDefaultTempAllocator));
  }
  else
  {
    mERROR_CHECK(mString_Substring(subAddress, &pathString, 0, lastSlashIndex));

    if (lastSlashIndex != subAddress.count)
      mERROR_CHECK(mString_Substring(subAddress, &valueName, lastSlashIndex + 1, subAddress.count - (lastSlashIndex + 2)));
    else
      mERROR_CHECK(mString_Create(&valueName, "", 1, &mDefaultTempAllocator));
  }

  wchar_t *wPathOrValue = nullptr;
  size_t wPathOrValueCount = 0;
  mERROR_CHECK(mString_GetRequiredWideStringCount(pathString, &wPathOrValueCount));

  mAllocator *pAllocator = &mDefaultTempAllocator;
  mDEFER(mAllocator_FreePtr(pAllocator, &wPathOrValue));
  mERROR_CHECK(mAllocator_AllocateZero(pAllocator, &wPathOrValue, wPathOrValueCount));

  mERROR_CHECK(mString_ToWideString(pathString, wPathOrValue, wPathOrValueCount));

  wchar_t *wValueName = nullptr;
  size_t wValueNameCount = 0;
  mERROR_CHECK(mString_GetRequiredWideStringCount(valueName, &wValueNameCount));

  mDEFER(mAllocator_FreePtr(pAllocator, &wValueName));
  mERROR_CHECK(mAllocator_AllocateZero(pAllocator, &wValueName, wValueNameCount));

  mERROR_CHECK(mString_ToWideString(valueName, wValueName, wValueNameCount));

  HKEY key = nullptr;
  DWORD disposition = 0;

  LSTATUS result = RegCreateKeyTransactedW(parentKey, wPathOrValue, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, &disposition, transaction->transactionHandle, nullptr);
  mDEFER(if (key != nullptr) RegCloseKey(key));
  mERROR_IF(result != ERROR_SUCCESS, mR_InternalError);

  if (pNewlyCreated != nullptr)
    *pNewlyCreated = (disposition == REG_CREATED_NEW_KEY);

  mERROR_CHECK(mString_GetRequiredWideStringCount(value, &wPathOrValueCount));

  mERROR_CHECK(mAllocator_Reallocate(pAllocator, &wPathOrValue, wPathOrValueCount));
  mERROR_CHECK(mString_ToWideString(value, wPathOrValue, wPathOrValueCount));

  result = RegSetValueExW(key, wValueName, 0, REG_SZ, reinterpret_cast<const BYTE *>(wPathOrValue), (DWORD)wPathOrValueCount * sizeof(wchar_t));
  mERROR_IF(result != ERROR_SUCCESS, mR_InternalError);

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_SetRegistryKeyAccessibleToAllUsers, mPtr<mFileTransaction> &transaction, const mString &keyUrl)
{
  mFUNCTION_SETUP();

  mString subAddress;
  subAddress.pAllocator = &mDefaultTempAllocator;

  const char classesRoot[] = "HKEY_CLASSES_ROOT\\";
  const char currentConfig[] = "HKEY_CURRENT_CONFIG\\";
  const char currentUser[] = "HKEY_CURRENT_USER\\";
  const char localMachine[] = "HKEY_LOCAL_MACHINE\\";
  const char users[] = "HKEY_USERS\\";

  HKEY parentKey = nullptr;

  if (keyUrl.StartsWith(classesRoot))
  {
    parentKey = HKEY_CLASSES_ROOT;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(classesRoot) - 1));
  }
  else if (keyUrl.StartsWith(currentConfig))
  {
    parentKey = HKEY_CURRENT_CONFIG;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(currentConfig) - 1));
  }
  else if (keyUrl.StartsWith(currentUser))
  {
    parentKey = HKEY_CURRENT_USER;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(currentUser) - 1));
  }
  else if (keyUrl.StartsWith(localMachine))
  {
    parentKey = HKEY_LOCAL_MACHINE;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(localMachine) - 1));
  }
  else if (keyUrl.StartsWith(users))
  {
    parentKey = HKEY_USERS;

    mERROR_CHECK(mString_Substring(keyUrl, &subAddress, mARRAYSIZE(users) - 1));
  }
  else
  {
    mRETURN_RESULT(mR_InvalidParameter);
  }

  wchar_t *wPath = nullptr;
  size_t wPathCount = 0;
  mERROR_CHECK(mString_GetRequiredWideStringCount(subAddress, &wPathCount));

  mAllocator *pAllocator = &mDefaultTempAllocator;
  mDEFER(mAllocator_FreePtr(pAllocator, &wPath));
  mERROR_CHECK(mAllocator_AllocateZero(pAllocator, &wPath, wPathCount));

  mERROR_CHECK(mString_ToWideString(subAddress, wPath, wPathCount));

  HKEY key = nullptr;

  LSTATUS result = RegOpenKeyTransactedW(parentKey, wPath, 0, KEY_QUERY_VALUE, &key, transaction->transactionHandle, nullptr);
  mDEFER(if (key != nullptr) RegCloseKey(key));
  mERROR_IF(result != ERROR_SUCCESS, mR_ResourceNotFound);

  ACL *pOldDACL = nullptr;
  SECURITY_DESCRIPTOR *pSD = nullptr;

  mDEFER(
    LocalFree(pSD);
  );

  mERROR_IF(ERROR_SUCCESS != GetSecurityInfo(key, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, nullptr, nullptr, &pOldDACL, nullptr, reinterpret_cast<void **>(&pSD)), mR_InternalError);

  PSID pSid = nullptr;
  mDEFER(FreeSid(pSid));

  SID_IDENTIFIER_AUTHORITY authNt = SECURITY_NT_AUTHORITY;

  if (0 == AllocateAndInitializeSid(&authNt, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_USERS, 0, 0, 0, 0, 0, 0, &pSid))
  {
    const DWORD error = GetLastError();
    mUnused(error);

    mRETURN_RESULT(mR_InternalError);
  }

  EXPLICIT_ACCESS ea = { 0 };
  ea.grfAccessMode = GRANT_ACCESS;
  ea.grfAccessPermissions = GENERIC_ALL;
  ea.grfInheritance = CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE;
  ea.Trustee.TrusteeType = TRUSTEE_IS_GROUP;
  ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
  ea.Trustee.ptstrName = (LPTSTR)pSid;

  ACL *pNewDACL = nullptr;
  mDEFER(LocalFree(pNewDACL));
  mERROR_IF(ERROR_SUCCESS != SetEntriesInAclW(1, &ea, pOldDACL, &pNewDACL), mR_InternalError);

  if (pNewDACL)
    mERROR_IF(ERROR_SUCCESS != SetSecurityInfo(key, SE_REGISTRY_KEY, DACL_SECURITY_INFORMATION, nullptr, nullptr, pNewDACL, nullptr), mR_InternalError);

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

struct mFileTransacted_ProgressCallbackParams
{
  const std::function<mResult(const size_t transferred, const size_t total)> &func;
  BOOL *pCanceled;
  mResult innerResult;

  mFileTransacted_ProgressCallbackParams(BOOL *pCanceled, const std::function<mResult(const size_t transferred, const size_t total)> &func) :
    pCanceled(pCanceled),
    func(func)
  { }
};

struct mFileTransacted_ProgressCallback_Internal
{
  static DWORD WINAPI Callback(
    _In_     LARGE_INTEGER TotalFileSize,
    _In_     LARGE_INTEGER TotalBytesTransferred,
    _In_     LARGE_INTEGER /* StreamSize */,
    _In_     LARGE_INTEGER /* StreamBytesTransferred */,
    _In_     DWORD /* dwStreamNumber */,
    _In_     DWORD /* dwCallbackReason */,
    _In_     HANDLE /* hSourceFile */,
    _In_     HANDLE /* hDestinationFile */,
    _In_opt_ LPVOID lpData)
  {
    mFileTransacted_ProgressCallbackParams *pParams = reinterpret_cast<mFileTransacted_ProgressCallbackParams *>(lpData);

    if (mFAILED(pParams->innerResult = pParams->func((size_t)TotalBytesTransferred.QuadPart, (size_t)TotalFileSize.QuadPart)))
    {
      *pParams->pCanceled = TRUE;
      return PROGRESS_CANCEL;
    }

    return ERROR_SUCCESS;
  }
};

mFUNCTION(mFileTransaction_CopyFile, mPtr<mFileTransaction> &transaction, const mString &target, const mString &source, const bool replaceIfExistent, const std::function<mResult(const size_t transferred, const size_t total)> &progressCallback)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);
  mERROR_IF(target.hasFailed || source.hasFailed || target.bytes <= 1 || source.bytes <= 1, mR_InvalidParameter);

  wchar_t wSource[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(source, wSource, mARRAYSIZE(wSource)));

  wchar_t wTarget[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(target, wTarget, mARRAYSIZE(wSource)));

  BOOL canceled = FALSE;
  mFileTransacted_ProgressCallbackParams parameters(&canceled, progressCallback);

  if (0 == CopyFileTransactedW(wSource, wTarget, &mFileTransacted_ProgressCallback_Internal::Callback, reinterpret_cast<void *>(&parameters), &canceled, replaceIfExistent ? 0 : COPY_FILE_FAIL_IF_EXISTS, transaction->transactionHandle))
  {
    if (canceled == TRUE)
      mRETURN_RESULT(mFAILED(parameters.innerResult) ? parameters.innerResult : mR_Break);

    DWORD error = GetLastError();

    mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

    if (error == ERROR_REQUEST_ABORTED)
    {
      mRETURN_RESULT(mFAILED(parameters.innerResult) ? parameters.innerResult : mR_Break);
    }
    else if (error == ERROR_PATH_NOT_FOUND)
    {
      HANDLE fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
      error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.

      mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

      if (fileHandle == INVALID_HANDLE_VALUE && error == ERROR_PATH_NOT_FOUND)
      {
        wchar_t parentDirectory[MAX_PATH];
        mERROR_CHECK(mStringCopy(parentDirectory, mARRAYSIZE(parentDirectory), wTarget, MAX_PATH));

#if (NTDDI_VERSION >= NTDDI_WIN8)
        HRESULT hr = S_OK;

        // Requires `pathcch.h` && `Pathcch.lib`.
        mERROR_IF(FAILED(hr = PathCchRemoveFileSpec(parentDirectory, mARRAYSIZE(parentDirectory))), mR_InternalError);
        mERROR_IF(hr == S_FALSE, mR_InvalidParameter);
#else
        // deprecated since windows 8.
        mERROR_IF(PathRemoveFileSpecW(parentDirectory), mR_InvalidParameter);
#endif

        mERROR_CHECK(mFileTransaction_CreateDirectoryRecursive_Internal(transaction, parentDirectory));

        fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
        error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.
      }

      mERROR_IF(fileHandle == INVALID_HANDLE_VALUE, mR_InternalError);
      CloseHandle(fileHandle);

      canceled = FALSE;

      if (0 == CopyFileTransactedW(wSource, wTarget, &mFileTransacted_ProgressCallback_Internal::Callback, reinterpret_cast<void *>(&parameters), &canceled, 0, transaction->transactionHandle))
      {
        error = GetLastError();

        if (canceled == TRUE || error == ERROR_REQUEST_ABORTED)
          mRETURN_RESULT(mFAILED(parameters.innerResult) ? parameters.innerResult : mR_Break);

        mRETURN_RESULT(mR_ResourceNotFound);
      }
      else
      {
        mRETURN_SUCCESS();
      }
    }

    mRETURN_RESULT(mR_IOFailure);
  }

  mRETURN_SUCCESS();
}

mFUNCTION(mFileTransaction_MoveFile, mPtr<mFileTransaction> &transaction, const mString &target, const mString &source, const bool replaceIfExistent, const std::function<mResult(const size_t transferred, const size_t total)> &progressCallback)
{
  mFUNCTION_SETUP();

  mERROR_IF(transaction == nullptr, mR_ArgumentNull);
  mERROR_IF(transaction->hasBeenTransacted, mR_ResourceStateInvalid);
  mERROR_IF(target.hasFailed || source.hasFailed || target.bytes <= 1 || source.bytes <= 1, mR_InvalidParameter);

  wchar_t wSource[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(source, wSource, mARRAYSIZE(wSource)));

  wchar_t wTarget[MAX_PATH];
  mERROR_CHECK(mString_ToWideString(target, wTarget, mARRAYSIZE(wSource)));

  BOOL canceled = FALSE;
  mFileTransacted_ProgressCallbackParams parameters(&canceled, progressCallback);

  if (0 == MoveFileTransactedW(wSource, wTarget, &mFileTransacted_ProgressCallback_Internal::Callback, reinterpret_cast<void *>(&parameters), MOVEFILE_COPY_ALLOWED | (replaceIfExistent ? MOVEFILE_REPLACE_EXISTING : 0), transaction->transactionHandle))
  {
    DWORD error = GetLastError();

    mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

    if (error == ERROR_REQUEST_ABORTED)
    {
      mRETURN_RESULT(mFAILED(parameters.innerResult) ? parameters.innerResult : mR_Break);
    }
    else if (error == ERROR_PATH_NOT_FOUND)
    {
      HANDLE fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
      error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.

      mERROR_IF(error == ERROR_ALREADY_EXISTS && !replaceIfExistent, mR_Failure);

      if (fileHandle == INVALID_HANDLE_VALUE && error == ERROR_PATH_NOT_FOUND)
      {
        wchar_t parentDirectory[MAX_PATH];
        mERROR_CHECK(mStringCopy(parentDirectory, mARRAYSIZE(parentDirectory), wTarget, MAX_PATH));

#if (NTDDI_VERSION >= NTDDI_WIN8)
        HRESULT hr = S_OK;

        // Requires `pathcch.h` && `Pathcch.lib`.
        mERROR_IF(FAILED(hr = PathCchRemoveFileSpec(parentDirectory, mARRAYSIZE(parentDirectory))), mR_InternalError);
        mERROR_IF(hr == S_FALSE, mR_InvalidParameter);
#else
        // deprecated since windows 8.
        mERROR_IF(PathRemoveFileSpecW(parentDirectory), mR_InvalidParameter);
#endif

        mERROR_CHECK(mFileTransaction_CreateDirectoryRecursive_Internal(transaction, parentDirectory));

        fileHandle = CreateFileTransactedW(wTarget, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr, transaction->transactionHandle, nullptr, nullptr);
        error = GetLastError(); // Might be `ERROR_ALREADY_EXISTS` even if the `fileHandle` is valid.
      }

      mERROR_IF(fileHandle == INVALID_HANDLE_VALUE, mR_InternalError);
      CloseHandle(fileHandle);

      if (0 == MoveFileTransactedW(wSource, wTarget, &mFileTransacted_ProgressCallback_Internal::Callback, reinterpret_cast<void *>(&parameters), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING, transaction->transactionHandle))
      {
        error = GetLastError();

        if (error == ERROR_REQUEST_ABORTED)
          mRETURN_RESULT(mFAILED(parameters.innerResult) ? parameters.innerResult : mR_Break);

        mRETURN_RESULT(mR_ResourceNotFound);
      }
      else
      {
        mRETURN_SUCCESS();
      }
    }

    mRETURN_RESULT(mR_IOFailure);
  }

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mFileTransaction_Destroy_Internal, IN_OUT mFileTransaction *pTransaction)
{
  mFUNCTION_SETUP();

  mERROR_IF(pTransaction == nullptr, mR_ArgumentNull);

  CloseHandle(pTransaction->transactionHandle);

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mFileTransaction_CreateDirectoryRecursive_Internal, mPtr<mFileTransaction> &transaction, const wchar_t *directoryPath)
{
  mFUNCTION_SETUP();

  if (0 == CreateDirectoryTransactedW(nullptr, directoryPath, nullptr, transaction->transactionHandle))
  {
    DWORD error = GetLastError();

    if (error == ERROR_ALREADY_EXISTS)
    {
      // Do nothing. We will return success later.
    }
    else if (error == ERROR_PATH_NOT_FOUND)
    {
      wchar_t parentDirectory[MAX_PATH];
      mERROR_CHECK(mStringCopy(parentDirectory, mARRAYSIZE(parentDirectory), directoryPath, MAX_PATH));

#if (NTDDI_VERSION >= NTDDI_WIN8)
      HRESULT hr = S_OK;

      // Requires `pathcch.h` && `Pathcch.lib`.
      mERROR_IF(FAILED(hr = PathCchRemoveFileSpec(parentDirectory, mARRAYSIZE(parentDirectory))), mR_InternalError);
      mERROR_IF(hr == S_FALSE, mR_InvalidParameter);
#else
      // deprecated since windows 8.
      mERROR_IF(PathRemoveFileSpecW(parentDirectory), mR_InvalidParameter);
#endif

      mERROR_CHECK(mFileTransaction_CreateDirectoryRecursive_Internal(transaction, parentDirectory));

      if (0 == CreateDirectoryTransactedW(nullptr, directoryPath, nullptr, transaction->transactionHandle))
      {
        error = GetLastError();

        mERROR_IF(error != ERROR_ALREADY_EXISTS, mR_InternalError);
      }
    }
    else
    {
      mRETURN_RESULT(mR_InternalError);
    }
  }

  mRETURN_SUCCESS();
}
