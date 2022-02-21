#ifndef RAPIDJSON_OBJ_H_
#define RAPIDJSON_OBJ_H_

#include <iostream>

RAPIDJSON_NAMESPACE_BEGIN

#define IS_EQUAL_FLT(a, b) (fabs(a - b) < 0.01)
#define IS_EQUAL_DBL(a, b) (abs(a - b) < 0.01)
#define OBJ_ELEMENT_INIT(word, lhs, args) {				\
    if      (word == "v"     ) lhs = new ObjVertex args;		\
    else if (word == "vp"    ) lhs = new ObjVParameter args;		\
    else if (word == "vn"    ) lhs = new ObjVNormal args;		\
    else if (word == "vt"    ) lhs = new ObjVTexture args;		\
    else if (word == "p"     ) lhs = new ObjPoint args;			\
    else if (word == "l"     ) lhs = new ObjLine args;			\
    else if (word == "f"     ) lhs = new ObjFace args;			\
    else if (word == "curv"  ) lhs = new ObjCurve args;			\
    else if (word == "curv2" ) lhs = new ObjCurve2D args;		\
    else if (word == "surf"  ) lhs = new ObjSurface args;		\
    else if (word == "cstype") lhs = new ObjFreeFormType args;		\
    else if (word == "deg"   ) lhs = new ObjDegree args;		\
    else if (word == "bmat"  ) lhs = new ObjBasisMatrix args;		\
    else if (word == "step"  ) lhs = new ObjStep args;			\
    else if (word == "parm"  ) lhs = new ObjParameter args;		\
    else if (word == "trim"  ) lhs = new ObjTrim args;			\
    else if (word == "hole"  ) lhs = new ObjHole args;			\
    else if (word == "scrv"  ) lhs = new ObjScrv args;			\
    else if (word == "sp"    ) lhs = new ObjSpecialPoints args;		\
    else if (word == "con"   ) lhs = new ObjConnect args;		\
    else if (word == "g"     ) lhs = new ObjGroup args;			\
    else if (word == "s"     ) lhs = new ObjSmoothingGroup args;	\
    else if (word == "mg"    ) lhs = new ObjMergingGroup args;		\
    else if (word == "o"     ) lhs = new ObjObjectName args;		\
    else if (word == "end"   ) lhs = NULL;				\
    else RAPIDJSON_ASSERT(!sizeof(std::string("Unsupported element signifier: ") + word)); \
  }
#define GENERIC_ELEMENT_CONSTRUCTOR(cls)				\
  /*! \copydoc ObjElement::ObjElement(const ObjElement*) */		\
  cls(const ObjElement* rhs) : cls(*static_cast<const cls*>(rhs)) {}	\
  /*! \brief Initialize an element by reading from an input stream. */	\
  /*! \param in Input stream to read from. */				\
  /*! \param parent0 The element's parent group. */			\
  cls(std::istream &in, const ObjGroupBase* parent0 = nullptr) :	\
    cls(parent0) {							\
    read_values(in);							\
    from_values();							\
  }									\
  /*! \copydoc ObjElement::copy() */					\
  cls* copy() const override { return new cls(*this); }
#define C_ARRAY_CONSTRUCTOR(cls)					\
  /*! \brief Initialize an element from a C array of values. */		\
  /*! \tparam T Array element type. */					\
  /*! \tparam N Array size. */						\
  /*! \param src Array of values. */					\
  template <typename T, size_t N>					\
  cls(const T (&values)[N]) : cls(std::vector<T>(values, values+N)) {}
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
		    internal::IsSame<T,uint64_t>>>>>>>>>>
#define COMPATIBLE_WITH_UINT(T)				       \
      internal::OrExpr<internal::IsSame<T,uint8_t>,	       \
	  internal::OrExpr<internal::IsSame<T,uint16_t>,       \
	      internal::OrExpr<internal::IsSame<T,uint32_t>,   \
		  internal::OrExpr<internal::IsSame<T,ObjRef>, \
		      internal::IsSame<T,uint64_t>>>>>
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
		      internal::IsSame<T,uint64_t>>>>>>>>>>>
#define COMPATIBLE_WITH_VERT(T)						\
  internal::OrExpr<internal::IsSame<T,ObjRefVertex>, COMPATIBLE_WITH_INT(T)>
#define COMPATIBLE_WITH_TYPE(T1, T2)		 \
  internal::IsSame<T1,T2>
