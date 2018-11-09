#include "mShader.h"
#include "mFile.h"

#if defined (mRENDERER_OPENGL)
GLuint mShader_CurrentlyBoundShader = (GLuint)-1;
#endif

mFUNCTION(mShader_Create, OUT mShader *pShader, const mString &vertexShader, const mString &fragmentShader, IN OPTIONAL const char *fragDataLocation /* = nullptr */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pShader == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mMemset(pShader, 1));
  pShader->initialized = false;

#if defined(mRENDERER_OPENGL)

  char *vertexSource = nullptr;
  mDEFER(mAllocator_FreePtr(nullptr, &vertexSource));
  mERROR_CHECK(mAllocator_Allocate(nullptr, &vertexSource, vertexShader.bytes));

  size_t position = 0;

  const mchar_t carriageReturn = mToChar<1>("\r");

  for (auto &&_char : vertexShader)
  {
    if (_char.codePoint != carriageReturn)
    {
      char *s = (char *)_char.character;

      for (size_t i = 0; i < _char.characterSize; i++)
      {
        vertexSource[position] = *s;
        position++;
        s++;
      }
    }
  }

  vertexSource[position] = '\0';

  GLuint vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
  mDEFER(glDeleteShader(vertexShaderHandle));
  glShaderSource(vertexShaderHandle, 1, &vertexSource, NULL);
  glCompileShader(vertexShaderHandle);

  GLint status;
  glGetShaderiv(vertexShaderHandle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    mPRINT_ERROR("Error compiling vertex shader:\n");
    mPRINT_ERROR(vertexSource);
    mPRINT_ERROR("\n\nThe following error occured:\n");
    char buffer[1024];
    glGetShaderInfoLog(vertexShaderHandle, sizeof(buffer), nullptr, buffer);
    mPRINT_ERROR(buffer);
    mERROR_IF(true, mR_ResourceInvalid);
  }

  char *fragmentSource = nullptr;
  mDEFER(mAllocator_FreePtr(nullptr, &fragmentSource));
  mERROR_CHECK(mAllocator_Allocate(nullptr, &fragmentSource, fragmentShader.bytes));

  position = 0;

  for (auto &&_char : fragmentShader)
  {
    if (_char.codePoint != carriageReturn)
    {
      char *s = (char *)_char.character;

      for (size_t i = 0; i < _char.characterSize; i++)
      {
        fragmentSource[position] = *s;
        position++;
        s++;
      }
    }
  }

  fragmentSource[position] = '\0';

  GLuint fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
  mDEFER(glDeleteShader(fragmentShaderHandle));
  glShaderSource(fragmentShaderHandle, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShaderHandle);

  status = GL_TRUE;
  glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    mPRINT_ERROR("Error compiling fragment shader:\n");
    mPRINT_ERROR(fragmentSource);
    mPRINT_ERROR("\n\nThe following error occured:\n");
    char buffer[1024];
    glGetShaderInfoLog(fragmentShaderHandle, sizeof(buffer), nullptr, buffer);
    mPRINT_ERROR(buffer);
    mERROR_IF(true, mR_ResourceInvalid);
  }

  pShader->shaderProgram = glCreateProgram();
  glAttachShader(pShader->shaderProgram, vertexShaderHandle);
  glAttachShader(pShader->shaderProgram, fragmentShaderHandle);

  if (fragDataLocation != nullptr)
    glBindFragDataLocation(pShader->shaderProgram, 0, fragDataLocation);

  glLinkProgram(pShader->shaderProgram);

  glGetProgramiv(pShader->shaderProgram, GL_LINK_STATUS, &status);
  mERROR_IF(status != GL_TRUE, mR_ResourceInvalid);

  pShader->initialized = true;

  mGL_DEBUG_ERROR_CHECK();
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_CreateFromFile, OUT mShader *pShader, const mString &filename)
{
  mFUNCTION_SETUP();

  mERROR_IF(pShader == nullptr, mR_ArgumentNull);

#if defined(mRENDERER_OPENGL)
  mERROR_CHECK(mShader_CreateFromFile(pShader, filename + ".vert", filename + ".frag", nullptr));
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_CreateFromFile, OUT mShader *pShader, const mString &vertexShaderPath, const mString &fragmentShaderPath, IN OPTIONAL const char *fragDataLocation /* = nullptr */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pShader == nullptr, mR_ArgumentNull);

