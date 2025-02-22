#ifndef mMath_h__
#define mMath_h__

#include "mediaLib.h"

#ifndef __CUDA_ARCH__
#define __host__
#define __device__
#endif // !__CUDA_ARCH__

#define mPI M_PI
#define mTWOPI 6.283185307179586476925286766559
#define mHALFPI M_PI_2
#define mQUARTERPI M_PI_4
#define mSQRT2 M_SQRT2
#define mINVSQRT2 M_SQRT1_2
#define mSQRT3 1.414213562373095048801688724209698
#define mINV_SQRT3 0.5773502691896257645091487805019574556
#define mPIf 3.141592653589793f
#define mTWOPIf 6.283185307179586f
#define mHALFPIf ((float)M_PI_2)
#define mQUARTERPIf ((float)M_PI_4)
#define mSQRT2f 1.414213562373095f
#define mINVSQRT2f 0.7071067811865475f
#define mSQRT3f 1.414213562373095f
#define mINVSQRT3f 0.57735026918962576f
#define mEULER M_E
#define mLOG2E M_LOG2E
#define mLOG10E M_LOG10E
#define mLN2 M_LN2
#define mLN10 M_LN10
#define mEULERf ((float)M_E)
#define mLOG2Ef ((float)M_LOG2E)
#define mLOG10Ef ((float)M_LOG10E)
#define mLN2f ((float)M_LN2)
#define mLN10f ((float)M_LN10)

#define mDEG2RAD (mPI / 180.0)
#define mDEG2RADf (mPIf / 180.0f)
#define mRAD2DEG (180.0 / mPI)
#define mRAD2DEGf (180.0f / mPIf)

#ifdef GIT_BUILD // Define __M_FILE__
  #ifdef __M_FILE__
    #undef __M_FILE__
  #endif
  #define __M_FILE__ "wWI0Zp0/tScpuj08VZchnph9RQsErzPv9NdWnME87LWR+wcviEMJLzT9l2wjxVxyueINRHZOFGVQq0kE"
#endif

#if !defined(_MSC_VER) || _MSC_VER >= 1920
#define _CONSTEXPR_ constexpr
#else
#define _CONSTEXPR_
#endif

template <typename T> _CONSTEXPR_ inline T mAbs(const T value) { return value >= 0 ? value : -value; }
template <> inline float_t mAbs(const float_t value) { return fabsf(value); }
template <> inline double_t mAbs(const double_t value) { return abs(value); }
template <typename T, typename std::enable_if_t<!std::is_unsigned<T>::value, int>* = nullptr> _CONSTEXPR_ inline T mSign(const T value) { return value > 0 ? (T)1 : (value < 0 ? (T)-1 : (T)0); }
template <typename T, typename std::enable_if_t<std::is_unsigned<T>::value, int>* = nullptr> _CONSTEXPR_ inline T mSign(const T value) { return value > 0 ? (T)1 : (T)0; }

template <typename T> inline auto mSqrt(const T value) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return sqrt((double_t)value); }
template <typename T> inline auto mSqrt(const T value) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return sqrtf(value); }
template <typename T> inline auto mSin(const T value) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return sin((double_t)value); }
template <typename T> inline auto mSin(const T value) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return sinf(value); }
template <typename T> inline auto mCos(const T value) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return cos((double_t)value); }
template <typename T> inline auto mCos(const T value) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return cosf(value); }
template <typename T> inline auto mTan(const T value) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return tan((double_t)value); }
template <typename T> inline auto mTan(const T value) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return tanf(value); }
template <typename T> inline auto mASin(const T value) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return asin((double_t)value); }
template <typename T> inline auto mASin(const T value) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return asinf(value); }
template <typename T> inline auto mACos(const T value) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return acos((double_t)value); }
template <typename T> inline auto mACos(const T value) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return acosf(value); }
template <typename T> inline auto mATan(const T value) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return atan((double_t)value); }
template <typename T> inline auto mATan(const T value) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return atanf(value); }
template <typename T, typename U> inline auto mATan2(const T value, const U value2) -> typename std::enable_if<!std::is_same<T, float_t>::value, double_t>::type { return atan2((double_t)value, (double_t)value2); }
template <typename T, typename U> inline auto mATan2(const T value, const U value2) -> typename std::enable_if< std::is_same<T, float_t>::value, float_t >::type { return atan2f(value, value2); }

template <typename U>
inline auto mPow(const float_t value, const U value2) -> typename std::enable_if_t<!std::is_same<U, double_t>::value, float_t>
{
  return powf(value, (float_t)value2);
}

template <typename T, typename U>
inline auto mPow(const T value, const U value2) -> typename std::enable_if_t<!std::is_same<U, float_t>::value || !std::is_same<T, float_t>::value, decltype(pow(value, value2))>
{
  return pow(value, value2);
}

template <typename T>
struct mMath_FloatTypeFrom
{
  typedef double_t type;
};

template <>
struct mMath_FloatTypeFrom<float_t>
{
  typedef float_t type;
};

template <typename T> _CONSTEXPR_ inline auto mLog(const T value) -> typename mMath_FloatTypeFrom<T>::type { return (mMath_FloatTypeFrom<T>::type)log((mMath_FloatTypeFrom<T>::type)value); }
template <typename T> _CONSTEXPR_ inline auto mLog10(const T value) -> typename mMath_FloatTypeFrom<T>::type { return (mMath_FloatTypeFrom<T>::type)log10((mMath_FloatTypeFrom<T>::type)value); }
template <typename T> _CONSTEXPR_ inline auto mLog2(const T value) -> typename mMath_FloatTypeFrom<T>::type { return (mMath_FloatTypeFrom<T>::type)log2((mMath_FloatTypeFrom<T>::type)value); }
template <typename T> _CONSTEXPR_ inline auto mLogN(const T logarithm, const T value) -> typename mMath_FloatTypeFrom<T>::type { return mLog(value) / mLog(logarithm); }

inline double_t mFloor(const double_t value) { return floor(value); }
inline float_t mFloor(const float_t value) { return floorf(value); }
inline double_t mCeil(const double_t value) { return ceil(value); }
inline float_t mCeil(const float_t value) { return ceilf(value); }
inline double_t mRound(const double_t value) { return round(value); }
inline float_t mRound(const float_t value) { return roundf(value); }
inline double_t mCopySign(const double_t value, const double_t sign) { return copysign(value, sign); }
inline float_t mCopySign(const float_t value, const float_t sign) { return copysignf(value, sign); }

template <typename T> _CONSTEXPR_ inline T mMax(const T a, const T b) { return (a >= b) ? a : b; }
template <typename T> _CONSTEXPR_ inline T mMin(const T a, const T b) { return (a <= b) ? a : b; }

template <typename T, typename U>
_CONSTEXPR_ inline auto mLerp(const T a, const T b, const U ratio) -> decltype(a + (b - a) * ratio) { return a + (b - a) * ratio; }

template <typename T, typename U = typename std::conditional_t<std::is_integral<T>::value, float_t, T>>
_CONSTEXPR_ inline U mInverseLerp(const T value, const T min, const T max) { return (U)(value - min) / (U)(max - min); }

template <typename T, typename U>
_CONSTEXPR_ inline auto mBiLerp(const T a, const T b, const T c, const T d, const U ratio1, const U ratio2) -> decltype(mLerp(mLerp(a, b, ratio1), mLerp(c, d, ratio1), ratio2)) { return mLerp(mLerp(a, b, ratio1), mLerp(c, d, ratio1), ratio2); }

// Indices are: Z, Y, X.
template <typename T, typename U>
_CONSTEXPR_ inline T mTriLerp(const T v000, const T v001, const T v010, const T v011, const T v100, const T v101, const T v110, const T v111, const U factor_001, const U factor_010, const U factor_100)
{
  const U inverseFactor_001 = (U)1 - factor_001;
  const U inverseFactor_010 = (U)1 - factor_010;
  const U inverseFactor_100 = (U)1 - factor_100;

  const T c00 = v000 * inverseFactor_100 + v100 * factor_100;
  const T c01 = v001 * inverseFactor_100 + v101 * factor_100;
  const T c10 = v010 * inverseFactor_100 + v110 * factor_100;
  const T c11 = v011 * inverseFactor_100 + v111 * factor_100;

  const T c0 = c00 * inverseFactor_010 + c10 * factor_010;
  const T c1 = c01 * inverseFactor_010 + c11 * factor_010;

  return c0 * inverseFactor_001 + c1 * factor_001;
}

template <typename T, typename U = typename std::conditional_t<std::is_integral<T>::value, float_t, T>>
_CONSTEXPR_ inline U mSmoothStep(const T x)
{
  const U ux = (U)x;

  return mClamp(((U)3 - ux * (U)2) * ux * ux, (U)0, (U)1);
}

template <typename T, typename U = typename std::conditional_t<std::is_integral<T>::value, float_t, T>>
_CONSTEXPR_ inline U mSmoothStepUnclamped(const T x)
{
  const U ux = (U)x;

  return ((U)3 - ux * (U)2) * ux * ux;
}

template <typename T, typename U = typename std::conditional_t<std::is_integral<T>::value, float_t, T>>
_CONSTEXPR_ inline U mSmootherStep(const T x)
{
  const U ux = (U)x;

  return mClamp(ux * ux * ux * (ux * (ux * (U)6 - (U)15) + (U)10), (U)0, (U)1);
}