#define GENERIC_CONSTRUCTOR_EMPTY(cls, code)				\
  /*! \brief Empty constructor. */					\
  /*! \param parent0 The element's parent group. */			\
  cls(const ObjGroupBase* parent0 = nullptr) :				\
    ObjElement(#code, parent0)
#define GENERIC_CONSTRUCTOR_COPY(cls)					\
  /*! \copydoc ObjElement::ObjElement(const ObjElement&) */		\
  cls(const cls& rhs) :							\
    ObjElement(rhs)
#define GENERIC_CONSTRUCTOR_VECTOR_INT(cls)				\
  /*! \brief Initialize and element from a C++ vector of int values. */	\
  /*! \tparam T Vector element type. Must be int castable. */		\
  /*! \param values0 Vector of values. */				\
  /*! \param parent0 The element's parent group. */			\
  template <typename T>							\
  cls(const std::vector<T> &values0,					\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_INT(T)))) : cls(parent0) {	\
    assign_values(values, values0);					\
    from_values();							\
  }									\
  /*! \brief Raise an error if non-int vector is provided. */		\
  /*! \tparam T Type of vector elements. */				\
  template <typename T>							\
  cls(const std::vector<T>&,						\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_INT(T)))) : cls(parent0) {	\
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from integers.")); \
  }									\
  C_ARRAY_CONSTRUCTOR(cls);						\
  GENERIC_ELEMENT_CONSTRUCTOR(cls);
#define GENERIC_CONSTRUCTOR_VECTOR_UINT(cls)				\
  /*! \brief Initialize and element from a C++ vector of uint values. */ \
  /*! \tparam T Vector element type. Must be unsigned int castable. */	\
  /*! \param values0 Vector of values. */				\
  /*! \param parent0 The element's parent group. */			\
  template <typename T>							\
  cls(const std::vector<T> &values0,					\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_UINT(T)))) : cls(parent0) {	\
    assign_values(values, values0);					\
    from_values();							\
  }									\
  /*! \brief Raise an error if non-uint vector is provided. */		\
  /*! \tparam T Type of vector elements. */				\
  template <typename T>							\
  cls(const std::vector<T>&,						\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_UINT(T)))) : cls(parent0) {	\
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from unsigned integers.")); \
  }									\
  C_ARRAY_CONSTRUCTOR(cls);						\
  GENERIC_ELEMENT_CONSTRUCTOR(cls);
#define GENERIC_CONSTRUCTOR_VECTOR_FLOAT(cls)				\
  /*! \brief Initialize and element from a C++ vector of float values. */ \
  /*! \tparam T Vector element type. Must be float castable. */		\
  /*! \param values0 Vector of values. */				\
  /*! \param parent0 The element's parent group. */			\
  template <typename T>							\
  cls(const std::vector<T> &values0,					\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_FLOAT(T)))) : cls(parent0) {	\
    assign_values(values, values0);					\
    from_values();							\
  }									\
  /*! \brief Raise an error if non-float vector is provided. */		\
  /*! \tparam T Type of vector elements. */				\
  template <typename T>							\
  cls(const std::vector<T>&,						\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_FLOAT(T)))) : cls(parent0) {	\
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from floats.")); \
  }									\
  C_ARRAY_CONSTRUCTOR(cls);						\
  GENERIC_ELEMENT_CONSTRUCTOR(cls);
#define GENERIC_CONSTRUCTOR_VECTOR_VERT(cls)				\
  /*! \brief Initialize and element from a C++ vector of vertex values. */ \
  /*! \tparam T Vector element type. Must be vertex castable. */	\
  /*! \param values0 Vector of values. */				\
  /*! \param parent0 The element's parent group. */			\
  template <typename T>							\
  cls(const std::vector<T> &values0,					\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_VERT(T)))) : cls(parent0) {	\
    assign_values(values, values0);					\
    from_values();							\
  }									\
  /*! \brief Raise an error if non-vert vector is provided. */		\
  /*! \tparam T Type of vector elements. */				\
  template <typename T>							\
  cls(const std::vector<T>&,						\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_VERT(T)))) :	cls(parent0) {	\
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from vertices.")); \
  }									\
  C_ARRAY_CONSTRUCTOR(cls);						\
  GENERIC_ELEMENT_CONSTRUCTOR(cls);
#define GENERIC_CONSTRUCTOR_VECTOR_TYPE(cls, T2)			\
  /*! \brief Initialize and element from a C++ vector of type values. */ \
  /*! \tparam T Vector element type. Must be castable to type. */	\
  /*! \param values0 Vector of values. */				\
  /*! \param parent0 The element's parent group. */			\
  template <typename T>							\
  cls(const std::vector<T> &values0,					\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_TYPE(T, T2)))) : cls(parent0) { \
    assign_values(values, values0);					\
    from_values();							\
  }									\
  /*! \brief Raise an error if non-type vector is provided. */		\
  /*! \tparam T Type of vector elements. */				\
  template <typename T>							\
  cls(const std::vector<T>&,						\
      const ObjGroupBase* parent0 = nullptr,				\
      RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_TYPE(T, T2)))) : cls(parent0){ \
    RAPIDJSON_ASSERT(!sizeof(#cls " must be initialized from" #T2 ".")); \
  }									\
  C_ARRAY_CONSTRUCTOR(cls);						\
  GENERIC_ELEMENT_CONSTRUCTOR(cls);

#define DUMMY_ARRAY_CONSTRUCTOR(cls)					\
  C_ARRAY_CONSTRUCTOR(cls);						\
  /*! \brief Raise an error. */						\
  template <typename T>							\
  cls(const std::vector<T> &) : cls() {					\
    RAPIDJSON_ASSERT(!sizeof(#cls " elements cannot be initialized from a vector")); \
  }
#define GENERIC_READ_VALUES						\
  /*! \copydoc ObjElement::read_values */				\
  void read_values(std::istream &in) override { ObjElement::read_values(in, values); }
#define GENERIC_WRITE_VALUES						\
  /*! \copydoc ObjElement::write_values */				\
  void write_values(std::ostream &out) const override { ObjElement::write_values(out, values); }


//! Checks if two values are equal using == operator.
template <typename T>
inline bool is_equal(const T &a, const T &b) {
  return (a == b);
};
//! Checks if two float values are equal using comparison.
template <>
inline bool is_equal(const float &a, const float &b) {
  return IS_EQUAL_FLT(a, b);
};
//! Checks if two double values are equal using comparison.
template <>
inline bool is_equal(const double &a, const double &b) {
  return IS_EQUAL_DBL(a, b);
};

//! Test if two vectors are equal element-by-element using is_equal
template <typename T>
inline bool is_equal_vectors(const std::vector<T>& a, const std::vector<T>& b) {
  if (a.size() != b.size()) return false;
  for (auto ait = a.begin(), bit = b.begin(); ait != a.end(); ait++, bit++)
    if (!is_equal(*ait, *bit)) return false;
  return true;
};

// Forward declaration
class ObjElement;
class ObjGroupBase;
inline void read_obj_elements(std::istream &in, std::vector<ObjElement*> &elements,
			      const char break_at = 0, const ObjGroupBase* parent = nullptr);

//! ObjWavefront color.
class ObjColor {
public:
  //! Empty initializer with (r,g,b) = (0,0,0)
  ObjColor() :
    r(0), g(0), b(0), is_set(false) {}
  //! Copy constructor
  ObjColor(const ObjColor& rhs) :
    r(rhs.r), g(rhs.g), b(rhs.b), is_set(rhs.is_set) {}
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
{ return lhs.is_equal(rhs); };

//! Object reference index.
typedef int64_t ObjRef;

//! ObjWavefront vertex reference
class ObjRefVertex {
public:
  ObjRefVertex() :
    v(-1), vt(-1), vn(-1), Nparam(-1) {}
  //! Copy constructor
  ObjRefVertex(const ObjRefVertex& rhs) :
    v(rhs.v), vt(rhs.vt), vn(rhs.vn), Nparam(rhs.Nparam) {}
  //! \brief Constructor
  //! \param v0 Index of the vertex's coordinates
  //! \param vt0 Index of the vertex's texcoord
  //! \param vn0 Index of the vertex's normal
  //! \param Nparam0 The number of parameters specified by the vertex. If
  //!    not provided, it will be determined by chcking the values of v0, vt0,
  //!    and vn0. (1: (v), 2: (v, vt), 3: (v, vt, vn)).
  ObjRefVertex(ObjRef v0, ObjRef vt0=-1, ObjRef vn0=-1,
	       int8_t Nparam0=-1) :
    v(v0), vt(vt0), vn(vn0), Nparam(Nparam0) {
    if (Nparam < 0) {
      if (vn >= 0)
	Nparam = 3;
      else if (vt >= 0)
	Nparam = 2;
      else
	Nparam = 1;
    }
  }
  //! \brief Constructor
  //! \tparam T Vertex index type
  //! \param v0 Index of the vertex's coordinates
  template <typename T>
  ObjRefVertex(const T& v0,
	       RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_INT(T)))) :
    ObjRefVertex(v0, -1, -1) {}
  //! \brief Write the vertex to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    out << v;
    if (Nparam > 1) {
      out << "/";
      if (vt >= 0)
	out << vt;
    }
    if (Nparam > 2) {
      out << "/";
      if (vn >= 0)
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
    std::istringstream ss_token;
    v = -1;
    vt = -1;
    vn = -1;
    Nparam = 0;
    // v
    RAPIDJSON_ASSERT(std::getline(ss_word, token, '/'));
    ss_token.str(token);
    ss_token >> v;
    // vt
    if (!std::getline(ss_word, token, '/')) {
      Nparam = 1;
      return in;
    }
    ss_token.str(token);
    ss_token >> vt;
    // vn
    if (!std::getline(ss_word, token, '/')) {
      Nparam = 1;
      return in;
    }
    ss_token.str(token);
    ss_token >> vt;
    Nparam = 3;
    return in;
  }
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
    if (lhs.Nparam != rhs.Nparam) return false;
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
{ return lhs.is_equal(rhs); };

//! Write an ObjRefVertex element to an output stream.
//! \param out Output stream.
//! \param p Element to write.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjRefVertex &p)
{ return p.write(out); };

//! Read an ObjRefVertex element from an input stream.
//! \param in Input stream.
//! \param p Element to read into.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjRefVertex &p)
{ return p.read(in); };


//! ObjWavefront curve reference.
class ObjRefCurve {
public:
  //! \brief Empty constructor.
  ObjRefCurve() :
    u0(0.0), u1(0.0), curv2d(-1) {}
  //! \brief Copy constructor.
  //! \param rhs Curve to copy.
  ObjRefCurve(const ObjRefCurve& rhs) :
    u0(rhs.u0), u1(rhs.u1), curv2d(rhs.curv2d) {}
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
    if (!IS_EQUAL_DBL(lhs.u0, rhs.u0)) return false;
    if (!IS_EQUAL_DBL(lhs.u1, rhs.u1)) return false;
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
{ return lhs.is_equal(rhs); };

//! Write an ObjRefCurve element to an output stream.
//! \param out Output stream.
//! \param p Element.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjRefCurve &p)
{ return p.write(out); };

//! Read an ObjRefCurve element from an input stream.
//! \param in Input stream.
//! \param p Element to read into.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjRefCurve &p)
{ return p.read(in); };


//! ObjWavefront surface reference.
class ObjRefSurface {
public:
  //! \brief Empty constructor.
  ObjRefSurface() :
    surf(-1), q0(0), q1(0), curv2d(-1) {}
  //! \brief Copy constructor.
  //! \param rhs Surface to copy.
  ObjRefSurface(const ObjRefSurface& rhs) :
    surf(rhs.surf), q0(rhs.q0), q1(rhs.q1), curv2d(rhs.curv2d) {}
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
    if (!IS_EQUAL_DBL(lhs.q0, rhs.q0)) return false;
    if (!IS_EQUAL_DBL(lhs.q1, rhs.q1)) return false;
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
{ return lhs.is_equal(rhs); };

//! Write an ObjRefSurface element to an output stream.
//! \param out Output stream.
//! \param p Element to write.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjRefSurface &p)
{ return p.write(out); };

//! Read an ObjRefSurface element from an input stream.
//! \param in Input stream.
//! \param p Element to read into.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjRefSurface &p)
{ return p.read(in); };

//! ObjWavefront element base class.
class ObjElement {
public:
  //! \brief Empty constructor.
  //! \param parent0 The element's parent group.
  ObjElement(const ObjGroupBase* parent0 = nullptr) : code(""), parent(parent0) {}
  //! \brief Initialize an element from an element code.
  //! \param code0 Element code.
  //! \param parent0 The element's parent group.
  ObjElement(const std::string& code0,
	     const ObjGroupBase* parent0 = nullptr) :
    code(code0), parent(parent0) {}
  //! \brief Copy constructor.
  //! \param rhs Element to copy.
  ObjElement(const ObjElement& rhs) :
    code(rhs.code), parent(rhs.parent) {}
  //! \brief Initialize and element from a C++ vector of values.
  //! \tparam T Vector element type. Must be an integer or floating point.
  //! \param parent0 The element's parent group.
  template <typename T, size_t N>
  ObjElement(const std::string& code0, const T (&src)[N],
	     const ObjGroupBase* parent0 = nullptr) :
    ObjElement(code0, std::vector<T>(src, src+N), parent0) {}
  //! \brief Initialize and element from a C++ vector of values.
  //! \tparam T Vector element type. Must be an integer or floating point.
  //! \param parent0 The element's parent group.
  template <typename T>
  ObjElement(const std::string& code0, const std::vector<T> &,
	     const ObjGroupBase* parent0 = nullptr) : code(code0), parent(parent0) {
    RAPIDJSON_ASSERT(!sizeof(code + " element cannot be constructed from a vector of the provided type."));
  }
  
  //! \brief Destructor.
  virtual ~ObjElement() {}
  //! \brief Create a copy of the element.
  //! \return Copied element.
  virtual ObjElement* copy() const = 0;
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
  void assign_values(std::vector<T1>& dst, const std::vector<T2> &src) {
    for (auto it = src.begin(); it != src.end(); it++)
      dst.emplace_back((T1)(*it));
  }
  //! \brief Assign element members from an array of values stored in another
  //!   class member during a previous call to assign_values.
  virtual void from_values() {} // Do nothing, keep values in vector
  //! \brief Read element members from an input stream.
  //! \param in Input stream.
  virtual void read_values(std::istream& in) = 0;
  //! \brief Write element member to an output stream.
  //! \param out Output stream.
  virtual void write_values(std::ostream& out) const = 0;
  //! \brief Check if another element is equivalent.
  //! \param rhs0 Element to compare.
  //! \return true if rhs is equivalent.
  virtual bool is_equal(const ObjElement* rhs0) const = 0;
  //! \brief Get element values as an array of strings.
  //! \return Array of string values.
  virtual std::vector<std::string> get_string_array() const {
    std::cerr << "get_string_array not implemented" << std::endl;
    std::vector<std::string> out;
    RAPIDJSON_ASSERT(out.size() > 0);
    return out;
  }
  //! \brief Get element values as an array of ints.
  //! \return Array of int values.
  virtual std::vector<int> get_int_array() const {
    std::cerr << "get_int_array not implemented" << std::endl;
    std::vector<int> out;
    RAPIDJSON_ASSERT(out.size() > 0);
    return out;
  }
  //! \brief Get element values as an array of doubles.
  //! \return Array of double values.
  virtual std::vector<double> get_double_array() const {
    std::cerr << "get_double_array not implemented" << std::endl;
    std::vector<double> out;
    RAPIDJSON_ASSERT(out.size() > 0);
    return out;
  }
  //! \brief Read element members from an input stream into a vector.
  //! \param in Input stream.
  //! \param values Vector to store read values in.
  template <typename T>
  void read_values(std::istream &in, std::vector<T> &values) {
    T x = 0;
    while ((in.peek() != '\n') && (in >> x))
      values.push_back(x);
  }
  //! \brief Write the element to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    out << code << " ";
    write_values(out);
    out << std::endl;
    return out;
  }
  //! \brief Write element member to an output stream from a vector.
  //! \param out Output stream.
  //! \param values Values to write.
  template <typename T>
  void write_values(std::ostream &out, const std::vector<T> &values) const {
    for (auto it = values.begin(); it != values.end(); it++) {
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
  //! Disable copy assignment for elements.
  ObjElement& operator=(const ObjElement* other);
  //! Code indicating the type of element.
  std::string code;
  //! Pointer to the parent element class.
  const ObjGroupBase* parent;
  friend std::ostream & operator << (std::ostream &out, const ObjElement &p);
};

//! Write an element to an output stream.
//! \param out Output stream.
//! \param p Element.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjElement &p)
{ return p.write(out); };

//! Vertex data
class ObjVertex : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjVertex, v),
    values(), x(0), y(0), z(0), w(-1), color() {}
  GENERIC_CONSTRUCTOR_COPY(ObjVertex),
    values(rhs.values), x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w), color(rhs.color) {}
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVertex);
  //! \copydoc ObjElement::from_values()
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 3)
		     || (values.size() == 4)
		     || (values.size() == 6)
		     || (values.size() == 7));
    x = values[0];
    y = values[1];
    z = values[2];
    if ((values.size() == 6) || (values.size() == 7)) {
      color.r = (uint8_t)values[3];
      color.g = (uint8_t)values[4];
      color.b = (uint8_t)values[5];
    }
    if ((values.size() == 4) || (values.size() == 7))
      w = values[values.size() - 1];
    else
      w = -1;
  }
  GENERIC_READ_VALUES;
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    out << x << " " << y << " " << z;
    if (color.is_set)
      out << " " << color.r << " " << color.g << " " << color.b;
    if (w >= 0)
      out << " " << w;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjVertex* lhs = this;
    const ObjVertex* rhs = static_cast<const ObjVertex*>(rhs0);
    if (!IS_EQUAL_DBL(lhs->x, rhs->x)) return false;
    if (!IS_EQUAL_DBL(lhs->y, rhs->y)) return false;
    if (!IS_EQUAL_DBL(lhs->z, rhs->z)) return false;
    if (!IS_EQUAL_DBL(lhs->w, rhs->w)) return false;
    return (lhs->color == rhs->color);
  }
  //! \copydoc ObjElement::get_double_array
  std::vector<double> get_double_array() const override {
    std::vector<double> out({x, y, z});
    return out;
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
  GENERIC_CONSTRUCTOR_EMPTY(ObjVParameter, vp),
    values(), u(0), v(0), w(-1) {}
  GENERIC_CONSTRUCTOR_COPY(ObjVParameter),
    values(), u(rhs.u), v(rhs.v), w(rhs.w) {}
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVParameter);
  //! \copydoc ObjElement::from_values()
  void from_values() override {
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
  void write_values(std::ostream &out) const override {
    out << u << " " << v;
    if (w >= 0)
      out << " " << w;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjVParameter* lhs = this;
    const ObjVParameter* rhs = static_cast<const ObjVParameter*>(rhs0);
    if (!IS_EQUAL_DBL(lhs->u, rhs->u)) return false;
    if (!IS_EQUAL_DBL(lhs->v, rhs->v)) return false;
    if (!IS_EQUAL_DBL(lhs->w, rhs->w)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_double_array
  std::vector<double> get_double_array() const override {
    std::vector<double> out({u, v, w});
    return out;
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
  GENERIC_CONSTRUCTOR_EMPTY(ObjVNormal, vn),
    values(), i(0), j(0), k(0) {}
  GENERIC_CONSTRUCTOR_COPY(ObjVNormal),
    values(), i(rhs.i), j(rhs.j), k(rhs.k) {}
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVNormal);
  //! \copydoc ObjElement::from_values
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() == 3);
    i = values[0];
    j = values[1];
    k = values[2];
  }
  GENERIC_READ_VALUES;
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    out << i << " " << j << " " << k;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjVNormal* lhs = this;
    const ObjVNormal* rhs = static_cast<const ObjVNormal*>(rhs0);
    if (!IS_EQUAL_DBL(lhs->i, rhs->i)) return false;
    if (!IS_EQUAL_DBL(lhs->j, rhs->j)) return false;
    if (!IS_EQUAL_DBL(lhs->k, rhs->k)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_double_array
  std::vector<double> get_double_array() const override {
    std::vector<double> out({i, j, k});
    return out;
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
  GENERIC_CONSTRUCTOR_EMPTY(ObjVTexture, vt),
    values(), u(0), v(0), w(0) {}
  GENERIC_CONSTRUCTOR_COPY(ObjVTexture),
    values(), u(rhs.u), v(rhs.v), w(rhs.w) {}
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjVTexture);
  //! \copydoc ObjElement::from_values
  void from_values() override {
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
  void write_values(std::ostream &out) const override {
    out << u;
    if (v >= 0)
      out << " " << v;
    if (w >= 0)
      out << " " << w;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    std::cerr << "before code" << std::endl;
    if (rhs0->code != this->code) return false;
    const ObjVTexture* lhs = this;
    const ObjVTexture* rhs = static_cast<const ObjVTexture*>(rhs0);
    std::cerr << "u here" << std::endl;
    if (!IS_EQUAL_DBL(lhs->u, rhs->u)) return false;
    std::cerr << "v here" << std::endl;
    if (!IS_EQUAL_DBL(lhs->v, rhs->v)) return false;
    std::cerr << "w here" << std::endl;
    if (!IS_EQUAL_DBL(lhs->w, rhs->w)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_double_array
  std::vector<double> get_double_array() const override {
    std::vector<double> out({u, v, w});
    return out;
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
  GENERIC_CONSTRUCTOR_EMPTY(ObjPoint, p), values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjPoint), values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_INT(ObjPoint);
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjPoint* lhs = this;
    const ObjPoint* rhs = static_cast<const ObjPoint*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::get_int_array
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(*it));
    return out;
  }
  //! Vector of reference values.
  std::vector<ObjRef> values;
};

//! Line element.
class ObjLine : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjLine, l), values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjLine), values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_VERT(ObjLine);
  //! \copydoc ObjElement::from_values()
  void from_values() override
  { RAPIDJSON_ASSERT(values.size() >= 2); }
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjLine* rhs = static_cast<const ObjLine*>(rhs0);
    if (!(is_equal_vectors(this->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::get_int_array
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(it->v));
    return out;
  }
  //! Vector of vertex refence values.
  std::vector<ObjRefVertex> values;
};

