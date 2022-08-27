#ifndef RAPIDJSON_OBJ_H_
#define RAPIDJSON_OBJ_H_

#include <iostream>
#include <set>
#include "internal/meta.h"

RAPIDJSON_NAMESPACE_BEGIN

//! \brief Convert from an alias for a geometry element to the base.
//! \param alias Name to check.
//! \return Base name associated with the provided alias.
static inline
std::string obj_alias2base(const std::string& alias) {
  if      (alias == "vertices") return std::string("v");
  else if (alias == "vertexes") return std::string("v");
  else if (alias == "vertex"  ) return std::string("v");
  else if (alias == "faces"   ) return std::string("f");
  else if (alias == "face"    ) return std::string("f");
  else if (alias == "edges"   ) return std::string("l");
  else if (alias == "edge"    ) return std::string("l");
  return std::string(alias);
}

//! \brief Convert from a code string to the long element name.
//! \param code Name to check.
//! \return Long name associated with the provided element.
static inline
std::string obj_code2long(const std::string& code) {
  if      (code == "v" ) return std::string("vertex");
  else if (code == "f" ) return std::string("face");
  else if (code == "l" ) return std::string("edge");
  return std::string(code);
}

enum ObjTypeFlag {
  ObjTypeNull     = 0x0000,
  ObjTypeInt      = 0x0008,
  ObjTypeString   = 0x0010,
  ObjTypeFloat    = 0x0020,
  ObjTypeVertex   = 0x0040,
  ObjTypeCurve    = 0x0080,
  ObjTypeSurface  = 0x0100,
  ObjTypeList     = 0x0800
};

typedef std::map<std::string, uint16_t> PropertiesMap;

#define REPORT_UNSUPPORTED_ELEMENT(src, name)	\
  {									\
    std::cerr << "Unsupported element signifier for " #src ": " << name << std::endl; \
    RAPIDJSON_ASSERT(!sizeof("Unsupported element signifier"));		\
  }
#define OBJ_ELEMENT_INIT(word, lhs, args) {				\
    bool first = true;							\
    while (true) {							\
      if      (word == "v"         ) lhs = new ObjVertex args;		\
      else if (word == "vp"        ) lhs = new ObjVParameter args;	\
      else if (word == "vn"        ) lhs = new ObjVNormal args;		\
      else if (word == "vt"        ) lhs = new ObjVTexture args;	\
      else if (word == "p"         ) lhs = new ObjPoint args;		\
      else if (word == "l"         ) lhs = new ObjLine args;		\
      else if (word == "f"         ) lhs = new ObjFace args;		\
      else if (word == "curv"      ) lhs = new ObjCurve args;		\
      else if (word == "curv2"     ) lhs = new ObjCurve2D args;		\
      else if (word == "surf"      ) lhs = new ObjSurface args;		\
      else if (word == "cstype"    ) lhs = new ObjFreeFormType args;	\
      else if (word == "deg"       ) lhs = new ObjDegree args;		\
      else if (word == "bmat"      ) lhs = new ObjBasisMatrix args;	\
      else if (word == "step"      ) lhs = new ObjStep args;		\
      else if (word == "parm"      ) lhs = new ObjParameter args;	\
      else if (word == "trim"      ) lhs = new ObjTrim args;		\
      else if (word == "hole"      ) lhs = new ObjHole args;		\
      else if (word == "scrv"      ) lhs = new ObjScrv args;		\
      else if (word == "sp"        ) lhs = new ObjSpecialPoints args;	\
      else if (word == "con"       ) lhs = new ObjConnect args;		\
      else if (word == "g"         ) lhs = new ObjGroup args;		\
      else if (word == "s"         ) lhs = new ObjSmoothingGroup args;	\
      else if (word == "mg"        ) lhs = new ObjMergingGroup args;	\
      else if (word == "o"         ) lhs = new ObjObjectName args;	\
      else if (word == "bevel"     ) lhs = new ObjBevel args;		\
      else if (word == "c_interp"  ) lhs = new ObjCInterp args;		\
      else if (word == "d_interp"  ) lhs = new ObjDInterp args;		\
      else if (word == "lod"       ) lhs = new ObjLOD args;		\
      else if (word == "maplib"    ) lhs = new ObjTextureMapLib args;	\
      else if (word == "usemap"    ) lhs = new ObjTextureMap args;	\
      else if (word == "usemtl"    ) lhs = new ObjMaterial args;	\
      else if (word == "mtllib"    ) lhs = new ObjMaterialLib args;	\
      else if (word == "shadow_obj") lhs = new ObjShadowFile args;	\
      else if (word == "trace_obj" ) lhs = new ObjTraceFile args;	\
      else if (word == "ctech"     ) lhs = new ObjCTech args;		\
      else if (word == "stech"     ) lhs = new ObjSTech args;		\
      else if (word == "end"       ) lhs = nullptr;			\
      else if (first) {							\
	first = false;							\
	word = obj_alias2base(word);					\
	continue;							\
      } else {								\
	REPORT_UNSUPPORTED_ELEMENT(init, word);				\
      }									\
      break;								\
    }									\
    }
#if RAPIDJSON_HAS_CXX11
#define OVERRIDE_CXX11 override
#else // RAPIDJSON_HAS_CXX11
#define OVERRIDE_CXX11
#endif // RAPIDJSON_HAS_CXX11
#define SINGLE_ARG(...) (__VA_ARGS__)

#define OBJ_P_(name, flag) PropertiesMap::value_type(name, uint16_t(flag))
#define SET_PROPS(props)					\
    const PropertiesMap::value_type pairs[] = {			\
      UNPACK_MACRO props						\
    };								\
    const int Npair = sizeof(pairs) / sizeof(pairs[0]);		\
    this->properties.insert(pairs, pairs + Npair)
#define COMPARE_IDX(x, nprev)						\
  (((int)x >= 0 && (size_t)x <= nprev) ||				\
   ((int)x < 0 && (int)x < -(int)nprev))

#define GENERIC_CONSTRUCTOR_READ(cls, base, codeS, init, props)		\
  /*! \brief Initialize an element by reading from an input stream. */	\
  /*! \param in Input stream to read from. */				\
  /*! \param parent0 The element's parent group. */			\
  /*! \param dont_descend If true, subelements will not be read from */ \
  /*!   the input stream. */						\
  cls(std::istream &in, const ObjGroupBase* parent0 = nullptr,		\
      const bool& dont_descend = false) : base(#codeS, parent0) UNPACK_MACRO init { \
    SET_PROPS(props);							\
    read_values(in, dont_descend);					\
    from_values();							\
  }
#define GENERIC_CONSTRUCTOR_COPY(cls, base, init)			\
  /*! \copydoc ObjElement::ObjElement(const ObjElement&) */		\
  cls(const cls& rhs) :							\
    base(rhs.code, rhs.parent) UNPACK_MACRO init {				\
    base::copy_members(*dynamic_cast<const base*>(&rhs));		\
    this->copy_members(rhs);						\
    this->properties = rhs.properties;					\
  }									\
  /*! \copydoc ObjElement::ObjElement(const ObjElement*) */		\
  cls(const ObjElement* rhs) :						\
    base(rhs->code, rhs->parent) UNPACK_MACRO init {				\
    base::copy_members(*dynamic_cast<const base*>(rhs));		\
    this->copy_members(*dynamic_cast<const cls*>(rhs));			\
    this->properties = rhs->properties;					\
  }									\
  /*! \copydoc ObjElement::copy() */					\
  cls* copy() const OVERRIDE_CXX11 { return new cls(*this); }
#define GENERIC_ELEMENT_CONSTRUCTOR(cls, base, code, init, props)	\
  /*! \brief Empty constructor. */					\
  /*! \param parent0 The element's parent group. */			\
  cls(const ObjGroupBase* parent0 = nullptr) :				\
    base(#code, parent0) UNPACK_MACRO init {					\
    SET_PROPS(props);							\
  }									\
  GENERIC_CONSTRUCTOR_COPY(cls, base, init);				\
  GENERIC_CONSTRUCTOR_READ(cls, base, code, init, props);
#define COMPATIBLE_WITH_INT(T)				       \
  internal::OrExpr<internal::IsSame<T,int>,	               \
    internal::OrExpr<internal::IsSame<T,int8_t>,               \
      internal::OrExpr<internal::IsSame<T,uint8_t>,	       \
	internal::OrExpr<internal::IsSame<T,int16_t>,	       \
	  internal::OrExpr<internal::IsSame<T,uint16_t>,       \
	    internal::OrExpr<internal::IsSame<T,int32_t>,      \
	      internal::OrExpr<internal::IsSame<T,uint32_t>,   \
		internal::OrExpr<internal::IsSame<T,int64_t>,  \
		  internal::OrExpr<internal::IsSame<T,ObjRef>, \
		    internal::IsSame<T,uint64_t> > > > > > > > > >
#define COMPATIBLE_WITH_UINT(T)				       \
      internal::OrExpr<internal::IsSame<T,uint8_t>,	       \
	  internal::OrExpr<internal::IsSame<T,uint16_t>,       \
	      internal::OrExpr<internal::IsSame<T,uint32_t>,   \
		  internal::OrExpr<internal::IsSame<T,ObjRef>, \
		      internal::IsSame<T,uint64_t> > > > >
#define COMPATIBLE_WITH_FLOAT(T)			       \
  internal::OrExpr<internal::IsSame<T,int>,	               \
    internal::OrExpr<internal::IsSame<T,int8_t>,               \
      internal::OrExpr<internal::IsSame<T,uint8_t>,	       \
	internal::OrExpr<internal::IsSame<T,int16_t>,	       \
	  internal::OrExpr<internal::IsSame<T,uint16_t>,       \
	    internal::OrExpr<internal::IsSame<T,int32_t>,      \
	      internal::OrExpr<internal::IsSame<T,uint32_t>,   \
		internal::OrExpr<internal::IsSame<T,int64_t>,  \
		  internal::OrExpr<internal::IsSame<T,float>, \
  		    internal::OrExpr<internal::IsSame<T,double>, \
		      internal::IsSame<T,uint64_t> > > > > > > > > > >
#define COMPATIBLE_WITH_VERT(T)						\
  internal::OrExpr<internal::IsSame<T,ObjRefVertex>, COMPATIBLE_WITH_INT(T)>
#define COMPATIBLE_WITH_CURV(T)					\
  internal::IsSame<T,ObjRefCurve>
#define COMPATIBLE_WITH_SURF(T)					\
  internal::IsSame<T,ObjRefSurface>
#define COMPATIBLE_WITH_TYPE(T1, T2)		 \
  internal::IsSame<T1,T2>
#define COMPATIBLE_WITH_ANY(T)						\
  internal::OrExpr<COMPATIBLE_WITH_INT(T),				\
    internal::OrExpr<COMPATIBLE_WITH_FLOAT(T),				\
      internal::OrExpr<COMPATIBLE_WITH_VERT(T),				\
       	internal::OrExpr<COMPATIBLE_WITH_CURV(T),	                \
       	  internal::OrExpr<COMPATIBLE_WITH_SURF(T),	                \
			   COMPATIBLE_WITH_TYPE(T, std::string)> > > > >
#define ASSERT_COMPATIBLE(T1, T2)					\
  std::cerr << typeid(T1).name() << " and " << typeid(T2).name() << " types are not compatible." << std::endl; \
  RAPIDJSON_ASSERT(!sizeof("T1 and T2 types are not compatible."))
#define GENERIC_CONSTRUCTOR_VECTOR_TYPE(cls, base, code, init, props, compat, T2) \
  GENERIC_ELEMENT_CONSTRUCTOR(cls, base, code, init, props)		\
  /*! \brief Initialize and element from a C++ vector of values. */	\
  /*! \tparam T Vector element type. Must be castable to the type. */	\
  /*! \param values0 Vector of values. */				\
  /*! \param parent0 The element's parent group. */			\
  template <typename T>							\
  cls(const std::vector<T> &values0,					\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((compat))) :					\
  base(#code, parent0) UNPACK_MACRO init {					\
    SET_PROPS(props);							\
    assign_values(values, values0);					\
    from_values();							\
  }									\
  /*! \brief Raise an error if non-compatible vector is provided. */	\
  /*! \tparam T Type of vector elements. */				\
  template <typename T>							\
  cls(const std::vector<T>&,						\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((compat))) :					\
    base(#code, parent0) UNPACK_MACRO init {					\
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from " #T2 "s.")); \
  }									\
  /*! \brief Initialize an element from a C array of values. */		\
  /*! \tparam T Array element type. */					\
  /*! \tparam N Array size. */						\
  /*! \param src Array of values. */					\
  template <typename T, size_t N>					\
  cls(const T (&values0)[N],						\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((compat))) :					\
    base(#code, parent0) UNPACK_MACRO init {					\
    SET_PROPS(props);							\
    assign_values(values, std::vector<T>(values0, values0+N));		\
    from_values();							\
  }									\
  /*! \brief Raise an error if non-compatible vector is provided. */	\
  /*! \tparam T Type of vector elements. */				\
  /*! \tparam N Array size. */						\
  template <typename T, size_t N>					\
  cls(const T (&)[N],							\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((compat))) :					\
    base(#code, parent0) UNPACK_MACRO init {					\
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from " #T2 "s.")); \
  }
#define GENERIC_CONSTRUCTOR_VECTOR_FLOAT(cls, base, code, init, props)	\
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(cls, base, code, init, props, COMPATIBLE_WITH_FLOAT(T), float)
#define DUMMY_ARRAY_CONSTRUCTOR(cls, base, code, init)			\
  /*! \brief Raise an error. */						\
  template <typename T>							\
  cls(const std::vector<T> &,						\
      const ObjGroupBase* parent0 = nullptr) : base(#code, parent0) UNPACK_MACRO init { \
    RAPIDJSON_ASSERT(!sizeof(#cls " elements cannot be initialized from a vector")); \
  }									\
  /*! \brief Raise an error. */						\
  template <typename T, size_t N>					\
  cls(const T (&)[N],							\
      const ObjGroupBase* parent0 = nullptr) : base(#code, parent0) UNPACK_MACRO init { \
    RAPIDJSON_ASSERT(!sizeof(#cls " elements cannot be initialized from an array")); \
  }
#define GENERIC_READ_VALUES						\
  /*! \copydoc ObjElement::read_values */				\
  void read_values(std::istream &in, const bool&) OVERRIDE_CXX11 { ObjElement::read_values(in, values); }
#define GENERIC_WRITE_VALUES						\
  /*! \copydoc ObjElement::write_values */				\
  void write_values(std::ostream &out) const OVERRIDE_CXX11 { ObjElement::write_values(out, values); }
#define GENERIC_FROM_VALUES(min, max)					\
  /*! \copydoc ObjElement::from_values() */				\
  void from_values() OVERRIDE_CXX11 {					\
    RAPIDJSON_ASSERT((values.size() >= min) && (values.size() <= max)); \
  }									\
  /*! \copydoc ObjElement::is_valid */					\
  bool is_valid(std::map<std::string,size_t>&) const OVERRIDE_CXX11 {	\
    return (values.size() >= min && values.size() <= max);		\
  }
#define GENERIC_IS_EQUAL(cls)						\
  /*! \copydoc ObjElement::is_equal */					\
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {		\
    if (rhs0->code != this->code) return false;				\
    const cls* lhs = this;						\
    const cls* rhs = dynamic_cast<const cls*>(rhs0);			\
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;	\
    return true;							\
  }
#define GENERIC_CLASS_SCALAR_TYPE_ISEQUAL(cls)				\
  /*! \copydoc ObjElement::is_equal */					\
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {		\
    if (rhs0->code != this->code) return false;				\
    const cls* lhs = this;						\
    const cls* rhs = dynamic_cast<const cls*>(rhs0);			\
    if (lhs->value != rhs->value) return false;				\
    return true;							\
  }
#define GENERIC_CLASS_SCALAR_TYPE_BASE(cls, base, codeS, type, def, props) \
  GENERIC_ELEMENT_CONSTRUCTOR(cls, base, codeS, SINGLE_ARG(, value(def)), props) \
  DUMMY_ARRAY_CONSTRUCTOR(cls, base, code, SINGLE_ARG(, value(def)))	\
  GENERIC_CLASS_SCALAR_TYPE_ISEQUAL(cls)				\
  /*! \brief Copy element specific members from another instance. */	\
  /*! \param[in] rhs Element to copy members from. */			\
  void copy_members(const cls& rhs) {					\
    value = rhs.value;							\
  }									\
  /*! \copydoc ObjElement::read_values */				\
  void read_values(std::istream &in, const bool&) OVERRIDE_CXX11 {	\
    in >> value;							\
  }									\
  /*! \copydoc ObjElement::write_values */				\
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {		\
    out << value;							\
  }									\
  type value;
#define GENERIC_CLASS_SCALAR_TYPE_BODY(cls, base, codeS, type, def, props) \
  GENERIC_CLASS_SCALAR_TYPE_BASE(cls, base, codeS, type, def, props)	\
  /*! \brief Initialize an element from a scalar. */			\
  /*! \param value0 Scalar value. */					\
  /*! \param parent0 Parent group. */					\
  template <typename T>							\
  cls(const T& value0,							\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_TYPE(T, type)))) :		\
    base(#codeS, parent0), value(value0) {}				\
  /*! \brief Initialize an element from a scalar. */			\
  /*! \param value0 Scalar value. */					\
  /*! \param parent0 Parent group. */					\
  template <typename T>							\
  cls(const T&,								\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_TYPE(T, type)))) :		\
    base(#codeS, parent0), value(def) {					\
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from" #type ".")); \
  }
#define GENERIC_CLASS_SCALAR_TYPE(cls, code, type, def, props)		\
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_SCALAR_TYPE_BODY(cls, ObjElement, code, type, def, props) \
  }
#define GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(cls, code, type, compat, props) \
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(cls, ObjElement, code, SINGLE_ARG(, values()), props, UNPACK_MACRO compat, type) \
  GENERIC_READ_VALUES;							\
  GENERIC_WRITE_VALUES;							\
  GENERIC_IS_EQUAL(cls);						\
  /*! \brief Copy element specific members from another instance. */	\
  /*! \param[in] rhs Element to copy members from. */			\
  void copy_members(const cls& rhs) {					\
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end()); \
  }									\
  /*! \copydoc ObjElement::size */					\
  size_t size(bool skipColors=false) const OVERRIDE_CXX11 {		\
    size_t out = 0;							\
    for (PropertiesMap::const_iterator it = properties.begin();		\
	 it != properties.end(); it++) {				\
      if (!this->has_property(it->first, true, skipColors)) continue;	\
      if (it->second & ObjTypeList)					\
	out += values.size();						\
      else								\
	out++;								\
    }									\
    return out;								\
  }									\
  std::vector<type> values;
