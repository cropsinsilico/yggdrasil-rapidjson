#ifndef RAPIDJSON_PRECISION_H_
#define RAPIDJSON_PRECISION_H_

/*! \file precision.h */

#include "internal/meta.h"
#include "rapidjson.h"

RAPIDJSON_NAMESPACE_BEGIN

#ifdef RAPIDJSON_YGGDRASIL

#define YGGDRASIL_IS_CASTABLE(T1, T2)					\
  internal::OrExpr<							\
    internal::AndExpr<YGGDRASIL_IS_INT_TYPE(T1),			\
		      YGGDRASIL_IS_INT_TYPE(T2)>,			\
    internal::OrExpr<							\
    internal::AndExpr<YGGDRASIL_IS_INT_TYPE(T1),			\
		      YGGDRASIL_IS_FLOAT_TYPE(T2)>,			\
    internal::OrExpr<							\
    internal::AndExpr<YGGDRASIL_IS_UINT_TYPE(T1),			\
		      YGGDRASIL_IS_UINT_TYPE(T2)>,			\
    internal::OrExpr<							\
    internal::AndExpr<YGGDRASIL_IS_UINT_TYPE(T1),			\
		      YGGDRASIL_IS_INT_TYPE(T2)>,			\
    internal::OrExpr<							\
    internal::AndExpr<YGGDRASIL_IS_UINT_TYPE(T1),			\
		      YGGDRASIL_IS_FLOAT_TYPE(T2)>,			\
    internal::OrExpr<							\
    internal::AndExpr<YGGDRASIL_IS_FLOAT_TYPE(T1),			\
		      YGGDRASIL_IS_FLOAT_TYPE(T2)>,			\
    internal::AndExpr<YGGDRASIL_IS_COMPLEX_TYPE(T1), \
		      YGGDRASIL_IS_COMPLEX_TYPE(T2)> > > > > > >
#define CASE_SUBTYPE_PRECISION(T, function, param, args)		\
  case sizeof(T): { return function<T, UNPACK_MACRO(param)> args; }
#ifdef YGGDRASIL_LONG_DOUBLE_AVAILABLE
#define CASE_FLOAT_SUBTYPE(precision, function, param, args, error)	\
  case kYggFloatSubType: {						\
    switch (precision) {						\
    CASE_SUBTYPE_PRECISION(float, function, param, args)	       	\
    CASE_SUBTYPE_PRECISION(double, function, param, args)	       	\
    CASE_SUBTYPE_PRECISION((long double), function, param, args)	\
    default: { error; }							\
    }									\
  }
#define CASE_COMPLEX_SUBTYPE(precision, function, param, args, error)	\
  case kYggComplexSubType: {						\
    switch (precision) {						\
    CASE_SUBTYPE_PRECISION(std::complex<float>, function, param, args)  \
    CASE_SUBTYPE_PRECISION(std::complex<double>, function, param, args) \
    CASE_SUBTYPE_PRECISION(std::complex<long double>, function, param, args) \
    default: { error; }							\
    }									\
  }
#else // YGGDRASIL_LONG_DOUBLE_AVAILABLE
#define CASE_FLOAT_SUBTYPE(precision, function, param, args, error)	\
  case kYggFloatSubType: {						\
    switch (precision) {						\
    CASE_SUBTYPE_PRECISION(float, function, param, args)       	        \
    CASE_SUBTYPE_PRECISION(double, function, param, args)	        \
    default: { error; }							\
    }									\
  }
#define CASE_COMPLEX_SUBTYPE(precision, function, param, args, error)	\
  case kYggComplexSubType: {						\
    switch (precision) {						\
    CASE_SUBTYPE_PRECISION(std::complex<float>, function, param, args)  \
    CASE_SUBTYPE_PRECISION(std::complex<double>, function, param, args) \
    default: { error; }							\
    }									\
  }
#endif // YGGDRASIL_LONG_DOUBLE_AVAILABLE

