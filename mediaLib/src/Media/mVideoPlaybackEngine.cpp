// Copyright 2018 Christoph Stiller
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "mVideoPlaybackEngine.h"
#include "mQueue.h"
#include "mMediaFileInputHandler.h"
#include "mTimeStamp.h"

struct mVideoPlaybackEngine
{
  mThread *pPlaybackThread;
  mPtr<mThreadPool> threadPool;
  mPtr<mQueue<mPtr<mImageBuffer>>> imageQueue;
  mPtr<mQueue<mPtr<mImageBuffer>>> freeImageBuffers;
  mMutex *pQueueMutex;
  mPtr<mMediaFileInputHandler> mediaFileInputHandler;
  mPtr<mMediaFileInputIterator> iterator;
  mTimeStamp startTimeStamp;
  mTimeStamp updateTimeStamp;
  mTimeStamp frameTime;
  mTimeStamp displayTime;
  mAllocator *pAllocator;
  std::atomic<bool> isRunning;
  size_t videoStreamIndex;
  mPixelFormat targetPixelFormat;
  size_t maxQueuedFrames;
  bool seekingEnabled;
  bool dropFramesAllowed;
  void *pLastFramePtr;
};

mFUNCTION(mVideoPlaybackEngine_Create_Internal, IN mVideoPlaybackEngine *pPlaybackEngine, IN mAllocator *pAllocator, const std::wstring &fileName, mPtr<mThreadPool> &threadPool, const size_t videoStreamIndex, const mPixelFormat outputPixelFormat, const size_t playbackFlags);
mFUNCTION(mVideoPlaybackEngine_Destroy_Internal, IN_OUT mVideoPlaybackEngine *pPlaybackEngine);

