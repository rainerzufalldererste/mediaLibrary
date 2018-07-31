// Copyright 2018 Christoph Stiller
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <mfapi.h>
#include <mfidl.h>
#include <Mferror.h>
#include <mfreadwrite.h>
#include "mMediaFileInputHandler.h"

static volatile size_t _referenceCount = 0;

struct mMediaTypeLookup
{
  size_t streamIndex;
  mMediaMajorType mediaType;
};

struct mMediaFileInputHandler
{
  IMFSourceReader *pSourceReader;
  ProcessVideoBufferFunction *pProcessVideoDataCallback;
  ProcessAudioBufferFunction *pProcessAudioDataCallback;

  size_t streamCount;
  mMediaTypeLookup *pStreamTypeLookup;

  size_t videoStreamCount;
  mVideoStreamType *pVideoStreams;

  size_t audioStreamCount;
  mAudioStreamType *pAudioStreams;
};

mFUNCTION(mMediaFileInputHandler_Create_Internal, IN mMediaFileInputHandler *pData, const std::wstring &fileName, const bool enableVideoProcessing, const bool enableAudioProcessing);
mFUNCTION(mMediaFileInputHandler_Destroy_Internal, IN mMediaFileInputHandler *pData);
mFUNCTION(mMediaFileInputHandler_RunSession_Internal, IN mMediaFileInputHandler *pData);
mFUNCTION(mMediaFileInputHandler_InitializeExtenalDependencies);
mFUNCTION(mMediaFileInputHandler_CleanupExtenalDependencies);
mFUNCTION(mMediaFileInputHandler_AddStream_Internal, IN mMediaFileInputHandler *pInputHandler, IN mMediaTypeLookup *pMediaType);
mFUNCTION(mMediaFileInputHandler_AddVideoStream_Internal, IN mMediaFileInputHandler *pInputHandler, IN mVideoStreamType *pVideoStreamType);
mFUNCTION(mMediaFileInputHandler_AddAudioStream_Internal, IN mMediaFileInputHandler *pInputHandler, IN mAudioStreamType *pAudioStreamType);