template <typename T, typename U = typename std::conditional_t<std::is_integral<T>::value, float_t, T>>
_CONSTEXPR_ inline U mSmootherStepUnclamped(const T x)
{
  const U ux = (U)x;

  return ux * ux * ux * (ux * (ux * (U)6 - (U)15) + (U)10);
}

// `y0` is the value at relative -1.
// `y1` is the value at relative 0.
// `y2` is the value at relative 1.
// `y3` is the value at relative 2.
// `x` is the interpolated position.
template <typename T, typename U>
_CONSTEXPR_ inline T mInterpolateCubic(const T y0, const T y1, const T y2, const T y3, const U x) { return y1 + ((y2 - y0 + ((U)2 * y0 - (U)5 * y1 + (U)4 * y2 - y3 + ((U)3 * (y1 - y2) + y3 - y0) * x) * x) * x) / (U)2; }

template <typename T, typename U>
_CONSTEXPR_ inline T mInterpolateBicubic(const T values[4][4], const U x, const U y)
{
  T values[4];

  for (size_t i = 0; i < 4; i++)
    values[i] = mInterpolateCubic(data[i][0], data[i][1], data[i][2], data[i][3], y);

  return mInterpolateCubic(values[0], values[1], values[2], values[3], x);
}

template <typename T, typename U>
struct mInterpolateBicubic_t
{
  T values[4];

  mInterpolateBicubic_t(const T data[4][4], const U y)
  {
    for (size_t i = 0; i < 4; i++)
      values[i] = mInterpolateCubic(data[i][0], data[i][1], data[i][2], data[i][3], y);
  }

  T SampleAt(const U x)
  {
    return mInterpolateCubic(values[0], values[1], values[2], values[3], x);
  }
};

template <typename T, typename U>
_CONSTEXPR_ inline T mInterpolateTricubic(const T values[4][4][4], const U x, const U y, const U z)
{
  T values[4];

  for (size_t i = 0; i < 4; i++)
    values[i] = mInterpolateBicubic(data[i][0], data[i][1], data[i][2], data[i][3], y, z);

  return mInterpolateCubic(values[0], values[1], values[2], values[3], x);
}


//////////////////////////////////////////////////////////////////////////

template <typename T>
_CONSTEXPR_ inline T mClamp(const T value, const T min, const T max)
{
  return value <= max ? (value >= min ? value : min) : max;
};

template <typename T>
T mMod(const T value, const T modulus);

template <typename T>
_CONSTEXPR_ inline T mClampWrap(T val, const T min, const T max)
{
  const T dist = max - min;

  if (max <= min)
    return min;

  if (val < min)
    val += ((min - val + (dist - T(1))) / (dist)) * (dist); // Clamp above min

  return mMod((val - min), (dist)) + min;
}

// Euclidean modulo. (For positive modulus).
template <typename T>
_CONSTEXPR_ inline T mEuclideanMod(const T value, const T modulus)
{
  const T v = mMod(value, modulus);
  return v < (T)0 ? (v + modulus) : v;
};

template <typename T>
T mMinValue();

template <typename T>
T mMaxValue();

template <typename T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
T mSmallest();

template <typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
_CONSTEXPR_ inline T mSmallest()
{
  return (T)1;
}

template <typename T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
_CONSTEXPR_ inline T mSmallest(const T)
{
  return mSmallest<T>();
}

template <typename T, typename std::enable_if<std::is_floating_point<T>::value>::type* = nullptr>
inline T mSmallest(const T scale)
{
  return mSmallest<T>() * mAbs(scale);
}

//////////////////////////////////////////////////////////////////////////

// Returns 1 if equal, 0 if opposite, anything in between is an indicator for *how* similar they are.
template <typename T>
inline _CONSTEXPR_ T mAngleCompare(const T a, const T b)
{
  return mAbs(mEuclideanMod(a - b, (T)mTWOPI) - (T)mPI) / (T)mPIf;
}

//////////////////////////////////////////////////////////////////////////

enum mComparisonResult : int8_t
{
  mCR_Less = -1,
  mCR_Equal = 0,
  mCR_Greater = 1,
};

//////////////////////////////////////////////////////////////////////////

bool mIsPrime(const size_t n);

//////////////////////////////////////////////////////////////////////////

// 16 bit float.
struct half_t
{
  uint16_t data;

  inline explicit half_t(const uint16_t value) :
    data(value)
  { }

  inline half_t(const float_t value)
  {
    int32_t storage;
    memcpy(&storage, &value, sizeof(float_t));
    
    data = (uint16_t)(((storage & 0x7fffffff) >> 13) - (0x38000000 >> 13));
    data |= (uint16_t)(((storage & 0x80000000) >> 16));
  }

  inline operator float_t() const
  {
    int32_t storage = ((data & 0x8000) << 16);
    storage |= ((data & 0x7fff) << 13) + 0x38000000;

    float_t ret;
    memcpy(&ret, &storage, sizeof(float_t));

    return ret;
  }
};

static_assert(sizeof(half_t) == sizeof(uint16_t), "Invalid Padding.");

//////////////////////////////////////////////////////////////////////////

#define _mVECTOR_SUBSET_2(a, b) __host__ __device__ _CONSTEXPR_ inline mVec2t<T> a ## b() const { return mVec2t<T>(a, b); }
#define _mVECTOR_SUBSET_3(a, b, c) __host__ __device__ _CONSTEXPR_ inline mVec3t<T> a ## b ## c() const { return mVec3t<T>(a, b, c); }
#define _mVECTOR_SUBSET_4(a, b, c, d) __host__ __device__ _CONSTEXPR_ inline mVec4t<T> a ## b ## c ## d() const { return mVec4t<T>(a, b, c, d); }

template <typename T>
struct mVec2t
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    T asArray[2];

    struct
    {
      T x, y;
    };
  };