mFUNCTION(mVideoPlaybackEngine_PlaybackThread, mVideoPlaybackEngine *pPlaybackEngine)
{
  mFUNCTION_SETUP();

  mTimeStamp playbackTime;
  mERROR_CHECK(mTimeStamp_FromSeconds(&playbackTime, 0));

  bool worthSkippingFrames = true;

  while (pPlaybackEngine->isRunning)
  {
    size_t queuedCount = 0;
    size_t count = 0;
    mTimeStamp now;
    mVideoStreamType videoStreamType;

    mPtr<mImageBuffer> currentImageBuffer;
    mDEFER_DESTRUCTION(&currentImageBuffer, mImageBuffer_Destroy);

    mPtr<mImageBuffer> rawImageBuffer;
    mDEFER_DESTRUCTION(&rawImageBuffer, mImageBuffer_Destroy);

    // Get ImageBuffer.
    {
      // Try to reuse a buffer.
      {
        mERROR_CHECK(mMutex_Lock(pPlaybackEngine->pQueueMutex));
        mDEFER_DESTRUCTION(pPlaybackEngine->pQueueMutex, mMutex_Unlock);

        mERROR_CHECK(mQueue_GetCount(pPlaybackEngine->imageQueue, &queuedCount));

        if (queuedCount > pPlaybackEngine->maxQueuedFrames)
        {
          goto sleep;
        }

        mERROR_CHECK(mQueue_GetCount(pPlaybackEngine->freeImageBuffers, &count));

        if (count > 0)
          mERROR_CHECK(mQueue_PopFront(pPlaybackEngine->freeImageBuffers, &currentImageBuffer));
      }

      if (currentImageBuffer == nullptr)
        mERROR_CHECK(mImageBuffer_Create(&currentImageBuffer, pPlaybackEngine->pAllocator));
    }


    mERROR_CHECK(mTimeStamp_Now(&now));

    mTimeStamp difference = (now - pPlaybackEngine->startTimeStamp) - (playbackTime + pPlaybackEngine->frameTime);
    mTimeStamp fiveSeconds;
    mERROR_CHECK(mTimeStamp_FromSeconds(&fiveSeconds, 5));

    if (difference.timePoint > 0)
    {
      if (difference > fiveSeconds && pPlaybackEngine->seekingEnabled)
      {
        mERROR_CHECK(mMediaFileInputIterator_SeekTo(pPlaybackEngine->iterator, now - pPlaybackEngine->startTimeStamp + fiveSeconds));
      }
      else if (worthSkippingFrames && pPlaybackEngine->dropFramesAllowed)
      {
        mTimeStamp before;
        mERROR_CHECK(mTimeStamp_Now(&before));

        mERROR_CHECK(mMediaFileInputIterator_SkipFrame(pPlaybackEngine->iterator));

        mTimeStamp after;
        mERROR_CHECK(mTimeStamp_Now(&after));

        if (after - before > pPlaybackEngine->frameTime)
          worthSkippingFrames = false;
      }
    }

    mERROR_CHECK(mMediaFileInputIterator_GetNextVideoFrame(pPlaybackEngine->iterator, &rawImageBuffer, &videoStreamType));
    playbackTime = videoStreamType.timePoint;

    mERROR_CHECK(mImageBuffer_AllocateBuffer(currentImageBuffer, rawImageBuffer->currentSize, pPlaybackEngine->targetPixelFormat));
    mERROR_CHECK(mPixelFormat_TransformBuffer(rawImageBuffer, currentImageBuffer, pPlaybackEngine->threadPool));

    // Push ImageBuffer to queue.
    {
      mERROR_CHECK(mMutex_Lock(pPlaybackEngine->pQueueMutex));
      mDEFER_DESTRUCTION(pPlaybackEngine->pQueueMutex, mMutex_Unlock);

      mERROR_CHECK(mQueue_PushBack(pPlaybackEngine->imageQueue, &currentImageBuffer));
      pPlaybackEngine->updateTimeStamp = playbackTime;
    }

    continue;

  sleep:
    mSleep(1);
  }

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mVideoPlaybackEngine_Create, OUT mPtr<mVideoPlaybackEngine> *pPlaybackEngine, IN mAllocator *pAllocator, const std::wstring & fileName, mPtr<mThreadPool> &threadPool, const size_t videoStreamIndex /* = 0 */, const mPixelFormat outputPixelFormat /* = mPF_B8G8R8A8 */, const size_t playbackFlags /* = mVideoPlaybackEngine_PlaybackFlags::mVPE_PF_None */)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mSharedPointer_Allocate(pPlaybackEngine, pAllocator, (std::function<void(mVideoPlaybackEngine *)>)[](mVideoPlaybackEngine *pData) { mVideoPlaybackEngine_Destroy_Internal(pData); }, 1));

  mERROR_CHECK(mVideoPlaybackEngine_Create_Internal(pPlaybackEngine->GetPointer(), pAllocator, fileName, threadPool, videoStreamIndex, outputPixelFormat, playbackFlags));

  mRETURN_SUCCESS();
}

mFUNCTION(mVideoPlaybackEngine_Destroy, IN_OUT mPtr<mVideoPlaybackEngine> *pPlaybackEngine)
{
  mFUNCTION_SETUP();

  mERROR_IF(pPlaybackEngine == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mSharedPointer_Destroy(pPlaybackEngine));
  *pPlaybackEngine = nullptr;

  mRETURN_SUCCESS();
}

