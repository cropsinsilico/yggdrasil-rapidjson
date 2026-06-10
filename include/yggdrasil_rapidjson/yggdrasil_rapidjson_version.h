///////////////////////////////////////////////////////////////////////////////
// YGGDRASIL_RAPIDJSON_VERSION_STRING
//
#ifndef YGGDRASIL_RAPIDJSON_VERSION_H_
#define YGGDRASIL_RAPIDJSON_VERSION_H_

//!@cond YGGDRASIL_RAPIDJSON_HIDDEN_FROM_DOXYGEN
// token stringification
#define YGGDRASIL_RAPIDJSON_STRINGIFY(x) YGGDRASIL_RAPIDJSON_DO_STRINGIFY(x)
#define YGGDRASIL_RAPIDJSON_DO_STRINGIFY(x) #x
//!@endcond

/*! \def YGGDRASIL_RAPIDJSON_MAJOR_VERSION
    \ingroup YGGDRASIL_RAPIDJSON_CONFIG
    \brief Major version of RapidJSON in integer
    that this version of YggdrasilRapidJSON is based on.
*/
/*! \def YGGDRASIL_RAPIDJSON_MINOR_VERSION
    \ingroup YGGDRASIL_RAPIDJSON_CONFIG
    \brief Minor version of RapidJSON in integer
    that this version of YggdrasilRapidJSON is based on.
*/
/*! \def YGGDRASIL_RAPIDJSON_PATCH_VERSION
    \ingroup YGGDRASIL_RAPIDJSON_CONFIG
    \brief Patch version of RapidJSON in integer
    that this version of YggdrasilRapidJSON is based on.
*/
/*! \def YGGDRASIL_RAPIDJSON_EXTEN_VERSION
    \ingroup YGGDRASIL_RAPIDJSON_CONFIG
    \brief Version of YggdrasilRapidJSON based on RapidJSON.
*/
/*! \def YGGDRASIL_RAPIDJSON_VERSION_STRING
    \ingroup YGGDRASIL_RAPIDJSON_CONFIG
    \brief Version of RapidJSON in "<major>.<minor>.<patch>.<exten>" string format.
*/
#define YGGDRASIL_RAPIDJSON_MAJOR_VERSION 1
#define YGGDRASIL_RAPIDJSON_MINOR_VERSION 1
#define YGGDRASIL_RAPIDJSON_PATCH_VERSION 0
#define YGGDRASIL_RAPIDJSON_EXTEN_VERSION 4
#define YGGDRASIL_RAPIDJSON_VERSION_STRING \
    YGGDRASIL_RAPIDJSON_STRINGIFY(YGGDRASIL_RAPIDJSON_MAJOR_VERSION.YGGDRASIL_RAPIDJSON_MINOR_VERSION.YGGDRASIL_RAPIDJSON_PATCH_VERSION.YGGDRASIL_RAPIDJSON_EXTEN_VERSION)

#endif // YGGDRASIL_RAPIDJSON_VERSION_H_
