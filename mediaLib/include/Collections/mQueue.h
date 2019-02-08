#ifndef mQueue_h__
#define mQueue_h__

#include "mediaLib.h"
#include "mStaticIf.h"

template <typename T>
struct mQueueIterator
{
  ptrdiff_t direction;
  T *pCurrent;
  T *pLineEnd;
  T *pLineStart;
  T *pLastPos;
  size_t count;
  size_t index;

  mQueueIterator(const ptrdiff_t direction, T *pFirst, T *pLineEnd, T *pLineStart, const size_t count);
  T& operator *();
  const T& operator *() const;
  bool operator != (const mQueueIterator<T> &iterator) const;
  mQueueIterator<T>& operator++();
};

template <typename T>
struct mQueue
{
  mAllocator *pAllocator;
  T *pData;
  size_t startIndex, size, count;

  mQueueIterator<T> begin() const 
  { 
    return mQueueIterator<T>(1, pData + startIndex, pData + size - 1, pData, count); 
  }

  mQueueIterator<T> end() const 
  { 
    if (size > 0) 
      return mQueueIterator<T>(-1, pData + ((startIndex + count - 1) % size), pData + size - 1, pData, count);
    else 
      return begin();
  };

  struct mQueueReverseIterator
  {
    mQueue<T> *pQueue;

    mQueueIterator<T> begin() const { return pQueue->end(); }
    mQueueIterator<T> end() const { return pQueue->begin(); }
  } IterateReverse() { return {this}; };

  struct mQueueIteratorWrapper
  {
    mQueue<T> *pQueue;
    
    mQueueIterator<T> begin() const { return pQueue->begin(); }
    mQueueIterator<T> end() const { return pQueue->end(); }

  } Iterate() { return {this}; };
};

template <typename T>
mFUNCTION(mQueue_Create, OUT mPtr<mQueue<T>> *pQueue, IN OPTIONAL mAllocator *pAllocator);

template <typename T>
mFUNCTION(mQueue_Destroy, IN_OUT mPtr<mQueue<T>> *pQueue);

template <typename T>
mFUNCTION(mQueue_PushBack, mPtr<mQueue<T>> &queue, IN T *pItem);

template <typename T>
mFUNCTION(mQueue_PushBack, mPtr<mQueue<T>> &queue, IN T item);

template <typename T>
mFUNCTION(mQueue_PushFront, mPtr<mQueue<T>> &queue, IN T *pItem);

template <typename T>
mFUNCTION(mQueue_PushFront, mPtr<mQueue<T>> &queue, IN T item);

template <typename T>
mFUNCTION(mQueue_PopFront, mPtr<mQueue<T>> &queue, OUT T *pItem);

template <typename T>
mFUNCTION(mQueue_PeekFront, mPtr<mQueue<T>> &queue, OUT T *pItem);

template <typename T>
mFUNCTION(mQueue_PeekBack, mPtr<mQueue<T>> &queue, OUT T *pItem);

template <typename T>
mFUNCTION(mQueue_PopBack, mPtr<mQueue<T>> &queue, OUT T *pItem);

template <typename T>
mFUNCTION(mQueue_PeekAt, mPtr<mQueue<T>> &queue, const size_t index, OUT T *pItem);

template <typename T>
mFUNCTION(mQueue_PopAt, mPtr<mQueue<T>> &queue, size_t index, OUT T *pItem);

// Handle with care. The retrieved pointer will just be valid until the size changes or an element is removed.
template <typename T>
mFUNCTION(mQueue_PointerAt, mPtr<mQueue<T>> &queue, const size_t index, OUT T **ppItem);

template <typename T>
mFUNCTION(mQueue_GetCount, mPtr<mQueue<T>> &queue, OUT size_t *pCount);

template <typename T>
mFUNCTION(mQueue_Reserve, mPtr<mQueue<T>> &queue, const size_t count);

template <typename T>
mFUNCTION(mQueue_Clear, mPtr<mQueue<T>> &queue);

//////////////////////////////////////////////////////////////////////////

template<typename T>
mFUNCTION(mQueue_Destroy_Internal, IN mQueue<T> *pQueue);

template<typename T>
mFUNCTION(mQueue_Grow_Internal, mPtr<mQueue<T>> &queue);