#if defined(mRENDERER_OPENGL)
  mString vert;
  mString frag;

  mERROR_CHECK(mFile_ReadAllText(vertexShaderPath, nullptr, &vert));
  mERROR_CHECK(mFile_ReadAllText(fragmentShaderPath, nullptr, &frag));

  mERROR_CHECK(mShader_Create(pShader, vert, frag, fragDataLocation));

  pShader->vertexShader = vertexShaderPath;
  pShader->fragmentShader = fragmentShaderPath;
  pShader->loadedFromFile = true;
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_Destroy, IN_OUT mShader *pShader)
{
  mFUNCTION_SETUP();

  mERROR_IF(pShader == nullptr, mR_ArgumentNull);

#if defined(mRENDERER_OPENGL)

  if (pShader->initialized)
    glDeleteProgram(pShader->shaderProgram);

#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  pShader->initialized = false;
  pShader->loadedFromFile = false;
  mERROR_CHECK(mDestruct(&pShader->vertexShader));
  mERROR_CHECK(mDestruct(&pShader->fragmentShader));

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetTo, mPtr<mShader> &shader, const mString &vertexShader, const mString &fragmentShader, IN OPTIONAL const char *fragDataLocation)
{
  mFUNCTION_SETUP();

  mERROR_IF(shader == nullptr, mR_ArgumentNull);
  mERROR_IF(!shader->initialized, mR_NotInitialized);

#if defined(mRENDERER_OPENGL)

  GLuint subShaders[3];
  GLsizei subShaderCount;
  glGetAttachedShaders(shader->shaderProgram, 3, &subShaderCount, subShaders);

  for (size_t i = 0; i < subShaderCount; ++i)
  {
    glDeleteShader(subShaders[i]);
    glDetachShader(shader->shaderProgram, subShaders[i]);
  }

  char *vertexSource = nullptr;
  mDEFER(mAllocator_FreePtr(nullptr, &vertexSource));
  mERROR_CHECK(mAllocator_Allocate(nullptr, &vertexSource, vertexShader.bytes));

  size_t position = 0;

  const mchar_t carriageReturn = mToChar<1>("\r");

  for (auto &&_char : vertexShader)
  {
    if (_char.codePoint != carriageReturn)
    {
      char *s = (char *)_char.character;

      for (size_t i = 0; i < _char.characterSize; i++)
      {
        vertexSource[position] = *s;
        position++;
        s++;
      }
    }
  }

  vertexSource[position] = '\0';

  GLuint vertexShaderHandle = glCreateShader(GL_VERTEX_SHADER);
  mDEFER(glDeleteShader(vertexShaderHandle));
  glShaderSource(vertexShaderHandle, 1, &vertexSource, NULL);
  glCompileShader(vertexShaderHandle);

  GLint status;
  glGetShaderiv(vertexShaderHandle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    mPRINT("Error compiling vertex shader:\n");
    mPRINT(vertexSource);
    mPRINT("\n\nThe following error occured:\n");
    char buffer[1024];
    glGetShaderInfoLog(vertexShaderHandle, sizeof(buffer), nullptr, buffer);
    mPRINT(buffer);
    mERROR_IF(true, mR_ResourceInvalid);
  }

  char *fragmentSource = nullptr;
  mDEFER(mAllocator_FreePtr(nullptr, &fragmentSource));
  mERROR_CHECK(mAllocator_Allocate(nullptr, &fragmentSource, fragmentShader.bytes));

  position = 0;

  for (auto &&_char : fragmentShader)
  {
    if (_char.codePoint != carriageReturn)
    {
      char *s = (char *)_char.character;

      for (size_t i = 0; i < _char.characterSize; i++)
      {
        fragmentSource[position] = *s;
        position++;
        s++;
      }
    }
  }

  fragmentSource[position] = '\0';

  GLuint fragmentShaderHandle = glCreateShader(GL_FRAGMENT_SHADER);
  mDEFER(glDeleteShader(fragmentShaderHandle));
  glShaderSource(fragmentShaderHandle, 1, &fragmentSource, NULL);
  glCompileShader(fragmentShaderHandle);

  status = GL_TRUE;
  glGetShaderiv(fragmentShaderHandle, GL_COMPILE_STATUS, &status);

  if (status != GL_TRUE)
  {
    mPRINT("Error compiling fragment shader:\n");
    mPRINT(fragmentSource);
    mPRINT("\n\nThe following error occured:\n");
    char buffer[1024];
    glGetShaderInfoLog(fragmentShaderHandle, sizeof(buffer), nullptr, buffer);
    mPRINT(buffer);
    mERROR_IF(true, mR_ResourceInvalid);
  }

  glUseProgram(shader->shaderProgram);

  glAttachShader(shader->shaderProgram, vertexShaderHandle);
  glAttachShader(shader->shaderProgram, fragmentShaderHandle);

  if (fragDataLocation != nullptr)
    glBindFragDataLocation(shader->shaderProgram, 0, fragDataLocation);

  glLinkProgram(shader->shaderProgram);

  glGetProgramiv(shader->shaderProgram, GL_LINK_STATUS, &status);
  mERROR_IF(status != GL_TRUE, mR_ResourceInvalid);

  mGL_DEBUG_ERROR_CHECK();
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetToFile, mPtr<mShader> &shader, const mString &filename)
{
  mFUNCTION_SETUP();

  mERROR_CHECK(mShader_SetToFile(shader, filename + L".vert", filename + L".frag", nullptr));

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_ReloadFromFile, mPtr<mShader> &shader)
{
  mFUNCTION_SETUP();

  mERROR_IF(shader == nullptr, mR_ArgumentNull);
  mERROR_IF(!shader->initialized, mR_NotInitialized);
  mERROR_IF(!shader->loadedFromFile, mR_ResourceIncompatible);

  mString vertexShader = shader->vertexShader;
  mString fragmentShader = shader->fragmentShader;

  mResult result = mShader_SetToFile(shader, vertexShader, fragmentShader);

  mERROR_IF(mFAILED(result) && result != mR_ResourceNotFound, result);

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetToFile, mPtr<mShader> &shader, const mString &vertexShaderPath, const mString &fragmentShaderPath, IN OPTIONAL const char *fragDataLocation /* = nullptr */)
{
  mFUNCTION_SETUP();

  mERROR_IF(shader == nullptr, mR_ArgumentNull);

  mString vertexShader, fragmentShader;

  mERROR_CHECK(mFile_ReadAllText(vertexShaderPath, nullptr, &vertexShader));
  mERROR_CHECK(mFile_ReadAllText(fragmentShaderPath, nullptr, &fragmentShader));

  mERROR_CHECK(mShader_SetTo(shader, vertexShader, fragmentShader, fragDataLocation));

  shader->vertexShader = vertexShaderPath;
  shader->fragmentShader = fragmentShaderPath;
  shader->loadedFromFile = true;
  
  mRETURN_SUCCESS();
}