#define GET_PROPERTY_ARRAY_VECTOR_(valueType, outType, typeFlag)	\
  /*! \copydoc ObjElement::get_property_array */			\
  void get_property_array(const std::string name,			\
			  std::vector<outType>& out,			\
			  bool skipColors=false) const OVERRIDE_CXX11 {	\
    PropertiesMap::const_iterator it = properties.find(name);		\
    if (it == properties.end()) return;					\
    if (typeFlag != ObjTypeNull && !(it->second & typeFlag))		\
      return ObjElement::get_property_array(name, out, skipColors);	\
    if (it->second & ObjTypeList) {					\
      for (std::vector<valueType>::const_iterator v = values.begin(); v != values.end(); v++) \
	out.push_back((outType)(*v));					\
    } else {								\
      size_t i = (size_t)std::abs(std::distance(it, properties.begin())); \
      out.push_back((outType)(values[i]));				\
    }									\
  }
#define GET_PROPERTY_ARRAY_SCALAR_(outType, typeFlag)			\
  /*! \copydoc ObjElement::get_property_array */			\
  void get_property_array(const std::string name,			\
			  std::vector<outType>& out,			\
			  bool skipColors=false) const OVERRIDE_CXX11 {	\
    PropertiesMap::const_iterator it = properties.find(name);		\
    if (it == properties.end()) return;					\
    if (typeFlag != ObjTypeNull && !(it->second & typeFlag))		\
      return ObjElement::get_property_array(name, out, skipColors);	\
    out.push_back((outType)(value));					\
  }
#define GENERIC_CLASS_SCALAR_TYPE_STRING(cls, code, type, def, props)	\
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_SCALAR_TYPE_BODY(cls, ObjElement, code, type, def, props) \
  GET_PROPERTY_ARRAY_SCALAR_(std::string, ObjTypeString)		\
  }
#define GENERIC_CLASS_VECTOR_TYPE_COMPARE(cls, code, type, compat, props) \
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(cls, code, type, compat, props) \
  }
#define GENERIC_CLASS_VECTOR_TYPE_COMPARE_INDEX(cls, code, type, compat, props) \
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(cls, code, type, compat, props) \
  GENERIC_CLASS_VECTOR_TYPE_IS_VALID("vp", type);			\
  GET_PROPERTY_ARRAY_VECTOR_(type, double, ObjTypeNull)			\
  GET_PROPERTY_ARRAY_VECTOR_(type, int, (ObjTypeInt | ObjTypeVertex))	\
  }
#define GENERIC_CLASS_VECTOR_TYPE_STRING(cls, code, type, props)	\
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(cls, code, std::string, SINGLE_ARG(COMPATIBLE_WITH_TYPE(T, std::string)), props) \
  GET_PROPERTY_ARRAY_VECTOR_(std::string, std::string, ObjTypeString);					\
  }
#define GENERIC_CLASS_VECTOR_OBJREFVERTEX(cls, code, min)		\
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(cls, code, ObjRefVertex, SINGLE_ARG(COMPATIBLE_WITH_VERT(T)), SINGLE_ARG(OBJ_P_("vertex_index", ObjTypeInt | ObjTypeList))) \
  GENERIC_CLASS_VECTOR_TYPE_IS_VALID_VERTREF(min)			\
  GET_PROPERTY_ARRAY_VECTOR_(ObjRefVertex, int, (ObjTypeInt | ObjTypeVertex)) \
  /*! \copydoc ObjElement::from_values() */				\
  void from_values() OVERRIDE_CXX11					\
    { RAPIDJSON_ASSERT(values.size() >= min); }				\
  }
#define GENERIC_CLASS_VECTOR_OBJREFCURVE(cls, code)			\
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(cls, code, ObjRefCurve, SINGLE_ARG(COMPATIBLE_WITH_TYPE(T, ObjRefCurve)), SINGLE_ARG(OBJ_P_("curves", ObjTypeCurve | ObjTypeList))) \
  /*! \copydoc ObjElement::is_valid */					\
  bool is_valid(std::map<std::string,size_t>& idx) const OVERRIDE_CXX11 { \
    std::map<std::string,size_t>::iterator x = idx.find("curv2d");	\
    size_t ncurv = 0;							\
    if (x != idx.end()) ncurv = x->second;				\
    for (std::vector<ObjRefCurve>::const_iterator it = values.begin();	\
	 it != values.end(); it++) {					\
      if (!COMPARE_IDX((it->curv2d), ncurv)) return false;		\
    }									\
    return true;							\
  }									\
  }
#define GENERIC_CLASS_VECTOR_OBJREFSURFACE(cls, code)			\
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(cls, code, ObjRefSurface, SINGLE_ARG(COMPATIBLE_WITH_TYPE(T, ObjRefSurface)), SINGLE_ARG(OBJ_P_("surfaces", ObjTypeSurface | ObjTypeList))) \
  /*! \copydoc ObjElement::is_valid */					\
  bool is_valid(std::map<std::string,size_t>& idx) const OVERRIDE_CXX11 { \
    std::map<std::string,size_t>::iterator curv = idx.find("curv2d");	\
    std::map<std::string,size_t>::iterator surf = idx.find("surf");	\
    size_t ncurv = 0, nsurf = 0;					\
    if (curv != idx.end()) ncurv = curv->second;			\
    if (surf != idx.end()) nsurf = surf->second;			\
    for (std::vector<ObjRefSurface>::const_iterator it = values.begin(); \
	 it != values.end(); it++) {					\
      if (!COMPARE_IDX((it->curv2d), ncurv)) return false;		\
      if (!COMPARE_IDX((it->surf), nsurf)) return false;		\
    }									\
    return true;							\
  }									\
  }
#define GENERIC_CLASS_VECTOR_TYPE_IS_VALID(code, type)			\
  /*! \copydoc ObjElement::is_valid */					\
  bool is_valid(std::map<std::string,size_t>& idx) const OVERRIDE_CXX11 { \
    std::map<std::string,size_t>::iterator x = idx.find(code);		\
    size_t nprev = 0;							\
    if (x != idx.end()) nprev = x->second;				\
    for (std::vector<type>::const_iterator it = values.begin();		\
	 it != values.end(); it++) {					\
      if (!COMPARE_IDX((*it), nprev)) return false;			\
    }									\
    return true;							\
  }
#define GENERIC_CLASS_VECTOR_TYPE_IS_VALID_VERTREF(min)			\
  /*! \copydoc ObjElement::is_valid */					\
  bool is_valid(std::map<std::string,size_t>& idx) const OVERRIDE_CXX11 { \
    if (values.size() < min) return false;				\
    std::map<std::string,size_t>::iterator v = idx.find("v");		\
    std::map<std::string,size_t>::iterator vt = idx.find("vt");		\
    std::map<std::string,size_t>::iterator vn = idx.find("vn");		\
    size_t nv = 0, nvt = 0, nvn = 0;					\
    if (v != idx.end()) nv = v->second;					\
    if (vt != idx.end()) nvt = vt->second;				\
    if (vn != idx.end()) nvn = vn->second;				\
    for (std::vector<ObjRefVertex>::const_iterator it = values.begin();	\
	 it != values.end(); it++) {					\
      if (!COMPARE_IDX((it->v), nv)) return false;			\
      if (!COMPARE_IDX((it->vt), nvt)) return false;			\
      if (!COMPARE_IDX((it->vn), nvn)) return false;			\
    }									\
    return true;							\
  }
  
  
//! Test if two vectors are equal element-by-element using is_equal
template <typename T>
inline bool is_equal_vectors(const std::vector<T>& a, const std::vector<T>& b) {
  if (a.size() != b.size()) return false;
  for (typename std::vector<T>::const_iterator ait = a.begin(), bit = b.begin(); ait != a.end(); ait++, bit++)
    if (!internal::values_eq(*ait, *bit)) return false;
  return true;
}

// Forward declaration
class ObjElement;
class ObjVParameter;
class ObjVNormal;
class ObjVTexture;
class ObjPoint;
class ObjLine;
class ObjFace;
class ObjCurve;
class ObjCurve2D;
class ObjSurface;
class ObjFreeFormType;
class ObjDegree;
class ObjBasisMatrix;
class ObjStep;
class ObjParameter;
class ObjTrim;
class ObjHole;
class ObjScrv;
class ObjSpecialPoints;
class ObjConnect;
class ObjGroupBase;
class ObjGroup;
class ObjSmoothingGroup;
class ObjMergingGroup;
class ObjObjectName;
inline ObjElement* read_obj_element(std::istream &in,
				    const ObjGroupBase* parent,
				    const bool& dont_descend=false);

//! ObjWavefront color.
class ObjColor {
public:
  //! Empty initializer with (r,g,b) = (0,0,0)
  ObjColor() :
    r(0), g(0), b(0), is_set(false) {}
  //! \brief Create a RGB color element.
  //! \param red Color index in red.
  //! \param green Color index in green.
  //! \param blue Color index in blue.
  ObjColor(uint8_t red, uint8_t green, uint8_t blue) :
    r(red), g(green), b(blue), is_set(true) {}
  //! Red color index.
  uint8_t r;
  //! Blue color index.
  uint8_t g;
  //! Green color index.
  uint8_t b;
  //! true if the color was set.
  bool is_set;
  //! \brief Check if another ObjColor object is equivalent.
  //! \param rhs Object for comparison.
  bool is_equal(const ObjColor& rhs) const {
    const ObjColor& lhs = *this;
    if (lhs.is_set != rhs.is_set) return false;
    if (lhs.r != rhs.r) return false;
    if (lhs.g != rhs.g) return false;
    if (lhs.b != rhs.b) return false;
    return true;
  }
  friend bool operator == (const ObjColor& lhs, const ObjColor& rhs);
};

//! \brief Check if two ObjColor instances are equivalent.
//! \param lhs First instance for comparison.
//! \param rhs Second instance for comparison.
//! \return true if the two instances are equivalent.
inline
bool operator == (const ObjColor& lhs, const ObjColor& rhs)
{ return lhs.is_equal(rhs); }

//! Object reference index.
typedef int64_t ObjRef;

//! ObjWavefront vertex reference
class ObjRefVertex {
public:
  //! \brief Constructor
  //! \param v0 Index of the vertex's coordinates
  //! \param vt0 Index of the vertex's texcoord
  //! \param vn0 Index of the vertex's normal
  //! \param Nparam0 The number of parameters specified by the vertex. If
  //!    not provided, it will be determined by chcking the values of v0, vt0,
  //!    and vn0. (1: (v), 2: (v, vt), 3: (v, vt, vn)).
  ObjRefVertex(ObjRef v0=0, ObjRef vt0=0, ObjRef vn0=0,
	       int8_t Nparam0=-1) :
    v(v0), vt(vt0), vn(vn0), Nparam(Nparam0) {}
  //! \brief Constructor
  //! \tparam T Vertex index type
  //! \param v0 Index of the vertex's coordinates
  template <typename T>
  ObjRefVertex(const T& v0,
	       RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_INT(T)))) :
    v(v0), vt(0), vn(0), Nparam(1) {}
  //! \brief Write the vertex to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    int8_t Nparam0 = Nparam;
    if (Nparam0 < 0) {
      if (vn != 0)
	Nparam0 = 3;
      else if (vt != 0)
	Nparam0 = 2;
      else
	Nparam0 = 1;
    }
    out << v;
    if (Nparam0 > 1) {
      out << "/";
      if (vt != 0)
	out << vt;
    }
    if (Nparam0 > 2) {
      out << "/";
      if (vn != 0)
	out << vn;
    }
    return out;
  }
  //! \brief Read the vertex from an input stream.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(std::istream &in) {
    std::string word;
    in >> word;
    std::istringstream ss_word(word);
    std::string token;
    // std::istringstream ss_token;
    v = 0;
    vt = 0;
    vn = 0;
    Nparam = 0;
    // v
    RAPIDJSON_ASSERT(std::getline(ss_word, token, '/'));
    {
      std::istringstream ss_token(token);
      ss_token >> v;
    }
    // vt
    if (!std::getline(ss_word, token, '/')) {
      Nparam = 1;
      return in;
    }
    {
      std::istringstream ss_token(token);
      ss_token >> vt;
    }
    // vn
    if (!std::getline(ss_word, token, '/')) {
      Nparam = 2;
      return in;
    }
    {
      std::istringstream ss_token(token);
      ss_token >> vn;
      Nparam = 3;
    }
    return in;
  }
  //! Allow casting to integer.
  operator int() const { return (int)v; }
  //! Index of vertex coordinates.
  ObjRef v;
  //! Index of vertex texcoords.
  ObjRef vt;
  //! Index of vertex normals.
  ObjRef vn;
  //! Number of parameters used in the vertex definition.
  int8_t Nparam;
  //! \brief Check if another vertex is equivalent.
  //! \param rhs Vertex to compare.
  //! \return true if rhs is equivalent.
  bool is_equal(const ObjRefVertex& rhs) const {
    const ObjRefVertex& lhs = *this;
    if (lhs.v != rhs.v) return false;
    if (lhs.vt != rhs.vt) return false;
    if (lhs.vn != rhs.vn) return false;
    return true;
  }
  friend bool operator == (const ObjRefVertex& lhs, const ObjRefVertex& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjRefVertex &p);
  friend std::istream & operator >> (std::istream &in, ObjRefVertex &p);
};

//! \brief Check if two ObjRefVertex instances are equivalent.
//! \param lhs First instance for comparison.
//! \param rhs Second instance for comparison.
//! \return true if the two instances are equivalent.
inline
bool operator == (const ObjRefVertex& lhs, const ObjRefVertex& rhs)
{ return lhs.is_equal(rhs); }

//! Write an ObjRefVertex element to an output stream.
//! \param out Output stream.
//! \param p Element to write.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjRefVertex &p)
{ return p.write(out); }

//! Read an ObjRefVertex element from an input stream.
//! \param in Input stream.
//! \param p Element to read into.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjRefVertex &p)
{ return p.read(in); }


