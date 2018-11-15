#include "mFile.h"

#include <sys\stat.h>

#include <shobjidl.h>
#include <shlobj.h>
#include <knownfolders.h>

mFUNCTION(mFile_Exists, const mString &filename, OUT bool *pExists)
{
  mFUNCTION_SETUP();

  mERROR_IF(pExists == nullptr, mR_ArgumentNull);

  std::wstring wfilename;
  mERROR_CHECK(mString_ToWideString(filename, &wfilename));

  mERROR_CHECK(mFile_Exists(wfilename, pExists));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_Exists, const std::wstring &filename, OUT bool *pExists)
{
  mFUNCTION_SETUP();

  mERROR_IF(pExists == nullptr, mR_ArgumentNull);

  struct _stat buffer;

  *pExists = (_wstat(filename.c_str(), &buffer) == 0);

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_ReadAllBytes, const std::wstring &filename, IN OPTIONAL mAllocator *pAllocator, OUT mArray<uint8_t> *pBytes)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_ReadAllItems(filename, pAllocator, pBytes));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_ReadAllText, const std::wstring &filename, IN OPTIONAL mAllocator *pAllocator, OUT std::string *pText, const mFile_Encoding /* encoding = mF_E_ASCII */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pText == nullptr, mR_ArgumentNull);

  size_t count = 0;
  char *text = nullptr;

  mDEFER(mAllocator_FreePtr(nullptr, &text));
  mERROR_CHECK(mFile_ReadRaw(filename, &text, pAllocator, &count));
  text[count] = '\0';
  *pText = std::string(text);
  mERROR_CHECK(mAllocator_FreePtr(pAllocator, &text));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_ReadAllText, const mString & filename, IN OPTIONAL mAllocator * pAllocator, OUT mString *pText, const mFile_Encoding /* encoding = mF_E_ASCII */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pText == nullptr, mR_ArgumentNull);

  std::wstring wstring;
  mERROR_CHECK(mString_ToWideString(filename, &wstring));

  size_t count = 0;
  char *text = nullptr;

  mDEFER(mAllocator_FreePtr(nullptr, &text));
  mERROR_CHECK(mFile_ReadRaw(wstring, &text, pAllocator, &count));
  text[count] = '\0';
  *pText = mString(text);
  mERROR_CHECK(mAllocator_FreePtr(pAllocator, &text));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_WriteAllBytes, const std::wstring &filename, mArray<uint8_t> &bytes)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_WriteRaw(filename, bytes.pData, bytes.count));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_WriteAllText, const std::wstring &filename, const std::string &text, const mFile_Encoding /* encoding = mF_E_ASCII */)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_WriteRaw(filename, text.c_str(), text.length()));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_WriteAllText, const mString &filename, const mString &text, const mFile_Encoding /* encoding = mF_E_ASCII */)
{
  mFUNCTION_SETUP();

  std::wstring wstring;
  mERROR_CHECK(mString_ToWideString(filename, &wstring));

  mERROR_CHECK(mFile_WriteRaw(wstring, text.c_str(), text.bytes - 1));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_CreateDirectory, const mString &folderPath)
{
  mFUNCTION_SETUP();

  mERROR_IF(folderPath.hasFailed, mR_InvalidParameter);

  mString path;
  mERROR_CHECK(mString_ToDirectoryPath(&path, folderPath));

  mString pathWithoutLastSlash;
  mERROR_CHECK(mString_Substring(path, &pathWithoutLastSlash, 0, path.Count() - 2));

  std::wstring directoryName;
  mERROR_CHECK(mString_ToWideString(pathWithoutLastSlash, &directoryName));

  if (0 == CreateDirectoryW(directoryName.c_str(), NULL))
  {
    const DWORD error = GetLastError();

    switch (error)
    {
    case ERROR_ALREADY_EXISTS:
      break;

    case ERROR_PATH_NOT_FOUND:
      mERROR_IF(true, mR_ResourceNotFound);
      break;

    default:
      mERROR_IF(true, mR_InternalError);
      break;
    }
  }

  mRETURN_SUCCESS();
}

HRESULT CreateAndInitializeFileOperation(REFIID riid, void **ppFileOperation)
{
  *ppFileOperation = nullptr;

  // Create the IFileOperation object
  IFileOperation *pfo;

  HRESULT hr = CoCreateInstance(__uuidof(FileOperation), NULL, CLSCTX_ALL, IID_PPV_ARGS(&pfo));

  if (SUCCEEDED(hr))
  {
    // Set the operation flags. Turn off all UI from being shown to the user during the operation. This includes error, confirmation and progress dialogs.
    hr = pfo->SetOperationFlags(FOF_NO_UI);

    if (SUCCEEDED(hr))
    {
      hr = pfo->QueryInterface(riid, ppFileOperation);
    }

    pfo->Release();
  }

  return hr;
}

