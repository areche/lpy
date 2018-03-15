/* ---------------------------------------------------------------------------
 #
 #       L-Py: L-systems in Python
 #
 #       Copyright 2003-2008 UMR Cirad/Inria/Inra Dap - Virtual Plant Team
 #
 #       File author(s): F. Boudon (frederic.boudon@cirad.fr)
 #
 # ---------------------------------------------------------------------------
 #
 #                      GNU General Public Licence
 #
 #       This program is free software; you can redistribute it and/or
 #       modify it under the terms of the GNU General Public License as
 #       published by the Free Software Foundation; either version 2 of
 #       the License, or (at your option) any later version.
 #
 #       This program is distributed in the hope that it will be useful,
 #       but WITHOUT ANY WARRANTY; without even the implied warranty of
 #       MERCHANTABILITY or FITNESS For A PARTICULAR PURPOSE. See the
 #       GNU General Public License for more details.
 #
 #       You should have received a copy of the GNU General Public
 #       License along with this program; see the file COPYING. If not,
 #       write to the Free Software Foundation, Inc., 59
 #       Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 #
 # ---------------------------------------------------------------------------
 */

#ifndef __LSYS_MODULES_H__
#define __LSYS_MODULES_H__

#include "error.h"
#include "lpy_variant.h"
#include <vector>
#ifdef USING_PYTHON
#include <plantgl/python/boost_python.h>
#endif
#include <plantgl/tool/util_string.h>

#include "moduleclass.h"
#include "argcollector.h"
#include <iostream>

#ifndef LPY_WITHOUT_QT
#include <QtCore/QSharedData>
#else
#include "copy_on_write_ptr.h"
#include "mutex_flag.h"
#endif

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/
class ConsiderFilter;
typedef RCPtr<ConsiderFilter> ConsiderFilterPtr;

class LPY_API Module {
public:
   friend class MatchingEngine;
   friend class MatchingEngineImplementation;

  Module();
  Module(const std::string& c);
  Module(size_t classid);
  Module(const ModuleClassPtr m);
  Module(const Module&);

  virtual ~Module();

  inline const std::string& name() const {  return __mclass->name; }
  inline void setName(const std::string& c) { __mclass = ModuleClassTable::get().getClass(c); }
  inline bool sameName(const Module& m) const { return __mclass == m.__mclass;  }

  inline ModuleClassPtr getClass() const { lpyassert(__mclass != NULL); return __mclass; }
  inline size_t getClassId() const { return getClass()->getId(); }
  inline bool isinstance(const ModuleClassPtr& mclass) const 
  { return __mclass->issubclass(mclass) ;  }

  virtual std::string str() const;
  inline const char* c_str() const { return str().c_str(); }
  virtual std::string repr() const;
  inline bool operator==(const Module& n) const { return sameName(n); }
  inline bool operator!=(const Module& other) const { return !operator==(other); }

  inline bool isLeftBracket() const {  return __mclass == ModuleClass::LeftBracket; }
  inline bool isRightBracket() const {  return __mclass == ModuleClass::RightBracket; }
  inline bool isExactRightBracket() const { return __mclass == ModuleClass::ExactRightBracket; }
  inline bool isBracket() const { return isLeftBracket() || isRightBracket() || isExactRightBracket(); }
  inline bool isRequest() const { 
		return __mclass == ModuleClass::QueryPosition || 
			   __mclass == ModuleClass::QueryHeading ||
			   __mclass == ModuleClass::QueryUp || 
			   __mclass == ModuleClass::QueryLeft || 
		       __mclass == ModuleClass::QueryRigth || 
		       __mclass == ModuleClass::QueryFrame; }
  inline bool isCut() const { return __mclass == ModuleClass::Cut; }
  inline bool isNone() const { return __mclass == ModuleClass::None; }
  inline bool isNull() const { return __mclass == ModuleClass::None || __mclass == ModuleClass::Star; }
  inline bool isStar() const { return __mclass == ModuleClass::Star; }
  inline bool isRepExp() const { return __mclass == ModuleClass::RepExp; }
  inline bool isOr() const { return __mclass == ModuleClass::Or; }
  inline bool isRE() const { return isRepExp() || isOr(); }
  inline bool isGetIterator() const { return __mclass == ModuleClass::GetIterator; }
  inline bool isGetModule() const { return __mclass == ModuleClass::GetModule; }
  bool isConsidered(const ConsiderFilterPtr filter) const;
  bool isIgnored(const ConsiderFilterPtr filter) const;