//! Face element.
class ObjFace : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjFace, f), values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjFace), values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_VERT(ObjFace);
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::from_values
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() >= 3);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjFace* lhs = this;
    const ObjFace* rhs = static_cast<const ObjFace*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::get_int_array
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(it->v));
    return out;
  }
  //! Vector of vertex reference values.
  std::vector<ObjRefVertex> values;
};

//! Curve element.
class ObjCurve : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjCurve, curv),
    values(), u0(0), u1(0) {}
  GENERIC_CONSTRUCTOR_COPY(ObjCurve),
    values(rhs.values), u0(rhs.u0), u1(rhs.u1) {}
  //! \brief Initialize an element from a C array of values.
  //! \tparam T Array element type.
  //! \tparam N Array size.
  //! \param u00 Starting curve parameter value.
  //! \param u10 Ending curve parameter value.
  //! \param parent0 The element's parent group.
  //! \param src Array of values.
  template <typename T, size_t N>
  ObjCurve(const double& u00, const double& u10, const T (&src)[N],
	   const ObjGroupBase* parent0 = nullptr) :
    ObjCurve(u00, u10, std::vector<T>(src, src+N), parent0) {}
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
    ObjElement("curv", parent0), values(), u0(u00), u1(u10) {
    assign_values(values, values0);
    from_values();
  }
  DUMMY_ARRAY_CONSTRUCTOR(ObjCurve);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjCurve);
  //! \copydoc ObjElement::from_values()
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() >= 2);
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    in >> u0;
    in >> u1;
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    out << u0 << " " << u1 << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjCurve* lhs = this;
    const ObjCurve* rhs = static_cast<const ObjCurve*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (!IS_EQUAL_DBL(lhs->u0, rhs->u0)) return false;
    if (!IS_EQUAL_DBL(lhs->u1, rhs->u1)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_double_array
  std::vector<double> get_double_array() const override {
    std::vector<double> out({u0, u1});
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((double)(*it));
    return out;
  }
  //! Vector for curve values.
  std::vector<ObjRef> values;
  //! Curve value in first parameter direction.
  double u0;
  //! Curve value in second parameter direction.
  double u1;
};

