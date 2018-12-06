#include "mRenderParams.h"
#include "mTexture.h"

mFUNCTION(mTexture_Create, OUT mTexture *pTexture, mPtr<mImageBuffer> &imageBuffer, const bool upload /* = true */, const size_t textureUnit /* = 0 */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pTexture == nullptr || imageBuffer == nullptr, mR_ArgumentNull);

  pTexture->uploadState = mRP_US_NotInitialized;
  pTexture->resolution = imageBuffer->currentSize;
  pTexture->resolutionF = (mVec2f)pTexture->resolution;
  pTexture->foreignTexture = false;

#if defined(mRENDERER_OPENGL)
  mERROR_IF(textureUnit >= 32, mR_IndexOutOfBounds);
  pTexture->textureUnit = (GLuint)textureUnit;

  glActiveTexture(GL_TEXTURE0 + (GLuint)pTexture->textureUnit);
  glGenTextures(1, &pTexture->textureId);

  pTexture->imageBuffer = imageBuffer;
#else
  mRETURN_RESULT(mR_NotInitialized);
#endif

  pTexture->uploadState = mRP_US_NotUploaded;

  if (upload)
    mERROR_CHECK(mTexture_Upload(*pTexture));

  mGL_DEBUG_ERROR_CHECK();

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_Create, OUT mTexture *pTexture, const mString &filename, const bool upload /* = true */, const size_t textureUnit /* = 0 */)
{
  mFUNCTION_SETUP();

  mPtr<mImageBuffer> imageBuffer;
  mDEFER_CALL(&imageBuffer, mImageBuffer_Destroy);
  mERROR_CHECK(mImageBuffer_CreateFromFile(&imageBuffer, nullptr, filename));

  mERROR_CHECK(mTexture_Create(pTexture, imageBuffer, upload, textureUnit));

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_Create, OUT mTexture *pTexture, const uint8_t *pData, const mVec2s &size, const mPixelFormat pixelFormat /* = mPF_B8G8R8A8 */, const bool upload /* = true */, const size_t textureUnit /* = 0 */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pTexture == nullptr || pData == nullptr, mR_ArgumentNull);

  mPtr<mImageBuffer> imageBuffer;
  mDEFER_CALL(&imageBuffer, mImageBuffer_Destroy);

  if (upload)
  {
    mERROR_CHECK(mImageBuffer_Create(&imageBuffer, nullptr, (void *)pData, size, pixelFormat));
  }
  else
  {
    mERROR_CHECK(mImageBuffer_Create(&imageBuffer, nullptr, size, pixelFormat));

    size_t bufferSize;
    mERROR_CHECK(mPixelFormat_GetSize(pixelFormat, size, &bufferSize));

    mERROR_CHECK(mAllocator_Copy(nullptr, imageBuffer->pPixels, pData, bufferSize));
  }

  mERROR_CHECK(mTexture_Create(pTexture, imageBuffer, upload, textureUnit));

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_CreateFromUnownedIndex, OUT mTexture *pTexture, int textureIndex, const size_t textureUnit /* = 0 */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pTexture == nullptr, mR_ArgumentNull);

  pTexture->uploadState = mRP_US_NotInitialized;
  pTexture->resolution = mVec2s(1);
  pTexture->resolutionF = (mVec2f)pTexture->resolution;
  pTexture->foreignTexture = true;

