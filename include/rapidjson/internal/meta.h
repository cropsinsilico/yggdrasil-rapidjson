// Tencent is pleased to support the open source community by making RapidJSON available.
// 
// Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.
//
// Licensed under the MIT License (the "License"); you may not use this file except
// in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, software distributed 
// under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
// CONDITIONS OF ANY KIND, either express or implied. See the License for the 
// specific language governing permissions and limitations under the License.

#ifndef RAPIDJSON_INTERNAL_META_H_
#define RAPIDJSON_INTERNAL_META_H_

#include "../rapidjson.h"

#ifdef __GNUC__
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

#if defined(_MSC_VER) && !defined(__clang__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(6334)
#endif

#if RAPIDJSON_HAS_CXX11_TYPETRAITS
#include <type_traits>
#endif

#ifdef RAPIDJSON_YGGDRASIL
#include <cstdarg>
#endif // RAPIDJSON_YGGDRASIL

//@cond RAPIDJSON_INTERNAL
RAPIDJSON_NAMESPACE_BEGIN

#ifdef RAPIDJSON_YGGDRASIL
//  Produce value of bit n.  n must be less than 32.
#define Bit_(n, p)  ((uint ## p ## _t) 1 << (n))
//  Create a mask of n bits in the low bits.  n must be less than 32.
#define Mask_(n, p) (Bit_(n, p) - 1)
//  Produce value of bit n.  n must be less than 32.
#define Bit(n)  ((uint32_t) 1 << (n))
//  Create a mask of n bits in the low bits.  n must be less than 32.
#define Mask(n) (Bit(n) - 1)

typedef struct float16_t {
  uint16_t mem;
  // float16_t(const double x) : mem(0) {
  //   float y = static_cast<float>(x);
  //   from_float(y);
  // }
  // float16_t(const float x) : mem(0) {
  //   from_float(x);
  // }
  template <typename T>
  float16_t(const T x) {
    float y = static_cast<float>(x);
    from_float(y);
  }
  void from_float(const float& x) {
    union { uint32_t enc; float  value; } tmp;
    tmp.value = x;
    uint16_t s = tmp.enc >> 31;
    if ((tmp.enc >> 23 & Mask( 8)) != 255) {
      // Use float arithmetic to ensure values are properly rounded
#if RAPIDJSON_HAS_CXX17
      tmp.value = (tmp.value * (1.0f + 0x1p-13f) - tmp.value) * 0x1p13f;
      tmp.value *= 0x1p112f;
      tmp.value *= 0x1p-112f;
      tmp.value *= 0x1p-112f;
#else // RAPIDJSON_HAS_CXX17
      float p13 = pow(2.0, 13), pn13 = pow(2.0, -13),
	p122 = pow(2.0, 122), pn122 = pow(2.0, -122);
      tmp.value = (tmp.value * (1.0f + pn13) - tmp.value) * p13;
      tmp.value *= p122;
      tmp.value *= pn122;
      tmp.value *= pn122;
#endif // RAPIDJSON_HAS_CXX17
    }
    mem = static_cast<uint16_t>(s << 15u) | static_cast<uint16_t>((tmp.enc >> 13u) & Mask_(15u, 16));
    // uint32_t e = tmp.enc >> 23 & Mask( 8);
    // uint32_t f = tmp.enc       & Mask(23);
    // f >>= 23 - 10;
    // f &= Mask(10);
    // switch (e) {
    // case 0:
    //   if (f != 0) {
    // 	e = 1 + (15 - 127);
    // 	while (f > Bit(10)) {
    // 	  f >>= 1;
    // 	  e += 1;
    // 	}
    // 	f &= Mask(10);
    //   }
    //   break;
    // default:
    //   e += 15 - 127;
    //   break;
    // case 255:
    //   e = 31;
    //   break;
    // }
    // if (e == 255) {
    //   mem = s << 15 | 31 << 10 | f;
    // } else {
    //   mem = s << 15 | e << 10 | f;
    // }
  }
  operator float() const {
    union { uint32_t enc; float  value; } tmp;
    uint32_t s = mem >> 15;
    uint32_t e = mem >> 10 & Mask( 5);
    uint32_t f = mem       & Mask(10);
    f <<= 23 - 10;
    // switch (e) {
    // case 0:
    //   if (f != 0) {
    // 	e = 1 + (127 - 15);
    // 	while (f < Bit(23)) {
    // 	  f <<= 1;
    // 	  e -= 1;
    // 	}
    // 	f &= Mask(23);
    //   }
    //   break;
    // default:
    //   e += 127 - 15;
    //   break;
    // case 31:
    //   e = 255;
    //   break;
    // }
    if (e == 31) {
      tmp.enc = s << 31 | 255 << 23 | f;
    } else {
      tmp.enc = s << 31 | e << 23 | f;
#if RAPIDJSON_HAS_CXX17
      tmp.value *= 0x1p112f;
#else // RAPIDJSON_HAS_CXX17
      tmp.value *= pow(2.0, 112);
#endif // RAPIDJSON_HAS_CXX17
    }
    return tmp.value;
  }
#define OP_(type)				\
  operator type() const {			\
    float tmp = float(*this);			\
    return static_cast<type>(tmp);		\
  }
  OP_(double);
  OP_(int8_t);
  OP_(int16_t);
  OP_(int32_t);
  OP_(int64_t);
  OP_(uint8_t);
  OP_(uint16_t);
  OP_(uint32_t);
  OP_(uint64_t);
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
  OP_(long double);
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
#undef OP_
} float16_t;