//! 2D curve element.
class ObjCurve2D : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjCurve2D, curv2), values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjCurve2D), values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_INT(ObjCurve2D);
  //! \copydoc ObjElement::from_values()
  void from_values() override {
    RAPIDJSON_ASSERT(values.size() >= 2);
  }
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjCurve2D* lhs = this;
    const ObjCurve2D* rhs = static_cast<const ObjCurve2D*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::get_int_array
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((int)(*it));
    return out;
  }
  //! Vector of object reference values.
  std::vector<ObjRef> values;
};

//! Surface element.
class ObjSurface : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjSurface, surf),
    values(), s0(0), s1(0), t0(0), t1(0) {}
  GENERIC_CONSTRUCTOR_COPY(ObjSurface),
    values(rhs.values), s0(rhs.s0), s1(rhs.s1), t0(rhs.t0), t1(rhs.t1) {}
  //! \brief Initialize an element from a C array of values.
  //! \tparam T Array element type.
  //! \tparam N Array size.
  //! \param s00 Starting curve parameter value in 1st dimension.
  //! \param s10 Ending curve parameter value in 1st dimension.
  //! \param t00 Starting curve parameter value in 2nd dimension.
  //! \param t10 Ending curve parameter value in 2nd dimension.
  //! \param src Array of values.
  //! \param parent0 The element's parent group.
  template <typename T, size_t N>
  ObjSurface(const double& s00, const double& s10,
	     const double& t00, const double& t10,
	     const T (&src)[N],
	     const ObjGroupBase* parent0 = nullptr) :
    ObjSurface(s00, s10, t00, t10, std::vector<T>(src, src+N), parent0) {}
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
	     RAPIDJSON_ENABLEIF((COMPATIBLE_WITH_INT(T)))) :
    ObjElement("surf", parent0), values(), s0(s00), s1(s10), t0(t00), t1(t10) {
    assign_values(values, values0);
    from_values();
  }
  DUMMY_ARRAY_CONSTRUCTOR(ObjSurface);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjSurface);
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    in >> s0;
    in >> s1;
    in >> t0;
    in >> t1;
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    out << s0 << " " << s1 << " " << t0 << " " << t1 << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjSurface* lhs = this;
    const ObjSurface* rhs = static_cast<const ObjSurface*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (!IS_EQUAL_DBL(lhs->s0, rhs->s0)) return false;
    if (!IS_EQUAL_DBL(lhs->s1, rhs->s1)) return false;
    if (!IS_EQUAL_DBL(lhs->t0, rhs->t0)) return false;
    if (!IS_EQUAL_DBL(lhs->t1, rhs->t1)) return false;
    return true;
  }
  //! \copydoc ObjElement::get_double_array
  std::vector<double> get_double_array() const override {
    std::vector<double> out({s0, s1, t0, t1});
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back((double)(it->v));
    return out;
  }
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

