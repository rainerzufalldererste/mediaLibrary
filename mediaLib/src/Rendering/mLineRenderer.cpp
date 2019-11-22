#include "mLineRenderer.h"

#include "mRenderParams.h"
#include "mRenderDataBuffer.h"
#include "mBinaryChunk.h"

extern const char mLineRenderer_PositionAttribute[] = "position0";
extern const char mLineRenderer_ColourAttribute[] = "colour0";
const char mLineRenderer_ScreenSizeUniformName[] = "screenSize";

struct mLineRenderer
{
  float_t subdivisionFactor;
  mPtr<mShader> shader;
  mRenderDataBuffer<mRDB_FloatAttribute<2, mLineRenderer_PositionAttribute>, mRDB_FloatAttribute<4, mLineRenderer_ColourAttribute>> renderDataBuffer;
  mPtr<mBinaryChunk> renderData;
  bool started;
};

static const char mLineRenderer_VertexShader[] = mGLSL(
  attribute vec2 position0;
  attribute vec4 colour0;

  out vec4 _colour0;

  uniform vec2 screenSize;

  void main()
  {
    _colour0 = colour0;

    gl_Position = vec4(position0 / screenSize, 0, 1.0);
    gl_Position.y = 1.0 - gl_Position.y;
    gl_Position.xy = vec2(-1, -1) + gl_Position.xy * 2.0;
  }
);

static const char mLineRenderer_FragmentShader[] = mGLSL(
  out vec4 outColour;
  
  in vec4 _colour0;
  
  void main()
  {
    outColour = _colour0;
  }
);

mFUNCTION(mLineRenderer_Destroy_Internal, OUT mLineRenderer *pLineRenderer);

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mLineRenderer_Create, OUT mPtr<mLineRenderer> *pLineRenderer, IN mAllocator *pAllocator, const float_t subdivisionFactor)
{
  mFUNCTION_SETUP();

  mERROR_IF(pLineRenderer == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mSharedPointer_Allocate(pLineRenderer, pAllocator, (std::function<void(mLineRenderer *)>)[](mLineRenderer *pData) {mLineRenderer_Destroy_Internal(pData);}, 1));

  mERROR_CHECK(mShader_Allocate(&(*pLineRenderer)->shader, pAllocator));
  mERROR_CHECK(mShader_Create((*pLineRenderer)->shader.GetPointer(), mLineRenderer_VertexShader, mLineRenderer_FragmentShader));
  
  mERROR_CHECK(mRenderDataBuffer_Create(&(*pLineRenderer)->renderDataBuffer, (*pLineRenderer)->shader, true));
  mERROR_CHECK(mRenderDataBuffer_SetVertexRenderMode((*pLineRenderer)->renderDataBuffer, mRP_VRM_TriangleList));
  mERROR_CHECK(mBinaryChunk_Create(&(*pLineRenderer)->renderData, pAllocator));

  (*pLineRenderer)->subdivisionFactor = subdivisionFactor;

  mRETURN_SUCCESS();
}

mFUNCTION(mLineRenderer_Destroy, IN_OUT mPtr<mLineRenderer> *pLineRenderer)
{
  mFUNCTION_SETUP();

  mERROR_IF(pLineRenderer == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mSharedPointer_Destroy(pLineRenderer));

  mRETURN_SUCCESS();
}

mFUNCTION(mLineRenderer_Begin, mPtr<mLineRenderer> &lineRenderer)
{
  mFUNCTION_SETUP();

  mERROR_IF(lineRenderer == nullptr, mR_ArgumentNull);
  mERROR_IF(lineRenderer->started, mR_ResourceStateInvalid);

  lineRenderer->started = true;

  mRETURN_SUCCESS();
}

mFUNCTION(mLineRenderer_DrawStraightLine, mPtr<mLineRenderer> &lineRenderer, const mLineRenderer_Point &startPoint, const mLineRenderer_Point &endPoint)
{
  mFUNCTION_SETUP();

  mERROR_IF(lineRenderer == nullptr, mR_ArgumentNull);
  mERROR_IF(!lineRenderer->started, mR_ResourceStateInvalid);

  const mVec2f direction = endPoint.position - startPoint.position;
  const mVec2f orthogonal = mVec2f(direction.y, -direction.x).Normalize();

  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, startPoint.position + orthogonal * startPoint.thickness));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, startPoint.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.position + orthogonal * endPoint.thickness));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.position - orthogonal * endPoint.thickness));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, startPoint.position + orthogonal * startPoint.thickness));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, startPoint.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, startPoint.position - orthogonal * startPoint.thickness));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, startPoint.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.position - orthogonal * endPoint.thickness));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.colour));

  mRETURN_SUCCESS();
}

mFUNCTION(mLineRenderer_DrawStraightArrow, mPtr<mLineRenderer> &lineRenderer, const mLineRenderer_Point &startPoint, const mLineRenderer_Point &endPoint, const float_t arrowSize)
{
  mFUNCTION_SETUP();

  mERROR_IF(lineRenderer == nullptr, mR_ArgumentNull);
  mERROR_IF(!lineRenderer->started, mR_ResourceStateInvalid);

  const mVec2f direction = endPoint.position - startPoint.position;
  const float_t length = (float_t)direction.Length();
  const mVec2f orthogonal = mVec2f(direction.y, -direction.x) / length;

  const mVec2f lineEndPoint = startPoint.position + direction / length * (length - arrowSize);

  mERROR_CHECK(mLineRenderer_DrawStraightLine(lineRenderer, startPoint, mLineRenderer_Point(lineEndPoint, endPoint.colour, endPoint.thickness)));

  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, lineEndPoint - orthogonal * (endPoint.thickness + arrowSize * 0.5f)));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, lineEndPoint + orthogonal * (endPoint.thickness + arrowSize * 0.5f)));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.position));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, endPoint.colour));

  mRETURN_SUCCESS();
}