mFUNCTION(mFile_DeleteFolder, const mString &folderPath)
{
  mFUNCTION_SETUP();

  mERROR_IF(folderPath.hasFailed, mR_InvalidParameter);

  mString path;
  mERROR_CHECK(mString_ToDirectoryPath(&path, folderPath));

  mString pathWithoutLastSlash;
  mERROR_CHECK(mString_Substring(path, &pathWithoutLastSlash, 0, path.Count() - 2));

  std::wstring directoryName;
  mERROR_CHECK(mString_ToWideString(pathWithoutLastSlash, &directoryName));

  HRESULT hr = S_OK;

  wchar_t absolutePath[1024 * 4];
  DWORD length = GetFullPathNameW(directoryName.c_str(), mARRAYSIZE(absolutePath), absolutePath, nullptr);
  mUnused(length);

  IShellItem *pItem = nullptr;
  mERROR_IF(FAILED(hr = SHCreateItemFromParsingName(absolutePath, nullptr, IID_PPV_ARGS(&pItem))), mR_InternalError);
  mDEFER(pItem->Release());

  IFileOperation *pFileOperation = nullptr;
  mERROR_IF(FAILED(hr = CreateAndInitializeFileOperation(IID_PPV_ARGS(&pFileOperation))), mR_InternalError);
  mDEFER(pFileOperation->Release());

  mERROR_IF(FAILED(hr = pFileOperation->DeleteItem(pItem, nullptr)), mR_InternalError);
  mERROR_IF(FAILED(hr = pFileOperation->PerformOperations()), mR_InternalError);

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_Copy, const mString &destinationFileName, const mString &sourceFileName, const bool overrideFileIfExistent /* = false */)
{
  mFUNCTION_SETUP();

  std::wstring source;
  mERROR_CHECK(mString_ToWideString(sourceFileName, &source));

  std::wstring target;
  mERROR_CHECK(mString_ToWideString(destinationFileName, &target));

  const BOOL succeeded = CopyFileW(source.c_str(), target.c_str(), !overrideFileIfExistent);
  mERROR_IF(succeeded, mR_Success);

  const DWORD errorCode = GetLastError();

  switch (errorCode)
  {
  case ERROR_ACCESS_DENIED:
    mRETURN_RESULT(mR_ResourceAlreadyExists);

  default:
    mRETURN_RESULT(mR_Failure);
  }
}

mFUNCTION(mFile_Move, const mString &destinationFileName, const mString &sourceFileName, const bool overrideFileIfExistent /* = true */)
{
  mFUNCTION_SETUP();

  bool destinationFileExists = false;
  mERROR_CHECK(mFile_Exists(destinationFileName, &destinationFileExists));

  if (destinationFileExists)
  {
    mERROR_IF(!overrideFileIfExistent, mR_ResourceAlreadyExists);
    mERROR_CHECK(mFile_Delete(destinationFileName));
  }

  std::wstring source;
  mERROR_CHECK(mString_ToWideString(sourceFileName, &source));

  std::wstring target;
  mERROR_CHECK(mString_ToWideString(destinationFileName, &target));

  const BOOL succeeded = MoveFileW(source.c_str(), target.c_str());
  mERROR_IF(succeeded, mR_Success);

  const DWORD errorCode = GetLastError();
  mUnused(errorCode);

  mRETURN_RESULT(mR_Failure);
}

mFUNCTION(mFile_Delete, const mString &fileName)
{
  mFUNCTION_SETUP();

  std::wstring wfile;
  mERROR_CHECK(mString_ToWideString(fileName, &wfile));

  const BOOL succeeded = DeleteFileW(wfile.c_str());
  mERROR_IF(succeeded, mR_Success);

  const DWORD errorCode = GetLastError();

  switch (errorCode)
  {
  case ERROR_FILE_NOT_FOUND:
    mRETURN_RESULT(mR_ResourceNotFound);

  default:
    mRETURN_RESULT(mR_Failure);
  }
}

mFUNCTION(mFile_GetTempDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_IF(pString == nullptr, mR_ArgumentNull);
  
  wchar_t buffer[MAX_PATH + 1];
  const DWORD length = GetTempPathW(mARRAYSIZE(buffer), buffer);

  mERROR_IF(length == 0, mR_InternalError); // This would theoretically set GetLastError() to some error code, however msdn doesn't list any possible error codes.

  mERROR_CHECK(mString_Create(pString, buffer, length, pString->pAllocator)); // According to msdn this is terminated by a backslash.

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetKnownPath_Internal, OUT mString *pString, const GUID &guid)
{
  mFUNCTION_SETUP();

  mERROR_IF(pString == nullptr, mR_ArgumentNull);

  wchar_t *pBuffer = nullptr;

  mDEFER(
    if (pBuffer != nullptr)
      CoTaskMemFree(pBuffer)
      );

  HRESULT result = S_OK;
  mERROR_IF(FAILED(result = SHGetKnownFolderPath(guid, 0, nullptr, &pBuffer)), mR_InternalError);

  mString tempString;
  mERROR_CHECK(mString_Create(&tempString, pBuffer, &mDefaultTempAllocator));
  mERROR_CHECK(mString_ToDirectoryPath(pString, tempString));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetAppDataDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_GetKnownPath_Internal(pString, FOLDERID_RoamingAppData));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetDesktopDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_GetKnownPath_Internal(pString, FOLDERID_Desktop));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetDocumentsDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_GetKnownPath_Internal(pString, FOLDERID_Documents));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetFontsDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_GetKnownPath_Internal(pString, FOLDERID_Fonts));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetCurrentUserDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_GetKnownPath_Internal(pString, FOLDERID_Profile));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetProgramFilesDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_GetKnownPath_Internal(pString, FOLDERID_ProgramFiles));

  mRETURN_SUCCESS();
}

mFUNCTION(mFile_GetStartupDirectory, OUT mString *pString)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mFile_GetKnownPath_Internal(pString, FOLDERID_Startup));

  mRETURN_SUCCESS();
}