//! Element grouping base class.
class ObjGroupBase : public ObjElement {
public:
  //! \brief Empty constructor.
  //! \param parent0 The element's parent group.
  ObjGroupBase(const ObjGroupBase* parent0 = nullptr)
    : ObjElement(parent0), elements() {}
  //! \brief Initialize an element group from an element code.
  //! \param parent0 The element's parent group.
  ObjGroupBase(const std::string& code,
	       const ObjGroupBase* parent0 = nullptr)
    : ObjElement(code, parent0), elements() {}
  //! \brief Initialize an element group from an element code.
  //! \param parent0 The element's parent group.
  ObjGroupBase(const std::string& code,
	       const std::vector<ObjElement*> &elements0,
	       const ObjGroupBase* parent0 = nullptr)
    : ObjGroupBase(code, parent0) {
    assign_values(elements, elements0);
  }
  GENERIC_CONSTRUCTOR_COPY(ObjGroupBase), elements() {
    for (auto it = rhs.elements.begin(); it != rhs.elements.end(); it++)
      elements.push_back((*it)->copy());
  }
  DUMMY_ARRAY_CONSTRUCTOR(ObjGroupBase);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjGroupBase);
  ~ObjGroupBase() {
    for (auto it = elements.begin(); it != elements.end(); it++)
      delete *it;
    elements.resize(0);
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    read_obj_elements(in, elements, 0, this);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    for (auto it = elements.begin(); it != elements.end(); it++)
      (*it)->write(out);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjGroupBase* lhs = this;
    const ObjGroupBase* rhs = static_cast<const ObjGroupBase*>(rhs0);
    if (lhs->elements.size() != rhs->elements.size()) return false;
    for (auto lit = lhs->elements.begin(), rit = rhs->elements.begin();
	 lit != lhs->elements.end(); lit++, rit++) {
      if (!((*lit)->is_equal(*rit)))
	return false;
    }
    return true;
  }
  //! Vector of elements in the group.
  std::vector<ObjElement*> elements;
};

// //! Free-form sub-selements.
// class ObjGroupMemberBase : public ObjElement {
// public:
//   //! \brief Empty constructor.
//   ObjGroupMemberBase() : ObjElement(), parent() {}
//   //! \brief Initialize an element from an element code.
//   //! \param code0 Element code.
//   //! \param parent0 The element's parent group.
//   ObjGroupMemberBase(const std::string& code, const ObjGroupBase* parent0 = nullptr) :
//     ObjElement(code), parent(parent0) {}
//   //! \copydoc ObjElement::ObjElement(const ObjElement&)
//   ObjGroupMemberBase(const ObjGroupMemberBase& rhs) :
//     ObjElement(rhs), parent(rhs.parent) {}
//   //! Pointer to the parent element class.
//   const ObjElement* parent;
// };