#define SWITCH_SUBTYPE(subtype, precision, function, param, args, error)	\
  switch (subtype) {							\
  case kYggIntSubType: {						\
    switch (precision) {						\
    CASE_SUBTYPE_PRECISION(int8_t, function, param, args)       	\
    CASE_SUBTYPE_PRECISION(int16_t, function, param, args)      	\
    CASE_SUBTYPE_PRECISION(int32_t, function, param, args)      	\
    CASE_SUBTYPE_PRECISION(int64_t, function, param, args)      	\
    default: { error; }							\
    }									\
  }									\
  case kYggUintSubType: {						\
    switch (precision) {						\
    CASE_SUBTYPE_PRECISION(uint8_t, function, param, args)      	\
    CASE_SUBTYPE_PRECISION(uint16_t, function, param, args)     	\
    CASE_SUBTYPE_PRECISION(uint32_t, function, param, args)     	\
    CASE_SUBTYPE_PRECISION(uint64_t, function, param, args)    	        \
    default: { error; }							\
    }									\
  }									\
  CASE_FLOAT_SUBTYPE(precision, function, param, args, error);	        \
  CASE_COMPLEX_SUBTYPE(precision, function, param, args, error) 	\
  default: { error; }							\
  }									\
  error


template <typename T1, typename T2>
bool canCast(bool allowDecreasedPrecision=true,
	     RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T1, T2)))) {
  if (!allowDecreasedPrecision && (sizeof(T2) < sizeof(T1)))
    return false;
  return true;
}
template <typename T1, typename T2>
bool canCast(bool=true,
	     RAPIDJSON_DISABLEIF((YGGDRASIL_IS_CASTABLE(T1, T2)))) {
  return false;
}
// template <typename T2, YggSubType S1, SizeType P1>
// bool canCast(bool allowDecreasedPrecision=true) {
//   SWITCH_SUBTYPE(S1, P1, canCast, 
// }
// template <YggSubType S1, SizeType P1, YggSubType S2, SizeType P2>
// bool canCast(bool allowDecreasedPrecision=true) {
//   SWITCH_SUBTYPE(S2, P2, canCast, (1), (allowDecreasedPrecision),
// 		 return false);
// }

template <typename T1, typename T2>
T2 castPrecision(const T1& v1,
		 RAPIDJSON_DISABLEIF((internal::OrExpr<YGGDRASIL_IS_COMPLEX_TYPE(T1),
				      YGGDRASIL_IS_COMPLEX_TYPE(T2)>)))
{ return static_cast<const T2>(v1); }
template <typename T1, typename T2>
T2 castPrecision(const T1& v1,
		 RAPIDJSON_ENABLEIF((internal::AndExpr<YGGDRASIL_IS_COMPLEX_TYPE(T1),
				     YGGDRASIL_IS_COMPLEX_TYPE(T2)>)))
{ return T2(static_cast<typename T2::value_type>(v1.real()),
	    static_cast<typename T2::value_type>(v1.imag())); }
template <typename T1, typename T2>
T2 castPrecision(const T1& v1,
		 RAPIDJSON_ENABLEIF((internal::AndExpr<YGGDRASIL_IS_COMPLEX_TYPE(T1),
				     internal::NotExpr<YGGDRASIL_IS_COMPLEX_TYPE(T2)> >)))
{ return static_cast<const T2>(v1.real()); }
template <typename T1, typename T2>
T2 castPrecision(const T1& v1,
		 RAPIDJSON_ENABLEIF((internal::AndExpr<internal::NotExpr<
				     internal::OrExpr<internal::IsSame<long, T1>,
				     internal::OrExpr<internal::IsSame<unsigned long, T1>,
				     internal::OrExpr<internal::IsSame<long long, T1>,
				     internal::OrExpr<internal::IsSame<unsigned long long, T1>,
				     YGGDRASIL_IS_COMPLEX_TYPE(T1)> > > > >,
				     YGGDRASIL_IS_COMPLEX_TYPE(T2)>)))
{ return T2(v1); }
template <typename T1, typename T2>
const T2 castPrecision(const T1& v1,
		       RAPIDJSON_ENABLEIF((internal::AndExpr<
					   internal::OrExpr<internal::IsSame<long, T1>,
					   internal::OrExpr<internal::IsSame<unsigned long, T1>,
					   internal::OrExpr<internal::IsSame<long long, T1>,
					   internal::IsSame<unsigned long long, T1> > > >,
					   YGGDRASIL_IS_COMPLEX_TYPE(T2)>)))
{ return T2(static_cast<const int>(v1)); }

#define CAST_SOURCE					\
  const T1* src = reinterpret_cast<const T1*>(bytes);
#define SAME_PRECISION							\
  if ((GetYggSubType<T1>() == GetYggSubType<T2>()) && (sizeof(T2) == sizeof(T1))) { \
    memcpy(dst, src, nelements * sizeof(T2));				\
    return;								\
  }