#if defined(mRENDERER_OPENGL)
  mERROR_IF(textureUnit >= 32, mR_IndexOutOfBounds);
  pTexture->textureUnit = (GLuint)textureUnit;
  pTexture->textureId = textureIndex;
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  pTexture->uploadState = mRP_US_Ready;

  mGL_DEBUG_ERROR_CHECK();

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_Allocate, OUT mTexture *pTexture, const mVec2s size, const mPixelFormat pixelFormat /* = mPF_R8G8B8A8 */, const size_t textureUnit /* = 0 */)
{
  mFUNCTION_SETUP();

  pTexture->uploadState = mRP_US_NotInitialized;
  pTexture->resolution = size;
  pTexture->resolutionF = (mVec2f)pTexture->resolution;
  pTexture->foreignTexture = false;

  mERROR_IF(!(pixelFormat == mPF_R8G8B8 || pixelFormat == mPF_R8G8B8A8 || pixelFormat == mPF_Monochrome8), mR_NotSupported);

#if defined(mRENDERER_OPENGL)
  mERROR_IF(textureUnit >= 32, mR_IndexOutOfBounds);
  pTexture->textureUnit = (GLuint)textureUnit;

  glActiveTexture(GL_TEXTURE0 + (GLuint)pTexture->textureUnit);
  glGenTextures(1, &pTexture->textureId);
  glBindTexture(GL_TEXTURE_2D, pTexture->textureId);

  if (pixelFormat == mPF_R8G8B8A8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  else if (pixelFormat == mPF_R8G8B8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)size.x, (GLsizei)size.y, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  else if (pixelFormat == mPF_Monochrome8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (GLsizei)size.x, (GLsizei)size.y, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, nullptr);
  else
    mRETURN_RESULT(mR_NotSupported);

  mGL_DEBUG_ERROR_CHECK();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  pTexture->uploadState = mRP_US_Ready;

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_Destroy, IN_OUT mTexture *pTexture)
{
  mFUNCTION_SETUP();

  mERROR_IF(pTexture == nullptr, mR_ArgumentNull);

  mGL_DEBUG_ERROR_CHECK();

  if (!pTexture->foreignTexture)
  {
#if defined(mRENDERER_OPENGL)
    if (pTexture->uploadState != mRP_US_NotInitialized)
      glDeleteTextures(1, &pTexture->textureId);

    pTexture->textureId = (GLuint)-1;
#endif
  }
  else
  {
#if defined(mRENDERER_OPENGL)
    pTexture->textureId = (GLuint)-1;
#endif
  }

  if (pTexture->imageBuffer != nullptr)
    mERROR_CHECK(mImageBuffer_Destroy(&pTexture->imageBuffer));

  mGL_DEBUG_ERROR_CHECK();

  pTexture->uploadState = mRP_US_NotInitialized;

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_GetUploadState, mTexture &texture, OUT mRenderParams_UploadState *pUploadState)
{
  mFUNCTION_SETUP();

  mERROR_IF(pUploadState == nullptr, mR_ArgumentNull);

  *pUploadState = texture.uploadState;

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_Upload, mTexture &texture)
{
  mFUNCTION_SETUP();

  mERROR_IF(texture.uploadState == mRP_US_NotInitialized, mR_NotInitialized);

  if (texture.uploadState == mRP_US_Ready)
    mRETURN_SUCCESS();

  mERROR_IF(texture.imageBuffer == nullptr, mR_NotInitialized);

  texture.uploadState = mRP_US_Uploading;

#if defined (mRENDERER_OPENGL)
  glBindTexture(GL_TEXTURE_2D, texture.textureId);

  if (texture.imageBuffer->pixelFormat == mPF_R8G8B8A8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)texture.imageBuffer->currentSize.x, (GLsizei)texture.imageBuffer->currentSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture.imageBuffer->pPixels);
  else if (texture.imageBuffer->pixelFormat == mPF_R8G8B8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei)texture.imageBuffer->currentSize.x, (GLsizei)texture.imageBuffer->currentSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, texture.imageBuffer->pPixels);
  else if (texture.imageBuffer->pixelFormat == mPF_Monochrome8)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, (GLsizei)texture.imageBuffer->currentSize.x, (GLsizei)texture.imageBuffer->currentSize.y, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, texture.imageBuffer->pPixels);
  else
  {
    mPtr<mImageBuffer> imageBuffer;
    mDEFER_CALL(&imageBuffer, mImageBuffer_Destroy);
    mERROR_CHECK(mImageBuffer_Create(&imageBuffer, nullptr, texture.imageBuffer->currentSize, mPF_R8G8B8A8));
    mERROR_CHECK(mPixelFormat_TransformBuffer(texture.imageBuffer, imageBuffer));

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)imageBuffer->currentSize.x, (GLsizei)imageBuffer->currentSize.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer->pPixels);
  }

  texture.resolution = texture.imageBuffer->currentSize;
  texture.resolutionF = (mVec2f)texture.resolution;

  mGL_DEBUG_ERROR_CHECK();

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

#else
  mRETURN_RESULT(mR_NotImplemented)