  inline int scale() const { return __mclass->getScale(); }
protected:
  inline void setClass(size_t cid) { __mclass = ModuleClassTable::get().find(cid); }
private:
  ModuleClassPtr __mclass;
};

/*---------------------------------------------------------------------------*/

#ifndef UNITY_MODULE
LPY_API extern LpyObject getFunctionRepr();
#endif

template<class Parameter>
class AbstractParamModule : public Module {
public:
	typedef Parameter ParameterType;
	typedef std::vector<ParameterType> ParameterList;
	typedef AbstractParamModule<ParameterType> BaseType;
	typedef typename ParameterList::iterator iterator;
	typedef typename ParameterList::const_iterator const_iterator;
	typedef typename ParameterList::reverse_iterator reverse_iterator;
	typedef typename ParameterList::const_reverse_iterator const_reverse_iterator;

	AbstractParamModule() : __argholder(new  ParamModuleInternal()) { }
	AbstractParamModule(const std::string& name) : Module(name), __argholder(new  ParamModuleInternal()) {}
    AbstractParamModule(size_t classid) : Module(classid), __argholder(new  ParamModuleInternal()) {}
    AbstractParamModule(const ModuleClassPtr mclass) : Module(mclass), __argholder(new  ParamModuleInternal()) {}

	~AbstractParamModule() { }

	inline size_t size() const { return __constargs().size(); }
	inline bool empty() const  { return __constargs().empty(); }

	/*attribute_deprecated*/ inline size_t argSize() const { return __constargs().size(); }
	/*attribute_deprecated*/ inline bool hasArg() const  { return !__constargs().empty(); }

    inline LpyObjectList getPyArgs() const
	{ return toPyList(__constargs()); }
	
    inline void setPyArgs(const LpyObjectList& l)
	{ __args() = toParameterList(l); }

    inline const ParameterType& getAt(size_t i) const 
	{ lpyassert(size() > i); return __constargs()[i]; }

    inline ParameterType& getAt(size_t i) 
	{ lpyassert(size() > i); return __args()[i]; }

    inline void setAt(size_t i, const ParameterType& value) 
	{ lpyassert(size() > i); __args()[i] = value; }

	inline void delAt(size_t i) 
	{ lpyassert(size() > i); __args().erase(__args().begin()+i); }

    inline ParameterType getItemAt(int i) const 
	{ return getAt(getValidIndex(i));  }

    inline void setItemAt(int i, const ParameterType& value)
	{ setAt(getValidIndex(i),value);  }

    inline void delItemAt(int i) 
	{ delAt(getValidIndex(i));  }

    LpyObjectList getSlice(size_t i, size_t j) const {
		lpyassert( i <= j && j <= size() );
        LpyObjectList res;
        for(const_iterator it = __constargs().begin()+i;
            it != __constargs().begin()+j; ++it) addToList(res,*it);
        return res;
	}

	inline void delSlice(size_t i, size_t j) 
	{ lpyassert( i <= j && j <= size() );
	  __args().erase(__args().begin()+i,__args().begin()+j); }

    inline LpyObjectList getSliceItemAt(int i, int j) const
	{ size_t ri, rj; getValidIndices(i,j,ri,rj) ; return getSlice(ri,rj);  }

    inline void delSliceItemAt(int i, int j)
	{ size_t ri, rj; getValidIndices(i,j,ri,rj) ; return delSlice(ri,rj);  }

	inline void append(const ParameterType& value)  { __args().push_back(value); }
	inline void prepend(const ParameterType& value) { __args().insert(__args().begin(), value); }

	inline bool operator==(const BaseType& other) const 
	{ return (sameName(other) && (__argholder == other.__argholder || __constargs() == other.__constargs())); }

    inline bool operator!=(const BaseType& other) const { return !operator==(other); }

#ifndef UNITY_MODULE
    virtual std::string str() const {
		std::string st = name();
		if(!empty())st +='('+strArg()+')'; 
		return st;
	}

    inline LpyTuple toTuple() const {
        LpyTuple res(name());
        res += LpyTuple(toPyList(__constargs()));
        return res;
    }

    virtual std::string repr() const {
        std::string st = name();
        if(!empty())st +='('+reprArg()+')';
        return st;
    }