#define DIFF_PRECISION				\
  if (sizeof(T2) < sizeof(T1))			\
    printf("WARNING: Loosing precision.");	\
  for (SizeType i = 0; i < nelements; i++)	\
    dst[i] = castPrecision<T1, T2>(src[i]);

template <typename T1, typename T2>
void changePrecision(const unsigned char* bytes, T2* dst, SizeType nelements,
		     RAPIDJSON_ENABLEIF((YGGDRASIL_IS_CASTABLE(T1,T2)))) {
  CAST_SOURCE;
  SAME_PRECISION;
  DIFF_PRECISION;
}

template <typename T1, typename T2>
void changePrecision(const unsigned char* bytes, T2* dst, SizeType nelements,
		     RAPIDJSON_DISABLEIF((YGGDRASIL_IS_CASTABLE(T1,T2)))) {
  std::cerr << typeid(T1).name() << " cannot be cast to " << typeid(T2).name() << std::endl;
  RAPIDJSON_ASSERT(!sizeof("Cannot change from T1 to T2"));
}

#undef CAST_SOURCE
#undef SAME_PRECISION
#undef DIFF_PRECISION

template <typename T1, typename T2, typename Allocator>
T2* changePrecision(const unsigned char* bytes, SizeType nelements,
		    Allocator& allocator) {
  T2* v2 = (T2*)allocator.Malloc(nelements * sizeof(T2));
  RAPIDJSON_ASSERT(v2);
  changePrecision<T1,T2>(bytes, v2, nelements);
  // RAPIDJSON_ASSERT(GetYggSubType<T1>() == GetYggSubType<T2>());
  // if (sizeof(T2) == sizeof(T1))
  //   return (T2*)bytes;
  // else if (sizeof(T2) < sizeof(T1))
  //   printf("WARNING: Loosing precision.");
  // T1* v1 = reinterpret_cast<T1*>(bytes);
  // T2* v2 = static_cast<T2*>(malloc(nelements * sizeof(T2)));
  // for (SizeType i = 0; i < nelements; i++)
  //   v2[i] = castPrecision<T1, T2>(v1[i]);
  return v2;
}

template <typename T>
void changePrecision(YggSubType subtype, SizeType precision,
		     const unsigned char* bytes, T* dst,
		     SizeType nelements) {
  SWITCH_SUBTYPE(subtype, precision, changePrecision,
		 PACK_MACRO(T), (bytes, dst, nelements),
		 RAPIDJSON_ASSERT(false));
}

template <typename T, size_t>
SizeType _sizeOf() { return sizeof(T); }

static inline
SizeType sizeOfSubtype(const YggSubType subtype, const SizeType precision) {
  SWITCH_SUBTYPE(subtype, precision, _sizeOf, PACK_MACRO(1), (), return 0);
}

template <typename T, size_t>
void changePrecision(YggSubType subtype, SizeType precision,
		     const unsigned char* src_bytes,
		     unsigned char* dst_bytes, SizeType& dst_nbytes,
		     SizeType nelements) {
  RAPIDJSON_ASSERT((nelements * sizeof(T)) <= dst_nbytes);
  SWITCH_SUBTYPE(subtype, precision, changePrecision,
		 PACK_MACRO(T), (src_bytes, (T*)dst_bytes, nelements),
		 RAPIDJSON_ASSERT(false));
  dst_nbytes = nelements * (SizeType)sizeof(T);
}

static inline
void changePrecision(const YggSubType src_subtype, const SizeType src_precision,
		     const unsigned char* src_bytes, const SizeType src_nbytes,
		     YggSubType dst_subtype, SizeType dst_precision,
		     unsigned char* dst_bytes, SizeType& dst_nbytes,
		     const SizeType nelements) {
  (void)src_nbytes;
  // SizeType src_size = sizeOfSubtype(src_subtype, src_precision);
  // SizeType dst_size = sizeOfSubtype(dst_subtype, dst_precision);
  // RAPIDJSON_ASSERT((nelements * src_size) == src_nbytes);
  // RAPIDJSON_ASSERT((nelements * dst_size) <= dst_nbytes);
  // RAPIDJSON_ASSERT(dst_size <= src_size);
  SWITCH_SUBTYPE(dst_subtype, dst_precision, changePrecision,
		 PACK_MACRO(1), (src_subtype, src_precision, src_bytes,
				 dst_bytes, dst_nbytes, nelements),
		 RAPIDJSON_ASSERT(false));
}

#endif // RAPIDJSON_YGGDRASIL

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_PRECISION_H_