//! ObjWavefront curve reference.
class ObjRefCurve {
public:
  //! \brief Empty constructor.
  ObjRefCurve() :
    u0(0.0), u1(0.0), curv2d(-1) {}
  //! \brief Constructor.
  //! \param u00 Curve parameter starting value.
  //! \param u10 Curve parameter ending value.
  //! \param curv2d0 Index of a 2D curve.
  ObjRefCurve(double u00, double u10=0.0, ObjRef curv2d0=-1) :
    u0(u00), u1(u10), curv2d(curv2d0) {}
  //! \brief Write the curve to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    out << u0 << " " << u1 << " " << curv2d;
    return out;
  }
  //! \brief Read the curve from an input stream.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(std::istream &in) {
    in >> u0;
    in >> u1;
    in >> curv2d;
    return in;
  }
  //! Curve parameter starting value.
  double u0;
  //! Curve parameter ending value.
  double u1;
  //! Index of a 2D curve definition.
  ObjRef curv2d;
  //! \brief Check if another curve is equivalent.
  //! \param rhs Curve to compare.
  //! \return true if rhs is equivalent.
  bool is_equal(const ObjRefCurve& rhs) const {
    const ObjRefCurve& lhs = *this;
    if (!internal::values_eq(lhs.u0, rhs.u0)) return false;
    if (!internal::values_eq(lhs.u1, rhs.u1)) return false;
    if (lhs.curv2d != rhs.curv2d) return false;
    return true;
  }
  friend bool operator == (const ObjRefCurve& lhs, const ObjRefCurve& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjRefCurve &p);
  friend std::istream & operator >> (std::istream &in, ObjRefCurve &p);
};

//! Check if two ObjRefCurve instances are equivalent.
//! \param lhs First element for comparison.
//! \param rhs Second element for comparison.
//! \return true if the two elements are equivalent.
inline
bool operator == (const ObjRefCurve& lhs, const ObjRefCurve& rhs)
{ return lhs.is_equal(rhs); }

//! Write an ObjRefCurve element to an output stream.
//! \param out Output stream.
//! \param p Element.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjRefCurve &p)
{ return p.write(out); }

//! Read an ObjRefCurve element from an input stream.
//! \param in Input stream.
//! \param p Element to read into.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjRefCurve &p)
{ return p.read(in); }


//! ObjWavefront surface reference.
class ObjRefSurface {
public:
  //! \brief Empty constructor.
  ObjRefSurface() :
    surf(-1), q0(0), q1(0), curv2d(-1) {}
  //! \brief Constructor.
  //! \brief surf0 Index of surface definition.
  //! \brief q00 Starting parameter value.
  //! \brief q10 Ending parameter value.
  //! \brief curv2d0 Index of curve definition.
  ObjRefSurface(ObjRef surf0, double q00=0.0, double q10=0.0, ObjRef curv2d0=-1) :
    surf(surf0), q0(q00), q1(q10), curv2d(curv2d0) {}
  //! \brief Write the surface to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    out << surf << " " << q0 << " " << q1 << " " << curv2d;
    return out;
  }
  //! \brief Read the surface from an input stream.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(std::istream &in) {
    in >> surf;
    in >> q0;
    in >> q1;
    in >> curv2d;
    return in;
  }
  //! Index of surface definition.
  ObjRef surf;
  //! Starting parameter value.
  double q0;
  //! Ending parameter value.
  double q1;
  //! Index of surface definition.
  ObjRef curv2d;
  //! \brief Check if another surface is equivalent.
  //! \param rhs Surface to compare.
  //! \return true if rhs is equivalent.
  bool is_equal(const ObjRefSurface& rhs) const {
    const ObjRefSurface& lhs = *this;
    if (lhs.surf != rhs.surf) return false;
    if (!internal::values_eq(lhs.q0, rhs.q0)) return false;
    if (!internal::values_eq(lhs.q1, rhs.q1)) return false;
    if (lhs.curv2d != rhs.curv2d) return false;
    return true;
  }
  friend bool operator == (const ObjRefSurface& lhs, const ObjRefSurface& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjRefSurface &p);
  friend std::istream & operator >> (std::istream &in, ObjRefSurface &p);
};

//! Check if two ObjRefSurface instances are equivalent.
//! \param lhs First element for comparison.
//! \param rhs Second element for comparison.
//! \return true if the two elements are equivalent.
inline
bool operator == (const ObjRefSurface& lhs, const ObjRefSurface& rhs)
{ return lhs.is_equal(rhs); }

//! Write an ObjRefSurface element to an output stream.
//! \param out Output stream.
//! \param p Element to write.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjRefSurface &p)
{ return p.write(out); }

//! Read an ObjRefSurface element from an input stream.
//! \param in Input stream.
//! \param p Element to read into.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjRefSurface &p)
{ return p.read(in); }

//! ObjWavefront element base class.
class ObjElement {
public:
  //! \brief Empty constructor.
  //! \param parent0 The element's parent group.
  ObjElement(const ObjGroupBase* parent0 = nullptr) :
    code(""), parent(parent0), properties() {}
  //! \brief Initialize an element from an element code.
  //! \tparam Number of properties/
  //! \param code0 Element code.
  //! \param parent0 The element's parent group.
  ObjElement(const std::string& code0,
	     const ObjGroupBase* parent0 = nullptr) :
    code(code0), parent(parent0), properties() {}
  //! \brief Copy constructor.
  //! \param rhs Element to copy.
  ObjElement(const ObjElement& rhs) :
    code(rhs.code), parent(rhs.parent), properties(rhs.properties) {}
  //! \brief Initialize and element from a C++ vector of values.
  //! \tparam T Vector element type. Must be an integer or floating point.
  //! \param parent0 The element's parent group.
  template <typename T, size_t N>
  ObjElement(const std::string& code0, const T (&)[N],
	     const ObjGroupBase* parent0 = nullptr) :
    code(code0), parent(parent0), properties() {
    RAPIDJSON_ASSERT(!sizeof(code + " element cannot be constructed from a vector of the provided type."));
  }
  //! \brief Initialize and element from a C++ vector of values.
  //! \tparam T Vector element type. Must be an integer or floating point.
  //! \param parent0 The element's parent group.
  template <typename T>
  ObjElement(const std::string& code0, const std::vector<T> &,
	     const ObjGroupBase* parent0 = nullptr) :
    code(code0), parent(parent0), properties() {
    RAPIDJSON_ASSERT(!sizeof(code + " element cannot be constructed from a vector of the provided type."));
  }
  
  //! \brief Destructor.
  virtual ~ObjElement() {}
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjElement& rhs) {
    RAPIDJSON_ASSERT(code == rhs.code);
    RAPIDJSON_ASSERT(parent == rhs.parent);
  }
  //! \brief Create a copy of the element.
  //! \return Copied element.
  virtual ObjElement* copy() const = 0;
  //! \brief Check if the element is a group of elements.
  //! \return true if the element is a group.
  virtual bool is_group() const { return false; }
  //! \brief Assign values to a vector from a pointer to an array.
  //! \tparam T1 Type of elements in the destination vector.
  //! \tparam T2 Type of elements in the source array.
  //! \param[in, out] dst Vector to assign values to.
  //! \param src Pointer to the source array.
  //! \param N Number of elements in the source array.
  template <typename T1, typename T2>
  void assign_values(std::vector<T1>& dst, const T2* src, const size_t &N)
  { assign_values(dst, std::vector<T2>(src, src+N)); }
  //! \brief Assign values to a vector from stack array.
  //! \tparam T1 Type of elements in the destination vector.
  //! \tparam T2 Type of elements in the source array.
  //! \param[in, out] dst Vector to assign values to.
  //! \param src Source array.
  template <typename T1, typename T2, size_t N>
  void assign_values(std::vector<T1>& dst, const T2 (&src)[N])
  { assign_values(dst, std::vector<T2>(src, src+N)); }
  //! \brief Assign values to a vector from a vector.
  //! \tparam T1 Type of elements in the destination vector.
  //! \tparam T2 Type of elements in the source vector.
  //! \param[in, out] dst Vector to assign values to.
  //! \param src Source vector.
  template <typename T1, typename T2>
  void assign_values(std::vector<T1>& dst, const std::vector<T2> &src,
		     RAPIDJSON_ENABLEIF((
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,ObjRef>,
       COMPATIBLE_WITH_INT(T2)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,double>,
       COMPATIBLE_WITH_FLOAT(T2)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,ObjRefVertex>,
       COMPATIBLE_WITH_VERT(T2)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,std::string>,
       COMPATIBLE_WITH_TYPE(T2, std::string)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,uint16_t>,
       COMPATIBLE_WITH_UINT(T2)>,
       COMPATIBLE_WITH_TYPE(T1, T2)> > > > >))) {
#if RAPIDJSON_HAS_CXX11
    for (auto it = src.begin(); it != src.end(); it++)
      dst.emplace_back((T1)(*it));
#else // RAPIDJSON_HAS_CXX11
    for (typename std::vector<T2>::const_iterator it = src.begin(); it != src.end(); it++)
      dst.push_back((T1)(*it));
#endif // RAPIDJSON_HAS_CXX11
  }
  //! \brief Raise an error if the types are not compatible.
  template <typename T1, typename T2>
  void assign_values(std::vector<T1>&, const std::vector<T2> &,
		     RAPIDJSON_DISABLEIF((
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,ObjRef>,
       COMPATIBLE_WITH_INT(T2)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,double>,
       COMPATIBLE_WITH_FLOAT(T2)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,ObjRefVertex>,
       COMPATIBLE_WITH_VERT(T2)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,std::string>,
       COMPATIBLE_WITH_TYPE(T2, std::string)>,
       internal::OrExpr<internal::AndExpr<internal::IsSame<T1,uint16_t>,
       COMPATIBLE_WITH_UINT(T2)>,
       COMPATIBLE_WITH_TYPE(T1, T2)> > > > >))) {
    ASSERT_COMPATIBLE(T1, T2);
  }
  //! \brief Assign element members from an array of values stored in another
  //!   class member during a previous call to assign_values.
  virtual void from_values() {} // Do nothing, keep values in vector
  //! \brief Read element members from an input stream.
  //! \param in Input stream.
  //! \param dont_descend If true, subelements will not be read from the
  //!   input stream.
  virtual void read_values(std::istream& in, const bool& dont_descend=false) = 0;
  //! \brief Write element member to an output stream.
  //! \param out Output stream.
  virtual void write_values(std::ostream& out) const = 0;
  //! \brief Check if another element is equivalent.
  //! \param rhs0 Element to compare.
  //! \return true if rhs is equivalent.
  virtual bool is_equal(const ObjElement* rhs0) const = 0;
  //! \brief Get element values as an array of strings.
  //! \return Array of string values.
  std::vector<std::string> get_string_array() const {
    std::vector<std::string> out;
    get_string_array(out);
    return out;
  }
  //! \brief Get element values as an array of strings.
  //! \param[out] out Array to put values in.
  void get_string_array(std::vector<std::string>& out) const {
    RAPIDJSON_ASSERT(!requires_double());
    for (PropertiesMap::const_iterator it = properties.begin();
	 it != properties.end(); it++) {
      if (!has_property(it->first, true)) continue;
      get_property_array(it->first, out);
    }
  }
  //! \brief Get element values as an array of ints.
  //! \param nvert Number of vertices previously added to a Ply object
  //!   being constructed from this geometry.
  //! \return Array of int values.
  std::vector<int> get_int_array(const size_t nvert=0) const {
    std::vector<int> out;
    get_int_array(out, nvert);
    return out;
  }
  //! \brief Get element values as an array of ints.
  //! \param nvert Number of vertices previously added to a Ply object
  //!   being constructed from this geometry.
  //! \param[out] out Array to put values in.
  void get_int_array(std::vector<int>& out,
		     const size_t nvert=0) const {
    RAPIDJSON_ASSERT(!requires_double());
    const std::vector<std::string> order = property_order();
    for (std::vector<std::string>::const_iterator it = order.begin();
	 it != order.end(); it++)
      get_property_array(*it, out);
    if (nvert > 0) {
      for (size_t i = 0; i < out.size(); i++) {
	if (out[i] < 0)
	  out[i] = (int)(nvert) + out[i] + 1;
	out[i]--;
      }
    }
  }
  //! \brief Get element values as an array of doubles.
  //! \param skipColors If true, color data will not be included.
  //! \return Array of double values.
  std::vector<double> get_double_array(bool skipColors=false) const {
    std::vector<double> out;
    get_double_array(out, skipColors);
    return out;
  }
  //! \brief Get element values as an array of doubles.
  //! \param skipColors If true, color data will not be included.
  //! \param[out] out Array to put values in.
  void get_double_array(std::vector<double>& out,
			bool skipColors=false) const {
    RAPIDJSON_ASSERT(requires_double());
    const std::vector<std::string> order = property_order(skipColors);
    for (std::vector<std::string>::const_iterator it = order.begin();
	 it != order.end(); it++)
      get_property_array(*it, out, skipColors);
  }
  //! \brief Get element values for one property as an array of the
  //!   requested type.
  //! \param name Property to get values for.
  //! \param[out] out Array to add values to.
  //! \param skipColors If true, color data will not be included.
  virtual void get_property_array(const std::string name,
				  std::vector<double>& out,
				  bool=false) const {
    std::cerr << "get_property_array not implemented for doubles (name = " << name << ")" << std::endl;
    RAPIDJSON_ASSERT(out.size() > 0);
  }
  //! \brief Get element values for one property as an array of the
  //!   requested type.
  //! \param name Property to get values for.
  //! \param[out] out Array to add values to.
  //! \param skipColors If true, color data will not be included.
  virtual void get_property_array(const std::string name,
				  std::vector<int>& out,
				  bool=false) const {
    std::cerr << "get_property_array not implemented for integers (name = " << name << ")" << std::endl;
    RAPIDJSON_ASSERT(out.size() > 0);
  }
  //! \brief Get element values for one property as an array of the
  //!   requested type.
  //! \param name Property to get values for.
  //! \param[out] out Array to add values to.
  //! \param skipColors If true, color data will not be included.
  virtual void get_property_array(const std::string name,
				  std::vector<std::string>& out,
				  bool=false) const {
    std::cerr << "get_property_array not implemented for strings (name = " << name << ")" << std::endl;
    RAPIDJSON_ASSERT(out.size() > 0);
  }
  //! \brief Get the properties associated with this element.
  //! \param skipColors If true, color data will not be included.
  //! \return Property names.
  std::vector<std::string> property_order(bool skipColors=false) const {
    std::vector<std::string> out;
    for (PropertiesMap::const_iterator it = properties.begin();
	 it != properties.end(); it++) {
      if (has_property(it->first, true, skipColors))
	out.push_back(it->first);
    }
    return out;
  }
  //! \brief Determine if a property is set.
  //! \param name Property name.
  //! \param dontCheckOrder If true, it is assumed that the property is in
  //!    the list of possible properties for this element.
  //! \param skipColors If true, color data will not be included.
  virtual bool has_property(const std::string name,
			    bool dontCheckOrder=false,
			    bool skipColors=false) const {
    if (dontCheckOrder)
      return true;
    if (skipColors && (name == "red" ||
		       name == "green" ||
		       name == "blue"))
      return false;
    PropertiesMap::const_iterator it = properties.find(name);
    return (it != properties.end());
  }
  //! \brief Determine if the specified property is a vector.
  //! \param name Property name.
  //! \return true if it is a vector, false otherwise.
  bool is_vector(const std::string name) const {
    PropertiesMap::const_iterator it = properties.find(name);
    return (it != properties.end() && it->second & ObjTypeList);
  }
  //! \brief Determine if any of the elements properties require doubles.
  //! \return true if it requires doubles, false otherwise.
  bool requires_double() const {
    for (PropertiesMap::const_iterator it = properties.begin();
	 it != properties.end(); it++)
      if ((it->second & (ObjTypeFloat | ObjTypeCurve | ObjTypeSurface))
	  && has_property(it->first, true))
	return true;
    return false;
  }
  //! \brief Determine if the specified property requires doubles.
  //! \param name Property to check.
  //! \return true if it requires doubles, false otherwise.
  bool requires_double(const std::string name) const {
    PropertiesMap::const_iterator it = properties.find(name);
    return (it != properties.end() &&
	    it->second & (ObjTypeFloat | ObjTypeCurve | ObjTypeSurface));
  }
  //! \brief Get the number of properties in the element.
  //! \param skipColors If true, the size will not include colors.
  //! \return Number of properties in the element.
  virtual size_t size(bool skipColors=false) const
  { return property_order(skipColors).size(); }
  //! \brief Determine if a structure is valid and there are vertexes for
  //!   all those referenced in faces and edges.
  //! \param idx Map containing the number of preceeding elements of each
  //!   type.
  //! \return true if the structure is valid, false otherwise.
  virtual bool is_valid(std::map<std::string,size_t>& idx) const
  { (void)idx; return true; }
  //! \brief Read elements from an input stream.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(std::istream &in, const bool& dont_descend=false) {
    this->read_values(in, dont_descend);
    return in;
  }
  //! \brief Read element members from an input stream into a vector.
  //! \param in Input stream.
  //! \param values Vector to store read values in.
  template <typename T>
  void read_values(std::istream &in, std::vector<T> &values,
		   RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_TYPE(T, std::string)))) {
    T x = 0;
    while ((in.peek() != '\n') && (in >> x))
      values.push_back(x);
  }
  //! \brief Read element members from an input stream into a vector.
  //! \param in Input stream.
  //! \param values Vector to store read values in.
  void read_values(std::istream &in, std::vector<std::string> &values) {
    std::string x = "";
    while ((in.peek() != '\n') && (in >> x))
      values.push_back(x);
    in >> std::skipws;
  }
  //! \brief Write the element to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    if (code != "")
      out << code << " ";
    this->write_values(out);
    if (code != "g")
      out << std::endl;
    return out;
  }
  //! \brief Write element member to an output stream from a vector.
  //! \param out Output stream.
  //! \param values Values to write.
  template <typename T>
  void write_values(std::ostream &out, const std::vector<T> &values) const {
    for (typename std::vector<T>::const_iterator it = values.begin(); it != values.end(); it++) {
      if (it != values.begin())
	out << " ";
      out << *it;
    }
  }
  //! \brief Get a string representation of the element.
  //! \return String representation.
  std::string as_string() const {
    std::string out;
    std::stringstream ss(out);
    write(ss);
    return out;
  }
  //! \brief Get the colors for an element set in array form.
  //! \param defaultValue Value to add if colors are missing.
  //! \returns The colors for the requirested type in array form.
  std::vector<uint8_t> get_colors_array(uint8_t defaultValue=0) const {
    std::vector<uint8_t> out;
    get_colors_array(out, defaultValue);
    return out;
  }
  //! \brief Get the colors for an element set in array form.
  //! \param out Array to add values to.
  //! \param defaultValue Value to add if colors are missing.
  virtual void get_colors_array(std::vector<uint8_t>&,
				uint8_t=0) const {}
  //! \brief Determine if the element contains color information.
  //! \return true if there are colors, false otherwise.
  virtual bool has_colors() const { return false; }
  //! \brief Add element colors to this element.
  //! \param arr Color property values for this element.
  //! \param N Number of color properties.
  //! \return true if successful, false otherwise.
  virtual bool add_colors(const uint8_t*, SizeType) {
    return false;
  }
  //! Disable copy assignment for elements.
  ObjElement& operator=(const ObjElement& other);
  //! Code indicating the type of element.
  std::string code;
  //! Pointer to the parent element class.
  const ObjGroupBase* parent;
  //! Properties contained by the element.
  PropertiesMap properties;
  
  friend std::ostream & operator << (std::ostream &out, const ObjElement &p);
};