mFUNCTION(mVideoPlaybackEngine_GetCurrentFrame, mPtr<mVideoPlaybackEngine> &playbackEngine, OUT mPtr<mImageBuffer> *pImageBuffer, OUT OPTIONAL bool *pIsNewFrame /* = nullptr */)
{
  mFUNCTION_SETUP();

  mERROR_IF(playbackEngine == nullptr || pImageBuffer == nullptr, mR_ArgumentNull);

  if (playbackEngine->pPlaybackThread == nullptr)
  {
    mERROR_CHECK(mTimeStamp_Now(&playbackEngine->startTimeStamp));
    mERROR_CHECK(mTimeStamp_FromSeconds(&playbackEngine->updateTimeStamp, 0));
    mERROR_CHECK(mTimeStamp_FromSeconds(&playbackEngine->displayTime, 0));

    playbackEngine->isRunning = true;
    mDEFER_ON_ERROR(playbackEngine->isRunning = false);

    mDEFER_DESTRUCTION_ON_ERROR(&playbackEngine->pPlaybackThread, mSetToNullptr);
    mERROR_CHECK(mThread_Create(&playbackEngine->pPlaybackThread, playbackEngine->pAllocator, mVideoPlaybackEngine_PlaybackThread, playbackEngine.GetPointer()));
    
    mVec2s resolution;
    mERROR_CHECK(mMediaFileInputHandler_GetVideoStreamResolution(playbackEngine->mediaFileInputHandler, &resolution, playbackEngine->videoStreamIndex));
  }

  size_t count = 0;
  mTimeStamp now;

  if (pIsNewFrame)
    *pIsNewFrame = false;

  while (true)
  {
    {
      mERROR_CHECK(mMutex_Lock(playbackEngine->pQueueMutex));
      mDEFER_DESTRUCTION(playbackEngine->pQueueMutex, mMutex_Unlock);

      mERROR_CHECK(mQueue_GetCount(playbackEngine->imageQueue, &count));

      if (count == 0)
        goto sleep; // unlocks the mutex.

      mERROR_CHECK(mTimeStamp_Now(&now));

      mPtr<mImageBuffer> imageBuffer;
      mDEFER_DESTRUCTION(&imageBuffer, mImageBuffer_Destroy);

      if (count > 1 && pIsNewFrame)
        *pIsNewFrame = true;

      for (size_t i = 0; i < count - 1; ++i)
      {
        mERROR_CHECK(mQueue_PeekFront(playbackEngine->imageQueue, &imageBuffer));

        if (now - playbackEngine->startTimeStamp > playbackEngine->displayTime)
        {
          playbackEngine->displayTime += playbackEngine->frameTime;
          mERROR_CHECK(mQueue_PopFront(playbackEngine->imageQueue, &imageBuffer));
          mERROR_CHECK(mQueue_PushBack(playbackEngine->freeImageBuffers, &imageBuffer));
        }
        else
        {
          *pImageBuffer = imageBuffer;
          goto break_all_loops;
        }
      }

      mERROR_CHECK(mQueue_PeekFront(playbackEngine->imageQueue, pImageBuffer));
      playbackEngine->displayTime = playbackEngine->updateTimeStamp;
    }

  break_all_loops:
    break;

  sleep:
    mERROR_CHECK(mSleep(1));
    continue;
  }

  if(pIsNewFrame)
    *pIsNewFrame = playbackEngine->pLastFramePtr != pImageBuffer->GetPointer();

  playbackEngine->pLastFramePtr = pImageBuffer->GetPointer();

  if (playbackEngine->pPlaybackThread->threadState == mThread_ThreadState::mT_TS_Stopped)
  {
    mERROR_IF(mFAILED(playbackEngine->pPlaybackThread->result), playbackEngine->pPlaybackThread->result);
    mRETURN_RESULT(mR_EndOfStream);
  }

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mVideoPlaybackEngine_Create_Internal, IN mVideoPlaybackEngine *pPlaybackEngine, IN mAllocator *pAllocator, const std::wstring &fileName, mPtr<mThreadPool> &threadPool, const size_t videoStreamIndex, const mPixelFormat outputPixelFormat, const size_t playbackFlags)
{
  mFUNCTION_SETUP();

  pPlaybackEngine->pAllocator = pAllocator;
  pPlaybackEngine->threadPool = threadPool;
  pPlaybackEngine->videoStreamIndex = videoStreamIndex;
  pPlaybackEngine->targetPixelFormat = outputPixelFormat;
  pPlaybackEngine->maxQueuedFrames = 8;
  pPlaybackEngine->pLastFramePtr = nullptr;

  pPlaybackEngine->seekingEnabled = (playbackFlags & mVideoPlaybackEngine_PlaybackFlags::mVPE_PF_SeekingInVideoAllowed) != 0;
  pPlaybackEngine->dropFramesAllowed = (playbackFlags & mVideoPlaybackEngine_PlaybackFlags::mVPE_PF_DroppingFramesOnProducerOverloadEnabled) != 0;

  new (&pPlaybackEngine->isRunning) std::atomic<bool>(false);

  mERROR_CHECK(mMediaFileInputHandler_Create(&pPlaybackEngine->mediaFileInputHandler, pAllocator, fileName, mMediaFileInputHandler_CreateFlags::mMMFIH_CF_VideoEnabled));
  mERROR_CHECK(mMediaFileInputHandler_GetIterator(pPlaybackEngine->mediaFileInputHandler, &pPlaybackEngine->iterator, mMediaMajorType::mMMT_Video, videoStreamIndex));

  mVideoStreamType videoStreamType;
  mERROR_CHECK(mMediaFileInputHandler_GetVideoStreamType(pPlaybackEngine->mediaFileInputHandler, videoStreamIndex, &videoStreamType));
  mERROR_CHECK(mTimeStamp_FromSeconds(&pPlaybackEngine->frameTime, 1.0 / videoStreamType.frameRate));

  mERROR_CHECK(mMutex_Create(&pPlaybackEngine->pQueueMutex, pAllocator));
  mERROR_CHECK(mQueue_Create(&pPlaybackEngine->imageQueue, pAllocator));
  mERROR_CHECK(mQueue_Create(&pPlaybackEngine->freeImageBuffers, pAllocator));

  mRETURN_SUCCESS();
}