    std::string reprArg() const
    {
        boost::python::str res(",");
        boost::python::list argstr;
        boost::python::object repr = getFunctionRepr();
        for (const_iterator it = __constargs().begin(); it != __constargs().end(); ++it)
        {
            LpyObject str = repr(*it);
            std::string s = boost::python::extract<std::string>(str);
            std::cout << s.c_str();
            argstr.append(str);
        }
        return boost::python::extract<std::string>(res.join(argstr));
    }

    virtual std::string strArg() const = 0;
#endif

    inline std::vector<std::string> getParameterNames() const
    { return getClass()->getParameterNames(); }

	inline size_t getNamedParameterNb() const
    { return getClass()->getNamedParameterNb(); }

    inline size_t getParameterPosition(const std::string& pname) const  
    { 
        int firstnamedparameter = std::max<int>(0,size() - getNamedParameterNb());
        return firstnamedparameter + getClass()->getParameterPosition(pname); 
    }

    inline bool hasParameter(const std::string& pname) const  
    { return getClass()->hasParameter(pname); }

    inline ParameterType getParameter(const std::string& pname) const
	{ return getAt(getValidParameterPosition(pname)); }

    inline void setParameter(const std::string& pname, const ParameterType& value)
	{ return setAt(getValidParameterPosition(pname),value); }

    inline const ParameterList& getParameterList() const { return __constargs(); }

    void getNamedParameters(LpyObjectMap& parameters, size_t fromIndex = 0) const
    {
        int firstnamedparameter = std::max<int>(0,size() - getNamedParameterNb());
        const ParameterNameDict& pnames = getClass()->getParameterNameDict();
        for(ParameterNameDict::const_iterator itp = pnames.begin(); itp != pnames.end(); ++itp){
            if(itp->second >= fromIndex) {
#ifndef USING_PYTHON
                parameters[itp->first] = getAt(firstnamedparameter+itp->second);
#else
                parameters[boost::python::object(itp->first)] = getAt(firstnamedparameter+itp->second);
#endif
            }
        }
    }

protected:
     inline size_t getValidParameterPosition(const std::string& pname) const
	 {
		size_t pos = getParameterPosition(pname);
		if (pos == ModuleClass::NOPOS) LsysError("Invalid parameter name '"+pname+"' for module '"+name()+"'.");
		if (pos >= size()) LsysError("Inexisting value for parameter '"+pname+" '("+TOOLS(number)(pos)+") for module '"+name()+"'.");
		return pos;
	 }

	 inline size_t getValidIndex(int i) const {
		size_t s = size();
		if( i < 0 ) i += s;
        assert ((i>=0) && (i<s) && "index out of range");
		return (size_t)i;
	 }

	 inline void getValidIndices(int i, int j, size_t& resi, size_t& resj) const {
		size_t s = size();
		if( i < 0 ) i += s;
		if( j < 0 ) j += s;
	    if( j > s ) j = s;
        assert (i >= 0  && i < s && j >= i && "index out of range");
		resi =(size_t)i;
		resj =(size_t)j;
	 }

 template<class PModule>
 struct LPY_API ParamListInternal 
 #ifndef LPY_WITHOUT_QT
     : public QSharedData 
 #endif
 {
	 typedef PModule ParamModule;
	 typedef typename ParamModule::ParameterList ParameterList;

     ParamListInternal() {}
     ParamListInternal(const ParamListInternal& other) : __args(other.__args) { }
     ~ParamListInternal() {}

     ParameterList __args;
 };
 
 typedef ParamListInternal<BaseType> ParamModuleInternal;
#ifndef LPY_WITHOUT_QT
 typedef QSharedDataPointer<ParamModuleInternal> ParamModuleInternalPtr;
#else
 typedef copy_on_write_ptr<ParamModuleInternal, cow_ownership_flags::mutex_flag> ParamModuleInternalPtr;
#endif

  ParamModuleInternalPtr __argholder;

  inline ParameterList& __args() { return __argholder->__args; }
  inline const ParameterList& __args() const { return __argholder->__args; }
  inline const ParameterList& __constargs() const { return __argholder->__args; }

  static inline LpyObjectList toPyList(const ParameterList& pl) {
    LpyObjectList result;
	for(const_iterator it = pl.begin(); it != pl.end(); ++it)
      addToList(result,*it);
    return result;
  }