mFUNCTION(mShader_Bind, mShader &shader)
{
  mFUNCTION_SETUP();

#if defined(_DEBUG)
  mERROR_IF(!shader.initialized, mR_NotInitialized);
#endif

#if defined(mRENDERER_OPENGL)
  if (shader.shaderProgram == mShader_CurrentlyBoundShader)
    mRETURN_SUCCESS();

  mShader_CurrentlyBoundShader = shader.shaderProgram;

  glUseProgram(shader.shaderProgram);
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const int32_t v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1i(index, v);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif
  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const uint32_t v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1ui(index, v);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif
  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const float_t v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1f(index, v);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif
  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVec2f &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform2f(index, v.x, v.y);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVec3f &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform3f(index, v.x, v.y, v.z);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVec4f &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform4f(index, v.x, v.y, v.z, v.w);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVector &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform4f(index, v.x, v.y, v.z, v.w);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mMatrix &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniformMatrix4fv(index, 1, false, &v._11);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, mTexture &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1i(index, (GLint)v.textureUnit);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, mPtr<mTexture> &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1i(index, (GLint)v->textureUnit);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, mPtr<mFramebuffer> &v)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1i(index, (GLint)v->textureUnit);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const int32_t *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1iv(index, (GLsizei)count, pV);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const uint32_t *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1uiv(index, (GLsizei)count, pV);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const float_t *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform1fv(index, (GLsizei)count, pV);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVec2f *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform2fv(index, (GLsizei)count, (float_t *)pV);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVec3f *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform3fv(index, (GLsizei)count, (float_t *)pV);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVec4f *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform4fv(index, (GLsizei)count, (float_t *)pV);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mVector *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  glUniform4fv(index, (GLsizei)count, (float_t *)pV);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader &shader, const shaderAttributeIndex_t index, const mTexture *pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  GLint *pValues;
  mDEFER_CALL(&pValues, mFreePtrStack);
  mERROR_CHECK(mAllocStackZero(&pValues, count));

  for (size_t i = 0; i < count; ++i)
    pValues[i] = (GLint)pV[i].textureId;

  glUniform1iv(index, (GLsizei)count, pValues);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_SetUniformAtIndex, mShader & shader, const shaderAttributeIndex_t index, const mPtr<mTexture>* pV, const size_t count)
{
  mFUNCTION_SETUP();
  mERROR_CHECK(mShader_Bind(shader));

#if defined (mRENDERER_OPENGL)
  GLint *pValues;
  mDEFER_CALL(&pValues, mFreePtrStack);
  mERROR_CHECK(mAllocStackZero(&pValues, count));

  for (size_t i = 0; i < count; ++i)
    pValues[i] = (GLint)pV[i]->textureId;

  glUniform1iv(index, (GLsizei)count, pValues);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}