#endif

  mGL_DEBUG_ERROR_CHECK();

  texture.uploadState = mRP_US_Ready;

  mERROR_CHECK(mImageBuffer_Destroy(&texture.imageBuffer));

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_Bind, mTexture &texture, const size_t textureUnit /* = 0 */)
{
  mFUNCTION_SETUP();

  if (texture.uploadState != mRP_US_Ready)
    mERROR_CHECK(mTexture_Upload(texture));

  texture.textureUnit = (GLuint)textureUnit;

  glActiveTexture(GL_TEXTURE0 + texture.textureUnit);
  glBindTexture(GL_TEXTURE_2D, texture.textureId);

  mGL_DEBUG_ERROR_CHECK();

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_SetTo, mTexture &texture, mPtr<mImageBuffer> &imageBuffer, const bool upload /* = true */)
{
  mFUNCTION_SETUP();

  mERROR_IF(imageBuffer == nullptr, mR_ArgumentNull);

  texture.uploadState = mRP_US_NotInitialized;
  texture.resolution = imageBuffer->currentSize;
  texture.resolutionF = (mVec2f)texture.resolution;

#if defined(mRENDERER_OPENGL)
  glActiveTexture(GL_TEXTURE0 + (GLuint)texture.textureUnit);

  texture.imageBuffer = imageBuffer;
#else
  mRETURN_RESULT(mR_NotInitialized);
#endif

  texture.uploadState = mRP_US_NotUploaded;

  if (upload)
    mERROR_CHECK(mTexture_Upload(texture));

  mGL_DEBUG_ERROR_CHECK();

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_SetTo, mTexture &texture, const uint8_t *pData, const mVec2s &size, const mPixelFormat pixelFormat /* = mPF_B8G8R8A8 */, const bool upload /* = true */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pData == nullptr, mR_ArgumentNull);

  mPtr<mImageBuffer> imageBuffer;
  mDEFER_CALL(&imageBuffer, mImageBuffer_Destroy);
  mERROR_CHECK(mImageBuffer_Create(&imageBuffer, nullptr, (void *)pData, size, pixelFormat));

  mERROR_CHECK(mTexture_SetTo(texture, imageBuffer, upload));

  mRETURN_SUCCESS();
}

mFUNCTION(mTexture_Download, mTexture &texture, OUT mPtr<mImageBuffer> *pImageBuffer, IN mAllocator *pAllocator, const mPixelFormat pixelFormat /* = mPF_R8G8B8A8 */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pImageBuffer == nullptr, mR_ArgumentNull);

#if defined(mRENDERER_OPENGL)
  if (texture.foreignTexture)
  {
    mVec2t<GLint> resolution;

    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &resolution.x);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &resolution.y);

    texture.resolution = (mVec2s)resolution;
    texture.resolutionF = (mVec2f)resolution;
  }

  GLenum glPixelFormat = GL_RGBA;
  mPixelFormat internalPixelFormat = mPF_B8G8R8A8;

  switch (pixelFormat)
  {
  case mPF_B8G8R8:
  case mPF_R8G8B8:
    glPixelFormat = GL_RGB;
    internalPixelFormat = mPF_R8G8B8;
    break;

  case mPF_B8G8R8A8:
  case mPF_R8G8B8A8:
    glPixelFormat = GL_RGBA;
    internalPixelFormat = mPF_R8G8B8A8;
    break;

  case mPF_Monochrome8:
  case mPF_Monochrome16:
    glPixelFormat = GL_RED;
    internalPixelFormat = mPF_Monochrome8;
    break;

  case mPF_YUV422:
  case mPF_YUV420:
  default:
    mRETURN_RESULT(mR_InvalidParameter);
    break;
  }

  mPtr<mImageBuffer> imageBuffer;
  mERROR_CHECK(mImageBuffer_Create(&imageBuffer, internalPixelFormat == pixelFormat ? pAllocator : &mDefaultTempAllocator, texture.resolution, internalPixelFormat));

  glBindTexture(GL_TEXTURE_2D, texture.textureId);
  glGetTexImage(GL_TEXTURE_2D, 0, glPixelFormat, GL_UNSIGNED_BYTE, imageBuffer->pPixels);

  mERROR_CHECK(mImageBuffer_FlipY(imageBuffer));

  if (internalPixelFormat != pixelFormat)
  {
    mERROR_CHECK(mImageBuffer_Create(pImageBuffer, pAllocator, texture.resolution, pixelFormat));
    mERROR_CHECK(mPixelFormat_TransformBuffer(imageBuffer, *pImageBuffer));
  }
  else
  {
    *pImageBuffer = imageBuffer;
  }
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mCreateResource, OUT mTexture *pTexture, const mString &filename)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mTexture_Create(pTexture, filename, false));

  mRETURN_SUCCESS();
}

mFUNCTION(mDestroyResource, IN_OUT mTexture *pTexture)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mTexture_Destroy(pTexture));

  mRETURN_SUCCESS();
}

mFUNCTION(mDestruct, IN_OUT mTexture *pTexture)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mTexture_Destroy(pTexture));

  mRETURN_SUCCESS();
}