template <typename T>
mFUNCTION(mQueue_PopAt_Internal, mPtr<mQueue<T>> &queue, const size_t index, OUT T *pItem);

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline mFUNCTION(mQueue_Create, OUT mPtr<mQueue<T>> *pQueue, IN OPTIONAL mAllocator *pAllocator)
{
  mFUNCTION_SETUP();

  mERROR_IF(pQueue == nullptr, mR_ArgumentNull);
  
  if (*pQueue != nullptr)
  {
    mERROR_CHECK(mSharedPointer_Destroy(pQueue));
    *pQueue = nullptr;
  }

  mQueue<T> *pQueueRaw = nullptr;
  mDEFER_ON_ERROR(mAllocator_FreePtr(pAllocator, &pQueueRaw));
  mERROR_CHECK(mAllocator_AllocateZero(pAllocator, &pQueueRaw, 1));

  mDEFER_CALL_ON_ERROR(pQueue, mSharedPointer_Destroy);
  mERROR_CHECK(mSharedPointer_Create(pQueue, pQueueRaw, (std::function<void (mQueue<T> *)>)[](mQueue<T> *pData) { mQueue_Destroy_Internal<T>(pData); }, pAllocator));
  pQueueRaw = nullptr;

  (*pQueue)->pAllocator = pAllocator;

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_Destroy, IN_OUT mPtr<mQueue<T>> *pQueue)
{
  mFUNCTION_SETUP();

  mERROR_IF(pQueue == nullptr, mR_ArgumentNull);
  
  mERROR_CHECK(mSharedPointer_Destroy(pQueue));

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PushBack, mPtr<mQueue<T>> &queue, IN T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pItem == nullptr, mR_ArgumentNull);

  if (queue->pData == nullptr)
    mERROR_CHECK(mQueue_Reserve(queue, 1));
  else if (queue->size == queue->count)
    mERROR_CHECK(mQueue_Grow_Internal(queue));

  const size_t index = (queue->startIndex + queue->count) % queue->size;

  if (!std::is_trivially_copy_constructible<T>::value)
    new (&queue->pData[index]) T(*pItem);
  else
    mERROR_CHECK(mAllocator_Copy(queue->pAllocator, &queue->pData[index], pItem, 1));

  ++queue->count;

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PushBack, mPtr<mQueue<T>> &queue, IN T item)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mQueue_PushBack(queue, &item));

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PushFront, mPtr<mQueue<T>> &queue, IN T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pItem == nullptr, mR_ArgumentNull);

  if (queue->pData == nullptr)
    mERROR_CHECK(mQueue_Reserve(queue, 1));
  else if (queue->size == queue->count)
    mERROR_CHECK(mQueue_Grow_Internal(queue));

  size_t index;

  if (queue->startIndex > 0)
    index = queue->startIndex - 1;
  else
    index = queue->size - 1;

  if (!std::is_trivially_copy_constructible<T>::value)
    new (&queue->pData[index]) T(*pItem);
  else
    mERROR_CHECK(mAllocator_Copy(queue->pAllocator, &queue->pData[index], pItem, 1));

  ++queue->count;
  queue->startIndex = index;

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PushFront, mPtr<mQueue<T>> &queue, IN T item)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mQueue_PushFront(queue, &item));

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PopFront, mPtr<mQueue<T>> &queue, OUT T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mQueue_PopAt_Internal(queue, 0, pItem));

  ++queue->startIndex;
  --queue->count;

  if (queue->startIndex >= queue->size)
    queue->startIndex = 0;

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PeekFront, mPtr<mQueue<T>> &queue, OUT T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mQueue_PeekAt(queue, 0, pItem));

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PeekBack, mPtr<mQueue<T>> &queue, OUT T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pItem == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mQueue_PeekAt(queue, queue->count - 1, pItem));

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PopBack, mPtr<mQueue<T>> &queue, OUT T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pItem == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mQueue_PopAt_Internal(queue, queue->count - 1, pItem));

  --queue->count;

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PeekAt, mPtr<mQueue<T>> &queue, const size_t index, OUT T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pItem == nullptr, mR_ArgumentNull);
  mERROR_IF(queue->count <= index, mR_IndexOutOfBounds);

  const size_t queueIndex = (queue->startIndex + index) % queue->size;

  *pItem = queue->pData[queueIndex];

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PopAt, mPtr<mQueue<T>> &queue, size_t index, OUT T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pItem == nullptr, mR_ArgumentNull);
  mERROR_IF(queue->count <= index, mR_IndexOutOfBounds);

  if (index == 0)
  {
    mERROR_CHECK(mQueue_PopFront(queue, pItem));
    mRETURN_SUCCESS();
  }
  else if (index == queue->count - 1)
  {
    mERROR_CHECK(mQueue_PopBack(queue, pItem));
    mRETURN_SUCCESS();
  }

  const size_t queueIndex = (queue->startIndex + index) % queue->size;
  *pItem = std::move(queue->pData[queueIndex]);

  if (queue->startIndex + index < queue->size)
  {
    if (queueIndex + 1 != queue->size)
      mERROR_CHECK(mAllocator_Move(queue->pAllocator, &queue->pData[queueIndex], &queue->pData[queueIndex + 1], queue->size - (queue->startIndex + index)));

    if (queue->startIndex + queue->count > queue->size)
    {
      new (&queue->pData[queue->size - 1]) T(std::move(queue->pData[0]));

      if (queue->startIndex + queue->count - 1 > queue->size)
        mERROR_CHECK(mAllocator_Move(queue->pAllocator, &queue->pData[0], &queue->pData[1], queue->count - (queue->size - queue->startIndex)));
    }
  }
  else
  {
    mERROR_CHECK(mAllocator_Move(queue->pAllocator, &queue->pData[queueIndex], &queue->pData[queueIndex + 1], queue->count - (queue->size - queue->startIndex) - queueIndex - 1));
  }

  --queue->count;

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PointerAt, mPtr<mQueue<T>> &queue, const size_t index, OUT T **ppItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || ppItem == nullptr, mR_ArgumentNull);
  mERROR_IF(queue->count <= index, mR_IndexOutOfBounds);

  const size_t queueIndex = (queue->startIndex + index) % queue->size;

  *ppItem = &queue->pData[queueIndex];

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_GetCount, mPtr<mQueue<T>> &queue, OUT size_t *pCount)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pCount == nullptr, mR_ArgumentNull);

  *pCount = queue->count;

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline mFUNCTION(mQueue_Reserve, mPtr<mQueue<T>> &queue, const size_t count)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr, mR_ArgumentNull);

  if (count <= queue->size)
    mRETURN_SUCCESS();

  if (queue->pData == nullptr)
  {
    mERROR_CHECK(mAllocator_Allocate(queue->pAllocator, &queue->pData, count));
  }
  else
  {
    mStaticIf<mIsTriviallyMemoryMovable<T>::value>([&](auto)
    {
      mSTDRESULT = mAllocator_Reallocate(queue->pAllocator, &queue->pData, count);
    })
      .Else([&](auto)
    {
      T *pNewData = nullptr;
      mDEFER(mAllocator_FreePtr(queue->pAllocator, &pNewData));
      mSTDRESULT = mAllocator_Allocate(queue->pAllocator, &pNewData, count);

      if (mFAILED(mSTDRESULT))
        RETURN;

      if (queue->startIndex + queue->count <= queue->size)
      {
        mMoveConstructMultiple(&pNewData[queue->startIndex], &queue->pData[queue->startIndex], queue->count);
      }
      else
      {
        mMoveConstructMultiple(&pNewData[queue->startIndex], &queue->pData[queue->startIndex], queue->size - queue->startIndex);
        mMoveConstructMultiple(pNewData, queue->pData, queue->count - (queue->size - queue->startIndex));
      }

      std::swap(queue->pData, pNewData);
    });

    if (mFAILED(mSTDRESULT))
      mRETURN_RESULT(mSTDRESULT);
  }

  queue->size = count;

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_Clear, mPtr<mQueue<T>> &queue)
{
  mFUNCTION_SETUP();
  
  mERROR_IF(queue == nullptr, mR_ArgumentNull);

  if (queue->pData != nullptr)
  {
    size_t index = queue->startIndex;

    for (size_t i = 0; i < queue->count; ++i)
    {
      mERROR_CHECK(mDestruct(queue->pData + index));

      index = (index + 1) % queue->size;
    }

    queue->startIndex = 0;
    queue->count = 0;
  }

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline mFUNCTION(mQueue_Destroy_Internal, IN mQueue<T> *pQueue)
{
  mFUNCTION_SETUP();

  mERROR_IF(pQueue == nullptr, mR_ArgumentNull);

  if (pQueue->pData != nullptr)
  {
    size_t index = pQueue->startIndex;

    for (size_t i = 0; i < pQueue->count; ++i)
    {
      mERROR_CHECK(mDestruct(pQueue->pData + index));

      index = (index + 1) % pQueue->size;
    }

    mAllocator *pAllocator = pQueue->pAllocator;
    mERROR_CHECK(mAllocator_FreePtr(pAllocator, &pQueue->pData));
  }

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_Grow_Internal, mPtr<mQueue<T>> &queue)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr, mR_ArgumentNull);

  const size_t originalSize = queue->size;

  if (originalSize == 0)
  {
    mERROR_CHECK(mQueue_Reserve(queue, 1));
    mRETURN_SUCCESS();
  }

  const size_t doubleSize = originalSize * 2;

  mStaticIf<mIsTriviallyMemoryMovable<T>::value>([&](auto)
  {
    mSTDRESULT = mAllocator_Reallocate(queue->pAllocator, &queue->pData, doubleSize);
  })
    .Else([&](auto)
  {
    T *pNewData = nullptr;
    mDEFER(mAllocator_FreePtr(queue->pAllocator, &pNewData));
    mSTDRESULT = mAllocator_Allocate(queue->pAllocator, &pNewData, doubleSize);

    if (mFAILED(mSTDRESULT))
      RETURN;

    if (queue->startIndex + queue->count <= queue->size)
    {
      mMoveConstructMultiple(&pNewData[queue->startIndex], &queue->pData[queue->startIndex], queue->count);
    }
    else
    {
      mMoveConstructMultiple(&pNewData[queue->startIndex], &queue->pData[queue->startIndex], queue->size - queue->startIndex);
      mMoveConstructMultiple(pNewData, queue->pData, queue->count - (queue->size - queue->startIndex));
    }

    std::swap(queue->pData, pNewData);
  });

  if (mFAILED(mSTDRESULT))
    mRETURN_RESULT(mSTDRESULT);

  queue->size = doubleSize;

  if (queue->startIndex > 0 && queue->startIndex + queue->count > originalSize)
  {
    const size_t wrappedCount = queue->count - (originalSize - queue->startIndex);
    mERROR_CHECK(mAllocator_Move(queue->pAllocator, &queue->pData[originalSize], &queue->pData[0], wrappedCount));
  }

  mRETURN_SUCCESS();
}

