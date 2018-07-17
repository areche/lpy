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


#include "module.h"
#include "lsyscontext.h"

#ifndef UNITY_MODULE
#include "patternmodule.h"
#include "matching.h"
#include "lpy_parser.h"
#include <strstream>
using namespace boost::python;
#endif

#include "tracker.h"
#include "packedargs.h"
#include <plantgl/math/util_vector.h>

LPY_USING_NAMESPACE

Module::Module() : 
	__mclass(ModuleClass::None)
{ IncTracker(Module) }

Module::Module(const std::string& c) : 
	__mclass(ModuleClassTable::get().getClass(c))
{ IncTracker(Module) }

Module::Module(const Module& m) :
	__mclass(m.__mclass)
{ IncTracker(Module) }

Module::Module(size_t classid):
    __mclass(ModuleClassTable::get().find(classid))
{ IncTracker(Module) }

Module::Module(const ModuleClassPtr m):
    __mclass(m)
{ IncTracker(Module) }

Module::~Module()
{ 
	DecTracker(Module)
	//  std::cerr << "Delete Module" << std::endl;
}


std::string 
Module::str() const 
{ return name(); }


std::string 
Module::repr() const 
{ return name(); }


/*---------------------------------------------------------------------------*/

#ifndef UNITY_MODULE
LpyObject LPY::getFunctionRepr() { return GlobalContext::getFunctionRepr(); }
#endif

/*---------------------------------------------------------------------------*/

// #include <iostream>

#define appendParam(args,p) args.push_back(p)

/*---------------------------------------------------------------------------*/

void processArgList(ModuleClassPtr mclass, ParamModule::ParameterList& args, LpyObject arg, size_t start = 0)
{
#ifndef USING_PYTHON
    try {
        LpyObjectMap map = boost::get<LpyObjectMap>(arg);

        LpyObjectMap::const_iterator iter_obj =  map.begin();
        LpyObjectMap::const_iterator iter_end =  map.end();
        size_t nbstdparam = args.size();
        if (nbstdparam + len(map.size()) < mclass->getNamedParameterNb()){
                std::stringstream str;
                str << mclass->name << " takes exactly " << mclass->getNamedParameterNb()<< " (" << nbstdparam + map.size() << " given)";
                LsysError(str.str());
        }
        pgl_hash_set<size_t> missingargs;

        while( iter_obj != iter_end )
        {
            std::string pname = iter_obj->first;
            size_t pposition = mclass->getParameterPosition(pname);
            if (pposition == ModuleClass::NOPOS) {
                std::stringstream str;
                str << "Invalid parameter name '" << pname << "' for module '" << mclass->name << "'.";
                LsysError(str.str());
            }
            else if (pposition < nbstdparam) {
                std::stringstream str;
                str << mclass->name << " got multiple values for parameter '" << pname << "'.";
                LsysError(str.str());
            }
            else {
                size_t nbactualparam = args.size();
                if(nbactualparam > pposition) {
                    args[pposition] = iter_obj->second;
                    pgl_hash_set<size_t>::const_iterator itmarg = missingargs.find(pposition);
                    if (itmarg != missingargs.end())
                        missingargs.erase(itmarg);
                }
                else {
                    for (size_t i = nbactualparam ; i < pposition; ++i ){
                        appendParam(args,LpyObject());
                        missingargs.insert(i);
                    }
                    appendParam(args,iter_obj->second);
                }
            }
            ++iter_obj;
        }
        if (missingargs.size() > 0) {
                std::stringstream str;
                str << mclass->name << " takes exactly " << mclass->getNamedParameterNb()<< " (" << missingargs.size() << " missing)";
                LsysError(str.str());
        }
    }
    catch (boost::exception & e) {
        try {
            LpyObjectList list = boost::get<LpyObjectList>(arg);

            LpyObjectList::const_iterator begin = list.begin();
            LpyObjectList::const_iterator end = list.end();
            for(size_t i = 0; i < start && begin != end; ++i) ++begin;
            while(begin != end) {
                appendParam(args,*begin);
                ++begin;
            }
        }
        catch (boost::exception & e) {
            try {
                LpyObjectDeque deque = boost::get<LpyObjectDeque>(arg);

                LpyObjectDeque::const_iterator begin = deque.begin() + start;
                LpyObjectDeque::const_iterator end = deque.end();
                while(begin != end) {
                    appendParam(args,*begin);
                    ++begin;
                }
            }
            catch(boost::exception & e){
            }
        }
    }
#else
    boost::python::extract<boost::python::dict> isdict(arg);
    if (!isdict.check()){
        LpyObject iter_obj = LpyObject( boost::python::handle<>( PyObject_GetIter( arg.ptr() ) ) );
        for(size_t i = 0; i < start; ++i) iter_obj.attr( "next" )();
        try { while( true ) appendParam(args,iter_obj.attr( "next" )()); }
        catch( boost::python::error_already_set ){ PyErr_Clear(); }
    }
    else {
        LpyObject iter_obj =  isdict().iteritems();
        size_t nbstdparam = args.size();
        if (nbstdparam + len(arg) < mclass->getNamedParameterNb()){
                std::stringstream str;
                str << mclass->name << " takes exactly " << mclass->getNamedParameterNb()<< " (" << nbstdparam + len(arg) << " given)";
                LsysError(str.str());
        }
        pgl_hash_set<size_t> missingargs;

        while( true )
        {
            LpyObject obj;
            try {  obj = iter_obj.attr( "next" )(); }
            catch( boost::python::error_already_set ){ PyErr_Clear(); break; }

            std::string pname = boost::python::extract<std::string>( obj[0] )();
            size_t pposition = mclass->getParameterPosition(pname);
            if (pposition == ModuleClass::NOPOS) {
                std::stringstream str;
                str << "Invalid parameter name '" << pname << "' for module '" << mclass->name << "'.";
                LsysError(str.str());
            }
            else if (pposition < nbstdparam) {
                std::stringstream str;
                str << mclass->name << " got multiple values for parameter '" << pname << "'.";
                LsysError(str.str());
            }
            else {
                size_t nbactualparam = args.size();
                if(nbactualparam > pposition) {
                    args[pposition] = obj[1];
                    pgl_hash_set<size_t>::const_iterator itmarg = missingargs.find(pposition);
                    if (itmarg != missingargs.end())
                        missingargs.erase(itmarg);
                }
                else {
                    for (size_t i = nbactualparam ; i < pposition; ++i ){
                        appendParam(args,LpyObject());
                        missingargs.insert(i);
                    }
                    appendParam(args,obj[1]);
                }
            }
        }
        if (missingargs.size() > 0) {
                std::stringstream str;
                str << mclass->name << " takes exactly " << mclass->getNamedParameterNb()<< " (" << missingargs.size() << " missing)";
                LsysError(str.str());
        }
    }
#endif
}