mFUNCTION(mLineRenderer_DrawCubicBezierLine, mPtr<mLineRenderer> &lineRenderer, const mCubicBezierCurve<float_t> &curve, const mLineRenderer_Attribute &start, const mLineRenderer_Attribute &end)
{
  return mLineRenderer_DrawCubicBezierLineSegment(lineRenderer, curve, start, end, 0, 1);
}

mFUNCTION(mLineRenderer_DrawCubicBezierArrow, mPtr<mLineRenderer> &lineRenderer, const mCubicBezierCurve<float_t> &curve, const mLineRenderer_Attribute &start, const mLineRenderer_Attribute &end, const float_t arrowSize)
{
  mFUNCTION_SETUP();

  mUnused(arrowSize);

  mERROR_CHECK(mLineRenderer_DrawCubicBezierLineSegment(lineRenderer, curve, start, end, 0, 1));

  const mVec2f direction = mInterpolate(curve, 1.f) - mInterpolate(curve, 1.f - (float_t)1e-3);
  const float_t length = (float_t)direction.Length();
  const mVec2f orthogonal = mVec2f(direction.y, -direction.x) / length;

  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, curve.endPoint - orthogonal * (end.thickness + arrowSize * 0.5f)));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, end.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, curve.endPoint + orthogonal * (end.thickness + arrowSize * 0.5f)));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, end.colour));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, curve.endPoint + arrowSize * direction / length));
  mERROR_CHECK(mBinaryChunk_WriteData(lineRenderer->renderData, end.colour));

  mRETURN_SUCCESS();
}

mFUNCTION(mLineRenderer_DrawCubicBezierLineSegment, mPtr<mLineRenderer> &lineRenderer, const mCubicBezierCurve<float_t> &curve, const mLineRenderer_Attribute &start, const mLineRenderer_Attribute &end, const float_t startT, const float_t endT)
{
  mFUNCTION_SETUP();

  // TODO: Move to adaptive recursive angle based method with no overlaps.

  constexpr float_t stepSize = 1.0f / 32.0f;

  mLineRenderer_Point lastPoint = mLineRenderer_Point(mInterpolate(curve, startT), mLerp(start.colour, end.colour, startT), mLerp(start.thickness, end.thickness, startT));
  float_t lastT = startT;

  while (true)
  {
    const float_t t = mClamp(lastT + stepSize, 0.f, 1.f);
    const mLineRenderer_Point point = mLineRenderer_Point(mInterpolate(curve, t), mLerp(start.colour, end.colour, t), mLerp(start.thickness, end.thickness, t));

    mERROR_CHECK(mLineRenderer_DrawStraightLine(lineRenderer, lastPoint, point));

    if (mMin(endT, 1.f) - mSmallest<float_t>() < t)
      break;

    lastT = t;
    lastPoint = point;
  }

  mRETURN_SUCCESS();
}

mFUNCTION(mLineRenderer_End, mPtr<mLineRenderer> &lineRenderer)
{
  mFUNCTION_SETUP();

  mERROR_IF(lineRenderer == nullptr, mR_ArgumentNull);
  mERROR_IF(!lineRenderer->started, mR_ResourceStateInvalid);

  mDEFER(mBinaryChunk_ResetWrite(lineRenderer->renderData));
  
  lineRenderer->started = false;
  
  size_t bytes = 0;
  mERROR_CHECK(mBinaryChunk_GetWriteBytes(lineRenderer->renderData, &bytes));

  if (bytes > 0)
  {
    mERROR_CHECK(mRenderDataBuffer_SetVertexBuffer(lineRenderer->renderDataBuffer, (uint8_t *)lineRenderer->renderData->pData, bytes));
    mERROR_CHECK(mShader_Bind(*lineRenderer->renderDataBuffer.shader));
    mERROR_CHECK(mShader_SetUniform(lineRenderer->renderDataBuffer.shader, mLineRenderer_ScreenSizeUniformName, mRenderParams_CurrentRenderResolutionF));
    mERROR_CHECK(mRenderParams_SetBlendFunc(mRP_BF_AlphaBlend));
    mERROR_CHECK(mRenderDataBuffer_Draw(lineRenderer->renderDataBuffer));
  }

  mRETURN_SUCCESS();
}

//////////////////////////////////////////////////////////////////////////

mFUNCTION(mLineRenderer_Destroy_Internal, OUT mLineRenderer *pLineRenderer)
{
  mFUNCTION_SETUP();

  mERROR_IF(pLineRenderer == nullptr, mR_ArgumentNull);

  mERROR_CHECK(mBinaryChunk_Destroy(&pLineRenderer->renderData));
  mERROR_CHECK(mRenderDataBuffer_Destroy(&pLineRenderer->renderDataBuffer));
  mERROR_CHECK(mSharedPointer_Destroy(&pLineRenderer->shader));

  mRETURN_SUCCESS();
}
