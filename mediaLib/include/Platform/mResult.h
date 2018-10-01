#ifndef mResult_h__
#define mResult_h__

#include <functional>
#include <type_traits>
#include <windows.h>
#include <debugapi.h>
#include <stdarg.h>

enum mResult
{
  mR_Success,

  mR_Break,

  mR_InvalidParameter,
  mR_ArgumentNull,
  mR_InternalError,
  mR_MemoryAllocationFailure,
  mR_NotImplemented,
  mR_NotInitialized,
  mR_IndexOutOfBounds,
  mR_ArgumentOutOfBounds,
  mR_Timeout,
  mR_OperationNotSupported,
  mR_ResourceNotFound,
  mR_ResourceInvalid,
  mR_ResourceStateInvalid,
  mR_ResourceIncompatible,
  mR_EndOfStream,
  mR_RenderingError,
  mR_Failure,
  mR_NotSupported,

  mResult_Count
};

#define mSTDRESULT __result
#define mSUCCEEDED(result) ((result) == mR_Success)
#define mFAILED(result) (!mSUCCEEDED(result))

#define mFUNCTION(FunctionName, ...) mResult FunctionName(__VA_ARGS__)
#define mFUNCTION_SETUP() mResult mSTDRESULT = mR_Success; mUnused(mSTDRESULT)
#define mRETURN_RESULT(result) do { mSTDRESULT = result; return mSTDRESULT; } while (0)
#define mRETURN_SUCCESS() mRETURN_RESULT(mR_Success)

extern const char *g_mResult_lastErrorFile;
extern size_t g_mResult_lastErrorLine;
extern mResult g_mResult_lastErrorResult;

extern bool g_mResult_breakOnError;

#ifdef _DEBUG
#define mBREAK_ON_FAILURE true
#else
#define mBREAK_ON_FAILURE false
#endif // _DEBUG

void mDebugOut(const char *format, ...);
void mPrintError(char *function, char *file, const int32_t line, const mResult error, const char *expression);

mFUNCTION(mResult_ToString, const mResult result, OUT struct mString *pString);

void mDeinit();
void mDeinit(const std::function<void(void)> &param);
template <typename ...Args> void mDeinit(const std::function<void(void)> &param, Args && ...args) { if (param) { param(); } mDeinit(std::forward<Args>(args)); };

#define mERROR_CHECK(functionCall, ...) \
  do \
  { mSTDRESULT = (functionCall); \
    if (mFAILED(mSTDRESULT)) \
    { g_mResult_lastErrorResult = mSTDRESULT; \
      g_mResult_lastErrorFile = __FILE__; \
      g_mResult_lastErrorLine = __LINE__; \
      mPrintError(__FUNCTION__, __FILE__, __LINE__, mSTDRESULT, #functionCall); \
      mDeinit(__VA_ARGS__); \
      if (g_mResult_breakOnError && mBREAK_ON_FAILURE) \
      { __debugbreak(); \
      } \
      mRETURN_RESULT(mSTDRESULT); \
    } \
  } while (0)

#define mERROR_IF(conditional, resultOnError, ...) \
  do \
  { if (conditional) \
    { mSTDRESULT = (resultOnError); \
      if (mFAILED(mSTDRESULT)) \
      { g_mResult_lastErrorResult = mSTDRESULT; \
        g_mResult_lastErrorFile = __FILE__; \
        g_mResult_lastErrorLine = __LINE__; \
        mPrintError(__FUNCTION__, __FILE__, __LINE__, mSTDRESULT, #conditional); \
        mDeinit(__VA_ARGS__); \
        if (g_mResult_breakOnError && mBREAK_ON_FAILURE) \
        { __debugbreak(); \
        } \
      } \
      mRETURN_RESULT(mSTDRESULT); \
    } \
  } while (0)

#define mERROR_CHECK_GOTO(functionCall, result, label, ...) \
  do \
  { result = (functionCall); \
    if (mFAILED(result)) \
    { g_mResult_lastErrorResult = result; \
      g_mResult_lastErrorFile = __FILE__; \
      g_mResult_lastErrorLine = __LINE__; \
      mDeinit(__VA_ARGS__); \
      if (g_mResult_breakOnError && mBREAK_ON_FAILURE) \
      { __debugbreak(); \
      } \
      goto label; \
    } \
  } while (0)

#define mERROR_IF_GOTO(conditional, resultOnError, result, label, ...) \
  do \
  { if (conditional) \
    { result = (resultOnError); \
      if (mFAILED(result)) \
      { g_mResult_lastErrorResult = result; \
        g_mResult_lastErrorFile = __FILE__; \
        g_mResult_lastErrorLine = __LINE__; \
        mDeinit(__VA_ARGS__); \
        if (g_mResult_breakOnError && mBREAK_ON_FAILURE) \
        { __debugbreak(); \
        } \
      } \
      goto label; \
    } \
  } while (0)

#endif // mResult_h__
