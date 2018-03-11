#ifndef VARIANT_H
#define VARIANT_H

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_LIST_SIZE 30

typedef enum {
    LPY_INT,
    LPY_BOOL,
    LPY_SIZE,
    LPY_REAL,
    LPY_UCHAR,
    LPY_UINT,
    LPY_STRING,
    LPY_DEQUE,
    LPY_LIST,
    LPY_VECTOR,
    LPY_MAP,
    LPY_PARAMMODULE,
    LPY_AXIALTREE,
    LPY_VECTOR2,
    LPY_VECTOR3,
    LPY_VECTOR4,
    LPY_QUANTISED_FUNC,
    LPY_APPEARANCE,
    LPY_MATERIAL,
    LPY_GEOMETRY,
    LPY_SHAPE,
    LPY_SCENE,
    LPY_CURVE2D,
    LPY_LINEICMODEL
} LpyType;

#ifndef USING_PYTHON

#include <boost/variant.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/variant/variant_fwd.hpp>
#include <list>
#include <queue>
#include <numeric>
#include <map>

#include <plantgl/scenegraph/function/function.h>
#include <plantgl/scenegraph/geometry/lineicmodel.h>
#include <plantgl/scenegraph/appearance/appearance.h>
#include <plantgl/scenegraph/appearance/material.h>
#include <plantgl/scenegraph/scene/scene.h>
#include <plantgl/math/util_vector.h>
#include "lpy_config.h"

LPY_BEGIN_NAMESPACE
class ParamModule;
class AxialTree;
LPY_END_NAMESPACE

typedef boost::make_recursive_variant<
            int,
            bool,
#ifndef ANDROID
            size_t,
#endif
            real_t,
            uchar_t,
            uint32_t,
            std::string,
            std::deque< boost::recursive_variant_ >,
//            std::list< boost::recursive_variant_ >,
            std::vector< boost::recursive_variant_ >,
            std::map< std::string, boost::recursive_variant_ >,
            LPY::ParamModule *,
            LPY::AxialTree *,
            TOOLS::Vector2,
            TOOLS::Vector3,
            TOOLS::Vector4,
            PGL::QuantisedFunctionPtr,
            PGL::AppearancePtr,
            PGL::MaterialPtr,
            PGL::GeometryPtr,
            PGL::ShapePtr,
            PGL::ScenePtr,
            PGL::Curve2DPtr,
            PGL::LineicModelPtr>::type                         LpyObject;

typedef std::vector<LpyObject>                                 LpyObjectList;
//typedef std::vector<LpyObject>                                 LpyObjectVector;
typedef std::deque<LpyObject>                                  LpyObjectDeque;

//typedef boost::variant<std::string, int, bool>                 LpyMapKeyType;
typedef std::string                                            LpyMapKeyType;
typedef std::map<LpyMapKeyType, LpyObject>                     LpyObjectMap;

inline void addToList(LpyObjectList & list, const LpyObject & obj)
{
    list.push_back(obj);
}

inline size_t length(const LpyObjectList & list)
{
    return list.size();
}

template<typename T>
inline bool is(const LpyObject & o) {
    try {
        T v = boost::get<T>(o);
        return true;
    }
    catch (boost::exception & e)
    {
        return false;
    }
}

template<typename T>
inline T get(const LpyObject & o) {
    return boost::get<T>(o);
}

class LpyVariantType : public boost::static_visitor<LpyType>
{
public:
    LpyType operator()(int)     const { return LPY_INT;  }
    LpyType operator()(bool)    const { return LPY_BOOL; }
#ifndef ANDROID
    LpyType operator()(size_t)  const { return LPY_SIZE; }
#endif
    LpyType operator()(real_t)  const { return LPY_REAL; }
    LpyType operator()(uchar_t) const { return LPY_UCHAR; }
    LpyType operator()(uint32_t)    const { return LPY_UINT; }

    LpyType operator()(const std::string &)             const { return LPY_STRING;         }
    LpyType operator()(const LpyObjectDeque &)          const { return LPY_DEQUE;          }
    LpyType operator()(const LpyObjectList &)           const { return LPY_LIST;           }
//    LpyType operator()(const LpyObjectVector &)         const { return LPY_VECTOR;         }
    LpyType operator()(const LpyObjectMap &)            const { return LPY_MAP;            }
    LpyType operator()(const LPY::ParamModule *)        const { return LPY_PARAMMODULE;    }
    LpyType operator()(const LPY::AxialTree *)          const { return LPY_AXIALTREE;      }
    LpyType operator()(const TOOLS::Vector2)            const { return LPY_VECTOR2;        }
    LpyType operator()(const TOOLS::Vector3)            const { return LPY_VECTOR3;        }
    LpyType operator()(const TOOLS::Vector4)            const { return LPY_VECTOR4;        }
    LpyType operator()(const PGL::QuantisedFunctionPtr) const { return LPY_QUANTISED_FUNC; }
    LpyType operator()(const PGL::AppearancePtr)        const { return LPY_APPEARANCE;     }
    LpyType operator()(const PGL::MaterialPtr)          const { return LPY_MATERIAL;       }
    LpyType operator()(const PGL::GeometryPtr)          const { return LPY_GEOMETRY;       }
    LpyType operator()(const PGL::ShapePtr)             const { return LPY_SHAPE;          }
    LpyType operator()(const PGL::ScenePtr)             const { return LPY_SCENE;          }
    LpyType operator()(const PGL::Curve2DPtr)           const { return LPY_CURVE2D;        }
    LpyType operator()(const PGL::LineicModelPtr)       const { return LPY_LINEICMODEL;    }
};