template<typename T>
inline mFUNCTION(mQueue_PopAt_Internal, mPtr<mQueue<T>> &queue, const size_t index, OUT T *pItem)
{
  mFUNCTION_SETUP();

  mERROR_IF(queue == nullptr || pItem == nullptr, mR_ArgumentNull);
  mERROR_IF(queue->count <= index, mR_IndexOutOfBounds);

  const size_t queueIndex = (queue->startIndex + index) % queue->size;

  *pItem = std::move(queue->pData[queueIndex]);

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

template<typename T>
inline mQueueIterator<T>::mQueueIterator(const ptrdiff_t direction, T *pFirst, T *pLineEnd, T *pLineStart, const size_t count) :
  direction(direction),
  pCurrent(pFirst),
  pLineEnd(pLineEnd),
  pLineStart(pLineStart),
  pLastPos(pCurrent),
  count(count),
  index(0)
{ }

template<typename T>
inline T & mQueueIterator<T>::operator* ()
{
  return *pCurrent;
}

template<typename T>
inline const T & mQueueIterator<T>::operator*() const
{
  return *pCurrent;
}

template<typename T>
inline bool mQueueIterator<T>::operator!=(const mQueueIterator<T> &) const
{
  return index != count;
}

template<typename T>
inline mQueueIterator<T> & mQueueIterator<T>::operator++()
{
  pLastPos = pCurrent;
  pCurrent += direction;
  index++;

  if (direction > 0)
  {
    if (pCurrent > pLineEnd)
      pCurrent = pLineStart;
  }
  else
  {
    if (pCurrent < pLineStart)
      pCurrent = pLineEnd;
  }

  return *this;
}

#endif // mQueue_h__