  static inline ParameterList toParameterList(const LpyObject& t){
    ParameterList result;
#ifndef USING_PYTHON
    if (is<LpyObjectList>(t)) {
        LpyObjectList list = get<LpyObjectList>(t);
        for(LpyObject v: list)
            result.push_back(v);
    }
    else {
        LsysError("The argument passed to toParameterList is not a LpyObjectList");
    }
#else
    bp::object iter_obj = bp::object( bp::handle<>( PyObject_GetIter( t.ptr() ) ) );
    try {
        while( 1 )
            result.push_back(bp::extract<ParameterType>(iter_obj.attr( "next" )())());
    }
    catch( bp::error_already_set ){
        PyErr_Clear();
    }
#endif
    return result;
 }

};

/*---------------------------------------------------------------------------*/

class PatternModule;

/*---------------------------------------------------------------------------*/

class LPY_API ParamModule : public AbstractParamModule<LpyObject> {
public:
  friend class ParametricProduction;

  typedef LpyObject Parameter;
  typedef AbstractParamModule<Parameter> BaseType;

  ParamModule(const std::string& name);
  ParamModule(const ParamModule& name);
  ParamModule(size_t classid);
  ParamModule(size_t classid, const std::string& args);

#ifndef UNITY_MODULE
  ParamModule(LpyTuple t);
#endif

  ParamModule(LpyObjectList t);

  ParamModule(const std::string& name, 
              const LpyObjectList& args);
  ParamModule(size_t classid, 
              const LpyObjectList& args);

#ifndef UNITY_MODULE
  ParamModule(const ModuleClassPtr m,
              const LpyTuple& args);
#endif

  ParamModule(const std::string& name, 
              const Parameter& args);

  ParamModule(const std::string& name, 
              const Parameter&,
              const Parameter&);

  ParamModule(const std::string& name, 
              const Parameter&,
              const Parameter&,
              const Parameter&);

  ParamModule(const std::string& name, 
              const Parameter&,
              const Parameter&,
              const Parameter&,
              const Parameter&);

  ParamModule(const std::string& name, 
              const Parameter&,
              const Parameter&,
              const Parameter&,
              const Parameter&,
              const Parameter&);

  virtual ~ParamModule();

  int  _getInt(int)   const;
  real_t _getReal(int) const;
  bool _getBool(int) const;
  std::string _getString(int) const;

  template<class T>
  T _get(int i) const { return get<T>(getAt(i)); }

  template<class T>
  bool _check(int i) const { return is<T>(getAt(i)); }

  virtual void _setValues(real_t,real_t,real_t) ;
  inline void _setValues(const TOOLS(Vector3)& v) { _setValues(v.x(),v.y(),v.z()); }
  virtual void _setFrameValues(const TOOLS(Vector3)& p, const TOOLS(Vector3)& h, const TOOLS(Vector3)& u, const TOOLS(Vector3)& l) ;

  bool match(const PatternModule&m) const;
  bool match(const PatternModule&m, ArgList&) const;
  bool match(const std::string&, size_t nbargs) const;

  inline void interpret(PGL::Turtle& t) { LsysWarning("Module::Interpret"); getClass()->interpret(*this,t); }

  inline bool operator==(const ParamModule& other) const 
  { return (sameName(other) && (__argholder == other.__argholder || __constargs() == other.__constargs())); }
  inline bool operator==(const std::string& other) const 
  { return other == name();}

  inline bool operator!=(const ParamModule& other) const { return !operator==(other); }
  inline bool operator!=(const std::string& other) const 
  { return other != name();}

  void appendArgumentList(const LpyObject& arglist) ;

#ifndef UNITY_MODULE
  virtual std::string strArg() const
  {
//#ifndef USING_PYTHON
//        return toString(__constargs());
//#else
        boost::python::str res(",");
        boost::python::list argstr;
        for (const_iterator it = __constargs().begin(); it != __constargs().end(); ++it)
            argstr.append(boost::python::str(*it));
        return boost::python::extract<std::string>(res.join(argstr));
//#endif
  }
#endif

protected:
  ParamModule();

};

/*---------------------------------------------------------------------------*/

inline bool is_lower_scale(int scale1, int scale2) { return scale1 < scale2; }
inline bool is_eq_scale(int scale1, int scale2)    { return scale1 == scale2; }
inline bool is_neq_scale(int scale1, int scale2)    { return scale1 != scale2; }
inline bool is_upper_scale(int scale1, int scale2) { return scale1 > scale2; }
inline int up_scale(int scale1) { return scale1 + 1; }
inline int down_scale(int scale1) { return scale1 - 1; }
// See Also ModuleClass::DEFAULT_SCALE

/*---------------------------------------------------------------------------*/


LPY_END_NAMESPACE

/*---------------------------------------------------------------------------*/
#endif

#include "consider.h"