//! Write an element to an output stream.
//! \param out Output stream.
//! \param p Element.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjElement &p)
{ return p.write(out); }

//! Element grouping base class.
class ObjGroupBase : public ObjElement {
public:
  //! \brief Empty constructor.
  //! \param parent0 The element's parent group.
  ObjGroupBase(const ObjGroupBase* parent0 = nullptr) :
    ObjElement(parent0), elements(), finalized(false) {}
  //! \brief Initialize an element group from an element code.
  //! \param code The element code string.
  //! \param parent0 The element's parent group.
  ObjGroupBase(const std::string& code,
	       const ObjGroupBase* parent0 = nullptr) :
    ObjElement(code, parent0), elements(), finalized(false) {}
  //! \brief Initialize an element group from an element code.
  //! \param code The element code string.
  //! \param elements0 Group elements.
  //! \param parent0 The element's parent group.
  ObjGroupBase(const std::string& code,
	       const std::vector<ObjElement*> &elements0,
	       const ObjGroupBase* parent0 = nullptr) :
    ObjElement(code, parent0), elements(), finalized(false) {
    assign_values(elements, elements0);
  }
  GENERIC_CONSTRUCTOR_COPY(ObjGroupBase, ObjElement, SINGLE_ARG(, elements(), finalized(false)));
  ~ObjGroupBase() {
    for (std::vector<ObjElement*>::iterator it = elements.begin(); it != elements.end(); it++)
      delete *it;
    elements.resize(0);
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjGroupBase& rhs) {
    finalized = rhs.finalized;
    for (std::vector<ObjElement*>::const_iterator it = rhs.elements.begin(); it != rhs.elements.end(); it++)
      elements.push_back((*it)->copy());
  }
  //! \brief Check if the element is a group of elements.
  //! \return true if the element is a group.
  bool is_group() const OVERRIDE_CXX11 { return true; }
  //! \brief Finalize the group.
  void finalize() { finalized = true; }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool& dont_descend=false) OVERRIDE_CXX11 {
    read_group_header(in);
    if (!dont_descend) {
      while (!finalized) {
	ObjElement* x = read_obj_element(in, this, true);
	add_element(x);
      }
    }
  }
  //! \brief Read group header information.
  //! \param in Input stream.
  virtual void read_group_header(std::istream &) {}
  //! \brief Write group header information.
  //! \param out Output stream.
  virtual void write_group_header(std::ostream &) const {}
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    if (this->code != "") {
      write_group_header(out);
      out << std::endl;
    }
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++)
      (*it)->write(out);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code)
      return false;
    const ObjGroupBase* lhs = this;
    const ObjGroupBase* rhs = dynamic_cast<const ObjGroupBase*>(rhs0);
    if (lhs->elements.size() != rhs->elements.size()) return false;
    for (std::vector<ObjElement*>::const_iterator lit = lhs->elements.begin(), rit = rhs->elements.begin();
	 lit != lhs->elements.end(); lit++, rit++)
      if (!((*lit)->is_equal(*rit))) return false;
    return true;
  }
  //! \brief Get the number of elements of a certain type.
  //! \param[in] Name of the element type to count.
  //! \return Number of elements of the requested type.
  size_t count_elements(const std::string name) const {
    std::string name2 = obj_alias2base(name);
    size_t out = 0;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code == name2)
	out++;
    }
    return out;
  }
  //! \brief Get a list of unique element types in the group.
  //! \return Unique element types.
  std::vector<std::string> element_types() const {
    std::set<std::string> unique_names;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin();
	 it != elements.end(); it++)
      unique_names.insert((*it)->code);
    std::vector<std::string> out(unique_names.begin(), unique_names.end());
    return out;
  }
  //! \copydoc ObjElement::is_valid
  bool is_valid(std::map<std::string,size_t>& idx) const OVERRIDE_CXX11 {
    for (std::vector<ObjElement*>::const_iterator it = elements.begin();
	 it != elements.end(); it++) {
      if (!(*it)->is_valid(idx)) return false;
      std::map<std::string,size_t>::iterator x = idx.find((*it)->code);
      if (x == idx.end()) {
	idx.insert({(*it)->code, 0});
	x = idx.find((*it)->code);
      }
      x->second++;
    }
    return true;
  }
  //! \brief Determine if a structure is valid and there are vertexes for
  //!   all those referenced in faces and edges.
  //! \return true if the structure is valid, false otherwise.
  bool is_valid() const {
    std::map<std::string,size_t> idx;
    return is_valid(idx);
  }
  //! \brief Append elements from another group to this one.
  //! \param rhs Group to append elements from.
  void append(const ObjGroupBase* rhs) {
    // TODO: Increment indexes
    for (std::vector<ObjElement*>::const_iterator rit = rhs->elements.begin();
	 rit != rhs->elements.end(); rit++)
      add_element(*rit);
  }
  //! \brief Determine if the specified element type requires doubles.
  //! \param name Element type to check.
  //! \return true if it requires doubles, false otherwise.
  bool requires_double(const std::string name) const {
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code != name) continue;
      if ((*it)->is_group()) {
	if ((dynamic_cast<ObjGroupBase*>(*it))->requires_double(name))
	  return true;
      } else {
	if ((*it)->requires_double()) return true;
      }
    }
    return false;
  }
  //! \brief Determine the maximum size of elements of a certain type.
  //! \param name Name of the element set to get the size of.
  //! \param skipColors If true, the parameters containing colors will not be
  //!   included.
  //! \return Maximum element size.
  size_t max_size(const std::string& name, bool skipColors=false) const {
    std::string name2 = obj_alias2base(name);
    size_t maxSize = 0;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code != name2) continue;
      if ((*it)->is_group())
	maxSize = (std::max)(maxSize, (dynamic_cast<ObjGroupBase*>(*it))->max_size(name2, skipColors));
      else
	maxSize = (std::max)(maxSize, (*it)->size(skipColors));
    }
    return maxSize;
  }
  //! \brief Get an element set in an array form.
  //! \param name Name of the element set to get.
  //! \param[out] out The element set of the requested type in array form.
  //! \param minSize Minimum number of values to add for each element.
  //! \param defaultValue Value to pad with for elements with fewer than
  //!   minSize values.
  void get_int_array(const std::string& name,
		     std::vector<int>& out,
		     const size_t minSize,
		     int defaultValue=-1) const {
    std::string name2 = obj_alias2base(name);
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code != name2) continue;
      if ((*it)->is_group()) {
	dynamic_cast<ObjGroupBase*>(*it)->get_int_array(name2, out, minSize,
							defaultValue);
      } else {
	size_t before = out.size();
	(*it)->get_int_array(out);
	for (size_t i = 0; i < (minSize - (out.size() - before)); i++)
	  out.push_back(defaultValue);
      }
    }
  }
  //! \brief Get an element set in an array form.
  //! \param name Name of the element set to get.
  //! \param[out] out The element set of the requested type in array form.
  //! \param minSize Minimum number of values to add for each element.
  //! \param defaultValue Value to pad with for elements with fewer than
  //!   minSize values.
  //! \param skipColors If true, the parameters containing colors will not be
  //!   included.
  void get_double_array(const std::string& name,
			std::vector<double>& out,
			const size_t minSize,
			double defaultValue=NAN,
			bool skipColors=false) const {
    std::string name2 = obj_alias2base(name);
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code != name2) continue;
      if ((*it)->is_group()) {
	dynamic_cast<ObjGroupBase*>(*it)->get_double_array(name2, out, minSize,
							   defaultValue,
							   skipColors);
      } else {
	size_t before = out.size();
	(*it)->get_double_array(out, skipColors);
	for (size_t i = 0; i < (minSize - (out.size() - before)); i++)
	  out.push_back(defaultValue);
      }
    }
  }
  //! Vector of elements in the group.
  std::vector<ObjElement*> elements;
  //! True if the group has been finalized.
  bool finalized;

  // ELEMENT METHODS
  
  //! \brief Find the last element of a given type.
  //! \param code Code of element to find.
  const ObjElement* last_element(const std::string& code0) const {
    for (std::vector<ObjElement*>::const_reverse_iterator it = elements.rbegin(); it != elements.rend(); it++)
      if ((*it)->code == code0)
	return *it;
    return nullptr;
  }
  //! \brief End a group.
  void end_group() {
    ObjElement* last = elements.back();
    if (last->is_group()) {
      ObjGroupBase* last_grp = dynamic_cast<ObjGroupBase*>(last);
      if (!(last_grp->finalized)) {
	last_grp->end_group();
	return;
      }
    }
    // Proper groups only end when a new group is added
    RAPIDJSON_ASSERT(this->code != "g");
    RAPIDJSON_ASSERT(!(this->finalized));
  }
  //! \brief Add an element to the geometry.
  //! \param x New element.
  //! \return New element.
  ObjElement* add_element(ObjElement* x) {
    RAPIDJSON_ASSERT(!finalized);
    if (elements.size() > 0) {
      ObjElement* last = elements.back();
      if (last->is_group()) {
	ObjGroupBase* last_grp = dynamic_cast<ObjGroupBase*>(last);
	if (!(last_grp->finalized)) {
	  if ((x != nullptr) && (last_grp->code == "g") && (x->code == "g"))
	    last_grp->finalize();
	  else
	    return last_grp->add_element(x);
	}
      }
    }
    if (x == nullptr) {
      finalize();
    } else {
      x->parent = this;
      elements.push_back(x);
    }
    return x;
  }
  //! \brief Add an element to the geometry.
  //! \param x New element.
  //! \return New element.
  ObjElement* add_element(const ObjElement& x);
  //! \brief Add an element to the geometry from a C++ vector of values.
  //! \tparam T Type of value in the values vector.
  //! \param name Name of the type of element being added.
  //! \param values Vector of values defining the element.
  //! \param ignore Value to ignore. After this value is encountered for an
  //!   element will be added.
  //! \return New element.
  template <typename T>
  ObjElement* add_element(std::string name, const std::vector<T> &values,
			  const T* ignore = 0);
  //! \brief Add an element to the geometry from a C array of values.
  //! \tparam T Type of value in the values array.
  //! \tparam N Number of elements in the values array.
  //! \param name Name of the type of element being added.
  //! \param values Array of values defining the element.
  //! \param ignore Value to ignore. After this value is encountered for an
  //!   element will be added.
  //! \return New element.
  template <typename T, size_t N>
  ObjElement* add_element(std::string name, const T (&values)[N],
			  const T* ignore = 0,
			  RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_ANY(T)))) {
    return add_element(name, std::vector<T>(values, values+N), ignore);
  }
  //! \brief Add an element to the geometry from a C array of values.
  //! \tparam T Type of value in the values array.
  //! \param name Name of the type of element being added.
  //! \param values Array of values defining the element.
  //! \param N Number of elements in the values array.
  //! \param ignore Value to ignore. After this value is encountered for an
  //!   element will be added.
  //! \return New element.
  template <typename T>
  ObjElement* add_element(std::string name, const T* values, size_t N,
			  const T* ignore = 0,
			  RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_ANY(T)))) {
    return add_element(name, std::vector<T>(values, values+N), ignore);
  }
  // template <size_t N>
  // ObjElement* ObjGroupBase::add_element(std::string name, const char* values[N]) {
  // std::vector<std::string> values_v;
  // for (size_t i = 0; i < N; i++)
  // values_v.push_back(std::string(values[i]));
  // return add_element(name, values_v);
  // };

  // Curve element methods
  //! \brief Add an element to the geometry.
  //! \tparam T Type of value in the values vector.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param values Vector of additional values defining the element.
  //! \return New element.
  template <typename T>
  ObjElement* add_element(const std::string name,
			  const double& u0, const double& u1,
			  const std::vector<T> &values);
  //! \brief Add an element to the geometry from a C array of values.
  //! \tparam T Type of value in the values array.
  //! \tparam N Number of elements in the values array.
  //! \param name Name of the type of element being added.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param values Array of values defining the element.
  //! \return New element.
  template <typename T, size_t N>
  ObjElement* add_element(std::string name,
			  const double& u0, const double& u1,
			  const T (&values)[N]) {
    return add_element(name, u0, u1, std::vector<T>(values, values+N));
  }

  // Surface element methods
  //! \brief Add an element to the geometry.
  //! \tparam T Type of value in the values vector.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param u2 3rd element parameter.
  //! \param u3 4th element parameter.
  //! \param values Vector of additional values defining the element.
  //! \return New element.
  template <typename T>
  ObjElement* add_element(const std::string name,
			  const double& u0, const double& u1,
			  const double& u2, const double& u3,
			  const std::vector<T> &values);
  //! \brief Add an element to the geometry from a C array of values.
  //! \tparam T Type of value in the values array.
  //! \tparam N Number of elements in the values array.
  //! \param name Name of the type of element being added.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param u2 3rd element parameter.
  //! \param u3 4th element parameter.
  //! \param values Array of values defining the element.
  //! \return New element.
  template <typename T, size_t N>
  ObjElement* add_element(std::string name,
			  const double& u0, const double& u1,
			  const double& u2, const double& u3,
			  const T (&values)[N]) {
    return add_element(name, u0, u1, u2, u3, std::vector<T>(values, values+N));
  }

  // Parameter element methods
  //! \brief Add a parameter element to the geometry.
  //! \tparam T Type of value in the values vector.
  //! \param name Name of the type of element being added.
  //! \param direction Name of the parameter direction.
  //! \param values Vector of additional values defining the element.
  //! \return New element.
  template <typename T>
  ObjElement* add_element(std::string name, std::string direction,
			  const std::vector<T> &values);
  //! \brief Add a parameter element to the geometry.
  //! \tparam T Type of value in the values array.
  //! \tparam N Number of elements in the values array.
  //! \param name Name of the type of element being added.
  //! \param direction Name of the parameter direction.
  //! \param values Array of values defining the element. 
  //! \return New element.
  template <typename T, size_t N>
  ObjElement* add_element(std::string name, std::string direction,
			  const T (&values)[N]) {
    return add_element(name, direction, std::vector<T>(values, values+N));
  }

  //! \brief Add a scalar group element.
  //! \param name Name of the type of element being added.
  //! \param value Scalar value.
  //! \return New element.
  ObjElement* add_element(std::string name, const char* value) {
    std::string s(value);
    return add_element(name, s);
  }

  //! \brief Add a scalar group element.
  //! \tparam T Type of value.
  //! \param name Name of the type of element being added.
  //! \param value Scalar value.
  //! \return New element.
  template<typename T>
  RAPIDJSON_DISABLEIF_RETURN((internal::IsPointer<T>), (ObjElement*)) add_element(std::string name, const T& value);

  //! \brief Add a merging group element.
  //! \tparam T Type of value.
  //! \param name Name of the type of element being added.
  //! \param value Scalar value.
  //! \param resolution Merge resolution.
  //! \return New element.
  ObjElement* add_element(std::string name, const int& value, const double& resolution);
  //! \brief Add a merging group element.
  //! \param name Name of the type of element being added.
  //! \param value Scalar value.
  //! \param resolution Merge resolution.
  //! \return New element.
  ObjElement* add_element(std::string name, const std::string& value, const double& resolution);

};

