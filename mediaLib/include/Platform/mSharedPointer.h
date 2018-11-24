#ifndef mSharedPointer_h__
#define mSharedPointer_h__

#include "mediaLib.h"

struct mAllocator;

#define mSHARED_POINTER_FOREIGN_RESOURCE ((mAllocator *)-1)

template <typename T>
class mSharedPointer
{
public:
  mSharedPointer();
  mSharedPointer(nullptr_t);
  mSharedPointer(const mSharedPointer<T> &copy);
  mSharedPointer(mSharedPointer<T> &&move);
  ~mSharedPointer();

  mSharedPointer<T>& operator = (const mSharedPointer<T> &copy);
  mSharedPointer<T>& operator = (mSharedPointer<T> &&move);

  T* operator -> ();
  const T* operator -> () const;
  T& operator * ();
  const T& operator * () const;

  bool operator ==(const mSharedPointer<T> &other) const;
  bool operator ==(nullptr_t) const;
  bool operator !=(const mSharedPointer<T> &other) const;
  bool operator !=(nullptr_t) const;

  operator bool() const;
  bool operator !() const;

  template <typename T2, typename std::enable_if_t<std::is_convertible<T2, bool>::value, int>* = nullptr >
  operator T2() const;

  const T* GetPointer() const;
  T* GetPointer();
  size_t GetReferenceCount() const;

  T *m_pData;
  
  struct PointerParams
  {
    volatile size_t referenceCount;
    uint8_t freeResource : 1;
    uint8_t freeParameters : 1;
    mAllocator *pAllocator;
    std::function<void (T*)> cleanupFunction;
    void *pUserData;
  } *m_pParams;
};

template <typename T>
using mPtr = mSharedPointer<T>;

template <typename T> void mDeinit(mPtr<T> *pParam) { if (pParam != nullptr) { mSharedPointer_Destroy(pParam); } };
template <typename T, typename ...Args> void mDeinit(mPtr<T> *pParam, Args... args) { if (pParam != nullptr) { mSharedPointer_Destroy(pParam); } mDeinit(std::forward<Args>(args)); };

template <typename T>
inline mFUNCTION(mSharedPointer_Create, OUT mSharedPointer<T> *pOutSharedPointer, IN T *pData, std::function<void(T *pData)> cleanupFunction, IN mAllocator *pAllocator)
{
  mFUNCTION_SETUP();
  mERROR_IF(pOutSharedPointer == nullptr, mR_ArgumentNull);

  if (*pOutSharedPointer != nullptr)
    *pOutSharedPointer = nullptr;

  mERROR_CHECK(mAllocator_AllocateZero(pAllocator == mSHARED_POINTER_FOREIGN_RESOURCE ? nullptr : pAllocator, &pOutSharedPointer->m_pParams, 1));
  pOutSharedPointer->m_pParams = new (pOutSharedPointer->m_pParams) typename mSharedPointer<T>::PointerParams();
  pOutSharedPointer->m_pParams->referenceCount = 1;
  pOutSharedPointer->m_pParams->pAllocator = pAllocator == mSHARED_POINTER_FOREIGN_RESOURCE ? nullptr : pAllocator;
  pOutSharedPointer->m_pParams->freeResource = (pAllocator != mSHARED_POINTER_FOREIGN_RESOURCE);
  pOutSharedPointer->m_pParams->freeParameters = true;
  pOutSharedPointer->m_pParams->cleanupFunction = cleanupFunction;
  pOutSharedPointer->m_pData = pData;
  pOutSharedPointer->m_pParams->pUserData = nullptr;

  mRETURN_SUCCESS();
}

template <typename T>
inline mFUNCTION(mSharedPointer_Create, OUT mSharedPointer<T> *pOutSharedPointer, IN T *pData, IN mAllocator *pAllocator)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mSharedPointer_Create(pOutSharedPointer, pData, std::function<void(T *pData)>(nullptr), pAllocator));
  mRETURN_SUCCESS();
}

template <typename T>
inline mFUNCTION(mSharedPointer_Allocate, OUT mSharedPointer<T> *pOutSharedPointer, IN mAllocator *pAllocator, const size_t count = 1)
{
  mFUNCTION_SETUP();

  mERROR_IF(pOutSharedPointer == nullptr, mR_ArgumentNull);

  T *pData = nullptr;
  mDEFER(mAllocator_FreePtr(pAllocator, &pData));
  mERROR_CHECK(mAllocator_AllocateZero(pAllocator, &pData, count));
  mERROR_CHECK(mSharedPointer_Create(pOutSharedPointer, pData, pAllocator));
  pData = nullptr; // to not get released on destruction.

  mRETURN_SUCCESS();
}