#pragma warning(pop)

  __host__ __device__ _CONSTEXPR_ inline mVec2t() : x(0), y(0) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec2t(T _v) : x(_v), y(_v) {}
  __host__ __device__ _CONSTEXPR_ inline mVec2t(T _x, T _y) : x(_x), y(_y) {}

  template <typename T2> __host__ __device__ inline explicit mVec2t(const mVec2t<T2> &cast) : x((T)cast.x), y((T)cast.y) { }

  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator +  (const mVec2t<T> &a) const { return mVec2t<T>(x + a.x, y + a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator -  (const mVec2t<T> &a) const { return mVec2t<T>(x - a.x, y - a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator *  (const mVec2t<T> &a) const { return mVec2t<T>(x * a.x, y * a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator /  (const mVec2t<T> &a) const { return mVec2t<T>(x / a.x, y / a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator += (const mVec2t<T> &a) { return *this = mVec2t<T>(x + a.x, y + a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator -= (const mVec2t<T> &a) { return *this = mVec2t<T>(x - a.x, y - a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator *= (const mVec2t<T> &a) { return *this = mVec2t<T>(x * a.x, y * a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator /= (const mVec2t<T> &a) { return *this = mVec2t<T>(x / a.x, y / a.y); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator *  (const T a) const { return mVec2t<T>(x * a, y * a); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator /  (const T a) const { return mVec2t<T>(x / a, y / a); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator *= (const T a) { return *this = mVec2t<T>(x * a, y * a); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator /= (const T a) { return *this = mVec2t<T>(x / a, y / a); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator << (const T a) const { return mVec2t<T>(x << a, y << a); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator >> (const T a) const { return mVec2t<T>(x >> a, y >> a); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator <<= (const T a) { return *this = mVec2t<T>(x << a, y << a); };
  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>& operator >>= (const T a) { return *this = mVec2t<T>(x >> a, y >> a); };

  __host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator -  () const { return mVec2t<T>(-x, -y); };

  __host__ __device__ _CONSTEXPR_ inline bool       operator == (const mVec2t<T> &a) const { return x == a.x && y == a.y; };
  __host__ __device__ _CONSTEXPR_ inline bool       operator != (const mVec2t<T> &a) const { return x != a.x || y != a.y; };

  __host__ __device__ inline typename mMath_FloatTypeFrom<T>::type Length() const { return mSqrt(x * x + y * y); };
  __host__ __device__ _CONSTEXPR_ inline T LengthSquared() const { return x * x + y * y; };
  __host__ __device__ inline mVec2t<T> Normalize() const { return *this / (T)Length(); };

  __host__ __device__ _CONSTEXPR_ inline typename mMath_FloatTypeFrom<T>::type Angle() const { return mATan2(mMath_FloatTypeFrom<T>::type(y), mMath_FloatTypeFrom<T>::type(x)); };

  __host__ __device__ _CONSTEXPR_ inline typename mMath_FloatTypeFrom<T>::type AngleTo(const mVec2t<T> &other) const
  {
    mMath_FloatTypeFrom<T>::type ret = other.Angle() - Angle();

    if (ret > (mMath_FloatTypeFrom<T>::type)mPI)
      return ret - (mMath_FloatTypeFrom<T>::type)mTWOPI;
    else if (ret < -(mMath_FloatTypeFrom<T>::type)mPI)
      return ret + (mMath_FloatTypeFrom<T>::type)mTWOPI;
    else
      return ret;
  };

  __host__ __device__ _CONSTEXPR_ inline typename mMath_FloatTypeFrom<T>::type AspectRatio() const { return mMath_FloatTypeFrom<T>::type(x) / mMath_FloatTypeFrom<T>::type(y); };
  __host__ __device__ _CONSTEXPR_ inline typename mMath_FloatTypeFrom<T>::type InverseAspectRatio() const { return mMath_FloatTypeFrom<T>::type(y) / mMath_FloatTypeFrom<T>::type(x); };

  __host__ __device__ _CONSTEXPR_ inline static T Dot(const mVec2t<T> a, const mVec2t<T> b)
  {
    return a.x * b.x + a.y * b.y;
  };

  _mVECTOR_SUBSET_2(y, x);
};

template <typename T>
__host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator *  (const T a, const mVec2t<T> b) { return mVec2t<T>(a * b.x, a * b.y); };

template <typename T>
__host__ __device__ _CONSTEXPR_ inline mVec2t<T>  operator /  (const T a, const mVec2t<T> b) { return mVec2t<T>(a / b.x, a / b.y); };

template <typename T> _CONSTEXPR_ inline T mMax(const mVec2t<T> &v) { return mMax(v.x, v.y); }
template <typename T> _CONSTEXPR_ inline T mMin(const mVec2t<T> &v) { return mMin(v.x, v.y); }

template <typename T> _CONSTEXPR_ inline mVec2t<T> mMax(const mVec2t<T> &a, const mVec2t<T> &b) { return mVec2t<T>(mMax(a.x, b.x), mMax(a.y, b.y)); }
template <typename T> _CONSTEXPR_ inline mVec2t<T> mMin(const mVec2t<T> &a, const mVec2t<T> &b) { return mVec2t<T>(mMin(a.x, b.x), mMin(a.y, b.y)); }

template <typename T> inline mVec2t<T> mAbs(const mVec2t<T> &a) { return mVec2t<T>(mAbs(a.x), mAbs(a.y)); }
template <typename T> inline mVec2t<T> mFloor(const mVec2t<T> &a) { return mVec2t<T>(mFloor(a.x), mFloor(a.y)); }
template <typename T> inline mVec2t<T> mCeil(const mVec2t<T> &a) { return mVec2t<T>(mCeil(a.x), mCeil(a.y)); }
template <typename T> inline mVec2t<T> mRound(const mVec2t<T> &a) { return mVec2t<T>(mRound(a.x), mRound(a.y)); }

typedef mVec2t<size_t> mVec2s;
typedef mVec2t<int64_t> mVec2i;
typedef mVec2t<uint64_t> mVec2u;
typedef mVec2t<int32_t> mVec2i32;
typedef mVec2t<uint32_t> mVec2u32;
typedef mVec2t<float_t> mVec2f;
typedef mVec2t<double_t> mVec2d;

template <typename T>
struct mVec3t
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    T asArray[3];

    struct
    {
      T x, y, z;
    };
  };
#pragma warning(pop)

  __host__ __device__ _CONSTEXPR_ inline mVec3t() : x(0), y(0), z(0) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec3t(T _v) : x(_v), y(_v), z(_v) {}

  // Cartesian: x, y, z;
  // Spherical: radius, theta, phi;
  // Cylindrical: rho, phi, z;
  __host__ __device__ _CONSTEXPR_ inline mVec3t(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec3t(mVec2t<T> vector2, T _z) : x(vector2.x), y(vector2.y), z(_z) {}

  template <typename T2> __host__ __device__ _CONSTEXPR_ inline explicit mVec3t(const mVec3t<T2> &cast) : x((T)cast.x), y((T)cast.y), z((T)cast.z) {}

  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator +  (const mVec3t<T> &a) const { return mVec3t<T>(x + a.x, y + a.y, z + a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator -  (const mVec3t<T> &a) const { return mVec3t<T>(x - a.x, y - a.y, z - a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator *  (const mVec3t<T> &a) const { return mVec3t<T>(x * a.x, y * a.y, z * a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator /  (const mVec3t<T> &a) const { return mVec3t<T>(x / a.x, y / a.y, z / a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>& operator += (const mVec3t<T> &a) { return *this = mVec3t<T>(x + a.x, y + a.y, z + a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>& operator -= (const mVec3t<T> &a) { return *this = mVec3t<T>(x - a.x, y - a.y, z - a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>& operator *= (const mVec3t<T> &a) { return *this = mVec3t<T>(x * a.x, y * a.y, z * a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>& operator /= (const mVec3t<T> &a) { return *this = mVec3t<T>(x / a.x, y / a.y, z / a.z); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator *  (const T a) const { return mVec3t<T>(x * a, y * a, z * a); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator /  (const T a) const { return mVec3t<T>(x / a, y / a, z / a); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>& operator *= (const T a) { return *this = mVec3t<T>(x * a, y * a, z * a); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>& operator /= (const T a) { return *this = mVec3t<T>(x / a, y / a, z / a); };

  __host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator -  () const { return mVec3t<T>(-x, -y, -z); };

  __host__ __device__ _CONSTEXPR_ inline bool       operator == (const mVec3t<T> &a) const { return x == a.x && y == a.y && z == a.z; };
  __host__ __device__ _CONSTEXPR_ inline bool       operator != (const mVec3t<T> &a) const { return x != a.x || y != a.y || z != a.z; };

  __host__ __device__ inline typename mMath_FloatTypeFrom<T>::type Length() const { return mSqrt(x * x + y * y + z * z); };
  __host__ __device__ _CONSTEXPR_ inline T LengthSquared() const { return x * x + y * y + z * z; };
  __host__ __device__ inline mVec3t<T> Normalize() const { return *this / (T)Length(); };

  __host__ __device__ _CONSTEXPR_ inline mVec2t<T> ToVector2() const { return mVec2t<T>(x, y); };

  __host__ __device__ inline mVec3t<T> CartesianToSpherical() const
  {
    const auto r = Length();

    return mVec3t<T>((T)r, (T)mATan2(y, x), (T)mATan2(mSqrt(x * x + y * y), z));
  };

  __host__ __device__ inline mVec3t<T> SphericalToCartesian() const
  {
    const auto sinTheta = mSin(y);

    return mVec3t<T>(x * sinTheta * mCos(z), x * sinTheta * mSin(z), x * mCos(y));
  };

  __host__ __device__ inline mVec3t<T> SphericalToCylindrical() const
  {
    return mVec3t<T>(x * mSin(y), z, x * mCos(y));
  };

  __host__ __device__ inline mVec3t<T> CylindricalToSpherical() const
  {
    const auto r = mSqrt(x * x + z * z);
    
    return mVec3t<T>((T)r, y, (T)mACos(z / r));
  };

  __host__ __device__ inline mVec3t<T> CylindricalToCartesian() const
  {
    return mVec3t<T>(x * mCos(y), x * mSin(y), z);
  };

  __host__ __device__ inline mVec3t<T> CartesianToCylindrical() const
  {
    const auto p = mSqrt(x * x + y * y);
    
    if (x == 0 && y == 0)
      return mVec3t<T>(p, 0, z);
    else if (x >= 0)
      return mVec3t<T>(p, (T)mASin(y / p), z);
    else
      return mVec3t<T>(p, (T)(-mASin(y / p) + mPI), z);
  };

  __host__ __device__ _CONSTEXPR_ inline static mVec3t<T> Cross(const mVec3t<T> a, const mVec3t<T> b)
  {
    return mVec3t<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
  };

  __host__ __device__ _CONSTEXPR_ inline static T Dot(const mVec3t<T> a, const mVec3t<T> b)
  {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  };

  _mVECTOR_SUBSET_2(x, y);
  _mVECTOR_SUBSET_2(x, z);
  _mVECTOR_SUBSET_2(y, x);
  _mVECTOR_SUBSET_2(y, z);
  _mVECTOR_SUBSET_2(z, x);
  _mVECTOR_SUBSET_2(z, y);

  _mVECTOR_SUBSET_3(x, z, y);
  _mVECTOR_SUBSET_3(y, x, z);
  _mVECTOR_SUBSET_3(y, z, x);
  _mVECTOR_SUBSET_3(z, x, y);
  _mVECTOR_SUBSET_3(z, y, x);
};

template <typename T>
__host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator *  (const T a, const mVec3t<T> b) { return mVec3t<T>(a * b.x, a * b.y, a * b.z); };

template <typename T>
__host__ __device__ _CONSTEXPR_ inline mVec3t<T>  operator /  (const T a, const mVec3t<T> b) { return mVec3t<T>(a / b.x, a / b.y, a / b.z); };

template <typename T> _CONSTEXPR_ inline T mMax(const mVec3t<T> &v) { return mMax(mMax(v.x, v.y), v.z); }
template <typename T> _CONSTEXPR_ inline T mMin(const mVec3t<T> &v) { return mMin(mMin(v.x, v.y), v.z); }

template <typename T> _CONSTEXPR_ inline mVec3t<T> mMax(const mVec3t<T> &a, const mVec3t<T> &b) { return mVec3t<T>(mMax(a.x, b.x), mMax(a.y, b.y), mMax(a.z, b.z)); }
template <typename T> _CONSTEXPR_ inline mVec3t<T> mMin(const mVec3t<T> &a, const mVec3t<T> &b) { return mVec3t<T>(mMin(a.x, b.x), mMin(a.y, b.y), mMin(a.z, b.z)); }

template <typename T> inline mVec3t<T> mAbs(const mVec3t<T> &a) { return mVec3t<T>(mAbs(a.x), mAbs(a.y), mAbs(a.z)); }
template <typename T> inline mVec3t<T> mFloor(const mVec3t<T> &a) { return mVec3t<T>(mFloor(a.x), mFloor(a.y), mFloor(a.z)); }
template <typename T> inline mVec3t<T> mCeil(const mVec3t<T> &a) { return mVec3t<T>(mCeil(a.x), mCeil(a.y), mCeil(a.z)); }
template <typename T> inline mVec3t<T> mRound(const mVec3t<T> &a) { return mVec3t<T>(mRound(a.x), mRound(a.y), mRound(a.z)); }

typedef mVec3t<size_t> mVec3s;
typedef mVec3t<int64_t> mVec3i;
typedef mVec3t<uint64_t> mVec3u;
typedef mVec3t<int32_t> mVec3i32;
typedef mVec3t<uint32_t> mVec3u32;
typedef mVec3t<float_t> mVec3f;
typedef mVec3t<double_t> mVec3d;

template <typename T>
struct mVec4t
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    T asArray[4];

    struct
    {
      T x, y, z, w;
    };
  };
#pragma warning(pop)

  __host__ __device__ _CONSTEXPR_ inline mVec4t() : x(0), y(0), z(0), w(0) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec4t(T _v) : x(_v), y(_v), z(_v), w(_v) {}
  __host__ __device__ _CONSTEXPR_ inline mVec4t(T _x, T _y, T _z, T _w) : x(_x), y(_y), z(_z), w(_w) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec4t(const mVec3t<T> vec3, const T _w) : x(vec3.x), y(vec3.y), z(vec3.z), w(_w) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec4t(const T _x, const mVec3t<T> vec3) : x(_x), y(vec3.x), z(vec3.y), w(vec3.z) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec4t(const mVec2t<T> vec2, const T _z, const T _w) : x(vec2.x), y(vec2.y), z(_z), w(_w) {}
  __host__ __device__ _CONSTEXPR_ inline explicit mVec4t(const mVec2t<T> vec2a, const mVec2t<T> vec2b) : x(vec2a.x), y(vec2a.y), z(vec2b.x), w(vec2b.y) {}

  template <typename T2> __host__ __device__ _CONSTEXPR_ inline explicit mVec4t(const mVec4t<T2> &cast) : x((T)cast.x), y((T)cast.y), z((T)cast.z), w((T)cast.w) {}

  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator +  (const mVec4t<T> &a) const { return mVec4t<T>(x + a.x, y + a.y, z + a.z, w + a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator -  (const mVec4t<T> &a) const { return mVec4t<T>(x - a.x, y - a.y, z - a.z, w - a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator *  (const mVec4t<T> &a) const { return mVec4t<T>(x * a.x, y * a.y, z * a.z, w * a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator /  (const mVec4t<T> &a) const { return mVec4t<T>(x / a.x, y / a.y, z / a.z, w / a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>& operator += (const mVec4t<T> &a) { return *this = mVec4t<T>(x + a.x, y + a.y, z + a.z, w + a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>& operator -= (const mVec4t<T> &a) { return *this = mVec4t<T>(x - a.x, y - a.y, z - a.z, w - a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>& operator *= (const mVec4t<T> &a) { return *this = mVec4t<T>(x * a.x, y * a.y, z * a.z, w * a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>& operator /= (const mVec4t<T> &a) { return *this = mVec4t<T>(x / a.x, y / a.y, z / a.z, w / a.w); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator *  (const T a) const { return mVec4t<T>(x * a, y * a, z * a, w * a); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator /  (const T a) const { return mVec4t<T>(x / a, y / a, z / a, w / a); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>& operator *= (const T a) { return *this = mVec4t<T>(x * a, y * a, z * a, w * a); };
  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>& operator /= (const T a) { return *this = mVec4t<T>(x / a, y / a, z / a, w / a); };

  __host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator -  () const { return mVec4t<T>(-x, -y, -z, -w); };

  __host__ __device__ _CONSTEXPR_ inline bool       operator == (const mVec4t<T> &a) const { return x == a.x && y == a.y && z == a.z && w == a.w; };
  __host__ __device__ _CONSTEXPR_ inline bool       operator != (const mVec4t<T> &a) const { return x != a.x || y != a.y || z != a.z || w != a.w; };

  __host__ __device__ inline typename mMath_FloatTypeFrom<T>::type Length() const { return mSqrt(x * x + y * y + z * z + w * w); };
  __host__ __device__ _CONSTEXPR_ inline T LengthSquared() const { return x * x + y * y + z * z + w * w; };
  __host__ __device__ inline mVec4t<T> Normalize() const { return *this / (T)Length(); };

  __host__ __device__ _CONSTEXPR_ inline mVec2t<T> ToVector2() const { return mVec2t<T>(x, y); };
  __host__ __device__ _CONSTEXPR_ inline mVec3t<T> ToVector3() const { return mVec3t<T>(x, y, z); };

  __host__ __device__ _CONSTEXPR_ inline static T Dot(const mVec4t<T> a, const mVec4t<T> b)
  {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
  };

  _mVECTOR_SUBSET_2(x, y);
  _mVECTOR_SUBSET_2(x, z);
  _mVECTOR_SUBSET_2(x, w);
  _mVECTOR_SUBSET_2(y, x);
  _mVECTOR_SUBSET_2(y, z);
  _mVECTOR_SUBSET_2(y, w);
  _mVECTOR_SUBSET_2(z, x);
  _mVECTOR_SUBSET_2(z, y);
  _mVECTOR_SUBSET_2(z, w);
  _mVECTOR_SUBSET_2(w, x);
  _mVECTOR_SUBSET_2(w, y);
  _mVECTOR_SUBSET_2(w, z);

  _mVECTOR_SUBSET_3(x, y, z);
  _mVECTOR_SUBSET_3(x, y, w);
  _mVECTOR_SUBSET_3(x, z, y);
  _mVECTOR_SUBSET_3(x, z, w);
  _mVECTOR_SUBSET_3(x, w, y);
  _mVECTOR_SUBSET_3(x, w, z);

  _mVECTOR_SUBSET_3(y, x, z);
  _mVECTOR_SUBSET_3(y, x, w);
  _mVECTOR_SUBSET_3(y, z, x);
  _mVECTOR_SUBSET_3(y, z, w);
  _mVECTOR_SUBSET_3(y, w, x);
  _mVECTOR_SUBSET_3(y, w, z);

  _mVECTOR_SUBSET_3(z, x, y);
  _mVECTOR_SUBSET_3(z, x, w);
  _mVECTOR_SUBSET_3(z, y, x);
  _mVECTOR_SUBSET_3(z, y, w);
  _mVECTOR_SUBSET_3(z, w, x);
  _mVECTOR_SUBSET_3(z, w, y);

  _mVECTOR_SUBSET_4(x, y, z, w);
  _mVECTOR_SUBSET_4(x, y, w, z);
  _mVECTOR_SUBSET_4(x, z, y, w);
  _mVECTOR_SUBSET_4(x, z, w, y);
  _mVECTOR_SUBSET_4(x, w, y, z);
  _mVECTOR_SUBSET_4(x, w, z, y);

  _mVECTOR_SUBSET_4(y, x, z, w);
  _mVECTOR_SUBSET_4(y, x, w, z);
  _mVECTOR_SUBSET_4(y, z, x, w);
  _mVECTOR_SUBSET_4(y, z, w, x);
  _mVECTOR_SUBSET_4(y, w, x, z);
  _mVECTOR_SUBSET_4(y, w, z, x);

  _mVECTOR_SUBSET_4(z, x, y, w);
  _mVECTOR_SUBSET_4(z, x, w, y);
  _mVECTOR_SUBSET_4(z, y, x, w);
  _mVECTOR_SUBSET_4(z, y, w, x);
  _mVECTOR_SUBSET_4(z, w, x, y);
  _mVECTOR_SUBSET_4(z, w, y, x);

  _mVECTOR_SUBSET_4(w, x, y, z);
  _mVECTOR_SUBSET_4(w, x, z, y);
  _mVECTOR_SUBSET_4(w, y, x, z);
  _mVECTOR_SUBSET_4(w, y, z, x);
  _mVECTOR_SUBSET_4(w, z, x, y);
  _mVECTOR_SUBSET_4(w, z, y, x);

};

template <typename T>
__host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator *  (const T a, const mVec4t<T> b) { return mVec4t<T>(a * b.x, a * b.y, a * b.z, a * b.w); };

template <typename T>
__host__ __device__ _CONSTEXPR_ inline mVec4t<T>  operator /  (const T a, const mVec4t<T> b) { return mVec4t<T>(a / b.x, a / b.y, a / b.z, a / b.w); };

template <typename T> _CONSTEXPR_ T mMax(const mVec4t<T> &v) { return mMax(mMax(v.x, v.y), mMax(v.z, v.w)); }
template <typename T> _CONSTEXPR_ T mMin(const mVec4t<T> &v) { return mMin(mMin(v.x, v.y), mMin(v.z, v.w)); }

template <typename T> _CONSTEXPR_ mVec4t<T> mMax(const mVec4t<T> &a, const mVec4t<T> &b) { return mVec4t<T>(mMax(a.x, b.x), mMax(a.y, b.y), mMax(a.z, b.z), mMax(a.w, b.w)); }
template <typename T> _CONSTEXPR_ mVec4t<T> mMin(const mVec4t<T> &a, const mVec4t<T> &b) { return mVec4t<T>(mMin(a.x, b.x), mMin(a.y, b.y), mMin(a.z, b.z), mMin(a.w, b.w)); }

template <typename T> mVec4t<T> mAbs(const mVec4t<T> &a) { return mVec4t<T>(mAbs(a.x), mAbs(a.y), mAbs(a.z), mAbs(a.w)); }
template <typename T> mVec4t<T> mFloor(const mVec4t<T> &a) { return mVec4t<T>(mFloor(a.x), mFloor(a.y), mFloor(a.z), mFloor(a.w)); }
template <typename T> mVec4t<T> mCeil(const mVec4t<T> &a) { return mVec4t<T>(mCeil(a.x), mCeil(a.y), mCeil(a.z), mCeil(a.w)); }
template <typename T> mVec4t<T> mRound(const mVec4t<T> &a) { return mVec4t<T>(mRound(a.x), mRound(a.y), mRound(a.z), mRound(a.w)); }

typedef mVec4t<size_t> mVec4s;
typedef mVec4t<int64_t> mVec4i;
typedef mVec4t<uint64_t> mVec4u;
typedef mVec4t<int32_t> mVec4i32;
typedef mVec4t<uint32_t> mVec4u32;
typedef mVec4t<float_t> mVec4f;
typedef mVec4t<double_t> mVec4d;

template <typename T>
struct mRectangle2D
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    T asArray[4];
    mVec4t<T> asVector4;

    struct
    {
      union
      {
        struct
        {
          T x, y;
        };

        mVec2t<T> position;
      };
      
      union
      {
        mVec2t<T> size;

        struct
        {
          T width, height;
        };

        struct
        {
          T w, h;
        };
      };
    };
  };
#pragma warning(pop)

  __host__ __device__ inline mRectangle2D() : x(0), y(0), w(0), h(0) { }
  __host__ __device__ inline mRectangle2D(const T x, const T y, const T w, const T h) : x(x), y(y), w(w), h(h) { }
  __host__ __device__ inline mRectangle2D(const mVec2t<T> &position, const mVec2t<T> size) : position(position), size(size) { }

  __host__ __device__ inline bool operator == (const mRectangle2D<T> &rect) const
  {
    return position == rect.position && size == rect.size;
  }

  __host__ __device__ inline bool operator != (const mRectangle2D<T> &rect) const
  {
    return position != rect.position || size != rect.size;
  }

  __host__ __device__ inline bool Contains(const mVec2t<T> &_position) const
  {
    return _position.x >= x && _position.y >= y && _position.x < x + w && _position.y < y + h;
  }

  __host__ __device__ inline bool Contains(const mRectangle2D<T> &rect) const
  {
    const mVec2t<T> rend = rect.position + rect.size;
    const mVec2t<T> end = position + size;

    return rect.x >= x && rect.y >= y && rect.x <= end.x && rect.y <= end.y && rend.x >= x && rend.y >= y && rend.x <= end.x && rend.y <= end.y;
  }

  __host__ __device__ inline mRectangle2D<T> Intersect(const mRectangle2D<T> &rect) const
  {
    const float_t _x = mMax(x, rect.x);
    const float_t _y = mMax(y, rect.y);
    const float_t _w = mMin(x + width, rect.x + rect.width) - _x;
    const float_t _h = mMin(y + height, rect.y + rect.height) - _y;

    return mRectangle2D<float_t>(_x, _y, _w, _h);
  }

  __host__ __device__ inline bool Intersects(const mRectangle2D<T> &rect) const
  {
    const float_t _sx = mMax(x, rect.x);
    const float_t _sy = mMax(y, rect.y);
    const float_t _ex = mMin(x + width, rect.x + rect.width);
    const float_t _ey = mMin(y + height, rect.y + rect.height);

    return _sx < _ex && _sy < _ey;
  }

  __host__ __device__ inline mRectangle2D<T> OffsetCopy(mVec2t<T> offset) const
  {
    return mRectangle2D<T>(position + offset, size);
  }

  __host__ __device__ inline mRectangle2D<T> & OffsetSelf(mVec2t<T> offset)
  {
    position += offset;
    return *this;
  }

  __host__ __device__ inline mRectangle2D<T> ScaleCopy(const T scale) const
  {
    return mRectangle2D<T>(position * scale, size * scale);
  }

  __host__ __device__ inline mRectangle2D<T> ScaleSelf(const T scale)
  {
    asVector4 *= scale;
    return *this;
  }

  __host__ __device__ inline mRectangle2D<T> ScaleCopy(const mVec2t<T> scale) const
  {
    return mRectangle2D<T>(position * scale, size * scale);
  }

  __host__ __device__ inline mRectangle2D<T> ScaleSelf(const mVec2t<T> scale)
  {
    position *= scale;
    size *= scale;
    return *this;
  }

  __host__ __device__ inline mRectangle2D<T> GrowToContain(const mRectangle2D<T> &rect)
  {
    if (position.x > rect.x)
    {
      size.x += (position.x - rect.x);
      position.x = rect.x;
    }

    if (position.y > rect.y)
    {
      size.y += (position.y - rect.y);
      position.y = rect.y;
    }

    if (position.x + size.x < rect.position.x + rect.size.x)
      size.x = (rect.position.x + rect.size.x) - position.x;

    if (position.y + size.y < rect.position.y + rect.size.y)
      size.y = (rect.position.y + rect.size.y) - position.y;

    return *this;
  }

  __host__ __device__ inline mRectangle2D<T> GrowToContain(const mVec2t<T> &v)
  {
    if (position.x > v.x)
    {
      size.x += (position.x - v.x);
      position.x = v.x;
    }

    if (position.y > v.y)
    {
      size.y += (position.y - v.y);
      position.y = v.y;
    }

    if (position.x + size.x <= v.x)
      size.x = v.x - position.x + mSmallest<T>(v.x);

    if (position.y + size.y <= v.y)
      size.y = v.y - position.y + mSmallest<T>(v.y);

    return *this;
  }

  __host__ __device__ inline void GetCorners(mVec2t<T> corners[4]) const
  {
    corners[0] = position;

    corners[1] = position;
    corners[1].x += width;

    corners[2] = position;
    corners[2].y += height;

    corners[3] = position + size;
  }
};

template <typename T>
struct mTriangle
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    struct
    {
      T position0, position1, position2;
    };

    T position[3];
  };
#pragma warning(pop)

  inline mTriangle()
  { }

  inline mTriangle(const T position0, const T position1, const T position2) :
    position0(position0),
    position1(position1),
    position2(position2)
  { }
};

template <typename T>
using mTriangle2D = mTriangle<mVec2t<T>>;

template <typename T>
using mTriangle3D = mTriangle<mVec3t<T>>;

template <typename T>
inline bool mIntersects(const mTriangle2D<T> &t, const mVec2t<T> point)
{
  const mVec2t<T> _a = t.position[2] - t.position[1];
  const mVec2t<T> _b = t.position[0] - t.position[2];
  const mVec2t<T> _c = t.position[1] - t.position[0];

  const mVec2t<T> ap = point - t.position[0];
  const mVec2t<T> bp = point - t.position[1];
  const mVec2t<T> cp = point - t.position[2];

  const T aXbp = _a.x * bp.y - _a.y * bp.x;
  const T cXap = _c.x * ap.y - _c.y * ap.x;
  const T bXcp = _b.x * cp.y - _b.y * cp.x;

  return aXbp >= (T)0 && bXcp >= (T)0 && cXap >= (T)0;
};

template <typename T>
struct mLine
{
#pragma warning(push)
#pragma warning(disable: 4201)
  union
  {
    struct
    {
      T position0, position1;
    };

    T position[2];
  };
#pragma warning(pop)

  inline mLine()
  { }

  inline mLine(const T position0, const T position1) :
    position0(position0),
    position1(position1)
  { }
};

template <typename T>
using mLine2D = mLine<mVec2t<T>>;

template <typename T>
using mLine3D = mLine<mVec3t<T>>;

template <typename T>
struct mPlane
{
  T position, direction0, direction1;

  static mPlane FromPoints(const T position0, const T position1, const T position2)
  {
    return { position0, position1 - position0, position2 - position0 };;
  }

  static mPlane FromPointAndDirections(const T position, const T direction0, const T direction1)
  {
    return { position, direction0, direction1 };
  }
};

template <typename T>
using mPlane2D = mPlane<mVec2t<T>>;

template <typename T>
using mPlane3D = mPlane<mVec3t<T>>;

struct mVector;

__host__ __device__ inline constexpr mVec3f mColor_UnpackBgraToVec3f(const uint32_t bgraColor)
{
  constexpr float_t v(1.0f / 0xFF);
  return mVec3f((float_t)((bgraColor & 0x00FF0000) >> 0x10), (float_t)((bgraColor & 0x0000FF00) >> 0x8), (float_t)(bgraColor & 0x000000FF)) * v;
}

__host__ __device__ inline constexpr mVec3d mColor_UnpackBgraToVec3d(const uint32_t bgraColor)
{
  constexpr double_t v(1.0 / 0xFF);
  return mVec3d((double_t)((bgraColor & 0x00FF0000) >> 0x10), (double_t)((bgraColor & 0x0000FF00) >> 0x8), (double_t)(bgraColor & 0x000000FF)) * v;
}

__host__ __device__ inline constexpr mVec4f mColor_UnpackBgraToVec4f(const uint32_t bgraColor)
{
  constexpr const float_t v(1.0f / 0xFF);
  return mVec4f((float_t)((bgraColor & 0x00FF0000) >> 0x10), (float_t)((bgraColor & 0x0000FF00) >> 0x8), (float_t)(bgraColor & 0x000000FF), (float_t)((bgraColor & 0xFF000000) >> 0x18)) * v;
}

__host__ __device__ inline constexpr mVec4d mColor_UnpackBgraToVec4d(const uint32_t bgraColor)
{
  constexpr double_t v(1.0 / 0xFF);
  return mVec4d((double_t)((bgraColor & 0x00FF0000) >> 0x10), (double_t)((bgraColor & 0x0000FF00) >> 0x8), (double_t)(bgraColor & 0x000000FF), (double_t)((bgraColor & 0xFF000000) >> 0x18)) * v;
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec3fToBgra(const mVec3f rgbVector)
{
  constexpr float_t v = (float_t)0xFF;
  const mVec3f v0 = rgbVector * v;
  return (mClamp((uint32_t)v0.x, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.z, 0u, 0xFFu) | 0xFF000000;
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec3dToBgra(const mVec3d rgbVector)
{
  constexpr double_t v = (double_t)0xFF;
  const mVec3d v0 = rgbVector * v;
  return (mClamp((uint32_t)v0.x, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.z, 0u, 0xFFu) | 0xFF000000;
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec4fToBgra(const mVec4f rgbaVector)
{
  constexpr float_t v = (float_t)0xFF;
  const mVec4f v0 = rgbaVector * v;
  return (mClamp((uint32_t)v0.x, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.z, 0u, 0xFFu) | (mClamp((uint32_t)v0.w, 0u, 0xFFu) << 0x18);
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec4dToBgra(const mVec4d rgbaVector)
{
  constexpr double_t v = (double_t)0xFF;
  const mVec4d v0 = rgbaVector * v;
  return (mClamp((uint32_t)v0.x, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.z, 0u, 0xFFu) | (mClamp((uint32_t)v0.w, 0u, 0xFFu) << 0x18);
}

__host__ __device__ inline constexpr mVec3f mColor_UnpackRgbaToVec3f(const uint32_t rgbaColor)
{
  constexpr float_t v(1.0f / 0xFF);
  return mVec3f((float_t)(rgbaColor & 0x000000FF), (float_t)((rgbaColor & 0x0000FF00) >> 0x8), (float_t)((rgbaColor & 0x00FF0000) >> 0x10)) * v;
}

__host__ __device__ inline constexpr mVec3d mColor_UnpackRgbaToVec3d(const uint32_t rgbaColor)
{
  constexpr double_t v(1.0 / 0xFF);
  return mVec3d((double_t)(rgbaColor & 0x000000FF), (double_t)((rgbaColor & 0x0000FF00) >> 0x8), (double_t)((rgbaColor & 0x00FF0000) >> 0x10)) * v;
}

__host__ __device__ inline constexpr mVec4f mColor_UnpackRgbaToVec4f(const uint32_t rgbaColor)
{
  constexpr const float_t v(1.0f / 0xFF);
  return mVec4f((float_t)(rgbaColor & 0x000000FF), (float_t)((rgbaColor & 0x0000FF00) >> 0x8), (float_t)((rgbaColor & 0x00FF0000) >> 0x10), (float_t)((rgbaColor & 0xFF000000) >> 0x18)) * v;
}

__host__ __device__ inline constexpr mVec4d mColor_UnpackRgbaToVec4d(const uint32_t rgbaColor)
{
  constexpr double_t v(1.0 / 0xFF);
  return mVec4d((double_t)(rgbaColor & 0x000000FF), (double_t)((rgbaColor & 0x0000FF00) >> 0x8), (double_t)((rgbaColor & 0x00FF0000) >> 0x10), (double_t)((rgbaColor & 0xFF000000) >> 0x18)) * v;
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec3fToRgba(const mVec3f rgbVector)
{
  constexpr float_t v = (float_t)0xFF;
  const mVec3f v0 = rgbVector * v;
  return (mClamp((uint32_t)v0.z, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.x, 0u, 0xFFu) | 0xFF000000;
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec3dToRgba(const mVec3d rgbVector)
{
  constexpr double_t v = (double_t)0xFF;
  const mVec3d v0 = rgbVector * v;
  return (mClamp((uint32_t)v0.z, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.x, 0u, 0xFFu) | 0xFF000000;
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec4fToRgba(const mVec4f rgbaVector)
{
  constexpr float_t v = (float_t)0xFF;
  const mVec4f v0 = rgbaVector * v;
  return (mClamp((uint32_t)v0.z, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.x, 0u, 0xFFu) | (mClamp((uint32_t)v0.w, 0u, 0xFFu) << 0x18);
}

__host__ __device__ inline constexpr uint32_t mColor_PackVec4dToRgba(const mVec4d rgbaVector)
{
  constexpr double_t v = (double_t)0xFF;
  const mVec4d v0 = rgbaVector * v;
  return (mClamp((uint32_t)v0.z, 0u, 0xFFu) << 0x10) | (mClamp((uint32_t)v0.y, 0u, 0xFFu) << 0x8) | mClamp((uint32_t)v0.x, 0u, 0xFFu) | (mClamp((uint32_t)v0.w, 0u, 0xFFu) << 0x18);
}

uint32_t mColor_PackVectorToBgra(const mVector rgbaVector);
mVector mColor_UnpackBgraToVector(const uint32_t bgraColor);
uint32_t mColor_PackVectorToRgba(const mVector rgbaVector);
mVector mColor_UnpackRgbaToVector(const uint32_t rgbaColor);

__host__ __device__ inline mVec3f mColor_HueToVec3f(const float_t hue)
{
  const float_t h = hue * 6;
  const float_t r = mAbs(h - 3) - 1;
  const float_t g = 2 - mAbs(h - 2);
  const float_t b = 2 - mAbs(h - 4);

  return mVec3f(mClamp(r, 0.0f, 1.0f), mClamp(g, 0.0f, 1.0f), mClamp(b, 0.0f, 1.0f));
}

__host__ __device__ inline uint32_t mColor_HueToBgra(const float_t hue)
{
  return mColor_PackVec3fToBgra(mColor_HueToVec3f(hue));
}

__host__ __device__ inline mVec3f mColor_YuvToRgb(const mVec3f yuv)
{
  return mVec3f(mClamp(yuv.x + 1.370705f * (yuv.z - .5f), 0.f, 1.f),
    mClamp(yuv.x - .698001f * (yuv.z - .5f) - 0.337633f * (yuv.y - .5f), 0.f, 1.f),
    mClamp(yuv.x + 1.732446f * (yuv.y - .5f), 0.f, 1.f));
}

__host__ __device__ inline mVec3f mColor_RgbToYuv(const mVec3f rgb)
{
  return mVec3f(mClamp(.2988222643743755f * rgb.x + .5868145917975452f * rgb.y + .1143631438280793f * rgb.z, 0.f, 1.f),
    mClamp(-(.1724857596567948f * rgb.x + .3387202786104417f * rgb.y - .5112060382672364f * rgb.z - .5f), 0.f, 1.f),
    mClamp(.5115453256722814f * rgb.x - .4281115132705763f * rgb.y - .08343381240170515f * rgb.z + .5f, 0.f, 1.f));
}

__host__ __device__ inline mVec3f mColor_RgbToHcv(const mVec3f rgb)
{
  const mVec4f p = (rgb.y < rgb.z) ? mVec4f(rgb.z, rgb.y, -1.f, 2.f / 3.f) : mVec4f(rgb.y, rgb.z, 0.f, -1.f / 3.f);
  const mVec4f q = (rgb.x < p.x) ? mVec4f(p.x, p.y, p.w, rgb.x) : mVec4f(rgb.x, p.y, p.z, p.x);
  const float_t c = q.x - mMin(q.w, q.y);
  const float_t h = abs((q.w - q.y) / (6.f * c + FLT_EPSILON) + q.z);

  return mVec3f(h, c, q.x);
}

__host__ __device__ inline mVec3f mColor_HslToRgb(const mVec3f hsl)
{
  const mVec3f rgb = mColor_HueToVec3f(hsl.x);
  const float_t c = (1.f - mAbs(2.f * hsl.z - 1.f)) * hsl.y;

  return (rgb - mVec3f(.5f)) * c + mVec3f(hsl.z);
}

__host__ __device__ inline mVec3f mColor_RgbToHsl(const mVec3f rgb)
{
  const mVec3f hcv = mColor_RgbToHcv(rgb);
  const float_t l = hcv.z - hcv.y * .5f;
  const float_t s = hcv.y / (1.f - mAbs(l * 2.f - 1.f) + FLT_EPSILON);

  return mVec3f(hcv.x, s, l);
}

template<typename T, typename U>
inline mVec2t<U> mBarycentricInterpolationFactors(const T &p, const T &q, const U &x)
{
  const U wp = (x - q) / (q - p);

  return mVec2t<U>(wp, 1 - wp);
}

template<typename T, typename U>
inline mVec3t<U> mBarycentricInterpolationFactors(const mVec2t<T> &p, const mVec2t<T> &q, const mVec2t<T> &r, const mVec2t<U> &x)
{
  const U divisor = (p.x * (r.y - q.y) + q.x * (p.y - r.y) + r.x * (q.y - p.y));
  const U wp = (x.x * (r.y - q.y) + q.x * (x.y - r.y) + r.x * (q.y - x.y)) / divisor;
  const U wq = -(x.x * (r.y - p.y) + p.x * (x.y - r.y) + r.x * (p.y - x.y)) / divisor;

  return mVec3t<U>(wp, wq, 1 - wp - wq);
}

template<typename T, typename U>
inline mVec4t<U> mBarycentricInterpolationFactors(const mVec3t<T> &p, const mVec3t<T> &q, const mVec3t<T> &r, const mVec3t<T> &s, const mVec3t<U> &x)
{
  const U val0 = (p.y * (s.z - r.z) + r.y * (p.z - s.z) + (r.z - p.z) * s.y);
  const U val1 = (s.y * (x.z - r.z) + r.y * (s.z - x.z) + (r.z - s.z) * x.y);
  const U val2 = (p.y * (x.z - s.z) + s.y * (p.z - x.z) + (s.z - p.z) * x.y);

  const U divisor = (q.x * val0 + p.x * (r.y * (s.z - q.z) + q.y * (r.z - s.z) + (q.z - r.z) * s.y) + r.x * (q.y * (s.z - p.z) + p.y * (q.z - s.z) + (p.z - q.z) * s.y) + (p.y * (r.z - q.z) + q.y * (p.z - r.z) + (q.z - p.z) * r.y) * s.x);
  const U wp = -(r.x * (q.y * (x.z - s.z) + s.y * (q.z - x.z) + (s.z - q.z) * x.y) + q.x * val1 + s.x * (r.y * (x.z - q.z) + q.y * (r.z - x.z) + (q.z - r.z) * x.y) + (q.y * (s.z - r.z) + r.y * (q.z - s.z) + (r.z - q.z) * s.y) * x.x) / divisor;
  const U wq = (r.x * val2 + p.x * val1 + s.x * (r.y * (x.z - p.z) + p.y * (r.z - x.z) + (p.z - r.z) * x.y) + val0 * x.x) / divisor;
  const U wr = -(q.x * val2 + p.x * (s.y * (x.z - q.z) + q.y * (s.z - x.z) + (q.z - s.z) * x.y) + s.x * (q.y * (x.z - p.z) + p.y * (q.z - x.z) + (p.z - q.z) * x.y) + (p.y * (s.z - q.z) + q.y * (p.z - s.z) + (q.z - p.z) * s.y) * x.x) / divisor;

  return mVec4t<U>(wp, wq, wr, 1 - wp - wq - wr)
}

template <typename T>
bool mIntersects(const mPlane3D<T> &plane, const mLine3D<T> &line, OUT OPTIONAL mVec3t<T> *pHitPosition)
{
  const mVec3t<T> diff = line.position0 - line.position1;
  const mVec3t<T> normal = mVec3t<T>::Cross(plane.direction0, plane.direction1).Normalize();

  const T denominator = normal.x * diff.x + normal.y * diff.y + normal.z * diff.z;
  const T u = (normal.x * line.position0.x + normal.y * line.position0.y + normal.z * line.position0.z -mVec3t<T>::Dot(plane.position, normal)) / denominator;

  const bool hit = (denominator != 0);

  if (pHitPosition != nullptr && hit)
    *pHitPosition = line.position0 + u * (line.position1 - line.position0);

  return hit;
}

// 3D Parallelogram on Line.
// paralellogramPos0 should be at uv (0, 0).
// paralellogramPos1 should be at uv (1, 0).
// paralellogramPos2 should be at uv (0, 1).
// paralellogramPos0 + (paralellogramPos1 - paralellogramPos0) + (paralellogramPos2 - paralellogramPos0) should be at uv (1, 1).
template <typename T>
bool mIntersects(const mVec3t<T> &paralellogramPos0, const mVec3t<T> &paralellogramPos1, const mVec3t<T> &paralellogramPos2, const mLine3D<T> &line, OUT OPTIONAL mVec2t<T> *pParalellogramUV = nullptr, OUT OPTIONAL T *pLineDirectionFactor = nullptr)
{
  const mVec3t<T> paraPos = paralellogramPos0;
  const mVec3t<T> paraDirX = paralellogramPos1 - paralellogramPos0;
  const mVec3t<T> paraDirY = paralellogramPos2 - paralellogramPos0;
  const mVec3t<T> linePosition = line.position0;
  const mVec3t<T> lineDirection = line.position1 - line.position0;

  // linepos_x + x * linedir_x = planepos_x + a * planedir0_x + b * planedir1_x
  // linepos_y + x * linedir_y = planepos_y + a * planedir0_y + b * planedir1_y
  // linepos_z + x * linedir_z = planepos_z + a * planedir0_z + b * planedir1_z
  //
  // Retrieves:
  //
  // a = (pd1_x*(ld_y*(pp_z-lp_z)-ld_z*pp_y+ld_z*lp_y)+ld_x*(pd1_y*(lp_z-pp_z)+pd1_z*pp_y-lp_y*pd1_z)+(ld_z*pd1_y-ld_y*pd1_z)*pp_x+lp_x*(ld_y*pd1_z-ld_z*pd1_y))/(ld_x*(pd0_z*pd1_y-pd0_y*pd1_z)+pd0_x*(ld_y*pd1_z-ld_z*pd1_y)+(ld_z*pd0_y-ld_y*pd0_z)*pd1_x)
  // b = -(pd0_x*(ld_y*(pp_z-lp_z)-ld_z*pp_y+ld_z*lp_y)+ld_x*(pd0_y*(lp_z-pp_z)+pd0_z*pp_y-lp_y*pd0_z)+(ld_z*pd0_y-ld_y*pd0_z)*pp_x+lp_x*(ld_y*pd0_z-ld_z*pd0_y))/(ld_x*(pd0_z*pd1_y-pd0_y*pd1_z)+pd0_x*(ld_y*pd1_z-ld_z*pd1_y)+(ld_z*pd0_y-ld_y*pd0_z)*pd1_x)
  // x = (pd1_x*(pd0_y*(pp_z-lp_z)-pd0_z*pp_y+lp_y*pd0_z)+pd0_x*(pd1_y*(lp_z-pp_z)+pd1_z*pp_y-lp_y*pd1_z)+(pd0_z*pd1_y-pd0_y*pd1_z)*pp_x+lp_x*(pd0_y*pd1_z-pd0_z*pd1_y))/(ld_x*(pd0_z*pd1_y-pd0_y*pd1_z)+pd0_x*(ld_y*pd1_z-ld_z*pd1_y)+(ld_z*pd0_y-ld_y*pd0_z)*pd1_x)
  //
  // Where: linepos = lp, linedir = ld, planepos = pp, planedir0 = pd0, planedir1 = pd1

  const T lpy_sdXz = linePosition.y * paraDirX.z;
  const T ldy_sdXz = lineDirection.y * paraDirX.z;
  const T ldz_sdXy = lineDirection.z * paraDirX.y;
  const T ldz_sdYy = lineDirection.z * paraDirY.y;
  const T ldy_sdYz = lineDirection.y * paraDirY.z;
  const T sdXy_sdYz = paraDirX.y * paraDirY.z;
  const T sdXz_sdYy = paraDirX.z * paraDirY.y;
  const T sdXz_ppy = paraDirX.z * paraPos.y;
  const T cross_sdX_sdY_x = (sdXz_sdYy - sdXy_sdYz);
  const T cross_ld_sdY_x = (ldy_sdYz - ldz_sdYy);
  const T cross_ld_sdX_x = (ldz_sdXy - ldy_sdXz);
  const T param2 = 1.f / (lineDirection.x * cross_sdX_sdY_x + paraDirX.x * cross_ld_sdY_x + cross_ld_sdX_x * paraDirY.x);
  const T param1 = (lineDirection.y * (paraPos.z - linePosition.z) - lineDirection.z * paraPos.y + lineDirection.z * linePosition.y);
  const T param0 = (paraDirY.y * (linePosition.z - paraPos.z) + paraDirY.z * paraPos.y - linePosition.y * paraDirY.z);

  const T a = (paraDirY.x * param1 + lineDirection.x * param0 + (ldz_sdYy - ldy_sdYz) * paraPos.x + linePosition.x * cross_ld_sdY_x) * param2;
  const T b = -(paraDirX.x * param1 + lineDirection.x * (paraDirX.y * (linePosition.z - paraPos.z) + sdXz_ppy - lpy_sdXz) + cross_ld_sdX_x * paraPos.x + linePosition.x * (ldy_sdXz - ldz_sdXy)) * param2;
  const T x = (paraDirY.x * (paraDirX.y * (paraPos.z - linePosition.z) - sdXz_ppy + lpy_sdXz) + paraDirX.x * param0 + cross_sdX_sdY_x * paraPos.x + linePosition.x * (sdXy_sdYz - sdXz_sdYy)) * param2;

  if (pParalellogramUV != nullptr)
    *pParalellogramUV = mVec2t<T>(a, b);

  if (pLineDirectionFactor != nullptr)
    *pLineDirectionFactor = x;

  return (x >= 0 || a >= 0 || a <= 1 || b >= 0 || b <= 1);
}

template <typename T>
bool mIntersects(const mTriangle3D<T> &triangle, const mLine3D<T> &line, OUT OPTIONAL T *pDistance = nullptr)
{
  mVec3t<T> e1, e2;
  mVec3t<T> p, q, r;
  T det, inv_det, u, v;
  T t;
  const mVec3t<T> lineDirection = line.position1 - line.position0;

  // Find vectors for two edges sharing V1
  e1 = triangle.position1 - triangle.position0;
  e2 = triangle.position2 - triangle.position0;

  // Begin calculating determinant - also used to calculate u parameter
  p = mVec3t<T>::Cross(lineDirection, e2);

  // if determinant is near zero, ray lies in plane of triangle
  det = mVec3t<T>::Dot(e1, p);

  if (mAbs(det) < mSmallest<T>())
    return false;
  
  inv_det = 1.f / det;

  // calculate distance from V1 to ray origin
  r = line.position0 - triangle.position0;

  // Calculate u parameter and test bound
  u = mVec3t<T>::Dot(r, p) * inv_det;

  // The intersection lies outside of the triangle
  if (u < 0.f || u > 1.f)
    return false;

  // Prepare to test v parameter
  q = mVec3t<T>::Cross(r, e1);

  // Calculate V parameter and test bound
  v = mVec3t<T>::Dot(lineDirection, q) * inv_det;

  // The intersection lies outside of the triangle
  if (v < 0.f || u + v  > 1.f)
    return false;

  // Calculate Time To Intersection
  t = mVec3t<T>::Dot(e2, q) * inv_det;

  if (t > mSmallest<T>()) // ray intersection
  {
    if (pDistance != nullptr)
      *pDistance = t;

    return true;
  }

  return false;
}

template <typename T>
mINLINE bool mIntersects(const mTriangle3D<T> &triangle, const mLine3D<T> &line, OUT mVec3t<T> *pBarycentricTrianglePos, OUT OPTIONAL T *pDistance)
{
  T distance;

  if (mIntersects(triangle, line, &distance))
  {
    const mVec3t<T> tri0 = triangle.position0;
    const mVec3t<T> tri1 = triangle.position1;
    const mVec3t<T> tri2 = triangle.position2;
    const mVec3t<T> p = line.position0 + (line.position1 - line.position0) * distance;

    // tri0_x * a + tri1_x * b + tri2_x * c = p_x,
    // tri0_y * a + tri1_y * b + tri2_y * c = p_y,
    // tri0_z * a + tri1_z * b + tri2_z * c = p_z
    //
    // Retrieves:
    //
    // a = (p_x*(tri1_z*tri2_y - tri1_y*tri2_z) + tri1_x*(p_y*tri2_z - p_z*tri2_y) + (p_z*tri1_y - p_y*tri1_z)*tri2_x) / (tri0_x*(tri1_z*tri2_y - tri1_y*tri2_z) + tri1_x*(tri0_y*tri2_z - tri0_z*tri2_y) + (tri0_z*tri1_y - tri0_y*tri1_z)*tri2_x)
    // b = -(p_x*(tri0_z*tri2_y - tri0_y*tri2_z) + tri0_x*(p_y*tri2_z - p_z*tri2_y) + (p_z*tri0_y - p_y*tri0_z)*tri2_x) / (tri0_x*(tri1_z*tri2_y - tri1_y*tri2_z) + tri1_x*(tri0_y*tri2_z - tri0_z*tri2_y) + (tri0_z*tri1_y - tri0_y*tri1_z)*tri2_x)
    // c = (p_x*(tri0_z*tri1_y - tri0_y*tri1_z) + tri0_x*(p_y*tri1_z - p_z*tri1_y) + (p_z*tri0_y - p_y*tri0_z)*tri1_x) / (tri0_x*(tri1_z*tri2_y - tri1_y*tri2_z) + tri1_x*(tri0_y*tri2_z - tri0_z*tri2_y) + (tri0_z*tri1_y - tri0_y*tri1_z)*tri2_x)

    const T param8 = p.y * tri1.z;
    const T param7 = p.z * tri1.y;
    const T param6 = tri0.z * tri2.y;
    const T param5 = tri0.y * tri2.z;
    const T param4 = (p.z * tri0.y - p.y * tri0.z);
    const T param3 = (tri0.z * tri1.y - tri0.y * tri1.z);
    const T param2 = (p.y * tri2.z - p.z * tri2.y);
    const T param1 = (tri1.z * tri2.y - tri1.y * tri2.z);
    const T param0 = (T)1 / (tri0.x * param1 + tri1.x * (param5 - param6) + param3 * tri2.x);

    const T a = (p.x * param1 + tri1.x * param2 + (param7 - param8) * tri2.x) * param0;
    const T b = -(p.x * (param6 - param5) + tri0.x * param2 + param4 * tri2.x) * param0;
    const T c = (p.x * param3 + tri0.x * (param8 - param7) + param4 * tri1.x) * param0;

    *pBarycentricTrianglePos = mVec3t<T>(a, b, c);

    if (pDistance != nullptr)
      *pDistance = distance;

    return true;
  }

  return false;
}

//////////////////////////////////////////////////////////////////////////

struct mFraction
{
  int64_t integralPart, numerator, denominator;
};

mFraction mToFraction(const double_t value, const int64_t precision = 1000000000);

template <typename T>
inline T mGreatestCommonDivisor(const T a, const T b)
{
  if (a == 0)
    return b;
  if (b == 0)
    return a;

  T _a = a;
  T _b = b;

  if (a > b)
    std::swap(_a, _b);

  while (_b != 0)
  {
    const T h = _a % _b;
    _a = _b;
    _b = h;
  }

  return _a;
}

//////////////////////////////////////////////////////////////////////////

template <typename T>
inline T mSinc(const T x)
{
  return mSin((T)mPI * x) / ((T)mPI * x);
}

template <typename T, size_t TWidth>
inline T mLanczos(const T x)
{
  static_assert(TWidth != 0, "Invalid Parameter for TWidth.");

  if (-(T)TWidth < x && (T)TWidth > x)
    return mSinc(x) * mSinc(x / (T)TWidth);
  else if (x == 0)
    return 1;
  else
    return 0;
}

//////////////////////////////////////////////////////////////////////////

template<>
_CONSTEXPR_ int8_t mMinValue<int8_t>()
{
  return INT8_MIN;
}

template<>
_CONSTEXPR_ int16_t mMinValue<int16_t>()
{
  return INT16_MIN;
}

template<>
_CONSTEXPR_ int32_t mMinValue<int32_t>()
{
  return INT32_MIN;
}

template<>
_CONSTEXPR_ int64_t mMinValue<int64_t>()
{
  return INT64_MIN;
}

template<>
_CONSTEXPR_ uint8_t mMinValue<uint8_t>()
{
  return 0;
}

template<>
_CONSTEXPR_ uint16_t mMinValue<uint16_t>()
{
  return 0;
}

template<>
_CONSTEXPR_ uint32_t mMinValue<uint32_t>()
{
  return 0;
}

template<>
_CONSTEXPR_ uint64_t mMinValue<uint64_t>()
{
  return 0;
}

template<>
_CONSTEXPR_ float_t mMinValue<float_t>()
{
  return -FLT_MAX;
}

template<>
_CONSTEXPR_ double_t mMinValue<double_t>()
{
  return -DBL_MAX;
}

template<>
_CONSTEXPR_ int8_t mMaxValue<int8_t>()
{
  return INT8_MAX;
}

template<>
_CONSTEXPR_ int16_t mMaxValue<int16_t>()
{
  return INT16_MAX;
}

template<>
_CONSTEXPR_ int32_t mMaxValue<int32_t>()
{
  return INT32_MAX;
}

template<>
_CONSTEXPR_ int64_t mMaxValue<int64_t>()
{
  return INT64_MAX;
}

template<>
_CONSTEXPR_ uint8_t mMaxValue<uint8_t>()
{
  return UINT8_MAX;
}

template<>
_CONSTEXPR_ uint16_t mMaxValue<uint16_t>()
{
  return UINT16_MAX;
}

template<>
_CONSTEXPR_ uint32_t mMaxValue<uint32_t>()
{
  return UINT32_MAX;
}

template<>
_CONSTEXPR_ uint64_t mMaxValue<uint64_t>()
{
  return UINT64_MAX;
}

template<>
_CONSTEXPR_ float_t mMaxValue<float_t>()
{
  return FLT_MAX;
}

template<>
_CONSTEXPR_ double_t mMaxValue<double_t>()
{
  return DBL_MAX;
}

template<>
_CONSTEXPR_ float_t mSmallest<float_t>()
{
  return FLT_EPSILON;
};

template<>
_CONSTEXPR_ double_t mSmallest<double_t>()
{
  return DBL_EPSILON;
};

template <>
uint64_t _CONSTEXPR_ mMod<uint64_t>(const uint64_t value, const uint64_t modulus)
{
  return value % modulus;
}

template <>
int64_t _CONSTEXPR_ mMod<int64_t>(const int64_t value, const int64_t modulus)
{
  return value % modulus;
}

template <>
uint32_t _CONSTEXPR_ mMod<uint32_t>(const uint32_t value, const uint32_t modulus)
{
  return value % modulus;
}

template <>
int32_t _CONSTEXPR_ mMod<int32_t>(const int32_t value, const int32_t modulus)
{
  return value % modulus;
}

template <>
uint16_t _CONSTEXPR_ mMod<uint16_t>(const uint16_t value, const uint16_t modulus)
{
  return value % modulus;
}

template <>
int16_t _CONSTEXPR_ mMod<int16_t>(const int16_t value, const int16_t modulus)
{
  return value % modulus;
}

template <>
uint8_t _CONSTEXPR_ mMod<uint8_t>(const uint8_t value, const uint8_t modulus)
{
  return value % modulus;
}

template <>
int8_t _CONSTEXPR_ mMod<int8_t>(const int8_t value, const int8_t modulus)
{
  return value % modulus;
}

template <>
inline float_t mMod<float_t>(const float_t value, const float_t modulus)
{
  return fmodf(value, modulus);
}

template <>
inline double_t mMod<double_t>(const double_t value, const double_t modulus)
{
  return fmod(value, modulus);
}

#endif // mMath_h__