//! Vertex data
class ObjVertex : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVertex, ObjElement, v,
				   SINGLE_ARG(, values(), x(0), y(0), z(0), w(-1), color()),
				   SINGLE_ARG(OBJ_P_("x", ObjTypeFloat), OBJ_P_("y", ObjTypeFloat), OBJ_P_("z", ObjTypeFloat), OBJ_P_("red", ObjTypeInt), OBJ_P_("green", ObjTypeInt), OBJ_P_("blue", ObjTypeInt), OBJ_P_("w", ObjTypeFloat)))
  //! \copydoc ObjElement::has_property
  bool has_property(const std::string name, bool dontCheckOrder=false, bool skipColors=false) const  OVERRIDE_CXX11 {
    return (ObjElement::has_property(name, dontCheckOrder, skipColors) &&
	    !(((skipColors || !color.is_set)
	       && (name == "red" || name == "green" || name == "blue")) ||
	      (w < 0 && name == "w")));
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjVertex& rhs) {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    w = rhs.w;
    color = rhs.color;
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT((values.size() == 3)
		     || (values.size() == 4)
		     || (values.size() == 6)
		     || (values.size() == 7));
    x = values[0];
    y = values[1];
    z = values[2];
    if ((values.size() == 6) || (values.size() == 7)) {
      color.is_set = true;
      color.r = static_cast<uint8_t>(values[3]);
      color.g = static_cast<uint8_t>(values[4]);
      color.b = static_cast<uint8_t>(values[5]);
    }
    if ((values.size() == 4) || (values.size() == 7))
      w = values[values.size() - 1];
    else
      w = -1;
  }
  GENERIC_READ_VALUES;
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << x << " " << y << " " << z;
    if (color.is_set)
      out << " " << (int)color.r << " " << (int)color.g << " " << (int)color.b;
    if (w >= 0)
      out << " " << w;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjVertex* lhs = this;
    const ObjVertex* rhs = dynamic_cast<const ObjVertex*>(rhs0);
    if (!internal::values_eq(lhs->x, rhs->x)) return false;
    if (!internal::values_eq(lhs->y, rhs->y)) return false;
    if (!internal::values_eq(lhs->z, rhs->z)) return false;
    if (!internal::values_eq(lhs->w, rhs->w)) return false;
    return (lhs->color == rhs->color);
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<double>& out,
			  bool skipColors=false) const OVERRIDE_CXX11{
    if (name == "x")
      out.push_back((double)x);
    else if (name == "y")
      out.push_back((double)y);
    else if (name == "z")
      out.push_back((double)z);
    else if (name == "red") {
      if (!skipColors)
	out.push_back((double)color.r);
    } else if (name == "green") {
      if (!skipColors)
	out.push_back((double)color.g);
    } else if (name == "blue") {
      if (!skipColors)
	out.push_back((double)color.b);
    } else if (name == "w")
      out.push_back((double)w);
    else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<int>& out,
			  bool skipColors=false) const OVERRIDE_CXX11{
    if (name == "red") {
      if (!skipColors)
	out.push_back((int)color.r);
    } else if (name == "green") {
      if (!skipColors)
	out.push_back((int)color.g);
    } else if (name == "blue") {
      if (!skipColors)
	out.push_back((int)color.b);
    } else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! \copydoc ObjElement::get_colors_array
  void get_colors_array(std::vector<uint8_t>& out,
			uint8_t defaultValue=0) const OVERRIDE_CXX11 {
    if (color.is_set) {
      out.push_back(color.r);
      out.push_back(color.g);
      out.push_back(color.b);
    } else {
      out.push_back(defaultValue);
      out.push_back(defaultValue);
      out.push_back(defaultValue);
    }
  }
  //! \copydoc ObjElement::has_colors
  bool has_colors() const OVERRIDE_CXX11 {
    return color.is_set;
  }
  //! \brief Add element colors to this element.
  //! \param arr Color property values for this element.
  //! \param N Number of color properties.
  //! \return true if successful, false otherwise.
  bool add_colors(const uint8_t* arr, SizeType N) OVERRIDE_CXX11 {
    if (N == 3) {
      color.is_set = true;
      color.r = arr[0];
      color.g = arr[1];
      color.b = arr[2];
      return true;
    }
    return false;
  }
  //! Vertex values vector.
  std::vector<double> values;
  //! Vertex coordinate in the x direction.
  double x;
  //! Vertex coordinate in the y direction.
  double y;
  //! Vertex coordinate in the z direction.
  double z;
  //! Vertex weight, negative values indicate a default weight of 1.
  double w;
  //! Vertex color.
  ObjColor color;
};

//! Object vertex parameter
class ObjVParameter : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVParameter, ObjElement, vp, SINGLE_ARG(, values(), u(0), v(0), w(-1)), SINGLE_ARG(OBJ_P_("u", ObjTypeFloat), OBJ_P_("v", ObjTypeFloat), OBJ_P_("w", ObjTypeFloat)))
  //! \copydoc ObjElement::has_property
  bool has_property(const std::string name, bool dontCheckOrder=false, bool skipColors=false) const  OVERRIDE_CXX11 {
    return (ObjElement::has_property(name, dontCheckOrder, skipColors) &&
	    (w >= 0 || name != "w"));
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjVParameter& rhs) {
    u = rhs.u;
    v = rhs.v;
    w = rhs.w;
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT((values.size() == 2)
		     || (values.size() == 3));
    u = values[0];
    v = values[1];
    if (values.size() == 3)
      w = values[values.size() - 1];
    else
      w = -1;
  }
  GENERIC_READ_VALUES;
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << u << " " << v;
    if (w >= 0)
      out << " " << w;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjVParameter* lhs = this;
    const ObjVParameter* rhs = dynamic_cast<const ObjVParameter*>(rhs0);
    if (!internal::values_eq(lhs->u, rhs->u)) return false;
    if (!internal::values_eq(lhs->v, rhs->v)) return false;
    if (!internal::values_eq(lhs->w, rhs->w)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<double>& out,
			  bool skipColors=false) const OVERRIDE_CXX11{
    if (name == "u")
      out.push_back((double)u);
    else if (name == "v")
      out.push_back((double)v);
    else if (name == "w")
      out.push_back((double)w);
    else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! Array of values.
  std::vector<double> values;
  //! Parameter value in first dimension.
  double u;
  //! Parameter value in second dimension.
  double v;
  //! Parameter weight, negative values indicate a default weight of 1.
  double w;
};

//! Vertex normal element.
class ObjVNormal : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVNormal, ObjElement, vn, SINGLE_ARG(, values(), i(0), j(0), k(0)), SINGLE_ARG(OBJ_P_("i", ObjTypeFloat), OBJ_P_("j", ObjTypeFloat), OBJ_P_("k", ObjTypeFloat)))
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjVNormal& rhs) {
    i = rhs.i;
    j = rhs.j;
    k = rhs.k;
  }
  //! \copydoc ObjElement::from_values
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT(values.size() == 3);
    i = values[0];
    j = values[1];
    k = values[2];
  }
  GENERIC_READ_VALUES;
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << i << " " << j << " " << k;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjVNormal* lhs = this;
    const ObjVNormal* rhs = dynamic_cast<const ObjVNormal*>(rhs0);
    if (!internal::values_eq(lhs->i, rhs->i)) return false;
    if (!internal::values_eq(lhs->j, rhs->j)) return false;
    if (!internal::values_eq(lhs->k, rhs->k)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<double>& out,
			  bool skipColors=false) const OVERRIDE_CXX11{
    if (name == "i")
      out.push_back((double)i);
    else if (name == "j")
      out.push_back((double)j);
    else if (name == "k")
      out.push_back((double)k);
    else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! Vector of normal values.
  std::vector<double> values;
  //! Normal vector in the x direction.
  double i;
  //! Normal vector in the y direction.
  double j;
  //! Normal vector in the z direction.
  double k;
};

//! Texture vertex element.
class ObjVTexture : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVTexture, ObjElement, vt, SINGLE_ARG(, values(), u(0), v(0), w(0)), SINGLE_ARG(OBJ_P_("u", ObjTypeFloat), OBJ_P_("v", ObjTypeFloat), OBJ_P_("w", ObjTypeFloat)))
  //! \copydoc ObjElement::has_property
  bool has_property(const std::string name, bool dontCheckOrder=false, bool skipColors=false) const  OVERRIDE_CXX11 {
    return (ObjElement::has_property(name, dontCheckOrder, skipColors) &&
	    !((v < 0 && name == "v") || (w < 0 && name == "w")));
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjVTexture& rhs) {
    u = rhs.u;
    v = rhs.v;
    w = rhs.w;
  }
  //! \copydoc ObjElement::from_values
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT((values.size() == 1)
		     || (values.size() == 2)
		     || (values.size() == 3));
    u = values[0];
    v = -1.0;
    w = -1.0;
    if (values.size() >= 2)
      v = values[1];
    if (values.size() == 3)
      w = values[2];
  }
  GENERIC_READ_VALUES;
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << u;
    if (v >= 0)
      out << " " << v;
    if (w >= 0)
      out << " " << w;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjVTexture* lhs = this;
    const ObjVTexture* rhs = dynamic_cast<const ObjVTexture*>(rhs0);
    if (!internal::values_eq(lhs->u, rhs->u)) return false;
    if (!internal::values_eq(lhs->v, rhs->v)) return false;
    if (!internal::values_eq(lhs->w, rhs->w)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<double>& out,
			  bool skipColors=false) const OVERRIDE_CXX11{
    if (name == "u")
      out.push_back((double)u);
    else if (name == "v")
      out.push_back((double)v);
    else if (name == "w")
      out.push_back((double)w);
    else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! Vector of texture values.
  std::vector<double> values;
  //! Texture coordinate in the horizontal direction.
  double u;
  //! Texture coordinate in the vertical direction; a negative value indicates a default of 0.
  double v;
  //! Texture coordinate in the depth direction; a negative value indicates a default of 0.
  double w;
};

// Elements

//! Point element.
class ObjPoint : public ObjElement {
public:
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(ObjPoint, p, ObjRef, SINGLE_ARG(COMPATIBLE_WITH_INT(T)), SINGLE_ARG(OBJ_P_("vertex_index", ObjTypeInt | ObjTypeList)))
  GENERIC_CLASS_VECTOR_TYPE_IS_VALID("v", ObjRef);
};

//! Line element.
GENERIC_CLASS_VECTOR_OBJREFVERTEX(ObjLine, l, 2);

//! Face element.
GENERIC_CLASS_VECTOR_OBJREFVERTEX(ObjFace, f, 3);

//! Free-form element group.
class ObjFreeFormElement : public ObjGroupBase {
public:
  //! \brief Empty constructor.
  //! \param parent0 The element's parent group.
  ObjFreeFormElement(const ObjGroupBase* parent0 = nullptr) :
    ObjGroupBase(parent0) { RAPIDJSON_ASSERT(parent0); }
  //! \brief Initialize an element from an element code.
  //! \param code0 Element code.
  //! \param parent0 The element's parent group.
  ObjFreeFormElement(const std::string& code0,
		     const ObjGroupBase* parent0 = nullptr) :
    ObjGroupBase(code0, parent0) { RAPIDJSON_ASSERT(parent0); }
  //! \copydoc ObjElement::ObjElement(const ObjElement&)
  ObjFreeFormElement(const ObjFreeFormElement& rhs) : ObjGroupBase(rhs) {}
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    ObjGroupBase::write_values(out);
    out << "end";
  }
};

