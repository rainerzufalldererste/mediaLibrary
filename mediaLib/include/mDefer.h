// Copyright 2018 Christoph Stiller
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef mDefer_h__
#define mDefer_h__

#include <functional>
#include <type_traits>
#include "mResult.h"


template <typename T>
class mDefer
{
public:
  typedef void OnExitFuncVoid();
  typedef void OnExitFuncT(T *);
  typedef mResult OnExitFuncResultVoid();
  typedef mResult OnExitFuncResultT(T *);

  mDefer();
  mDefer(const std::function<void()> &onExit, const mResult *pResult = nullptr);
  mDefer(std::function<void()> &&onExit, const mResult *pResult = nullptr);
  mDefer(const std::function<void(T*)> &onExit, T *pData, const mResult *pResult = nullptr);
  mDefer(std::function<void(T*)> &&onExit, T *pData, const mResult *pResult = nullptr);
  mDefer(OnExitFuncVoid *pOnExit, const mResult *pResult = nullptr);
  mDefer(OnExitFuncT *pOnExit, T *pData, const mResult *pResult = nullptr);
  mDefer(OnExitFuncResultVoid *pOnExit, const mResult *pResult = nullptr);
  mDefer(OnExitFuncResultT *pOnExit, T *pData, const mResult *pResult = nullptr);
  ~mDefer();

  mDefer<T>& operator = (const mDefer<T> &copy) = delete;
  mDefer<T>& operator = (mDefer<T> &&move);
  

private:
  enum mDeferType
  {
    mDT_None,
    mDT_LambdaVoid,
    mDT_FuctionPointerVoid,
    mDT_FuctionPointerResultVoid,
    mDT_LambdaTPtr,
    mDT_FuctionPointerTPtr,
    mDT_FuctionPointerResultTPtr,
  };

  mDeferType m_deferType;
  T *m_pData;
  const mResult *m_pResult;

  std::function<void()> m_onExitLV;
  std::function<void(T *)> m_onExitLP;

  union
  {
    OnExitFuncVoid *m_pOnExitFV;
    OnExitFuncT *m_pOnExitFP;
    OnExitFuncResultVoid *m_pOnExitFRV;
    OnExitFuncResultT *m_pOnExitFRP;
  };
};

mDefer<void>&& mDefer_Create(const std::function<void()> &onExit, const mResult *pResult = nullptr);
mDefer<void>&& mDefer_Create(std::function<void()> &&onExit, const mResult *pResult = nullptr);

template <typename T>
mDefer<T>&& mDefer_Create(const std::function<void(T *)> &onExit, T *pData, const mResult *pResult = nullptr);

template <typename T>
mDefer<T>&& mDefer_Create(std::function<void(T *)> &&onExit, T *pData, const mResult *pResult = nullptr);

mDefer<void>&& mDefer_Create(mDefer<void>::OnExitFuncVoid *pOnExit, const mResult *pResult = nullptr);

template <typename T>
mDefer<T>&& mDefer_Create(typename mDefer<T>::OnExitFuncT *pOnExit, T *pData, const mResult *pResult = nullptr);

mDefer<void>&& mDefer_Create(mDefer<void>::OnExitFuncResultVoid *pOnExit, const mResult *pResult = nullptr);

template <typename T>
mDefer<T>&& mDefer_Create(typename mDefer<T>::OnExitFuncResultT *pOnExit, T *pData, const mResult *pResult = nullptr);

template<typename T>
inline mDefer<T>::mDefer()
{
  m_deferType = mDeferType::mDT_None;
  m_pData = nullptr;
  m_pResult = nullptr;
}