void processLastArg(ModuleClassPtr mclass, ParamModule::ParameterList& args, LpyObject arg){
    if (isContainer(arg))
        processArgList(mclass,args,arg);
    else
        appendParam(args,arg);
}

void processConstruction(ParamModule& module,
					ParamModule::ParameterList& args, 
                    LpyObject arg, size_t start = 0)
{
    LpyObjectList list = get<LpyObjectList>(arg);
    size_t l = length(list);
    if(module.getClass() == ModuleClass::New)
    {
        assert(l>0);
        module.setName(get<std::string>(list[start]));
        start += 1;
    }
    if (l > 0) {
        args.reserve(l);

        for(size_t i = start; i < l-1; ++i){
            appendParam(args, list[i]);
        }
        if(l > start) {
            processLastArg(module.getClass(), args, list[l-1]);
        }
    }
}


/*---------------------------------------------------------------------------*/

ParamModule::ParamModule():
BaseType() {}

ParamModule::ParamModule(size_t classid):
    BaseType(classid) {}

ParamModule::ParamModule(const std::string& name) :
    BaseType() 
{
  std::string::const_iterator _it = name.begin();
  while(_it != name.end() && *_it != '(')_it++;
  if(_it == name.end()){
	setName(name);
  }
  else {
	setName(std::string(name.begin(),_it));
	_it++;
	if(_it != name.end()){
	  std::string::const_iterator _it2 = name.end()-1;
	  while(_it2 != _it && *_it2 != ')')_it2--;
#ifdef UNITY_MODULE
      std::cerr << "Convert : " << std::string(_it,_it2) << " into LpyObjectList";
      assert(false);
#else
      LpyObject o = LsysContext::currentContext()->evaluate('['+std::string(_it,_it2)+']');
      if(o != LpyObject())
          processConstruction(*this,__args(),boost::python::extract<list>(o)());
#endif
	}
  }
}