template <typename T>
static void _ReleaseReference(T **pData)
{
  if (pData && *pData)
  {
    (*pData)->Release();
    *pData = nullptr;
  }
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mMediaFileInputHandler_Create, OUT mPtr<mMediaFileInputHandler> *pPtr, const std::wstring &fileName, const mMediaFileInputHandler_CreateFlags createFlags)
{
  mFUNCTION_SETUP();

  mERROR_IF(pPtr == nullptr, mR_ArgumentNull);

  if (pPtr != nullptr)
  {
    mERROR_CHECK(mSharedPointer_Destroy(pPtr));
    *pPtr = nullptr;
  }

  mMediaFileInputHandler *pInputHandler = nullptr;
  mDEFER_DESTRUCTION_ON_ERROR(&pInputHandler, mFreePtr);
  mERROR_CHECK(mAllocZero(&pInputHandler, 1));

  mDEFER_DESTRUCTION_ON_ERROR(pPtr, mSharedPointer_Destroy);
  mERROR_CHECK(mSharedPointer_Create<mMediaFileInputHandler>(pPtr, pInputHandler, [](mMediaFileInputHandler *pData) { mMediaFileInputHandler_Destroy_Internal(pData); }, mAT_mAlloc));
  pInputHandler = nullptr; // to not be destroyed on error.

  mERROR_CHECK(mMediaFileInputHandler_Create_Internal(pPtr->GetPointer(), fileName, (createFlags & mMediaFileInputHandler_CreateFlags::mMMFIH_CF_VideoEnabled) != 0, (createFlags & mMediaFileInputHandler_CreateFlags::mMMFIH_CF_AudioEnabled) != 0));
  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_Destroy, IN_OUT mPtr<mMediaFileInputHandler> *pPtr)
{
  mFUNCTION_SETUP();

  mERROR_IF(pPtr == nullptr || *pPtr == nullptr, mR_ArgumentNull);
  mERROR_CHECK(mSharedPointer_Destroy(pPtr));

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_Play, mPtr<mMediaFileInputHandler> &ptr)
{
  mFUNCTION_SETUP();

  mERROR_IF(ptr == nullptr, mR_ArgumentNull);
  mERROR_CHECK(mMediaFileInputHandler_RunSession_Internal(ptr.GetPointer()));

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_GetVideoStreamResolution, mPtr<mMediaFileInputHandler> &ptr, OUT mVec2s *pResolution, const size_t videoStreamIndex /* = 0 */)
{
  mFUNCTION_SETUP();

  mERROR_IF(ptr == nullptr, mR_ArgumentNull);

  mERROR_IF(ptr->videoStreamCount < videoStreamIndex, mR_IndexOutOfBounds);

  if (pResolution)
    *pResolution = ptr->pVideoStreams[videoStreamIndex].resolution;

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_SetVideoCallback, mPtr<mMediaFileInputHandler> &ptr, IN ProcessVideoBufferFunction *pCallback)
{
  mFUNCTION_SETUP();

  mERROR_IF(ptr == nullptr, mR_ArgumentNull);
  ptr->pProcessVideoDataCallback = pCallback;

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_SetAudioCallback, mPtr<mMediaFileInputHandler> &ptr, IN ProcessAudioBufferFunction *pCallback)
{
  mFUNCTION_SETUP();

  mERROR_IF(ptr == nullptr, mR_ArgumentNull);
  ptr->pProcessAudioDataCallback = pCallback;

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_AddStream_Internal, IN mMediaFileInputHandler *pInputHandler, IN mMediaTypeLookup *pMediaType)
{
  mFUNCTION_SETUP();

  mERROR_IF(pInputHandler == nullptr || pMediaType == nullptr, mR_ArgumentNull);

  ++pInputHandler->streamCount;

  if (pInputHandler->pStreamTypeLookup == nullptr)
    mERROR_CHECK(mAlloc(&pInputHandler->pStreamTypeLookup, pInputHandler->streamCount));
  else
    mERROR_CHECK(mRealloc(&pInputHandler->pStreamTypeLookup, pInputHandler->streamCount));

  mERROR_CHECK(mMemcpy(&pInputHandler->pStreamTypeLookup[pInputHandler->streamCount - 1], pMediaType, 1));

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_AddVideoStream_Internal, IN mMediaFileInputHandler *pInputHandler, IN mVideoStreamType *pVideoStreamType)
{
  mFUNCTION_SETUP();

  mERROR_IF(pInputHandler == nullptr || pVideoStreamType == nullptr, mR_ArgumentNull);

  ++pInputHandler->videoStreamCount;

  if (pInputHandler->pVideoStreams == nullptr)
    mERROR_CHECK(mAlloc(&pInputHandler->pVideoStreams, pInputHandler->videoStreamCount));
  else
    mERROR_CHECK(mRealloc(&pInputHandler->pVideoStreams, pInputHandler->videoStreamCount));

  mERROR_CHECK(mMemcpy(&pInputHandler->pVideoStreams[pInputHandler->videoStreamCount - 1], pVideoStreamType, 1));

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_AddAudioStream_Internal, IN mMediaFileInputHandler *pInputHandler, IN mAudioStreamType *pAudioStreamType)
{
  mFUNCTION_SETUP();

  mERROR_IF(pInputHandler == nullptr || pAudioStreamType == nullptr, mR_ArgumentNull);

  ++pInputHandler->audioStreamCount;

  if (pInputHandler->pVideoStreams == nullptr)
    mERROR_CHECK(mAlloc(&pInputHandler->pAudioStreams, pInputHandler->audioStreamCount));
  else
    mERROR_CHECK(mRealloc(&pInputHandler->pAudioStreams, pInputHandler->audioStreamCount));

  mERROR_CHECK(mMemcpy(&pInputHandler->pAudioStreams[pInputHandler->audioStreamCount - 1], pAudioStreamType, 1));

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_Create_Internal, IN mMediaFileInputHandler *pInputHandler, const std::wstring &fileName, const bool enableVideoProcessing, const bool enableAudioProcessing)
{
  mFUNCTION_SETUP();
  mERROR_IF(pInputHandler == nullptr, mR_ArgumentNull);

  HRESULT hr = S_OK;
  mUnused(hr);

  const int64_t referenceCount = ++_referenceCount;

  if (referenceCount == 1)
    mERROR_CHECK(mMediaFileInputHandler_InitializeExtenalDependencies());

  IMFAttributes *pAttributes = nullptr;
  mDEFER_DESTRUCTION(&pAttributes, _ReleaseReference);
  mERROR_IF(FAILED(MFCreateAttributes(&pAttributes, 1)), mR_InternalError);
  mERROR_IF(FAILED(pAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE)), mR_InternalError);

  IMFMediaType *pVideoMediaType = nullptr;
  mDEFER_DESTRUCTION(&pVideoMediaType, _ReleaseReference);

  if (enableVideoProcessing)
  {
    mERROR_IF(FAILED(MFCreateMediaType(&pVideoMediaType)), mR_InternalError);
    mERROR_IF(FAILED(pVideoMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video)), mR_InternalError);
    mERROR_IF(FAILED(pVideoMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32)), mR_InternalError);
  }

  IMFMediaType *pAudioMediaType = nullptr;
  mDEFER_DESTRUCTION(&pAudioMediaType, _ReleaseReference);

  if (enableAudioProcessing)
  {
    mERROR_IF(FAILED(MFCreateMediaType(&pAudioMediaType)), mR_InternalError);
    mERROR_IF(FAILED(pAudioMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio)), mR_InternalError);
    mERROR_IF(FAILED(pAudioMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM)), mR_InternalError);
  }

  mERROR_IF(FAILED(hr = MFCreateSourceReaderFromURL(fileName.c_str(), pAttributes, &pInputHandler->pSourceReader)), mR_InvalidParameter);

  GUID majorType;
  GUID minorType;
  DWORD streamIndex = 0;
  DWORD mediaTypeIndex = 0;
  bool isValid = false;

  while (true)
  {
    bool usableMediaTypeFoundInStream = false;
    mediaTypeIndex = 0;

    while (true)
    {
      IMFMediaType *pType = nullptr;
      mDEFER_DESTRUCTION(&pType, _ReleaseReference);
      hr = pInputHandler->pSourceReader->GetNativeMediaType(streamIndex, mediaTypeIndex, &pType);

      if (hr == MF_E_NO_MORE_TYPES)
      {
        hr = S_OK;
        break;
      }
      else if (hr == MF_E_INVALIDSTREAMNUMBER)
      {
        break;
      }
      else if (SUCCEEDED(hr))
      {
        pType->GetMajorType(&majorType);
        pType->GetGUID(MF_MT_SUBTYPE, &minorType);
        mUnused(minorType);

        if (enableVideoProcessing && majorType == MFMediaType_Video)
        {
          if (!usableMediaTypeFoundInStream)
          {
            usableMediaTypeFoundInStream = true;

            uint32_t resolutionX = 0, resolutionY = 0;

            mERROR_IF(FAILED(hr = pInputHandler->pSourceReader->SetStreamSelection(streamIndex, true)), mR_InternalError);
            mERROR_IF(FAILED(hr = pInputHandler->pSourceReader->GetCurrentMediaType(streamIndex, &pType)), mR_InternalError);
            mERROR_IF(FAILED(hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &resolutionX, &resolutionY)), mR_InternalError);
            mERROR_IF(FAILED(hr = pInputHandler->pSourceReader->SetCurrentMediaType(streamIndex, nullptr, pVideoMediaType)), mR_InternalError);

            isValid = true;

            mMediaTypeLookup mediaTypeLookup;
            mERROR_CHECK(mMemset(&mediaTypeLookup, 1));
            mediaTypeLookup.mediaType = mMMT_Video;
            mediaTypeLookup.streamIndex = pInputHandler->videoStreamCount;

            mERROR_CHECK(mMediaFileInputHandler_AddStream_Internal(pInputHandler, &mediaTypeLookup));

            mVideoStreamType videoStreamType;
            mERROR_CHECK(mMemset(&videoStreamType, 1));
            videoStreamType.mediaType = mediaTypeLookup.mediaType;
            videoStreamType.wmf_streamIndex = streamIndex;
            videoStreamType.streamIndex = mediaTypeLookup.streamIndex;
            videoStreamType.pixelFormat = mPixelFormat::mPF_B8G8R8A8;
            videoStreamType.resolution.x = resolutionX;
            videoStreamType.resolution.y = resolutionY;
            videoStreamType.stride = 0;

            mERROR_CHECK(mMediaFileInputHandler_AddVideoStream_Internal(pInputHandler, &videoStreamType));
          }
        }
        else if (enableAudioProcessing && majorType == MFMediaType_Audio)
        {
          if (!usableMediaTypeFoundInStream)
          {
            usableMediaTypeFoundInStream = true;

            uint32_t samplesPerSecond, bitsPerSample, channelCount;

            mERROR_IF(FAILED(hr = pInputHandler->pSourceReader->SetStreamSelection(streamIndex, true)), mR_InternalError);
            mERROR_IF(FAILED(hr = pInputHandler->pSourceReader->GetCurrentMediaType(streamIndex, &pType)), mR_InternalError);
            mERROR_IF(FAILED(hr = pType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond)), mR_InternalError);
            mERROR_IF(FAILED(hr = pType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample)), mR_InternalError);
            mERROR_IF(FAILED(hr = pType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channelCount)), mR_InternalError);
            mERROR_IF(FAILED(hr = pInputHandler->pSourceReader->SetCurrentMediaType(streamIndex, nullptr, pAudioMediaType)), mR_InternalError);

            isValid = true;

            mMediaTypeLookup mediaTypeLookup;
            mERROR_CHECK(mMemset(&mediaTypeLookup, 1));
            mediaTypeLookup.mediaType = mMMT_Audio;
            mediaTypeLookup.streamIndex = pInputHandler->audioStreamCount;

            mERROR_CHECK(mMediaFileInputHandler_AddStream_Internal(pInputHandler, &mediaTypeLookup));

            mAudioStreamType audioStreamType;
            mERROR_CHECK(mMemset(&audioStreamType, 1));
            audioStreamType.mediaType = mediaTypeLookup.mediaType;
            audioStreamType.wmf_streamIndex = streamIndex;
            audioStreamType.streamIndex = mediaTypeLookup.streamIndex;
            audioStreamType.bitsPerSample = bitsPerSample;
            audioStreamType.samplesPerSecond = samplesPerSecond;
            audioStreamType.channelCount = channelCount;
            audioStreamType.bufferSize = 0;

            mERROR_CHECK(mMediaFileInputHandler_AddAudioStream_Internal(pInputHandler, &audioStreamType));
          }
        }
      }

      ++mediaTypeIndex;
    }

    if (hr == MF_E_INVALIDSTREAMNUMBER)
      break;

    if (!usableMediaTypeFoundInStream)
    {
      mMediaTypeLookup mediaTypeLookup;
      mERROR_CHECK(mMemset(&mediaTypeLookup, 1));
      mediaTypeLookup.mediaType = mMMT_Undefined;
      mediaTypeLookup.streamIndex = 0;

      mERROR_CHECK(mMediaFileInputHandler_AddStream_Internal(pInputHandler, &mediaTypeLookup));
    }

    ++streamIndex;
  }

  mERROR_IF(!isValid, mR_InvalidParameter);

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_Destroy_Internal, IN mMediaFileInputHandler *pData)
{
  mFUNCTION_SETUP();

  mERROR_IF(pData == nullptr, mR_ArgumentNull);

  if (pData->pSourceReader)
  {
    pData->pSourceReader->Release();
    pData->pSourceReader = nullptr;
  }

  if (pData->pProcessVideoDataCallback)
    pData->pProcessVideoDataCallback = nullptr;

  if (pData->pProcessAudioDataCallback)
    pData->pProcessAudioDataCallback = nullptr;

  if (pData->pStreamTypeLookup)
    mERROR_CHECK(mFreePtr(&pData->pStreamTypeLookup));

  pData->streamCount = 0;

  if (pData->pVideoStreams)
    mERROR_CHECK(mFreePtr(&pData->pVideoStreams));

  pData->videoStreamCount = 0;

  if (pData->pAudioStreams)
    mERROR_CHECK(mFreePtr(&pData->pAudioStreams));

  pData->audioStreamCount = 0;

  const size_t referenceCount = --_referenceCount;

  if (referenceCount == 0)
    mERROR_CHECK(mMediaFileInputHandler_CleanupExtenalDependencies());

  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_InitializeExtenalDependencies)
{
  mFUNCTION_SETUP();
  
  HRESULT hr = S_OK;
  mUnused(hr);

  mERROR_IF(FAILED(hr = MFStartup(MF_VERSION, MFSTARTUP_FULL)), mR_InternalError);
  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_CleanupExtenalDependencies)
{
  mFUNCTION_SETUP();

  HRESULT hr = S_OK;
  mUnused(hr);

  mERROR_IF(FAILED(hr = MFShutdown()), mR_InternalError);
  mRETURN_SUCCESS();
}