//! Curve element.
class ObjCurve : public ObjFreeFormElement {
public:
  GENERIC_ELEMENT_CONSTRUCTOR(ObjCurve, ObjFreeFormElement, curv, SINGLE_ARG(, values(), u0(0), u1(0)), SINGLE_ARG(OBJ_P_("u0", ObjTypeFloat), OBJ_P_("u1", ObjTypeFloat), OBJ_P_("vertex_index", ObjTypeInt | ObjTypeList)))
  DUMMY_ARRAY_CONSTRUCTOR(ObjCurve, ObjFreeFormElement, curv, SINGLE_ARG(, values(), u0(0), u1(0)))
  GENERIC_CLASS_VECTOR_TYPE_IS_VALID("v", ObjRef)
  //! \brief Initialize and element from a C++ vector of values.
  //! \param u00 Starting curve parameter value.
  //! \param u10 Ending curve parameter value.
  //! \param values0 Vector of values.
  //! \param parent0 The element's parent group.
  //! \tparam T Vector element type. Must be an integer or floating point.
  //!   Only integer values are allowed for ObjPoint elements.
  template <typename T>
  ObjCurve(const double& u00, const double& u10,
	   const std::vector<T> &values0,
	   const ObjGroupBase* parent0 = nullptr,
	   RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_INT(T)))) :
    ObjFreeFormElement("curv", parent0), values(), u0(u00), u1(u10) {
    assign_values(values, values0);
    from_values();
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjCurve& rhs) {
    u0 = rhs.u0;
    u1 = rhs.u1;
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT(values.size() >= 2);
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool& dont_descend=false) OVERRIDE_CXX11 {
    in >> u0;
    in >> u1;
    ObjElement::read_values(in, values);
    ObjFreeFormElement::read_values(in, dont_descend);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << u0 << " " << u1 << " ";
    ObjElement::write_values(out, values);
    ObjFreeFormElement::write_values(out);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (!(ObjFreeFormElement::is_equal(rhs0))) return false;
    const ObjCurve* lhs = this;
    const ObjCurve* rhs = dynamic_cast<const ObjCurve*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (!internal::values_eq(lhs->u0, rhs->u0)) return false;
    if (!internal::values_eq(lhs->u1, rhs->u1)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<double>& out,
			  bool skipColors=false) const OVERRIDE_CXX11 {
    if (name == "u0")
      out.push_back((double)u0);
    else if (name == "u1")
      out.push_back((double)u1);
    else if (name == "vertex_index") {
      for (std::vector<ObjRef>::const_iterator it = values.begin(); it != values.end(); it++)
	out.push_back((double)(*it));
    } else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<int>& out,
			  bool skipColors=false) const OVERRIDE_CXX11 {
    if (name == "vertex_index") {
      for (std::vector<ObjRef>::const_iterator it = values.begin(); it != values.end(); it++)
	out.push_back((int)(*it));
    } else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! \copydoc ObjElement::size
  size_t size(bool=false) const OVERRIDE_CXX11 { return values.size() + 2; }
  //! Vector for curve values.
  std::vector<ObjRef> values;
  //! Curve value in first parameter direction.
  double u0;
  //! Curve value in second parameter direction.
  double u1;
};

//! 2D curve element.
class ObjCurve2D : public ObjFreeFormElement {
public:
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(ObjCurve2D, ObjFreeFormElement, curv2, SINGLE_ARG(, values()), SINGLE_ARG(OBJ_P_("parameter_index", ObjTypeInt | ObjTypeList)), COMPATIBLE_WITH_INT(T), int)
  GENERIC_CLASS_VECTOR_TYPE_IS_VALID("vp", ObjRef)
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjCurve2D& rhs) {
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool& dont_descend=false) OVERRIDE_CXX11 {
    ObjElement::read_values(in, values);
    ObjFreeFormElement::read_values(in, dont_descend);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    ObjElement::write_values(out, values);
    ObjFreeFormElement::write_values(out);
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT(values.size() >= 2);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (!(ObjFreeFormElement::is_equal(rhs0))) return false;
    const ObjCurve2D* lhs = this;
    const ObjCurve2D* rhs = dynamic_cast<const ObjCurve2D*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! Vector of object reference values.
  std::vector<ObjRef> values;
};

//! Surface element.
class ObjSurface : public ObjFreeFormElement {
public:
  GENERIC_ELEMENT_CONSTRUCTOR(ObjSurface, ObjFreeFormElement, surf, SINGLE_ARG(, values(), s0(0), s1(0), t0(0), t1(0)), SINGLE_ARG(OBJ_P_("s0", ObjTypeFloat), OBJ_P_("s1", ObjTypeFloat), OBJ_P_("t0", ObjTypeFloat), OBJ_P_("t1", ObjTypeFloat), OBJ_P_("vertex_index", ObjTypeInt | ObjTypeList)))
  GENERIC_CLASS_VECTOR_TYPE_IS_VALID_VERTREF(1);
  //! \brief Initialize and element from a C++ vector of values.
  //! \tparam T Vector element type. Must be an integer.
  //! \param s00 Starting curve parameter value in 1st dimension.
  //! \param s10 Ending curve parameter value in 1st dimension.
  //! \param t00 Starting curve parameter value in 2nd dimension.
  //! \param t10 Ending curve parameter value in 2nd dimension.
  //! \param values0 Vector of values.
  //! \param parent0 The element's parent group.
  template <typename T>
  ObjSurface(const double& s00, const double& s10,
	     const double& t00, const double& t10,
	     const std::vector<T> &values0,
	     const ObjGroupBase* parent0 = nullptr,
	     RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_VERT(T)))) :
    ObjFreeFormElement("surf", parent0), values(), s0(s00), s1(s10), t0(t00), t1(t10) {
    assign_values(values, values0);
    from_values();
  }
  DUMMY_ARRAY_CONSTRUCTOR(ObjSurface, ObjFreeFormElement, surf, SINGLE_ARG(, values(), s0(0), s1(0), t0(0), t1(0)))
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjSurface& rhs) {
    s0 = rhs.s0;
    s1 = rhs.s1;
    t0 = rhs.t0;
    t1 = rhs.t1;
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool& dont_descend=false) OVERRIDE_CXX11 {
    in >> s0;
    in >> s1;
    in >> t0;
    in >> t1;
    ObjElement::read_values(in, values);
    ObjFreeFormElement::read_values(in, dont_descend);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << s0 << " " << s1 << " " << t0 << " " << t1 << " ";
    ObjElement::write_values(out, values);
    ObjFreeFormElement::write_values(out);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (!(ObjFreeFormElement::is_equal(rhs0))) return false;
    const ObjSurface* lhs = this;
    const ObjSurface* rhs = dynamic_cast<const ObjSurface*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (!internal::values_eq(lhs->s0, rhs->s0)) return false;
    if (!internal::values_eq(lhs->s1, rhs->s1)) return false;
    if (!internal::values_eq(lhs->t0, rhs->t0)) return false;
    if (!internal::values_eq(lhs->t1, rhs->t1)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<double>& out,
			  bool skipColors=false) const OVERRIDE_CXX11 {
    if (name == "s0")
      out.push_back((double)s0);
    else if (name == "s1")
      out.push_back((double)s1);
    else if (name == "t0")
      out.push_back((double)t0);
    else if (name == "t1")
      out.push_back((double)t1);
    else if (name == "vertex_index") {
      for (std::vector<ObjRefVertex>::const_iterator it = values.begin(); it != values.end(); it++)
	out.push_back((double)(*it));
    } else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! \copydoc ObjElement::get_property_array
  void get_property_array(const std::string name,
			  std::vector<int>& out,
			  bool skipColors=false) const OVERRIDE_CXX11 {
    if (name == "vertex_index") {
      for (std::vector<ObjRefVertex>::const_iterator it = values.begin(); it != values.end(); it++)
	out.push_back((int)(*it));
    } else
      ObjElement::get_property_array(name, out, skipColors);
  }
  //! \copydoc ObjElement::size
  size_t size(bool=false) const OVERRIDE_CXX11 { return values.size() + 4; }
  //! Vector of surface values.
  std::vector<ObjRefVertex> values;
  //! Surface starting parameter in first dimension.
  double s0;
  //! Surface ending parameter in first dimension.
  double s1;
  //! Surface starting parameter in second dimension.
  double t0;
  //! Surface ending parameter in second dimension.
  double t1;
};

#undef FREEFORM_CONSTRUCTOR_EMPTY
#undef FREEFORM_CONSTRUCTOR_COPY

//! Free-form element group.
class ObjFreeFormType : public ObjElement {
public:
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(ObjFreeFormType, cstype, std::string, SINGLE_ARG(COMPATIBLE_WITH_TYPE(T, std::string)), SINGLE_ARG(OBJ_P_("rat", ObjTypeString), OBJ_P_("type", ObjTypeString)))
  GET_PROPERTY_ARRAY_VECTOR_(std::string, std::string, ObjTypeString);
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT((values.size() == 1)
		     || (values.size() == 2));
    if (!(values.size() == 1 || values.size() == 2)) return;
    std::string ival = values[0];
    if (ival == "rat") {
      RAPIDJSON_ASSERT(values.size() == 2);
      if (values.size() == 2)
	ival = values[1];
    }
    RAPIDJSON_ASSERT(((ival == "bmatrix" ) || // basis matrix
		      (ival == "bezier"  ) || // Bezier
		      (ival == "bspline" ) || // B-spline
		      (ival == "cardinal") || // Cardinal
		      (ival == "taylor"  ))); // Taylor
  }
  //! \copydoc ObjElement::is_valid
  bool is_valid(std::map<std::string,size_t>&) const OVERRIDE_CXX11 {
    if (!(values.size() == 1 || values.size() == 2)) return false;
    std::string ival = values[0];
    if (ival == "rat" && values.size() == 2)
      ival = values[1];
    return ((ival == "bmatrix" ) || // basis matrix
	    (ival == "bezier"  ) || // Bezier
	    (ival == "bspline" ) || // B-spline
	    (ival == "cardinal") || // Cardinal
	    (ival == "taylor"  )); // Taylor    
  }
};

//! Degree element.
class ObjDegree : public ObjElement {
public:
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(ObjDegree, deg, uint16_t, SINGLE_ARG(COMPATIBLE_WITH_UINT(T)), SINGLE_ARG(OBJ_P_("degu", ObjTypeInt), OBJ_P_("degv", ObjTypeInt)))
  GENERIC_FROM_VALUES(1, 2)
};


//! Basis matrix element.
class ObjBasisMatrix : public ObjElement {
public:
  GENERIC_ELEMENT_CONSTRUCTOR(ObjBasisMatrix, ObjElement, bmat, SINGLE_ARG(, values(), direction("")), SINGLE_ARG(OBJ_P_("direction", ObjTypeString), OBJ_P_("matrix", ObjTypeFloat)))
  //! \brief Initialize an element from a C array of values.
  //! \tparam T Array element type.
  //! \tparam N Array size.
  //! \param direction0 Basis direction.
  //! \param src Array of values.
  //! \param parent0 The element's parent group.
  template <typename T, size_t N>
  ObjBasisMatrix(const std::string& direction0, const T (&src)[N],
		 const ObjGroupBase* parent0 = nullptr) :
    ObjElement("bmat", parent0), values(), direction(direction0) {
    std::vector<T> values0(src, src+N);
    assign_values(values, values0);
    from_values();
  }
  //! \brief Initialize and element from a C++ vector of values.
  //! \param direction0 Basis direction.
  //! \param values0 Vector of values.
  //! \param parent0 The element's parent group.
  ObjBasisMatrix(const std::string& direction0, const std::vector<double> &values0,
		 const ObjGroupBase* parent0 = nullptr) :
    ObjElement("bmat", parent0), values(), direction(direction0) {
    assign_values(values, values0);
    from_values();
  }
  DUMMY_ARRAY_CONSTRUCTOR(ObjBasisMatrix, ObjElement, bmat, SINGLE_ARG(, values(), direction("")))
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjBasisMatrix& rhs) {
    direction = rhs.direction;
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT((direction == "u") || (direction == "v"));
    RAPIDJSON_ASSERT(this->parent);
    const ObjElement* deg0 = this->parent->last_element("deg");
    RAPIDJSON_ASSERT(deg0);
    const ObjDegree* deg = dynamic_cast<const ObjDegree*>(deg0);
    uint16_t n;
    if (direction == "u")
      n = deg->values[0];
    else {
      RAPIDJSON_ASSERT(deg->values.size() == 2);
      n = deg->values[1];
    }
    (void)n;
    RAPIDJSON_ASSERT(values.size() == ((n + 1)*(n + 1)));
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool&) OVERRIDE_CXX11 {
    in >> direction;
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << direction << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjBasisMatrix* lhs = this;
    const ObjBasisMatrix* rhs = dynamic_cast<const ObjBasisMatrix*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->direction != rhs->direction) return false;
    return true;
  }
  //! \copydoc ObjElement::is_valid
  bool is_valid(std::map<std::string,size_t>&) const OVERRIDE_CXX11 {
    if (!this->parent) return false;
    const ObjElement* deg0 = this->parent->last_element("deg");
    if (!deg0) return false;
    const ObjDegree* deg = dynamic_cast<const ObjDegree*>(deg0);
    uint16_t n = 0;
    if (direction == "u")
      n = deg->values[0];
    else if (direction == "v") {
      if (deg->values.size() != 2) return false;
      n = deg->values[1];
    } else {
      return false;
    }
    return (values.size() == ((n + 1)*(n + 1)));
  }
  //! Vector of matrix values.
  std::vector<double> values;
  //! Basis matrix direction.
  std::string direction;
};

//! Step element.
class ObjStep : public ObjElement {
public:
  GENERIC_CLASS_VECTOR_TYPE_BODY_COMPARE(ObjStep, step, double, SINGLE_ARG(COMPATIBLE_WITH_FLOAT(T)), SINGLE_ARG(OBJ_P_("stepu", ObjTypeFloat), OBJ_P_("stepv", ObjTypeFloat)))
  GENERIC_FROM_VALUES(1, 2)
};

//! Parameter element.
class ObjParameter : public ObjElement {
public:
  GENERIC_ELEMENT_CONSTRUCTOR(ObjParameter, ObjElement, parm, SINGLE_ARG(, values(), direction("")), SINGLE_ARG(OBJ_P_("direction", ObjTypeString), OBJ_P_("parameter", ObjTypeFloat)))
  DUMMY_ARRAY_CONSTRUCTOR(ObjParameter, ObjElement, parm, SINGLE_ARG(, values(), direction("")))
  //! \brief Initialize and element from a C++ vector of values.
  //! \tparam T Vector element type. Must be catable to double.
  //! \param direction0 Parameter direction.
  //! \param values0 Vector of values.
  //! \param parent0 The element's parent group.
  template<typename T>
  ObjParameter(const std::string& direction0, const std::vector<T> &values0,
	       const ObjGroupBase* parent0 = nullptr,
	       RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_FLOAT(T)))) :
    ObjElement("parm", parent0), values(), direction(direction0) {
    assign_values(values, values0);
    from_values();
  }
  //! \brief Initialize and element from a C++ vector of values.
  //! \tparam T Vector element type. Must be an integer or floating point.
  //! \param direction0 Parameter direction.
  //! \param parent0 The element's parent group.
  template <typename T>
  ObjParameter(const std::string&, const std::vector<T> &,
	       const ObjGroupBase* parent0 = nullptr,
	       RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_FLOAT(T)))) :
    ObjElement("parm", parent0), values(), direction("") {
    RAPIDJSON_ASSERT(sizeof("ObjParameter type is double"));
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjParameter& rhs) {
    direction = rhs.direction;
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT((direction == "u") || (direction == "v"));
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool&) OVERRIDE_CXX11 {
    in >> direction;
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << direction << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjParameter* lhs = this;
    const ObjParameter* rhs = dynamic_cast<const ObjParameter*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->direction != rhs->direction) return false;
    return true;
  }
  //! \copydoc ObjElement::is_valid
  bool is_valid(std::map<std::string,size_t>&) const OVERRIDE_CXX11 {
    return ((direction == "u") || (direction == "v"));
  }
  //! Vector of parameter values.
  std::vector<double> values;
  //! Parameter direction.
  std::string direction;
};

//! Trim element.
GENERIC_CLASS_VECTOR_OBJREFCURVE(ObjTrim, trim);

//! Hole element.
GENERIC_CLASS_VECTOR_OBJREFCURVE(ObjHole, hole);

//! Special curve element.
GENERIC_CLASS_VECTOR_OBJREFCURVE(ObjScrv, scrv);

//! Special points element.
GENERIC_CLASS_VECTOR_TYPE_COMPARE_INDEX(ObjSpecialPoints, sp, ObjRef, SINGLE_ARG(COMPATIBLE_WITH_INT(T)), SINGLE_ARG(OBJ_P_("param_index", ObjTypeInt | ObjTypeList)));

//! Connection element.
GENERIC_CLASS_VECTOR_OBJREFSURFACE(ObjConnect, con);

//! Group of elements.
class ObjGroup : public ObjGroupBase {
public:
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(ObjGroup, ObjGroupBase, g, SINGLE_ARG(, values()), SINGLE_ARG(OBJ_P_("labels", ObjTypeString | ObjTypeList)), COMPATIBLE_WITH_TYPE(T, std::string), std::string)
  GET_PROPERTY_ARRAY_VECTOR_(std::string, std::string, ObjTypeString);
  //! \brief Initialize and element from a scalar.
  //! \tparam T Type of value.
  //! \param value Scalar value.
  //! \param parent0 The element's parent group.
  template<typename T>
  ObjGroup(const T &value,
	   const ObjGroupBase* parent0 = nullptr,
	   RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_TYPE(T, std::string)))) :
    ObjGroupBase("g", parent0), values() {
    std::vector<T> values0;
    values0.push_back(value);
    assign_values(values, values0);
    from_values();
  }
  //! \brief Raise an error for incompatible types.
  //! \tparam T Type of value.
  //! \param parent0 The element's parent group.
  template<typename T>
  ObjGroup(const T &,
	   const ObjGroupBase* parent0 = nullptr,
	   RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_TYPE(T, std::string)))) :
    ObjGroupBase("g", parent0), values() {
    ASSERT_COMPATIBLE(T, std::string);
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjGroup& rhs) {
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjGroupBase::read_group_header
  void read_group_header(std::istream &in) OVERRIDE_CXX11 {
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjGroupBase::write_group_header
  void write_group_header(std::ostream &out) const OVERRIDE_CXX11 {
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (!ObjGroupBase::is_equal(rhs0))
      return false;
    const ObjGroup* lhs = this;
    const ObjGroup* rhs = dynamic_cast<const ObjGroup*>(rhs0);
    std::string lhs_str = "";
    std::string rhs_str = "";
    for (std::vector<std::string>::const_iterator it = lhs->values.begin(); it != lhs->values.end(); it++) {
      if (it != lhs->values.begin())
	lhs_str.append(" ");
      lhs_str.append(*it);
    }
    for (std::vector<std::string>::const_iterator it = rhs->values.begin(); it != rhs->values.end(); it++) {
      if (it != rhs->values.begin())
	rhs_str.append(" ");
      rhs_str.append(*it);
    }
    if (lhs_str != rhs_str)
      return false;
    return true;
  }
  //! Vector of element values in the group.
  std::vector<std::string> values;
};

//! Smoothing group element.
class ObjSmoothingGroup : public ObjElement {
public:
  GENERIC_CLASS_SCALAR_TYPE_BASE(ObjSmoothingGroup, ObjElement, s, std::string, "off", SINGLE_ARG(OBJ_P_("state", ObjTypeString)))
  GET_PROPERTY_ARRAY_SCALAR_(std::string, ObjTypeString)
  //! \brief Initialize the smoothing group from a string.
  //! \param value0 Scalar value.
  //! \param parent0 Parent group.
  template<typename T>
  ObjSmoothingGroup(const T& value0, const ObjGroupBase* parent0 = nullptr,
		    RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_TYPE(T, std::string)))) :
    ObjElement("s", parent0), value(value0) {}
  //! \brief Initialize the smoothing group from an integer.
  //! \param value0 Scalar value.
  //! \param parent0 Parent group.
  template<typename T>
  ObjSmoothingGroup(const T& value0, const ObjGroupBase* parent0 = nullptr,
		    RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_INT(T)))) :
    ObjElement("s", parent0), value(std::to_string(value0)) {}
  //! \brief Raise an error for non string or integer types.
  template<typename T>
  ObjSmoothingGroup(const T&, const ObjGroupBase* parent0 = nullptr,
		    RAPIDJSON_DISABLEIF((internal::OrExpr<
					 COMPATIBLE_WITH_INT(T),
					 COMPATIBLE_WITH_TYPE(T, std::string)>))) :
    ObjElement("s", parent0), value("off") {
    RAPIDJSON_ASSERT(!sizeof("ObjSmoothingGroup must be initialized from a string or integer."));
  }
};

