// Copyright 2018 Christoph Stiller
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include "mShader.h"
#include "mFile.h"

#if defined (mRENDERER_OPENGL)
GLuint mShader_CurrentlyBoundShader = (GLuint)-1;
#endif

mFUNCTION(mShader_Create, OUT mShader *pShader, const std::string & vertexShader, const std::string & fragmentShader, IN OPTIONAL const char *fragDataLocation /* = nullptr */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pShader == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mMemset(pShader, 1));
  pShader->initialized = false;

#if defined(mRENDERER_OPENGL)

  const char *vertexSource = vertexShader.c_str();
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

  const char *fragmentSource = fragmentShader.c_str();
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

  pShader->shaderProgram = glCreateProgram();
  glAttachShader(pShader->shaderProgram, vertexShaderHandle);
  glAttachShader(pShader->shaderProgram, fragmentShaderHandle);

  if (fragDataLocation != nullptr)
    glBindFragDataLocation(pShader->shaderProgram, 0, fragDataLocation);

  glLinkProgram(pShader->shaderProgram);

  pShader->initialized = true;

  mGL_DEBUG_ERROR_CHECK();
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_CreateFromFile, OUT mShader *pShader, const std::wstring & filename, IN OPTIONAL const char *fragDataLocation /* = nullptr */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pShader == nullptr, mR_ArgumentNull);

#if defined(mRENDERER_OPENGL)
  mERROR_CHECK(mShader_CreateFromFile(pShader, filename + L".frag", filename + L".vert", fragDataLocation));
#else
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}

mFUNCTION(mShader_CreateFromFile, OUT mShader *pShader, const std::wstring & vertexShaderPath, const std::wstring & fragmentShaderPath, IN OPTIONAL const char *fragDataLocation /* = nullptr */)
{
  mFUNCTION_SETUP();

  mERROR_IF(pShader == nullptr, mR_ArgumentNull);

#if defined(mRENDERER_OPENGL)
  std::string vert;
  std::string frag;

  mERROR_CHECK(mFile_ReadAllText(vertexShaderPath, nullptr, &vert));
  mERROR_CHECK(mFile_ReadAllText(fragmentShaderPath, nullptr, &frag));

  mERROR_CHECK(mShader_Create(pShader, vert, frag, fragDataLocation));
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
  glUniformMatrix4fv(index, 4, false, &v._11);
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
  mDEFER_DESTRUCTION(&pValues, mFreePtrStack);
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
  mDEFER_DESTRUCTION(&pValues, mFreePtrStack);
  mERROR_CHECK(mAllocStackZero(&pValues, count));

  for (size_t i = 0; i < count; ++i)
    pValues[i] = (GLint)pV[i]->textureId;

  glUniform1iv(index, (GLsizei)count, pValues);
#else 
  mRETURN_RESULT(mR_NotImplemented);
#endif

  mRETURN_SUCCESS();
}