inline LpyType getType(const LpyObject & o) {
    return boost::apply_visitor( LpyVariantType(), o );
}

class LpyVariantStringfier : public boost::static_visitor<std::string>
{
public:
    std::string operator()(const LpyObjectDeque & item) const
    {
        if (item.empty())
            return "()";
        return "(" + std::accumulate(std::next(item.begin()), item.end(),
                                     boost::apply_visitor(LpyVariantStringfier(), item[0]), // start with first element
                                     [](const std::string & a, const LpyObject & b) {
                                        return a + ',' + boost::apply_visitor(LpyVariantStringfier(), b);
                                     }) + ")";
    }

    std::string operator()(const LpyObjectList & item) const
    {
        if (item.empty())
            return "()";
        return "(" + std::accumulate(std::next(item.begin()), item.end(),
                                     boost::apply_visitor(LpyVariantStringfier(), *item.begin()), // start with first element
                                     [](const std::string & a, const LpyObject & b) {
                                        return a + ',' + boost::apply_visitor(LpyVariantStringfier(), b);
                                     }) + ")";
    }

//    std::string operator()(const LpyObjectVector & item) const
//    {
//        if (item.empty())
//            return "[]";
//        return "[" + std::accumulate(std::next(item.begin()), item.end(),
//                                     boost::apply_visitor(LpyVariantStringfier(), item[0]), // start with first element
//                                     [](const std::string & a, const LpyObject & b) {
//                                        return a + ',' + boost::apply_visitor(LpyVariantStringfier(), b);
//                                     }) + "]";
//    }

    std::string operator()(const LpyObjectMap & item) const
    {
        if (item.empty())
            return "{}";
        return "{" + std::accumulate(std::next(item.begin()), item.end(),
                                     item.begin()->first + ":" + boost::apply_visitor(LpyVariantStringfier(), item.begin()->second), // start with first element
                                     [](const std::string & a, const LpyObjectMap::value_type & b) {
                                        return a + ',' + b.first + ":" + boost::apply_visitor(LpyVariantStringfier(), b.second);
                                     }) + "}";
    }

    std::string operator()(const LPY::ParamModule * item) const
    {
        return "";
    }

    std::string operator()(const LPY::AxialTree * item) const
    {
        return "";
    }

    std::string operator()(const PGL::QuantisedFunctionPtr item) const
    {
        return "";
    }

    std::string operator()(const PGL::AppearancePtr item) const
    {
        return "";
    }

    template <typename T>
    std::string operator()(const T & item) const
    {
        return boost::lexical_cast<std::string>(item);
    }
};

inline std::string toString(const LpyObject & o) {
    return boost::apply_visitor(LpyVariantStringfier(), o);
}

typedef LpyVariantStringfier                                   LpyObjectStringfier;

#else

#include <boost/python.hpp>

typedef boost::python::object                                  LpyObject;
typedef boost::python::object                                  LpyObjectStringfier;

// #define USE_PYTHON_LIST_COLLECTOR
typedef boost::python::list                                    LpyObjectList;

#ifndef USE_PYTHON_LIST_COLLECTOR
#define USE_OBJECTVEC_COLLECTOR
#endif
typedef std::vector<LpyObject>                                 LpyObjectVector;
typedef std::deque<LpyObject>                                  LpyObjectDeque;
typedef boost::python::dict                                    LpyObjectMap;
typedef boost::python::tuple                                   LpyTuple;

inline void addToList(LpyObjectList & list, const LpyObject & obj)
{
    list.append(obj);
}

inline size_t length(const LpyObjectList & list)
{
    return boost::python::len(list);
}

template<typename T>
inline bool is(const LpyObject & o) {
    boost::python::extract<T> ex(o);
    return ex.check();
}

template<typename T>
inline T get(const LpyObject & o) {
    boost::python::extract<T> ex(o);
    assert(ex.check());
    return ex();
}

inline LpyType getType(const LpyObject & o) {
    assert(false);
}

inline std::string toString(const LpyObject & o) {
    return boost::python::extract<std::string>(boost::python::str(o))();
//    boost::python::str res(",");
//    boost::python::list argstr;
//    for (const_iterator it = __constargs().begin(); it != __constargs().end(); ++it)
//        argstr.append(boost::python::str(*it));
//    return boost::python::extract<std::string>(res.join(argstr));
}

#endif

inline bool isContainer(const LpyObject & o) {
    return is<LpyObjectDeque>(o) || is<LpyObjectList>(o) || is<LpyObjectMap>(o);
}

#endif // VARIANT_H