//! Free-form element group.
class ObjFreeFormType : public ObjGroupBase {
public:
  //! \brief Empty constructor.
  //! \param parent0 The element's parent group.
  ObjFreeFormType(const ObjGroupBase* parent0 = nullptr) :
    ObjGroupBase("cstype", parent0), values() {}
  //! \copydoc ObjElement::ObjElement(const ObjElement&)
  ObjFreeFormType(const ObjFreeFormType& rhs) :
    ObjGroupBase(rhs), values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(ObjFreeFormType, std::string);
  //! \copydoc ObjElement::from_values()
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 1)
		     || (values.size() == 2));
  }
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    ObjElement::read_values(in, values);
    ObjGroupBase::read_values(in);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    ObjElement::write_values(out, values);
    out << std::endl;
    ObjGroupBase::write_values(out);
    out << "end";
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (!ObjGroupBase::is_equal(rhs0)) return false;
    const ObjFreeFormType* lhs = this;
    const ObjFreeFormType* rhs = static_cast<const ObjFreeFormType*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::get_string_array
  std::vector<std::string> get_string_array() const override {
    std::vector<std::string> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back(*it);
    for (auto it = elements.begin(); it != elements.end(); it++)
      out.push_back((*it)->as_string());
    return out;
  }
  //! Vector of free form type values.
  std::vector<std::string> values;
};

//! Degree element.
class ObjDegree : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjDegree, deg), values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjDegree), values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_UINT(ObjDegree);
  //! \copydoc ObjElement::from_values()
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 1) || (values.size() == 2));
  }
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjDegree* lhs = this;
    const ObjDegree* rhs = static_cast<const ObjDegree*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::get_int_array
  std::vector<int> get_int_array() const override {
    std::vector<int> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back(*it);
    return out;
  }
  //! Vector of degree elements.
  std::vector<uint16_t> values;
};


//! Basis matrix element.
class ObjBasisMatrix : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjBasisMatrix, bmat),
    values(), direction("") {}
  GENERIC_CONSTRUCTOR_COPY(ObjBasisMatrix),
    values(rhs.values), direction(rhs.direction) {}
  //! \brief Initialize an element from a C array of values.
  //! \tparam T Array element type.
  //! \tparam N Array size.
  //! \param direction0 Basis direction.
  //! \param src Array of values.
  //! \param parent0 The element's parent group.
  template <typename T, size_t N>
  ObjBasisMatrix(const std::string& direction0, const T (&src)[N],
		 const ObjGroupBase* parent0 = nullptr) :
    ObjBasisMatrix(direction0, std::vector<T>(src, src+N), parent0) {}
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
  DUMMY_ARRAY_CONSTRUCTOR(ObjBasisMatrix);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjBasisMatrix);
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    in >> direction;
    RAPIDJSON_ASSERT((direction == "u") || (direction == "v"));
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    out << direction << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjBasisMatrix* lhs = this;
    const ObjBasisMatrix* rhs = static_cast<const ObjBasisMatrix*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->direction != rhs->direction) return false;
    return true;
  }
  //! Vector of matrix values.
  std::vector<double> values;
  //! Basis matrix direction.
  std::string direction;
};

//! Step element.
class ObjStep : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjStep, step), values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjStep), values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_FLOAT(ObjStep);
  //! \copydoc ObjElement::from_values()
  void from_values() override {
    RAPIDJSON_ASSERT((values.size() == 1) || (values.size() == 2));
  }
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjStep* lhs = this;
    const ObjStep* rhs = static_cast<const ObjStep*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! \copydoc ObjElement::get_double_array
  std::vector<double> get_double_array() const override {
    std::vector<double> out;
    for (auto it = values.begin(); it != values.end(); it++)
      out.push_back(*it);
    return out;
  }
  //! Vector of step values.
  std::vector<double> values;
};

//! Parameter element.
class ObjParameter : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjParameter, parm),
    values(), direction("") {}
  GENERIC_CONSTRUCTOR_COPY(ObjParameter),
    values(rhs.values), direction(rhs.direction) {}
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
  template <typename T>
  ObjParameter(const std::vector<T> &,
	       const ObjGroupBase* parent0 = nullptr,
	       RAPIDJSON_DISABLEIF((COMPATIBLE_WITH_FLOAT(T)))) :
    ObjParameter(parent0) {
    RAPIDJSON_ASSERT(sizeof("ObjParameter type is double"));
  }
  DUMMY_ARRAY_CONSTRUCTOR(ObjParameter);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjParameter);
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    in >> direction;
    RAPIDJSON_ASSERT((direction == "u") || (direction == "v"));
    ObjElement::read_values(in, values);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    out << direction << " ";
    ObjElement::write_values(out, values);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjParameter* lhs = this;
    const ObjParameter* rhs = static_cast<const ObjParameter*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    if (lhs->direction != rhs->direction) return false;
    return true;
  }
  //! Vector of parameter values.
  std::vector<double> values;
  //! Parameter direction.
  std::string direction;
};

//! Trim element.
class ObjTrim : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjTrim, trim),
    values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjTrim),
    values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(ObjTrim, ObjRefCurve);
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjTrim* lhs = this;
    const ObjTrim* rhs = static_cast<const ObjTrim*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! Vector of curve reference values.
  std::vector<ObjRefCurve> values;
};

//! Hole element.
class ObjHole : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjHole, hole),
    values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjHole),
    values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(ObjHole, ObjRefCurve);
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjHole* lhs = this;
    const ObjHole* rhs = static_cast<const ObjHole*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! Vector of curve reference values.
  std::vector<ObjRefCurve> values;
};

//! Special curve element.
class ObjScrv : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjScrv, scrv),
    values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjScrv),
    values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(ObjScrv, ObjRefCurve);
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjScrv* lhs = this;
    const ObjScrv* rhs = static_cast<const ObjScrv*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! Vector of ObjRefCurve values.
  std::vector<ObjRefCurve> values;
};

//! Special points element.
class ObjSpecialPoints : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjSpecialPoints, sp),
    values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjSpecialPoints),
    values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_INT(ObjSpecialPoints);
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjSpecialPoints* lhs = this;
    const ObjSpecialPoints* rhs = static_cast<const ObjSpecialPoints*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! Vector of ObjRef values.
  std::vector<ObjRef> values;
};

//! Connection element.
class ObjConnect : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjConnect, con),
    values() {}
  GENERIC_CONSTRUCTOR_COPY(ObjConnect),
    values(rhs.values) {}
  GENERIC_CONSTRUCTOR_VECTOR_TYPE(ObjConnect, ObjRefSurface);
  GENERIC_READ_VALUES;
  GENERIC_WRITE_VALUES;
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjConnect* lhs = this;
    const ObjConnect* rhs = static_cast<const ObjConnect*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! Vector of surface reference values.
  std::vector<ObjRefSurface> values;
};

// Grouping