ParamModule::ParamModule(size_t classid, const std::string& args):
    BaseType(classid) 
{
	if (!args.empty()){
#ifdef UNITY_MODULE
        LpyObjectList list;
        std::istringstream f(args);
        std::string s;
        while (getline(f, s, ',')) {
            std::stringstream st(s);
            if (s == "True" || s == "False")
                addToList(list, LpyObject(s == "True"));
            else if (s.find_first_of("0123456789") != std::string::npos && s.find_last_of("0123456789") != std::string::npos)
            {
                if (s.find('.') != std::string::npos)
                {
                    float valueF;
                    st >> valueF;
                    addToList(list, LpyObject(valueF));
                }
                else {
                    int valueI;
                    st >> valueI;
                    addToList(list, LpyObject(valueI));
                }
            }
            else
                addToList(list, LpyObject(s));
        }
        processConstruction(*this, __args(), list);
#else
      LsysWarning("ParamModule arguments " + args);
      LpyObject o = LsysContext::currentContext()->evaluate('['+args+']');
      if(o != LpyObject())
        processConstruction(*this,__args(),boost::python::extract<boost::python::list>(o)());
#endif
    }
}

ParamModule::ParamModule(const ParamModule& mod):
BaseType(mod)  {}

ParamModule::ParamModule(const std::string& name, const LpyObjectList& arg):
BaseType(name) 
{ processConstruction(*this,__args(),arg); }

ParamModule::ParamModule(size_t classid, const LpyObjectList& arg):
BaseType(classid) 
{ processConstruction(*this,__args(),arg); }


#ifndef UNITY_MODULE
ParamModule::ParamModule(const ModuleClassPtr m,
              const boost::python::tuple& args):
    BaseType(m) 
{ processConstruction(*this,__args(),args); }

ParamModule::ParamModule(boost::python::tuple t):
 BaseType() 
{  
  boost::python::extract<size_t> id_extract(t[0]);
  if (id_extract.check()) setClass(id_extract());
  else setName(boost::python::extract<std::string>(t[0]));
  processConstruction(*this,__args(),t,1);
}
#endif

ParamModule::ParamModule(LpyObjectList t):
 BaseType() 
{ 
#ifndef USING_PYTHON
    LpyObjectList::const_iterator begin = t.begin();
    try {
        size_t id = boost::get<size_t>(*begin);
        setClass(id);
    }
    catch (boost::exception & e) {
        std::string id = boost::get<std::string>(*begin);
        setName(id);
    }
    processConstruction(*this, __args(), t, 1);
#else
  boost::python::extract<size_t> id_extract(t[0]);
  if (id_extract.check())setClass(id_extract());
  else setName(boost::python::extract<std::string>(t[0]));
  processConstruction (*this,__args(),t,1);
#endif
}

ParamModule::ParamModule(const std::string& name, 
                         const LpyObject& a):
BaseType(name) { processLastArg(getClass(),__args(),a); }

ParamModule::ParamModule(const std::string& name, 
                         const LpyObject& a,
                         const LpyObject& b):
BaseType(name) { appendParam(__args(),a); processLastArg(getClass(), __args(),b); }

ParamModule::ParamModule(const std::string& name, 
                         const LpyObject& a,
                         const LpyObject& b,
                         const LpyObject& c):
BaseType(name) 
{ appendParam(__args(),a); appendParam(__args(),b); processLastArg(getClass(),__args(),c); }

ParamModule::ParamModule(const std::string& name, 
                         const LpyObject& a,
                         const LpyObject& b,
                         const LpyObject& c,
                         const LpyObject& d):
BaseType(name) 
 { appendParam(__args(),a); appendParam(__args(),b); 
			  appendParam(__args(),c); processLastArg(getClass(), __args(),d); }

ParamModule::ParamModule(const std::string& name, 
                         const LpyObject& a,
                         const LpyObject& b,
                         const LpyObject& c,
                         const LpyObject& d,
                         const LpyObject& e):
BaseType(name)
{ appendParam(__args(),a); appendParam(__args(),b); 
  appendParam(__args(),c); appendParam(__args(),d);
  processLastArg(getClass(), __args(),e); }



ParamModule::~ParamModule() 
{ 
}

void ParamModule::appendArgumentList(const LpyObject& arglist)
{
    processArgList(getClass(),__args(),arglist);
}