template<typename T>
inline mDefer<T>::mDefer(const std::function<void()> &onExit, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_LambdaVoid;
  m_pData = nullptr;
  m_onExitLV = onExit;
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::mDefer(std::function<void()> &&onExit, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_LambdaVoid;
  m_pData = nullptr;
  m_onExitLV = std::move(onExit);
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::mDefer(const std::function<void(T*)> &onExit, T* pData, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_LambdaTPtr;
  m_pData = pData;
  m_onExitLT = onExit;
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::mDefer(std::function<void(T*)> &&onExit, T* pData, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_LambdaTPtr;
  m_pData = pData;
  m_onExitLT = std::move(onExit);
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::mDefer(OnExitFuncVoid *pOnExit, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_FuctionPointerVoid;
  m_pData = nullptr;
  m_pOnExitFV = pOnExit;
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::mDefer(OnExitFuncT *pOnExit, T *pData, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_FuctionPointerTPtr;
  m_pData = pData;
  m_pOnExitFP = pOnExit;
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::mDefer(OnExitFuncResultVoid *pOnExit, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_FuctionPointerResultVoid;
  m_pData = nullptr;
  m_pOnExitFRV = pOnExit;
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::mDefer(OnExitFuncResultT *pOnExit, T *pData, const mResult *pResult /* = nullptr */)
{
  m_deferType = mDeferType::mDT_FuctionPointerResultTPtr;
  m_pData = pData;
  m_pOnExitFRP = pOnExit;
  m_pResult = pResult;
}

template<typename T>
inline mDefer<T>::~mDefer()
{
  if (m_pResult != nullptr)
    if (mSUCCEEDED(*m_pResult))
      return;

  switch (m_deferType)
  {
  case mDefer::mDeferType::mDT_LambdaVoid:
    if (m_onExitLV)
      m_onExitLV();
    break;

  case mDefer::mDeferType::mDT_LambdaTPtr:
    if (m_onExitLP)
      m_onExitLP(m_pData);
    break;

  case mDefer::mDeferType::mDT_FuctionPointerVoid:
    if (m_pOnExitFV)
      (*m_pOnExitFV)();
    break;

  case mDefer::mDeferType::mDT_FuctionPointerTPtr:
    if (m_pOnExitFP)
      (*m_pOnExitFP)(m_pData);
    break;

  case mDefer::mDeferType::mDT_FuctionPointerResultVoid:
    if (m_pOnExitFRV)
      (*m_pOnExitFRV)();
    break;

  case mDefer::mDeferType::mDT_FuctionPointerResultTPtr:
    if (m_pOnExitFRP)
      (*m_pOnExitFRP)(m_pData);
    break;

  case mDefer::mDeferType::mDT_None:
  default:
    break;
  }
}

template<typename T>
inline mDefer<T>& mDefer<T>::operator = (mDefer<T> &&move)
{
  m_deferType = move.m_deferType;
  m_pData = move.m_pData;
  m_pResult = move.m_pResult;

  m_onExitLV = move.m_onExitLV;
  m_onExitLP = move.m_pOnExitFP;
  
  // assign union:
  m_pOnExitFP = move.m_pOnExitFP;

  move.m_deferType = mDT_None;
  move.m_pResult = nullptr;
  move.m_pData = nullptr;
  move.m_onExitLP = nullptr;
  move.m_onExitLV = nullptr;
  move.m_pOnExitFP = nullptr;
}

template<typename T>
inline mDefer<T>&& mDefer_Create(const std::function<void(T*)> &onExit, T *pData, const mResult *pResult /* = nullptr */)
{
  return std::forward<mDefer<T>>(mDefer<T>(onExit, pData, pResult));
}

template<typename T>
inline mDefer<T>&& mDefer_Create(std::function<void(T*)> &&onExit, T *pData, const mResult *pResult /* = nullptr */)
{
  return std::forward<mDefer<T>>(mDefer<T>(std::forward<std::function<void(T*)>>(onExit), pData, pResult));
}

template<typename T>
inline mDefer<T>&& mDefer_Create(typename mDefer<T>::OnExitFuncT *pOnExit, T *pData, const mResult *pResult /* = nullptr */)
{
  return std::forward<mDefer<T>>(mDefer<T>(pOnExit, pData, pResult));
}

template<typename T>
inline mDefer<T>&& mDefer_Create(typename mDefer<T>::OnExitFuncResultT *pOnExit, T *pData, const mResult *pResult /* = nullptr */)
{
  return std::forward<mDefer<T>>(mDefer<T>(pOnExit, pData, pResult));
}

#define mCOMBINE_LITERALS(x, y) x ## y
#define mCOMBINE_LITERALS_INDIRECTION(x, y) mCOMBINE_LITERALS(x, y)

#ifdef __COUNTER__
#define mDEFER(...) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __COUNTER__) = std::move(mDefer_Create([&](){ __VA_ARGS__; }))
#define mDEFER_IF(conditional, ...) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __COUNTER__) = std::move(mDefer_Create([&](){ if (conditional) { __VA_ARGS__; } }))
#define mDEFER_ON_ERROR(...) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __COUNTER__) = std::move(mDefer_Create([&](){ { __VA_ARGS__; } }, &(mSTDRESULT)))
#define mDEFER_DESTRUCTION(Ressource, DestructionFunction) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __COUNTER__) = std::move(mDefer_Create((DestructionFunction), (Ressource)))
#define mDEFER_DESTRUCTION_ON_ERROR(Ressource, DestructionFunction) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __COUNTER__) = std::move(mDefer_Create((DestructionFunction), (Ressource), &(mSTDRESULT)))
#else
#define mDEFER(...) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __LINE__) = std::move(mDefer_Create([&](){ __VA_ARGS__; }))
#define mDEFER_IF(conditional, ...) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __LINE__) = std::move(mDefer_Create([&](){ if (conditional) { __VA_ARGS__; } }))
#define mDEFER_ON_ERROR(...) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __LINE__) = std::move(mDefer_Create([&](){ { __VA_ARGS__; } }, &(mSTDRESULT)))
#define mDEFER_DESTRUCTION(Ressource, DestructionFunction) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __LINE__) = std::move(mDefer_Create((DestructionFunction), (Ressource)))
#define mDEFER_DESTRUCTION_ON_ERROR(Ressource, DestructionFunction) const auto mCOMBINE_LITERALS_INDIRECTION(__defer__, __LINE__) = std::move(mDefer_Create((DestructionFunction), (Ressource), &(mSTDRESULT)))
#endif

#endif // mDefer_h__