//! Group of elements.
class ObjGroup : public ObjGroupBase {
public:
  //! \brief Empty constructor.
  //! \param parent0 The element's parent group.
  ObjGroup(const ObjGroupBase* parent0 = nullptr) :
    ObjGroupBase("g", parent0), values() {}
  //! \copydoc ObjElement::ObjElement(const ObjElement&)
  ObjGroup(const ObjGroup& rhs) :
    ObjGroupBase(rhs), values(rhs.values) {}
  //! \copydoc ObjElement::ObjElement(const ObjElement*)
  ObjGroup(const ObjElement* rhs) :
    ObjGroup(*static_cast<const ObjGroup*>(rhs)) {}
  //! \brief Initialize an element by reading from an input stream.
  //! \param in Input stream to read from.
  //! \param parent0 The element's parent group.
  ObjGroup(std::istream &in, const ObjGroupBase* parent0 = nullptr) :
    ObjGroupBase(parent0), values() {
    read_values(in);
    from_values();
  }
  //! \brief Initialize and element from a C++ vector of values.
  //! \param values0 Vector of values.
  //! \param elements0 Vector of group elements.
  //! \param parent0 The element's parent group.
  ObjGroup(const std::vector<std::string> &values0,
	   const std::vector<ObjElement*> &elements0,
	   const ObjGroupBase* parent0 = nullptr)
    : ObjGroupBase("g", elements0, parent0), values() {
    assign_values(values, values0);
    from_values();
  }
  DUMMY_ARRAY_CONSTRUCTOR(ObjGroup);
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    ObjElement::read_values(in, values);
    read_obj_elements(in, elements, 'g', this);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    ObjElement::write_values(out, values);
    ObjGroupBase::write_values(out);
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (!ObjGroupBase::is_equal(rhs0)) return false;
    const ObjGroup* lhs = this;
    const ObjGroup* rhs = static_cast<const ObjGroup*>(rhs0);
    if (!(is_equal_vectors(lhs->values, rhs->values))) return false;
    return true;
  }
  //! Vector of element values in the group.
  std::vector<std::string> values;
};

//! Smoothing group element.
class ObjSmoothingGroup : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjSmoothingGroup, s),
    group_number(-1) {}
  GENERIC_CONSTRUCTOR_COPY(ObjSmoothingGroup),
    group_number(rhs.group_number) {}
  DUMMY_ARRAY_CONSTRUCTOR(ObjSmoothingGroup);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjSmoothingGroup);
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    std::string word;
    in >> word;
    if (word == "off")
      group_number = -1;
    else
      group_number = stoi(word);
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    if (group_number < 0)
      out << "off";
    else
      out << group_number;
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjSmoothingGroup* lhs = this;
    const ObjSmoothingGroup* rhs = static_cast<const ObjSmoothingGroup*>(rhs0);
    if (lhs->group_number != rhs->group_number) return false;
    return true;
  }
  //! Group number.
  ObjRef group_number;
};

//! Merging group.
class ObjMergingGroup : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjMergingGroup, mg),
    group_number(-1), resolution(0) {}
  GENERIC_CONSTRUCTOR_COPY(ObjMergingGroup),
    group_number(rhs.group_number), resolution(rhs.resolution) {}
  DUMMY_ARRAY_CONSTRUCTOR(ObjMergingGroup);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjMergingGroup);
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override {
    std::string word;
    in >> word;
    if (word == "off") {
      group_number = -1;
      resolution = 0.0;
    } else {
      group_number = stoi(word);
      in >> resolution;
    }
  }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override {
    if (group_number < 0)
      out << "off";
    else {
      out << group_number << " " << resolution;
    }
  }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjMergingGroup* lhs = this;
    const ObjMergingGroup* rhs = static_cast<const ObjMergingGroup*>(rhs0);
    if (lhs->group_number != rhs->group_number) return false;
    if (!(IS_EQUAL_DBL(lhs->resolution, rhs->resolution))) return false;
    return true;
  }
  //! Group number.
  ObjRef group_number;
  //! Group resolution.
  double resolution;
};

//! Object name element.
class ObjObjectName : public ObjElement {
public:
  GENERIC_CONSTRUCTOR_EMPTY(ObjObjectName, o), name("") {}
  GENERIC_CONSTRUCTOR_COPY(ObjObjectName), name(rhs.name) {}
  DUMMY_ARRAY_CONSTRUCTOR(ObjObjectName);
  GENERIC_ELEMENT_CONSTRUCTOR(ObjObjectName);
  //! \brief Initialize an object name element.
  //! \param name0 Object name.
  ObjObjectName(const std::string& name0) : ObjElement("o"), name(name0) {}
  //! \copydoc ObjElement::read_values
  void read_values(std::istream &in) override { in >> name; }
  //! \copydoc ObjElement::write_values
  void write_values(std::ostream &out) const override { out << name; }
  //! \copydoc ObjElement::is_equal
  bool is_equal(const ObjElement* rhs0) const override {
    if (rhs0->code != this->code) return false;
    const ObjObjectName* rhs = static_cast<const ObjObjectName*>(rhs0);
    if (this->name != rhs->name) return false;
    return true;
  }
  //! Object name.
  std::string name;
};

//! \brief Read an element from an input stream.
//! \param in Input stream.
//! \param parent Optional element group that will contain the read elements.
//! \return New element.
inline ObjElement* read_obj_element(std::istream &in, const ObjGroupBase* parent = nullptr) {
  std::string word = "";
  ObjElement* out = NULL;
  if (in >> word)
    OBJ_ELEMENT_INIT(word, out, (in, parent));
  return out;
};

//! \brief Read elements from an input stream until a certain character is
//!    encountered.
//! \param in Input stream.
//! \param elements Vector to read elements into.
//! \param parent Optional element group that will contain the read elements.
//! \param break_at Character to stop at.
inline void read_obj_elements(std::istream &in, std::vector<ObjElement*> &elements,
			      const char break_at, const ObjGroupBase* parent) {
  while (true) {
    if (break_at) {
      in >> std::skipws;
      if (in.peek() == break_at)
	return;
    }
    ObjElement* x = read_obj_element(in, parent);
    if (x == NULL)
      break;
    elements.push_back(x);
  }
};