int 
ParamModule::_getInt(int i) const 
{ 
	const ParameterList& p = __constargs();
	assert(p.size() > i);
    const ParameterType & pi = p[i];
    if (is<int>(pi))
        return get<int>(pi);
    else {
        std::stringstream str;
        str << "Invalid type for parameter " << i << " in module '" << name() << "'. Looking for int.";
        LsysError(str.str());
        return 0;
    }
}

real_t 
ParamModule::_getReal(int i) const 
{ 
	const ParameterList& p = __constargs();
	assert(p.size() > i);
    const ParameterType & pi = p[i];
    if (is<real_t>(pi))
        return get<real_t>(pi);
    if (is<uint32_t>(pi))
        return get<uint32_t>(pi);
    if (is<int>(pi))
        return get<int>(pi);
    std::stringstream str;
    str << "Invalid type for parameter " << i << " in module '" << name() << "'. Looking for real.";
    LsysError(str.str());
    return 0.0;
}

bool ParamModule::_getBool(int i) const
{
	const ParameterList& p = __constargs();
	assert(p.size() > i);
    const ParameterType & pi = p[i];
    if (is<bool>(pi))
        return get<bool>(pi);
    else {
        std::stringstream str;
        str << "Invalid type for parameter " << i << " in module '" << name() << "'. Looking for bool.";
        LsysError(str.str());
        return false;
    }
}


std::string 
ParamModule::_getString(int i) const 
{ 
	const ParameterList& p = __constargs();
	assert(p.size() > i);
    const ParameterType & pi = p[i];
    if (is<std::string>(pi))
        return get<std::string>(pi);
    else {
        std::stringstream str;
        str << "Invalid type for parameter " << i << " in module '" << name() << "'. Looking for string.";
        LsysError(str.str());
        return "";
    }
}

void 
ParamModule::_setValues(real_t x,real_t y,real_t z)
{ 
  ParameterList& args = __args();
  size_t nbArg = args.size();
  if (nbArg > 3) nbArg = 3;
  switch(nbArg){
  case 3:
    args[2] = LpyObject(z);
  case 2:
    args[1] = LpyObject(y);
    args[0] = LpyObject(x);
	break;
  case 1:
      {
#ifndef USING_PYTHON
      try {
          real_t test = boost::get<real_t>(args[0]);
          args[0] = LpyObject(x);
      }
      catch (boost::exception & e) {
          assert(false && "Arg! We should not be here!");
//            int size = len(args[0]);
//            if (size > 0)args[0].attr("__setitem__")(0,x);
//            if (size > 1)args[0].attr("__setitem__")(1,y);
//            if (size > 2)args[0].attr("__setitem__")(2,z);
       }
#else
        boost::python::extract<float> ext(args[0]);
        if (ext.check()){
            args[0] = LpyObject(x);
        }
        else {
            int size = len(args[0]);
            if (size > 0)args[0].attr("__setitem__")(0,x);
            if (size > 1)args[0].attr("__setitem__")(1,y);
            if (size > 2)args[0].attr("__setitem__")(2,z);
        }
#endif
	    break;
      }
  case 0:
      appendParam(args,LpyObject(TOOLS(Vector3(x,y,z))));
    // appendParam(__args,LpyObject(y));
    // appendParam(__args,LpyObject(z));
  default :
	break;
  }
}

void ParamModule::_setFrameValues(const TOOLS(Vector3)& p, const TOOLS(Vector3)& h, 
								  const TOOLS(Vector3)& u, const TOOLS(Vector3)& l)
{
  ParameterList& args = __args();
  size_t nbArg = args.size();
  if(nbArg >= 1 ) args[0] = LpyObject(p);
  else args.push_back(LpyObject(p));
  if(nbArg >= 2 ) args[1] = LpyObject(h);
  else args.push_back(LpyObject(h));
  if(nbArg >= 3 ) args[2] = LpyObject(u);
  else args.push_back(LpyObject(u));
  if(nbArg >= 4 ) args[3] = LpyObject(l);
  else args.push_back(LpyObject(l));
}

/*---------------------------------------------------------------------------*/

bool ParamModule::match(const std::string& _name, size_t nbargs) const
{ return name() == _name && argSize() == nbargs; }

#ifndef UNITY_MODULE
bool ParamModule::match(const PatternModule& pattern) const 
{ ArgList l; return MatchingEngine::module_match(*this,pattern,l); }

bool ParamModule::match(const PatternModule& pattern, ArgList& l) const 
{ return MatchingEngine::module_match(*this,pattern,l); }
#endif