#undef Bit
#undef Mask
#endif // RAPIDJSON_YGGDRASIL

namespace internal {

// Helper to wrap/convert arbitrary types to void, useful for arbitrary type matching
template <typename T> struct Void { typedef void Type; };

///////////////////////////////////////////////////////////////////////////////
// BoolType, TrueType, FalseType
//
template <bool Cond> struct BoolType {
    static const bool Value = Cond;
    typedef BoolType Type;
};
typedef BoolType<true> TrueType;
typedef BoolType<false> FalseType;


///////////////////////////////////////////////////////////////////////////////
// SelectIf, BoolExpr, NotExpr, AndExpr, OrExpr
//

template <bool C> struct SelectIfImpl { template <typename T1, typename T2> struct Apply { typedef T1 Type; }; };
template <> struct SelectIfImpl<false> { template <typename T1, typename T2> struct Apply { typedef T2 Type; }; };
template <bool C, typename T1, typename T2> struct SelectIfCond : SelectIfImpl<C>::template Apply<T1,T2> {};
template <typename C, typename T1, typename T2> struct SelectIf : SelectIfCond<C::Value, T1, T2> {};

template <bool Cond1, bool Cond2> struct AndExprCond : FalseType {};
template <> struct AndExprCond<true, true> : TrueType {};
template <bool Cond1, bool Cond2> struct OrExprCond : TrueType {};
template <> struct OrExprCond<false, false> : FalseType {};

template <typename C> struct BoolExpr : SelectIf<C,TrueType,FalseType>::Type {};
template <typename C> struct NotExpr  : SelectIf<C,FalseType,TrueType>::Type {};
template <typename C1, typename C2> struct AndExpr : AndExprCond<C1::Value, C2::Value>::Type {};
template <typename C1, typename C2> struct OrExpr  : OrExprCond<C1::Value, C2::Value>::Type {};


///////////////////////////////////////////////////////////////////////////////
// AddConst, MaybeAddConst, RemoveConst
template <typename T> struct AddConst { typedef const T Type; };
template <bool Constify, typename T> struct MaybeAddConst : SelectIfCond<Constify, const T, T> {};
template <typename T> struct RemoveConst { typedef T Type; };
template <typename T> struct RemoveConst<const T> { typedef T Type; };


///////////////////////////////////////////////////////////////////////////////
// IsSame, IsConst, IsMoreConst, IsPointer
//
template <typename T, typename U> struct IsSame : FalseType {};
template <typename T> struct IsSame<T, T> : TrueType {};

template <typename T> struct IsConst : FalseType {};
template <typename T> struct IsConst<const T> : TrueType {};

template <typename CT, typename T>
struct IsMoreConst
    : AndExpr<IsSame<typename RemoveConst<CT>::Type, typename RemoveConst<T>::Type>,
              BoolType<IsConst<CT>::Value >= IsConst<T>::Value> >::Type {};

template <typename T> struct IsPointer : FalseType {};
template <typename T> struct IsPointer<T*> : TrueType {};

///////////////////////////////////////////////////////////////////////////////
// IsBaseOf
//
#if RAPIDJSON_HAS_CXX11_TYPETRAITS

template <typename B, typename D> struct IsBaseOf
    : BoolType< ::std::is_base_of<B,D>::value> {};

#else // simplified version adopted from Boost

template<typename B, typename D> struct IsBaseOfImpl {
    RAPIDJSON_STATIC_ASSERT(sizeof(B) != 0);
    RAPIDJSON_STATIC_ASSERT(sizeof(D) != 0);

    typedef char (&Yes)[1];
    typedef char (&No) [2];

    template <typename T>
    static Yes Check(const D*, T);
    static No  Check(const B*, int);

    struct Host {
        operator const B*() const;
        operator const D*();
    };

    enum { Value = (sizeof(Check(Host(), 0)) == sizeof(Yes)) };
};

template <typename B, typename D> struct IsBaseOf
    : OrExpr<IsSame<B, D>, BoolExpr<IsBaseOfImpl<B, D> > >::Type {};

#endif // RAPIDJSON_HAS_CXX11_TYPETRAITS


//////////////////////////////////////////////////////////////////////////
// EnableIf / DisableIf
//
template <bool Condition, typename T = void> struct EnableIfCond  { typedef T Type; };
template <typename T> struct EnableIfCond<false, T> { /* empty */ };

template <bool Condition, typename T = void> struct DisableIfCond { typedef T Type; };
template <typename T> struct DisableIfCond<true, T> { /* empty */ };

template <typename Condition, typename T = void>
struct EnableIf : EnableIfCond<Condition::Value, T> {};

template <typename Condition, typename T = void>
struct DisableIf : DisableIfCond<Condition::Value, T> {};

// SFINAE helpers
struct SfinaeTag {};
template <typename T> struct RemoveSfinaeTag;
template <typename T> struct RemoveSfinaeTag<SfinaeTag&(*)(T)> { typedef T Type; };

#define RAPIDJSON_REMOVEFPTR_(type) \
    typename ::RAPIDJSON_NAMESPACE::internal::RemoveSfinaeTag \
        < ::RAPIDJSON_NAMESPACE::internal::SfinaeTag&(*) type>::Type

#define RAPIDJSON_ENABLEIF(cond) \
    typename ::RAPIDJSON_NAMESPACE::internal::EnableIf \
        <RAPIDJSON_REMOVEFPTR_(cond)>::Type * = NULL

#define RAPIDJSON_DISABLEIF(cond) \
    typename ::RAPIDJSON_NAMESPACE::internal::DisableIf \
        <RAPIDJSON_REMOVEFPTR_(cond)>::Type * = NULL

#define RAPIDJSON_ENABLEIF_RETURN(cond,returntype) \
    typename ::RAPIDJSON_NAMESPACE::internal::EnableIf \
        <RAPIDJSON_REMOVEFPTR_(cond), \
         RAPIDJSON_REMOVEFPTR_(returntype)>::Type

#define RAPIDJSON_DISABLEIF_RETURN(cond,returntype) \
    typename ::RAPIDJSON_NAMESPACE::internal::DisableIf \
        <RAPIDJSON_REMOVEFPTR_(cond), \
         RAPIDJSON_REMOVEFPTR_(returntype)>::Type

#ifdef RAPIDJSON_YGGDRASIL
// https://stackoverflow.com/questions/257288/templated-check-for-the-existence-of-a-class-member-function
template<typename Type, typename ValueType>  //, typename SchemaType>
class HasYggdrasilMethodImpl
{
  typedef char Yes;
  typedef long No;
  template <typename T, typename VT>
  static Yes HasYggdrasil(decltype(&T::template YggdrasilString<VT>));
  template <typename T, typename VT>
  static No  HasYggdrasil(...);
public:
  static const bool Value = (sizeof(HasYggdrasil<Type,ValueType>(0)) == sizeof(Yes));
};
template <typename T, typename VT> struct HasYggdrasilMethod
  : BoolExpr<HasYggdrasilMethodImpl<T, VT> >::Type {};

template<typename T>
std::vector<T> pack_vector__(const size_t N, const T first...) {
  std::vector<T> out;
  out.push_back(first);
  va_list args;
  va_start(args, first);
  size_t i = 1;
  while (i < N) {
    const T x = va_arg(args, T);
    out.push_back(x);
    i++;
  }
  va_end(args);
  return out;
}
  
/*! @brief Define macros to allow counts of variables. */
// https://codecraft.co/2014/11/25/variadic-macros-tricks/
#ifdef _MSC_VER
// https://stackoverflow.com/questions/48710758/how-to-fix-variadic-macro-related-issues-with-macro-overloading-in-msvc-mic
#define MSVC_BUG(MACRO, ARGS) MACRO ARGS  // name to remind that bug fix is due to MSVC :-)
#define _GET_NTH_ARG_2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, N, ...) N
#define _GET_NTH_ARG(...) MSVC_BUG(_GET_NTH_ARG_2, (__VA_ARGS__))
#define COUNT_VARARGS(...) _GET_NTH_ARG("ignored", ##__VA_ARGS__, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define VA_MACRO(MACRO, ...) MSVC_BUG(CONCATE, (MACRO, COUNT_VARARGS(__VA_ARGS__)))(__VA_ARGS__)
#else
#define _GET_NTH_ARG(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _11, _12, _13, _14, N, ...) N
#define COUNT_VARARGS(...) _GET_NTH_ARG("ignored", ##__VA_ARGS__, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#endif
#define UNUSED(arg) ((void)&(arg))

#define pack_vector_(x, ...) pack_vector__(COUNT_VARARGS(__VA_ARGS__) + 1, x, __VA_ARGS__)
#define pack_vector_T_(T, ...) pack_vector__<T>(COUNT_VARARGS(__VA_ARGS__), __VA_ARGS__)

#define _Args(...) __VA_ARGS__
#define STRIP_PARENS(X) X
#define PACK_MACRO_(X) STRIP_PARENS( _Args X )
#define PACK_MACRO(...) PACK_MACRO_((__VA_ARGS__))
#define UNPACK_MACRO(...) __VA_ARGS__

#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
  
#define YGGDRASIL_IS_COMPLEX_TYPE(T)					\
  internal::OrExpr<internal::IsSame<T,std::complex<float> >,		\
    internal::OrExpr<internal::IsSame<T,std::complex<double> >,	\
		     internal::IsSame<T,std::complex<long double> > > >
#define YGGDRASIL_IS_FLOAT_TYPE(T)				\
  internal::OrExpr<internal::IsSame<T,float16_t>,		\
    internal::OrExpr<internal::IsSame<T,float>,			\
    internal::OrExpr<internal::IsSame<T,double>,		\
    internal::IsSame<T,long double> > > >
#define YGGDRASIL_IS_SCALAR_TYPE(T)				\
  internal::OrExpr<internal::IsSame<T,uint8_t>,			\
    internal::OrExpr<internal::IsSame<T,uint16_t>,		\
    internal::OrExpr<internal::IsSame<T,int8_t>,		\
    internal::OrExpr<internal::IsSame<T,int16_t>,		\
    internal::OrExpr<internal::IsSame<T,long double>,	        \
		     YGGDRASIL_IS_COMPLEX_TYPE(T)> > > > >
#else // YGGDRASIL_LONG_DOUBLE_AVAILABLE
#define YGGDRASIL_IS_COMPLEX_TYPE(T)				\
    internal::OrExpr<internal::IsSame<T,std::complex<float> >,	\
		     internal::IsSame<T,std::complex<double> > >
#define YGGDRASIL_IS_FLOAT_TYPE(T)				\
  internal::OrExpr<internal::IsSame<T,float16_t>,		\
    internal::OrExpr<internal::IsSame<T,float>,			\
    internal::IsSame<T, double> > >
#define YGGDRASIL_IS_SCALAR_TYPE(T)				\
  internal::OrExpr<internal::IsSame<T,uint8_t>,			\
    internal::OrExpr<internal::IsSame<T,uint16_t>,		\
    internal::OrExpr<internal::IsSame<T,int8_t>,		\
    internal::OrExpr<internal::IsSame<T,int16_t>,		\
		     YGGDRASIL_IS_COMPLEX_TYPE(T)> > > >
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE
#define YGGDRASIL_IS_INT_TYPE(T)			\
  internal::OrExpr<internal::IsSame<T,int8_t>,		\
    internal::OrExpr<internal::IsSame<T,int16_t>,	\
    internal::OrExpr<internal::IsSame<T,int32_t>,	\
    internal::IsSame<T,int64_t> > > >
#define YGGDRASIL_IS_UINT_TYPE(T)			\
  internal::OrExpr<internal::IsSame<T,uint8_t>,		\
    internal::OrExpr<internal::IsSame<T,uint16_t>,	\
    internal::OrExpr<internal::IsSame<T,uint32_t>,	\
    internal::IsSame<T,uint64_t> > > >
#define YGGDRASIL_IS_ANY_SCALAR(T)			\
  internal::OrExpr<YGGDRASIL_IS_SCALAR_TYPE(T),		\
    internal::OrExpr<internal::IsSame<T,float>,		\
    internal::OrExpr<internal::IsSame<T,double>,	\
    internal::OrExpr<internal::IsSame<T,uint32_t>,	\
    internal::OrExpr<internal::IsSame<T,uint64_t>,	\
    internal::OrExpr<internal::IsSame<T,int32_t>,	\
		     internal::IsSame<T,int64_t> > > > > > >
#define YGGDRASIL_IS_CHAR(T)			\
  internal::OrExpr<internal::IsSame<T,char>,	\
		   internal::IsSame<T,wchar_t> >

template<typename Ta, typename Tb>
inline bool values_eq(const Ta& a, const Tb& b,
		      RAPIDJSON_DISABLEIF((internal::OrExpr<
					   YGGDRASIL_IS_FLOAT_TYPE(Ta),
					   internal::OrExpr<
					   YGGDRASIL_IS_FLOAT_TYPE(Tb),
					   internal::OrExpr<
					   YGGDRASIL_IS_COMPLEX_TYPE(Ta),
					   YGGDRASIL_IS_COMPLEX_TYPE(Tb)> > >))) {
  return a == static_cast<Ta>(b);
}
  
template<typename Ta, typename Tb>
inline bool values_eq(const Ta& a0, const Tb& b0,
		      RAPIDJSON_ENABLEIF((internal::OrExpr<
			         	  YGGDRASIL_IS_FLOAT_TYPE(Ta),
					  YGGDRASIL_IS_FLOAT_TYPE(Tb)>))) {
  double a = static_cast<double>(a0);
  double b = static_cast<double>(b0);
  // double abs_precision = 1.0e-13; // std::numeric_limits<double>::epsilon();
  double abs_precision = std::numeric_limits<double>::epsilon();
  double rel_precision = std::numeric_limits<double>::epsilon();
  if ((std::abs(a) < abs_precision) || (std::abs(b) < abs_precision))
    return (std::abs((a - b)*(b - a)) <= abs_precision);
  return (std::abs(((a - b)*(b - a)) / (a * b)) <= rel_precision);
}

template<typename Ta, typename Tb>
inline bool values_eq(const Ta& a, const Tb& b,
		      RAPIDJSON_ENABLEIF((internal::AndExpr<
			         	  YGGDRASIL_IS_COMPLEX_TYPE(Ta),
					  YGGDRASIL_IS_COMPLEX_TYPE(Tb)>))) {
  return (values_eq(a.real(), b.real()) && values_eq(a.imag(), b.imag()));
}
  
template<typename Ta, typename Tb>
inline bool values_lt(const Ta& a, const Tb& b,
		      RAPIDJSON_DISABLEIF((internal::OrExpr<
			          	   YGGDRASIL_IS_COMPLEX_TYPE(Ta),
					   YGGDRASIL_IS_COMPLEX_TYPE(Tb)>)))
{ return a < static_cast<Ta>(b); }
template<typename Ta, typename Tb>
inline bool values_lt(const Ta& a, const Tb& b,
		      RAPIDJSON_ENABLEIF((internal::AndExpr<
					  YGGDRASIL_IS_COMPLEX_TYPE(Ta),
					  YGGDRASIL_IS_COMPLEX_TYPE(Tb)>)))
{ return values_lt(std::abs(a), std::abs(b)); }

template<typename Ta, typename Tb>
inline bool values_gt(const Ta& a, const Tb& b,
		      RAPIDJSON_DISABLEIF((internal::OrExpr<
			          	   YGGDRASIL_IS_COMPLEX_TYPE(Ta),
					   YGGDRASIL_IS_COMPLEX_TYPE(Tb)>)))
{ return a > static_cast<Ta>(b); }
template<typename Ta, typename Tb>
inline bool values_gt(const Ta& a, const Tb& b,
		      RAPIDJSON_ENABLEIF((internal::AndExpr<
					  YGGDRASIL_IS_COMPLEX_TYPE(Ta),
					  YGGDRASIL_IS_COMPLEX_TYPE(Tb)>)))
{ return values_gt(std::abs(a), std::abs(b)); }
  
template<typename T>
inline T value_floor(const T& x) {
  return static_cast<T>(std::floor(static_cast<double>(x)));
}
template<typename T>
inline std::complex<T> value_floor(const std::complex<T>& x) {
  return std::complex<T>(value_floor(x.real()), value_floor(x.imag()));
}
  
#endif // RAPIDJSON_YGGDRASIL
  
} // namespace internal

RAPIDJSON_NAMESPACE_END
//@endcond

#if defined(_MSC_VER) && !defined(__clang__)
RAPIDJSON_DIAG_POP
#endif

#ifdef __GNUC__
RAPIDJSON_DIAG_POP
#endif

#endif // RAPIDJSON_INTERNAL_META_H_