mFUNCTION(mMediaFileInputHandler_RunSession_Internal, IN mMediaFileInputHandler *pData)
{
  mFUNCTION_SETUP();

  size_t sampleCount = 0;
  HRESULT hr = S_OK;
  mUnused(hr);

  IMFSample *pSample = nullptr;
  mDEFER_DESTRUCTION(&pSample, _ReleaseReference);

  bool quit = false;

  while (!quit)
  {
    DWORD streamIndex, flags;
    LONGLONG timeStamp;

    mDEFER_DESTRUCTION(&pSample, _ReleaseReference);
    mERROR_IF(FAILED(hr = pData->pSourceReader->ReadSample((DWORD)MF_SOURCE_READER_ANY_STREAM, 0, &streamIndex, &flags, &timeStamp, &pSample)), mR_InternalError);

    mPRINT("Stream %d (%I64d)\n", streamIndex, timeStamp);
    
    if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
    {
      mPRINT("End of stream\n");
      break;
    }

    mERROR_IF(flags & MF_SOURCE_READERF_NEWSTREAM, mR_InvalidParameter);
    mERROR_IF(flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED, mR_InvalidParameter);
    mERROR_IF(flags & MF_SOURCE_READERF_NATIVEMEDIATYPECHANGED, mR_InvalidParameter);

    if (flags & MF_SOURCE_READERF_STREAMTICK)
      mPRINT("Stream tick\n");

    if (pSample)
    {
      ++sampleCount;

      IMFMediaBuffer *pMediaBuffer = nullptr;
      mDEFER_DESTRUCTION(&pMediaBuffer, _ReleaseReference);
      mERROR_IF(FAILED(hr = pSample->ConvertToContiguousBuffer(&pMediaBuffer)), mR_InternalError);

      mERROR_IF(streamIndex >= pData->streamCount, mR_IndexOutOfBounds);
      mMediaTypeLookup mediaTypeLookup = pData->pStreamTypeLookup[streamIndex];

      if (mediaTypeLookup.mediaType == mMMT_Undefined)
        continue;

      uint8_t *pSampleData;
      DWORD sampleDataCurrentLength;
      DWORD sampleDataMaxLength;

      mERROR_IF(FAILED(hr = pMediaBuffer->Lock(&pSampleData, &sampleDataCurrentLength, &sampleDataMaxLength)), mR_InternalError);
      mDEFER(pMediaBuffer->Unlock());
      
      switch (mediaTypeLookup.mediaType)
      {
      case mMMT_Video:
      {
        if (pData->pProcessVideoDataCallback)
        {
          mERROR_IF(pData->videoStreamCount < mediaTypeLookup.streamIndex, mR_IndexOutOfBounds);
          mVideoStreamType videoStreamType = pData->pVideoStreams[mediaTypeLookup.streamIndex];

          mERROR_CHECK((*pData->pProcessVideoDataCallback)(pSampleData, videoStreamType));
        }

        break;
      }

      case mMMT_Audio:
      {
        if (pData->pProcessAudioDataCallback)
        {
          mERROR_IF(pData->audioStreamCount < mediaTypeLookup.streamIndex, mR_IndexOutOfBounds);
          mAudioStreamType audioStreamType = pData->pAudioStreams[mediaTypeLookup.streamIndex];
          audioStreamType.bufferSize = sampleDataCurrentLength;

          mERROR_CHECK((*pData->pProcessAudioDataCallback)(pSampleData, audioStreamType));
        }

        break;
      }

      default:
        break;
      }

      //IMF2DBuffer2 *p2DBuffer = nullptr;
      //mDEFER_DESTRUCTION(&p2DBuffer, _ReleaseReference);
      //mERROR_IF(FAILED(hr = pMediaBuffer->QueryInterface(__uuidof(IMF2DBuffer2), (void **)&p2DBuffer)), mR_InternalError);
    }
  }

  mPRINT("Processed %" PRIu64 " samples\n", (uint64_t)sampleCount);

  mRETURN_SUCCESS();
}