//! Obj wavefront 3D geometry container class.
class ObjWavefront {
public:
  ObjWavefront() : elements() {}
  //! \brief Copy constructor.
  //! \param rhs Instance to copy.
  ObjWavefront(const ObjWavefront& rhs) : elements() {
    for (auto it = rhs.elements.begin(); it != rhs.elements.end(); it++)
      elements.push_back((*it)->copy());
  }
  //! \brief Create an ObjWavefront instance from C arrays of vertices.
  //! \tparam Tv Type of value in vertex value arrays.
  //! \tparam Mv Number of vertex elements.
  //! \tparam Nv Number of values in the array for each vertex element.
  //! \param vertices Array of vertex element value arrays.
  template<typename Tv, size_t Mv, size_t Nv>
  ObjWavefront(const Tv (&vertices)[Mv][Nv]) :
    elements() {
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
  ObjWavefront(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf]={}) :
    ObjWavefront(vertices) {
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
  ObjWavefront(const Tv (&vertices)[Mv][Nv], const Tf (&faces)[Mf][Nf]={},
	       const Te (&edges)[Me][Ne]={}) :
    ObjWavefront(vertices, faces) {
    add_element_set("l", edges);
  }
  ~ObjWavefront() {
    for (auto it = elements.begin(); it != elements.end(); it++)
      delete *it;
    // elements.resize(0);
  }
  //! All sets of elements in the 3D mesh.
  std::vector<ObjElement*> elements;

  //! \brief Add a set of elements to the geometry from a C array of value
  //!   arrays.
  //! \tparam T Type of value in the values array.
  //! \tparam M Number of elements being added.
  //! \tparam N Number of entries in each element's values array.
  //! \param name Name of the type of elements being added.
  //! \param values Array of value arrays defining the elements.
  template <typename T, size_t M, size_t N>
  void add_element_set(std::string name, const T (&values)[M][N]) {
    for (SizeType i = 0; i < M; i++)
      add_element(name, values[i]);
  }
  //! \brief Add an element to the geometry from a C array of values.
  //! \tparam T Type of value in the values array.
  //! \tparam N Number of elements in the values array.
  //! \param name Name of the type of element being added.
  //! \param values Array of values defining the element.
  template <typename T, size_t N>
  void add_element(std::string name, const T (&values)[N]) {
    ObjElement* x = NULL;
    OBJ_ELEMENT_INIT(name, x, (std::vector<T>(values, values+N)));
    elements.push_back(x);
  }
  //! \brief Add an element to the geometry from a C++ vector of values.
  //! \tparam T Type of value in the values vector.
  //! \param name Name of the type of element being added.
  //! \param values Vector of values defining the element.
  template <typename T>
  void add_element(std::string name, const std::vector<T> &values) {
    ObjElement* x = NULL;
    OBJ_ELEMENT_INIT(name, x, (values));
    elements.push_back(x);
  }

  //! \brief Add an element to the geometry.
  //! \param x New element.
  void add_element(const ObjElement& x) {
    ObjElement* x_cpy = NULL;
    OBJ_ELEMENT_INIT(x.code, x_cpy, (&x));
    elements.push_back(x_cpy);
  }
  //! \brief Add an element to the geometry.
  //! \tparam T Type of value in the values vector.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param values Vector of additional values defining the element.
  template <typename T>
  void add_element(const std::string name,
		   const double& u0, const double& u1,
		   const std::vector<T> &values) {
    ObjElement* x = NULL;
    if (name == "curv") x = new ObjCurve(u0, u1, values);
    else RAPIDJSON_ASSERT(!sizeof(std::string("Unsupported element signifier: ") + name));
    elements.push_back(x);
  }
  //! \brief Add an element to the geometry from a C array of values.
  //! \tparam T Type of value in the values array.
  //! \tparam N Number of elements in the values array.
  //! \param name Name of the type of element being added.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param values Array of values defining the element.
  template <typename T, size_t N>
  void add_element(std::string name,
		   const double& u0, const double& u1,
		   const T (&values)[N]) {
    add_element(name, u0, u1, std::vector<T>(values, values+N));
  }
  //! \brief Add an element to the geometry.
  //! \tparam T Type of value in the values vector.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param u2 3rd element parameter.
  //! \param u3 4th element parameter.
  //! \param values Vector of additional values defining the element.
  template <typename T>
  void add_element(const std::string name,
		   const double& u0, const double& u1,
		   const double& u2, const double& u3,
		   const std::vector<T> &values) {
    ObjElement* x = NULL;
    if (name == "surf") x = new ObjSurface(u0, u1, u2, u3, values);
    else RAPIDJSON_ASSERT(!sizeof(std::string("Unsupported element signifier: ") + name));
    elements.push_back(x);
  }
  //! \brief Add an element to the geometry from a C array of values.
  //! \tparam T Type of value in the values array.
  //! \tparam N Number of elements in the values array.
  //! \param name Name of the type of element being added.
  //! \param u0 1st element parameter.
  //! \param u1 2nd element parameter.
  //! \param u2 3rd element parameter.
  //! \param u3 4th element parameter.
  //! \param values Array of values defining the element.
  template <typename T, size_t N>
  void add_element(std::string name,
		   const double& u0, const double& u1,
		   const double& u2, const double& u3,
		   const T (&values)[N]) {
    add_element(name, u0, u1, u2, u3, std::vector<T>(values, values+N));
  }

  //! \brief Check if another ObjWavefront instance is equivalent by
  //!   comparing elements.
  //! \param rhs Other instance for comparison.
  //! \return true if rhs is equivalent.
  bool is_equal(const ObjWavefront& rhs) const {
    const ObjWavefront& lhs = *this;
    if (lhs.elements.size() != rhs.elements.size())
      return false;
    for (auto lit = lhs.elements.begin(), rit = rhs.elements.begin();
	 lit != lhs.elements.end(); lit++, rit++) {
      if (!((*lit)->is_equal(*rit)))
	return false;
    }
    return true;
  }
  //! \brief Write elements to an output stream.
  //! \param out Output stream.
  //! \return Output stream.
  std::ostream & write(std::ostream &out) const {
    for (auto it = elements.begin(); it != elements.end(); it++)
      (*it)->write(out);
    return out;
  }
  //! \brief Read elements from an input stream.
  //! \param in Input stream.
  //! \return Input stream.
  std::istream & read(std::istream &in) {
    read_obj_elements(in, elements);
    return in;
  }
  friend bool operator == (const ObjWavefront& lhs, const ObjWavefront& rhs);
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
{ return lhs.is_equal(rhs); };

//! \brief Write an ObjWavefront object to an output stream.
//! \param out Output stream.
//! \param p ObjWavefront object.
//! \return Output stream.
inline
std::ostream & operator << (std::ostream &out, const ObjWavefront &p)
{ return p.write(out); };

//! \brief Read an ObjWavefront object from an input stream.
//! \param in Input stream.
//! \param p ObjWavefrotn object.
//! \return Input stream.
inline
std::istream & operator >> (std::istream &in, ObjWavefront &p)
{ return p.read(in); };

#undef GENERIC_WRITE_VALUES
#undef GENERIC_READ_VALUES
#undef DUMMY_ARRAY_CONSTRUCTOR
#undef GENERIC_CONSTRUCTOR_VECTOR_TYPE
#undef GENERIC_CONSTRUCTOR_VECTOR_VERT
#undef GENERIC_CONSTRUCTOR_VECTOR_FLOAT
#undef GENERIC_CONSTRUCTOR_VECTOR_INT
#undef GENERIC_CONSTRUCTOR_VECTOR_UINT
#undef COMPATIBLE_WITH_TYPE
#undef COMPATIBLE_WITH_VERT
#undef COMPATIBLE_WITH_FLOAT
#undef COMPATIBLE_WITH_INT
#undef COMPATIBLE_WITH_UINT
#undef C_ARRAY_CONSTRUCTOR
#undef GENERIC_ELEMENT_CONSTRUCTOR
#undef OBJ_ELEMENT_INIT
#undef IS_EQUAL_DBL
#undef IS_EQUAL_FLT

RAPIDJSON_NAMESPACE_END

#endif // RAPIDJSON_OBJ_H_