//! Merging group.
class ObjMergingGroup : public ObjElement {
public:
  GENERIC_ELEMENT_CONSTRUCTOR(ObjMergingGroup, ObjElement, mg, SINGLE_ARG(, value("off"), resolution(0)), SINGLE_ARG(OBJ_P_("state", ObjTypeString), OBJ_P_("resolution", ObjTypeFloat)))
  DUMMY_ARRAY_CONSTRUCTOR(ObjMergingGroup, ObjElement, mg, SINGLE_ARG(, value("off"), resolution(0)))
  //! \brief Initialize the smoothing group from a string.
  //! \param value0 Scalar value.
  //! \param parent0 Parent group.
  template <typename T>
  ObjMergingGroup(const T& value0, const double& resolution0,
		  const ObjGroupBase* parent0 = nullptr,
		  RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_TYPE(T, std::string)))) :
    ObjElement("mg", parent0), value(value0), resolution(resolution0) {}
  //! \brief Initialize the smoothing group from an integer.
  //! \param value0 Scalar value.
  //! \param parent0 Parent group.
  template <typename T>
  ObjMergingGroup(const T& value0, const double& resolution0,
		  const ObjGroupBase* parent0 = nullptr,
		  RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_INT(T)))) :
    ObjElement("mg", parent0), value(std::to_string(value0)), resolution(resolution0) {}
  //! \brief Raise an error for non string or integer types.
  template <typename T>
  ObjMergingGroup(const T&, const double&,
		  const ObjGroupBase* parent0 = nullptr,
		  RAPIDJSON_DISABLEIF((internal::OrExpr<
				       COMPATIBLE_WITH_INT(T),
				       COMPATIBLE_WITH_TYPE(T, std::string)>))) :
    ObjElement("mg", parent0), value("off"), resolution(0) {
    RAPIDJSON_ASSERT(!sizeof("ObjMergingGroup must be initialized from a string or integer."));
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjMergingGroup& rhs) {
    value = rhs.value;
    resolution = rhs.resolution;
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool&) OVERRIDE_CXX11 {
    in >> value;
    if (value == "off")
      resolution = 0.0;
    else
      in >> resolution;
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << value;
    if (value != "off")
      out << " " << resolution;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (this->code != rhs0->code) return false;
    const ObjMergingGroup* lhs = this;
    const ObjMergingGroup* rhs = dynamic_cast<const ObjMergingGroup*>(rhs0);
    if (lhs->value != rhs->value) return false;
    if (!(internal::values_eq(lhs->resolution, rhs->resolution))) return false;
    return true;
  }
  //! Merging group.
  std::string value;
  //! Group resolution.
  double resolution;
};

//! Object name element.
GENERIC_CLASS_SCALAR_TYPE_STRING(ObjObjectName, o, std::string, "", SINGLE_ARG(OBJ_P_("value", ObjTypeString)));

// Display/render attributes

#define BOOL_ELEMENT_CLASS(cls, code)					\
  class cls : public ObjElement {					\
  public:								\
  GENERIC_CLASS_SCALAR_TYPE_BODY(cls, ObjElement, code, std::string, "off", SINGLE_ARG(OBJ_P_("state", ObjTypeString))) \
  GET_PROPERTY_ARRAY_SCALAR_(std::string, ObjTypeString)		\
  /*! \copydoc ObjElement::from_values() */				\
  void from_values() OVERRIDE_CXX11 {					\
    RAPIDJSON_ASSERT(((value == "on") || (value == "off")));		\
  }									\
  /*! \copydoc ObjElement::is_valid */					\
  bool is_valid(std::map<std::string,size_t>&) const OVERRIDE_CXX11 {	\
    return ((value == "on") || (value == "off"));			\
  }									\
  }

//! Bevel interpolation.
BOOL_ELEMENT_CLASS(ObjBevel, bevel);

//! Color interpolation.
BOOL_ELEMENT_CLASS(ObjCInterp, c_interp);

//! Dissolve interpolation.
BOOL_ELEMENT_CLASS(ObjDInterp, d_interp);

#undef BOOL_ELEMENT_CLASS

//! Level of detail.
GENERIC_CLASS_SCALAR_TYPE(ObjLOD, lod, int, 0, SINGLE_ARG(OBJ_P_("value", ObjTypeInt)));

//! Map library file.
GENERIC_CLASS_VECTOR_TYPE_STRING(ObjTextureMapLib, maplib, std::string, SINGLE_ARG(OBJ_P_("value", ObjTypeString | ObjTypeList)));

//! Texture map.
GENERIC_CLASS_SCALAR_TYPE_STRING(ObjTextureMap, usemap, std::string, "off", SINGLE_ARG(OBJ_P_("value", ObjTypeString)));

//! Material name.
GENERIC_CLASS_SCALAR_TYPE_STRING(ObjMaterial, usemtl, std::string, "", SINGLE_ARG(OBJ_P_("value", ObjTypeString)));

//! Matrial library file.
GENERIC_CLASS_VECTOR_TYPE_STRING(ObjMaterialLib, mtllib, std::string, SINGLE_ARG(OBJ_P_("value", ObjTypeString | ObjTypeList)));

//! Shadow object file.
GENERIC_CLASS_SCALAR_TYPE_STRING(ObjShadowFile, shadow_obj, std::string, "", SINGLE_ARG(OBJ_P_("value", ObjTypeString)));

//! Ray tracing object file.
GENERIC_CLASS_SCALAR_TYPE_STRING(ObjTraceFile, trace_obj, std::string, "", SINGLE_ARG(OBJ_P_("value", ObjTypeString)));

//! Curve technique resolution.
class ObjCTech : public ObjElement {
public:
  GENERIC_ELEMENT_CONSTRUCTOR(ObjCTech, ObjElement, ctech, SINGLE_ARG(, technique(""), values()), SINGLE_ARG(OBJ_P_("technique", ObjTypeString), OBJ_P_("resolution", ObjTypeFloat), OBJ_P_("maxlength", ObjTypeFloat), OBJ_P_("maxdist", ObjTypeFloat), OBJ_P_("maxangle", ObjTypeFloat)))
  DUMMY_ARRAY_CONSTRUCTOR(ObjCTech, ObjElement, ctech, SINGLE_ARG(, technique(""), values()))
  template<typename T>
  ObjCTech(const std::string& technique0,
	   const std::vector<T> &values0,
	   const ObjGroupBase* parent0 = nullptr,
	   RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_FLOAT(T)))) :
    ObjElement("ctech", parent0), technique(technique0), values() {
    assign_values(values, values0);
    from_values();
  }
  template<typename T>
  ObjCTech(const std::string& technique0,
	   const std::vector<T> &,
	   const ObjGroupBase* parent0 = nullptr,
	   RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_FLOAT(T)))) :
    ObjElement("ctech", parent0), technique(technique0), values() {
    RAPIDJSON_ASSERT(sizeof("ObjCTech type is double"));
  }
  //! \copydoc ObjElement::has_property
  bool has_property(const std::string name, bool dontCheckOrder=false, bool skipColors=false) const  OVERRIDE_CXX11 {
    return (ObjElement::has_property(name, dontCheckOrder, skipColors) &&
	    ((technique == "cparm" && (name == "technique" ||
				       name == "resolution")) ||
	     (technique == "cspace" && (name == "technique" ||
					name == "maxlength")) ||
	     (technique == "curv" && (name == "technique" ||
				      name == "maxdist" ||
				      name == "maxangle"))));
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjCTech& rhs) {
    technique = rhs.technique;
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT(((technique == "cparm" ) ||
		      (technique == "cspace") ||
		      (technique == "curv"  )));
    if ((technique == "cparm") || (technique == "cspace"))
      RAPIDJSON_ASSERT((values.size() == 1));
    else
      RAPIDJSON_ASSERT((values.size() == 2));
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool&) OVERRIDE_CXX11 {
    in >> technique;
    RAPIDJSON_ASSERT(((technique == "cparm" ) ||
		      (technique == "cspace") ||
		      (technique == "curv"  )));
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << technique << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjCTech* lhs = this;
    const ObjCTech* rhs = dynamic_cast<const ObjCTech*>(rhs0);
    if (lhs->technique != rhs->technique) return false;
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::is_valid
  bool is_valid(std::map<std::string,size_t>&) const OVERRIDE_CXX11 {
    return (((technique == "cparm" ||
	      technique == "cspace") && values.size() == 1) ||
	    (technique == "curv" && values.size() == 2));
  }
  //! Technique used for the resolution.
  std::string technique;
  //! Vector of resolution parameters.
  std::vector<double> values;
};