template <typename T>
inline mFUNCTION(mSharedPointer_Allocate, OUT mSharedPointer<T> *pOutSharedPointer, IN mAllocator *pAllocator, const std::function<void(T *)> &function, const size_t count)
{
  mFUNCTION_SETUP();

  mERROR_IF(pOutSharedPointer == nullptr, mR_ArgumentNull);

  T *pData = nullptr;
  mDEFER(mAllocator_FreePtr(pAllocator, &pData));
  mERROR_CHECK(mAllocator_AllocateZero(pAllocator, &pData, count));
  mERROR_CHECK(mSharedPointer_Create(pOutSharedPointer, pData, function, pAllocator));
  pData = nullptr; // to not get released on destruction.

  mRETURN_SUCCESS();
}

template <typename T, typename TInherited>
inline mFUNCTION(mSharedPointer_AllocateInherited, OUT mSharedPointer<T> *pOutSharedPointer, IN mAllocator *pAllocator, const std::function<void(TInherited *)> &function, OUT OPTIONAL TInherited **ppInherited)
{
  mFUNCTION_SETUP();

  mERROR_IF(pOutSharedPointer == nullptr, mR_ArgumentNull);

  TInherited *pData = nullptr;
  mDEFER(mAllocator_FreePtr(pAllocator, &pData));
  mERROR_CHECK(mAllocator_AllocateZero(pAllocator, &pData, 1));
  mERROR_CHECK(mSharedPointer_Create(pOutSharedPointer, (T *)pData, (std::function<void(T *)>)[function](T *pDestructData) { function((TInherited *)pDestructData); }, pAllocator));
  
  if (ppInherited != nullptr)
    *ppInherited = pData;

  pData = nullptr; // to not get released on destruction.

  mRETURN_SUCCESS();
}

template <typename T>
inline mFUNCTION(mSharedPointer_CreateInplace, IN_OUT mSharedPointer<T> *pOutSharedPointer, IN typename mSharedPointer<T>::PointerParams *pPointerParams, IN T *pData, IN mAllocator *pAllocator, const std::function<void(T *)> &cleanupFunction)
{
  mFUNCTION_SETUP();

  mERROR_IF(pOutSharedPointer == nullptr || pPointerParams == nullptr || pData == nullptr, mR_ArgumentNull);

  if (*pOutSharedPointer != nullptr)
    *pOutSharedPointer = nullptr;

  pOutSharedPointer->m_pParams = pPointerParams;

  new (pOutSharedPointer->m_pParams) typename mSharedPointer<T>::PointerParams();
  pOutSharedPointer->m_pParams->referenceCount = 1;
  pOutSharedPointer->m_pParams->pAllocator = pAllocator;
  pOutSharedPointer->m_pParams->freeResource = (pAllocator != mSHARED_POINTER_FOREIGN_RESOURCE);
  pOutSharedPointer->m_pParams->cleanupFunction = cleanupFunction;
  pOutSharedPointer->m_pParams->freeParameters = false;
  pOutSharedPointer->m_pParams->pUserData = nullptr;

  pOutSharedPointer->m_pData = pData;

  mRETURN_SUCCESS();
}