mFUNCTION(mVideoPlaybackEngine_Destroy_Internal, IN_OUT mVideoPlaybackEngine * pPlaybackEngine)
{
  mFUNCTION_SETUP();

  pPlaybackEngine->isRunning = false;

  mERROR_CHECK(mThread_Join(pPlaybackEngine->pPlaybackThread));
  mERROR_CHECK(mThread_Destroy(&pPlaybackEngine->pPlaybackThread));

  mERROR_CHECK(mThreadPool_Destroy(&pPlaybackEngine->threadPool));
  mERROR_CHECK(mMutex_Destroy(&pPlaybackEngine->pQueueMutex));

  mPtr<mImageBuffer> imageBuffer;
  size_t count = 0;
  mERROR_CHECK(mQueue_GetCount(pPlaybackEngine->imageQueue, &count));
  
  for (size_t i = 0; i < count; ++i)
  {
    mERROR_CHECK(mQueue_PopFront(pPlaybackEngine->imageQueue, &imageBuffer));
    mERROR_CHECK(mImageBuffer_Destroy(&imageBuffer));
  }

  mERROR_CHECK(mQueue_Destroy(&pPlaybackEngine->imageQueue));

  count = 0;
  mERROR_CHECK(mQueue_GetCount(pPlaybackEngine->freeImageBuffers, &count));

  for (size_t i = 0; i < count; ++i)
  {
    mERROR_CHECK(mQueue_PopFront(pPlaybackEngine->freeImageBuffers, &imageBuffer));
    mERROR_CHECK(mImageBuffer_Destroy(&imageBuffer));
  }

  mERROR_CHECK(mQueue_Destroy(&pPlaybackEngine->freeImageBuffers));

  mERROR_CHECK(mMediaFileInputIterator_Destroy(&pPlaybackEngine->iterator));
  mERROR_CHECK(mMediaFileInputHandler_Destroy(&pPlaybackEngine->mediaFileInputHandler));

  pPlaybackEngine->isRunning.~atomic();

  mRETURN_SUCCESS();
}