//! Surface technique resolution.
class ObjSTech : public ObjElement {
public:
  GENERIC_ELEMENT_CONSTRUCTOR(ObjSTech, ObjElement, stech, SINGLE_ARG(, technique(""), values()), SINGLE_ARG(OBJ_P_("technique", ObjTypeString), OBJ_P_("ures", ObjTypeFloat), OBJ_P_("vres", ObjTypeFloat), OBJ_P_("uvres", ObjTypeFloat), OBJ_P_("maxlength", ObjTypeFloat), OBJ_P_("maxdist", ObjTypeFloat), OBJ_P_("maxangle", ObjTypeFloat)))
  DUMMY_ARRAY_CONSTRUCTOR(ObjSTech, ObjElement, stech, SINGLE_ARG(, technique(""), values()))
  template<typename T>
  ObjSTech(const std::string& technique0,
	   const std::vector<T> &values0,
	   const ObjGroupBase* parent0 = nullptr,
	   RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_FLOAT(T)))) :
    ObjElement("stech", parent0), technique(technique0), values() {
    assign_values(values, values0);
    from_values();
  }
  template<typename T>
  ObjSTech(const std::string& technique0,
	   const std::vector<T> &,
	   const ObjGroupBase* parent0 = nullptr,
	   RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_FLOAT(T)))) :
    ObjElement("stech", parent0), technique(technique0), values() {
    RAPIDJSON_ASSERT(!sizeof("ObjSTech type is double"));
  }
  //! \copydoc ObjElement::has_property
  bool has_property(const std::string name, bool dontCheckOrder=false, bool skipColors=false) const  OVERRIDE_CXX11 {
    return (ObjElement::has_property(name, dontCheckOrder, skipColors) &&
	    ((technique == "cparma" && (name == "technique" ||
					name == "ures" ||
					name == "vres")) ||
	     (technique == "cparmb" && (name == "technique" ||
					name == "uvres")) ||
	     (technique == "cspace" && (name == "technique" ||
					name == "maxlength")) ||
	     (technique == "curv" && (name == "technique" ||
				      name == "maxdist" ||
				      name == "maxangle"))));
  }
  //! \brief Copy element specific members from another instance.
  //! \param[in] rhs Element to copy members from.
  void copy_members(const ObjSTech& rhs) {
    technique = rhs.technique;
    values.insert(values.begin(), rhs.values.begin(), rhs.values.end());
  }
  //! \copydoc ObjElement::from_values()
  void from_values() OVERRIDE_CXX11 {
    RAPIDJSON_ASSERT(((technique == "cparma") ||
		      (technique == "cparmb") ||
		      (technique == "cspace") ||
		      (technique == "curv"  )));
    if ((technique == "cparmb") || (technique == "cspace"))
      RAPIDJSON_ASSERT((values.size() == 1));
    else
      RAPIDJSON_ASSERT((values.size() == 2));
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in, const bool&) OVERRIDE_CXX11 {
    in >> technique;
    RAPIDJSON_ASSERT(((technique == "cparma") ||
		      (technique == "cparmb") ||
		      (technique == "cspace") ||
		      (technique == "curv"  )));
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const OVERRIDE_CXX11 {
    out << technique << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const OVERRIDE_CXX11 {
    if (rhs0->code != this->code) return false;
    const ObjSTech* lhs = this;
    const ObjSTech* rhs = dynamic_cast<const ObjSTech*>(rhs0);
    if (lhs->technique != rhs->technique) return false;
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::is_valid
  bool is_valid(std::map<std::string,size_t>&) const OVERRIDE_CXX11 {
    return (((technique == "cparmb" || technique == "cspace") &&
	     values.size() == 1) ||
	    ((technique == "cparma" || technique == "curv") &&
	     values.size() == 2));
  }
  //! Technique used for the resolution.
  std::string technique;
  //! Vector of resolution parameters.
  std::vector<double> values;
};

//! \brief Read an element from an input stream.
//! \param in Input stream.
//! \param parent Optional element group that will contain the read elements.
//! \param dont_descend If true, subelements will not be read from the
//!   input stream.
//! \return New element.
inline ObjElement* read_obj_element(std::istream &in,
				    const ObjGroupBase* parent,
				    const bool& dont_descend) {
  std::string word = "";
  ObjElement* out = nullptr;
  if (in >> word)
    OBJ_ELEMENT_INIT(word, out, (in, parent, dont_descend));
  return out;
}

//! Obj wavefront 3D geometry container class.
class ObjWavefront : public ObjGroupBase {
public:
  ObjWavefront() : ObjGroupBase("") {}
  //! \brief Copy constructor.
  //! \param rhs Instance to copy.
  ObjWavefront(const ObjWavefront& rhs) : ObjGroupBase(rhs) {}
  //! \brief Create an ObjWavefront instance from C arrays of vertices.
  //! \tparam Tv Type of value in vertex value arrays.
  //! \tparam Mv Number of vertex elements.
  //! \tparam Nv Number of values in the array for each vertex element.
  //! \param vertices Array of vertex element value arrays.
  template<typename Tv, size_t Mv, size_t Nv>
  ObjWavefront(const Tv (&vertices)[Mv][Nv]) : ObjGroupBase("") {
    add_element_set("v", vertices);
  }
  //! \brief Create an ObjWavefront instance from C arrays of vertices and
  //!   faces.
  //! \tparam Tv Type of value in vertex value arrays.
  //! \tparam Mv Number of vertex elements.
  //! \tparam Nv Number of values in the array for each vertex element.
  //! \tparam Tf Type of value in face value arrays.
  //! \tparam Mf Number of face elements.
  //! \tparam Nf Number of values in the array for each face element.
  //! \param vertices Array of vertex element value arrays.
  //! \param faces Array of face element value arrays.
  template<typename Tv, size_t Mv, size_t Nv,
	   typename Tf, size_t Mf, size_t Nf>
  ObjWavefront(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf]) :
    ObjGroupBase("") {
    add_element_set("v", vertices);
    add_element_set("f", faces);
  }
  //! \brief Create an ObjWavefront instance from C arrays of vertices,
  //!   faces, and edges.
  //! \tparam Tv Type of value in vertex value arrays.
  //! \tparam Mv Number of vertex elements.
  //! \tparam Nv Number of values in the array for each vertex element.
  //! \tparam Tf Type of value in face value arrays.
  //! \tparam Mf Number of face elements.
  //! \tparam Nf Number of values in the array for each face element.
  //! \tparam Te Type of value in edge value arrays.
  //! \tparam Me Number of edge elements.
  //! \tparam Ne Number of values in the array for each edge element.
  //! \param vertices Array of vertex element value arrays.
  //! \param faces Array of face element value arrays.
  //! \param edges Array of edge element value arrays.
  template<typename Tv, size_t Mv, size_t Nv,
	   typename Tf, size_t Mf, size_t Nf,
	   typename Te, size_t Me, size_t Ne>
  ObjWavefront(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf],
	       const Te (&edges)[Me][Ne]) :
    ObjGroupBase("") {
    add_element_set("v", vertices);
    add_element_set("f", faces);
    add_element_set("l", edges);
  }
  //! \brief Add a set of elements to the geometry from a C array of value
  //!   arrays.
  //! \tparam T Type of value in the values array.
  //! \tparam M Number of elements being added.
  //! \tparam N Number of entries in each element's values array.
  //! \param name Name of the type of elements being added.
  //! \param values Array of value arrays defining the elements.
  //! \param ignore Value to ignore. After this value is encountered for an
  //!   element will be added.
  template <typename T, size_t M, size_t N>
  void add_element_set(std::string name, const T (&values)[M][N],
		       const T* ignore = 0) {
    std::string name2 = obj_alias2base(name);
    for (SizeType i = 0; i < M; i++)
      this->add_element(name2, values[i], ignore);
  }
  //! \brief Add a new element set to the geometry.
  //! \tparam T Type of property values.
  //! \param name Name of the type of element in the set.
  //! \param arr Property values for each of the elements in the set.
  //! \param M Number of elements in the set.
  //! \param N Number of properties for each element.
  //! \param ignore Value to ignore. After this value is encountered for an
  //!   element will be added.
  template <typename T>
  void add_element_set(const std::string& name,
		       const T* arr, SizeType M, SizeType N,
		       const T* ignore = 0) {
    std::string name2 = obj_alias2base(name);
    const T* p = arr;
    for (SizeType i = 0; i < M; i++, p += N)
      this->add_element(name2, p, N, ignore);
  }
  //! \brief Get the minimum bounds of the structure in 3D.
  //! \return Minimum extend of structure in x, y, z.
  std::vector<double> minimums() const {
    std::vector<double> out = {NAN, NAN, NAN};
    bool first = true;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code == "v") {
	if (first) {
	  out = (*it)->get_double_array(true);
	  first = false;
	} else {
	  std::vector<double> iarr = (*it)->get_double_array(true);
	  out[0] = (std::min)(out[0], iarr[0]);
	  out[1] = (std::min)(out[1], iarr[1]);
	  out[2] = (std::min)(out[2], iarr[2]);
	}
      }
    }
    return out;
  }
  //! \brief Get the maximum bounds of the structure in 3D.
  //! \return Maximum extend of structure in x, y, z.
  std::vector<double> maximums() const {
    std::vector<double> out = {NAN, NAN, NAN};
    bool first = true;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code == "v") {
	if (first) {
	  out = (*it)->get_double_array(true);
	  first = false;
	} else {
	  std::vector<double> iarr = (*it)->get_double_array(true);
	  out[0] = (std::max)(out[0], iarr[0]);
	  out[1] = (std::max)(out[1], iarr[1]);
	  out[2] = (std::max)(out[2], iarr[2]);
	}
      }
    }
    return out;
  }
  //! \brief Get the mesh for the structure.
  //! \return Structure mesh with each row representing a face with vertex
  //!    information provided in sequence for each face.
  std::vector<std::vector<double>> mesh() const {
    std::vector<std::vector<double>> out;
    std::vector<size_t> vert_idx;
    size_t i = 0, iFace = 0;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++, i++) {
      if ((*it)->code == "v")
	vert_idx.push_back(i);
      else if ((*it)->code == "f") {
	const std::vector<int> idx = (*it)->get_int_array();
	out.push_back(std::vector<double>());
	for (std::vector<int>::const_iterator f = idx.begin(); f != idx.end(); f++) {
	  RAPIDJSON_ASSERT((*f >= 0 && (size_t)(*f) <= vert_idx.size()) ||
			   (*f < 0 && (size_t)(-(*f)) <= vert_idx.size()));
	  if (*f > 0 && (size_t)(*f) <= vert_idx.size()) {
	    elements[vert_idx[(size_t)(*f - 1)]]->get_double_array(out[iFace], true);
	  } else if (*f < 0 && (size_t)(-(*f)) <= vert_idx.size()) {
	    elements[vert_idx[(size_t)((int)vert_idx.size() + (*f))]]->get_double_array(out[iFace], true);
	  } else {
	    out.clear();
	    return out;
	  }
	}
	iFace++;
      }
    }
    return out;
  }
  //! \copydoc ObjElement::has_colors
  bool has_colors() const OVERRIDE_CXX11 {
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->has_colors()) return true;
    }
    return false;
  }
  //! \brief Determine if any elements of a given type contain color
  //!   information.
  //! \param name Name of the element set to check.
  //! \return true if there are colors, false otherwise.
  bool has_colors(const std::string& name) const {
    std::string name2 = obj_alias2base(name);
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code != name2) continue;
      if ((*it)->has_colors()) return true;
    }
    return false;
  }
  //! \brief Get an element set in an array form.
  //! \param name0 Name of the element set to get.
  //! \param[out] N Number of elements in the returned array.
  //! \param[out[ M Number of values for each element in the returned array.
  //! \param skipColors If true, the parameters containing colors will not be
  //!   included.
  //! \returns The element set of the requested type in array form.
  std::vector<int> get_int_array(const std::string& name0,
				 size_t &N, size_t &M,
				 bool=false) const {
    std::string name = obj_alias2base(name0);
    std::vector<int> out;
    size_t minSize = this->max_size(name);
    ObjGroupBase::get_int_array(name, out, minSize, 0);
    M = minSize;
    N = out.size() / M;
    return out;
  }
  //! \brief Get an element set in an array form.
  //! \param name0 Name of the element set to get.
  //! \param[out] N Number of elements in the returned array.
  //! \param[out[ M Number of values for each element in the returned array.
  //! \param skipColors If true, the parameters containing colors will not be
  //!   included.
  //! \returns The element set of the requested type in array form.
  std::vector<double> get_double_array(const std::string& name0,
				       size_t &N, size_t &M,
				       bool skipColors=false) const {
    std::string name = obj_alias2base(name0);
    std::vector<double> out;
    size_t minSize = this->max_size(name, skipColors);
    ObjGroupBase::get_double_array(name, out, minSize, NAN, skipColors);
    M = minSize;
    N = out.size() / M;
    return out;
  }
  //! \copydoc ObjElement::get_colors_array
  void get_colors_array(std::vector<uint8_t>& out,
			uint8_t defaultValue=0) const OVERRIDE_CXX11 {
    if (!has_colors()) return;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++)
      (*it)->get_colors_array(out, defaultValue);
  }
  //! \brief Get the colors for an element set in arrayform.
  //! \param name Name of the element set to get.
  //! \param[out] N Number of elements in the returned array.
  //! \param[out] M Number of values for each element in the returned array.
  //! \param defaultValue Value to add if colors are missing.
  //! \returns The colors for the requirested type in array form.
  std::vector<uint8_t> get_colors_array(const std::string& name,
					size_t &N, size_t &M,
					uint8_t defaultValue=0) const {
    std::string name2 = obj_alias2base(name);
    std::vector<uint8_t> out;
    if (!has_colors(name2)) return out;
    N = 0;
    for (std::vector<ObjElement*>::const_iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code != name2) continue;
      (*it)->get_colors_array(out, defaultValue);
      N++;
    }
    M = out.size() / N;
    RAPIDJSON_ASSERT(M == 3);
    return out;
  }
  //! \brief Add element colors to a set.
  //! \param name Name of the type of element in the set.
  //! \param arr Colors for each of the elements in the set.
  //! \param M Number of elements in the set.
  //! \param N Number of color properties for each element.
  //! \return true if successful, false otherwise.
  bool add_element_set_colors(const std::string& name,
			      const uint8_t* arr, SizeType M, SizeType N) {
    std::string name2 = obj_alias2base(name);
    if (N != 3 || M != count_elements(name2)) return false;
    size_t i = 0;
    for (std::vector<ObjElement*>::iterator it = elements.begin(); it != elements.end(); it++) {
      if ((*it)->code != name2) continue;
      if (!(*it)->add_colors(arr + (N * i), N)) return false;
      i++;
    }
    return true;
  }

  friend bool operator == (const ObjWavefront& lhs, const ObjWavefront& rhs);
  friend bool operator != (const ObjWavefront& lhs, const ObjWavefront& rhs);
  friend std::ostream & operator << (std::ostream &out, const ObjWavefront &p);
  friend std::istream & operator >> (std::istream &in,  ObjWavefront &p);
};
//! \brief Check the equivalent of two ObjWavefront instances by comparing
//!   elements (calls ObjWavefront::is_equal method).
//! \param lhs First instance for comparison.
//! \param rhs Second instance for comparison.
//! \return true if the two instances are equivalent.
inline
bool operator == (const ObjWavefront& lhs, const ObjWavefront& rhs)
{ return lhs.is_equal(&rhs); }

//! \brief Check the inequivalance of two ObjWavefront instances by comparing
//!   elements (calls ObjWavefront::is_equal method).
//! \param lhs First instance for comparison.
//! \param rhs Second instance for comparison.
//! \return true if the two instances are not equivalent.
inline
bool operator != (const ObjWavefront& lhs, const ObjWavefront& rhs)
{ return !lhs.is_equal(&rhs); }

//! \brief Write an ObjWavefront object to an output stream.
//! \param out Output stream.
//! \param p ObjWavefront object.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjWavefront &p)
{ return p.write(out); }

//! \brief Read an ObjWavefront object from an input stream.
//! \param in Input stream.
//! \param p ObjWavefrotn object.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjWavefront &p)
{ return p.read(in); }

inline
ObjElement* ObjGroupBase::add_element(const ObjElement& x) {
  ObjElement* x_cpy = nullptr;
  std::string name_copy = x.code;
  OBJ_ELEMENT_INIT(name_copy, x_cpy, (&x));
  return ObjGroupBase::add_element(x_cpy);
}
template <typename T>
ObjElement* ObjGroupBase::add_element(std::string name,
				      const std::vector<T> &values,
				      const T* ignore) {
  ObjElement* x = nullptr;
  if (ignore) {
    std::vector<T> values2;
    for (typename std::vector<T>::const_iterator it = values.begin(); it != values.end(); it++) {
      if (internal::values_eq(*it, *ignore)) break;
      values2.push_back(*it);
    }
    OBJ_ELEMENT_INIT(name, x, (values2, this));
  } else {
    OBJ_ELEMENT_INIT(name, x, (values, this));
  }
  return ObjGroupBase::add_element(x);
}
template <typename T>
ObjElement* ObjGroupBase::add_element(const std::string name,
				      const double& u0, const double& u1,
				      const std::vector<T> &values) {
  ObjElement* x = nullptr;
  if (name == "curv") x = new ObjCurve(u0, u1, values, this);
  else REPORT_UNSUPPORTED_ELEMENT(ObjCurve, name);
  return ObjGroupBase::add_element(x);
}
template <typename T>
ObjElement* ObjGroupBase::add_element(const std::string name,
				      const double& u0, const double& u1,
				      const double& u2, const double& u3,
				      const std::vector<T> &values) {
  ObjElement* x = nullptr;
  if (name == "surf") x = new ObjSurface(u0, u1, u2, u3, values, this);
  else REPORT_UNSUPPORTED_ELEMENT(ObjSurface, name);
  return ObjGroupBase::add_element(x);
}
template <typename T>
ObjElement* ObjGroupBase::add_element(std::string name, std::string direction,
				      const std::vector<T> &values) {
  ObjElement* x = nullptr;
  if      (name == "parm" ) x = new ObjParameter(direction, values, this);
  else if (name == "ctech") x = new ObjCTech(direction, values, this);
  else if (name == "stech") x = new ObjSTech(direction, values, this);
  else REPORT_UNSUPPORTED_ELEMENT(direction, name);
  return ObjGroupBase::add_element(x);
}
template<typename T>
RAPIDJSON_DISABLEIF_RETURN((internal::IsPointer<T>), (ObjElement*)) ObjGroupBase::add_element(std::string name, const T& value) {
  ObjElement* x = nullptr;
  if      (name == "g"         ) x = new ObjGroup(value, this);
  else if (name == "s"         ) x = new ObjSmoothingGroup(value, this);
  else if (name == "bevel"     ) x = new ObjBevel(value, this);
  else if (name == "c_interp"  ) x = new ObjCInterp(value, this);
  else if (name == "d_interp"  ) x = new ObjDInterp(value, this);
  else if (name == "lod"       ) x = new ObjLOD(value, this);
  else if (name == "usemap"    ) x = new ObjTextureMap(value, this);
  else if (name == "usemtl"    ) x = new ObjMaterial(value, this);
  else if (name == "shadow_obj") x = new ObjShadowFile(value, this);
  else if (name == "trace_obj" ) x = new ObjTraceFile(value, this);
  else if (name == "maplib"    ) {
    std::vector<std::string> values;
    values.push_back(name);
    x = new ObjTextureMapLib(values, this);
  } else if (name == "mtllib"  ) {
    std::vector<std::string> values;
    values.push_back(name);
    x = new ObjMaterialLib(values, this);
  } else if ((name == "trim") || (name == "scrv") || (name == "hole")) {
    std::vector<T> values;
    values.push_back(value);
    return ObjGroupBase::add_element(name, values);
  }
  else REPORT_UNSUPPORTED_ELEMENT(scalar, name);
  return ObjGroupBase::add_element(x);
}

inline
ObjElement* ObjGroupBase::add_element(std::string name, const int& value,
				      const double& resolution) {
  ObjElement* x = nullptr;
  if      (name == "mg"   ) x = new ObjMergingGroup(value, resolution, this);
  else REPORT_UNSUPPORTED_ELEMENT(ObjMergingGroup, name);
  return ObjGroupBase::add_element(x);
}
inline
ObjElement* ObjGroupBase::add_element(std::string name,
				      const std::string& value,
				      const double& resolution) {
  ObjElement* x = nullptr;
  if      (name == "mg"   ) x = new ObjMergingGroup(value, resolution, this);
  else if (name == "parm" ) {
    std::vector<double> vres;
    vres.push_back(resolution);
    x = new ObjParameter(value, vres, this);
  }
  else if (name == "ctech") {
    std::vector<double> vres;
    vres.push_back(resolution);
    x = new ObjCTech(value, vres, this);
  }
  else if (name == "stech") {
    std::vector<double> vres;
    vres.push_back(resolution);
    x = new ObjSTech(value, vres, this);
  }
  else REPORT_UNSUPPORTED_ELEMENT(ObjMergingGroupString, name);
  return ObjGroupBase::add_element(x);
}

#undef GENERIC_CLASS_VECTOR_OBJREFVERTEX
#undef GENERIC_CLASS_VECTOR_TYPE
#undef GENERIC_CLASS_SCALAR_TYPE
#undef GENERIC_CLASS_SCALAR_TYPE_BODY
#undef GENERIC_CLASS_SCALAR_TYPE_BASE
#undef GENERIC_CLASS_SCALAR_TYPE_ISEQUAL
#undef GENERIC_IS_EQUAL
#undef GENERIC_FROM_VALUES
#undef GENERIC_WRITE_VALUES
#undef GENERIC_READ_VALUES
#undef DUMMY_ARRAY_CONSTRUCTOR
#undef GENERIC_CONSTRUCTOR_VECTOR_TYPE
#undef GENERIC_CONSTRUCTOR_VECTOR_FLOAT
#undef GENERIC_CONSTRUCTOR_COPY
#undef ASSERT_COMPATIBLE
#undef COMPATIBLE_WITH_ANY
#undef COMPATIBLE_WITH_TYPE
#undef COMPATIBLE_WITH_SURF
#undef COMPATIBLE_WITH_CURV
#undef COMPATIBLE_WITH_VERT
#undef COMPATIBLE_WITH_FLOAT
#undef COMPATIBLE_WITH_UINT
#undef COMPATIBLE_WITH_INT
#undef GENERIC_ELEMENT_CONSTRUCTOR
#undef OBJ_ELEMENT_INIT
#undef REPORT_UNSUPPORTED_ELEMENT
#undef OVERRIDE_CXX11

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_OBJ_H_