template <typename T>
inline mFUNCTION(mSharedPointer_Destroy, IN_OUT mSharedPointer<T> *pPointer)
{
  mFUNCTION_SETUP();
  mERROR_IF(pPointer == nullptr, mR_ArgumentNull);

  *pPointer = nullptr;

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline mSharedPointer<T>::mSharedPointer()
  : m_pData(nullptr), m_pParams(nullptr)
{ }

template<typename T>
inline mSharedPointer<T>::mSharedPointer(nullptr_t)
  : m_pData(nullptr), m_pParams(nullptr)
{ }

template<typename T>
inline mSharedPointer<T>::mSharedPointer(const mSharedPointer<T> &copy)
  : m_pData(nullptr), m_pParams(nullptr)
{
  if (copy == nullptr)
    return;

  m_pData = copy.m_pData;
  m_pParams = copy.m_pParams;

  ++m_pParams->referenceCount;
}

template<typename T>
inline mSharedPointer<T>::mSharedPointer(mSharedPointer<T> &&move)
  : m_pData(nullptr), m_pParams(nullptr)
{
  if (move == nullptr)
    return;

  m_pData = move.m_pData;
  m_pParams = move.m_pParams;

  move.m_pData = nullptr;
  move.m_pParams = nullptr;

  move.~mSharedPointer();
}

template<typename T>
inline mSharedPointer<T>::~mSharedPointer()
{
  if (*this == nullptr)
  {
    m_pData = nullptr;
    m_pParams = nullptr;

    return;
  }

  size_t referenceCount = --m_pParams->referenceCount;

  if (referenceCount == 0)
  {
    if (m_pParams->cleanupFunction)
      m_pParams->cleanupFunction(m_pData);

    if (m_pParams)
    {
      mAllocator *pAllocator = m_pParams->pAllocator;

      if (m_pParams->freeResource)
        mAllocator_FreePtr(m_pParams->pAllocator, &m_pData);

      m_pParams->~PointerParams();

      if (m_pParams->freeParameters)
        mAllocator_FreePtr(pAllocator, &m_pParams);
    }
  }

  m_pParams = nullptr;
  m_pData = nullptr;
}

template<typename T>
inline mSharedPointer<T>& mSharedPointer<T>::operator=(const mSharedPointer<T> &copy)
{
  this->~mSharedPointer<T>();

  if (copy == nullptr)
    return *this;

  m_pData = copy.m_pData;
  m_pParams = copy.m_pParams;

  ++m_pParams->referenceCount;

  return *this;
}

template<typename T>
inline mSharedPointer<T>& mSharedPointer<T>::operator=(mSharedPointer<T> &&move)
{
  if (move != nullptr && *this != nullptr && move.m_pData == m_pData)
  {
    move.~mSharedPointer<T>();
    return *this;
  }

  this->~mSharedPointer<T>();

  if (move == nullptr)
    return *this;

  m_pData = move.m_pData;
  m_pParams = move.m_pParams;

  move.m_pData = nullptr;
  move.m_pParams = nullptr;

  return *this;
}

template<typename T>
inline T* mSharedPointer<T>::operator->()
{
  return m_pData;
}

template<typename T>
inline const T* mSharedPointer<T>::operator->() const
{
  return m_pData;
}

template<typename T>
inline T& mSharedPointer<T>::operator*()
{
  return *m_pData;
}

template<typename T>
inline const T& mSharedPointer<T>::operator*() const
{
  return *m_pData;
}

template<typename T>
inline bool mSharedPointer<T>::operator==(const mSharedPointer<T> &other) const
{
  if (*this == nullptr)
  {
    if (other == nullptr)
      return true;

    return false;
  }

  if (other == nullptr)
    return false;

  if (m_pData == other.m_pData)
    return true;

  return false;
}

template<typename T>
inline bool mSharedPointer<T>::operator==(nullptr_t) const
{
  if (m_pData == nullptr || m_pParams == nullptr)
    return true;

  return false;
}

template<typename T>
inline bool mSharedPointer<T>::operator!=(const mSharedPointer<T> &other) const
{
  return !(*this == other);
}

template<typename T>
inline bool mSharedPointer<T>::operator!=(nullptr_t) const
{
  return !(*this == nullptr);
}

template<typename T>
inline mSharedPointer<T>::operator bool() const
{
  return *this != nullptr;
}

template<typename T>
inline bool mSharedPointer<T>::operator!() const
{
  return *this == nullptr;
}

template<typename T>
template<typename T2, typename std::enable_if_t<std::is_convertible<T2, bool>::value, int>* /* = nullptr */ >
inline mSharedPointer<T>::operator T2() const
{
  mSTATIC_ASSERT((std::is_same<bool, T2>::value), "mSharedPointer cannot be cast to the given type.");

  return (bool)*this;
}

template<typename T>
inline const T* mSharedPointer<T>::GetPointer() const
{
  return m_pData;
}

template<typename T>
inline T* mSharedPointer<T>::GetPointer()
{
  return m_pData;
}

template<typename T>
inline size_t mSharedPointer<T>::GetReferenceCount() const
{
  if (*this == nullptr)
    return 0;

  return m_pParams->referenceCount;
}

template<typename T>
inline mFUNCTION(mDestruct, IN mPtr<T> *pData)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mSharedPointer_Destroy(pData));

  mRETURN_SUCCESS();
}

template <typename T>
struct mIsTriviallyMemoryMovable<mPtr<T>>
{
  static constexpr bool value = true;
};

#endif // mSharedPointer_h__
