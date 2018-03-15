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

#ifndef __arg_collector_h__
#define __arg_collector_h__

#include "lpy_variant.h"

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/

inline LpyObjectList toPyList( LpyObjectList obj ) { return obj; }

inline size_t len( const LpyObjectDeque& obj ) { return obj.size(); }
inline LpyObject toPyList( const LpyObjectDeque& obj ) {
    return LpyObject(obj);
}

inline LpyObjectMap toPyDict( const LpyObjectDeque& obj, const std::vector<std::string> names ) {
    LpyObjectDeque::const_iterator itobj = obj.begin();
	
    LpyObjectMap result;
    for(std::vector<std::string>::const_iterator itname = names.begin(); itname != names.end() && itobj != obj.end(); ++itobj, ++itname)
        result[*itname] = *itobj;

    return result;
}


class LPY_API PyObjRef {
public:
	PyObjRef(): __toDelete(false), __object(NULL) {}

    PyObjRef(const LpyObject& obj , bool owner = true):
		__toDelete(owner), __object(owner?NULL:&obj) 
		{ if(owner)copy_obj(obj); }

	PyObjRef(const PyObjRef& other): 
		__toDelete(other.__toDelete), __object(other.__object) 
		{ if(__toDelete) copy_obj(*other.__object); }

	PyObjRef& operator=(const PyObjRef& other) {
		if(__toDelete  && __object) delete __object;
		__toDelete = other.__toDelete;
		if(!__toDelete) __object = other.__object;
		else copy_obj(*other.__object);
		return *this;
	}

	~PyObjRef() { if(__toDelete  && __object) delete __object; }

    inline const LpyObject& get() const { return *__object; }

private:

    inline void copy_obj(const LpyObject& obj) {
#ifndef USING_PYTHON
            __object = new LpyObject(obj);
#else
            __object = new LpyObject(boost::python::handle<>(boost::python::borrowed<>(obj.ptr())));
#endif
	}

	bool __toDelete;
    const LpyObject * __object;
};

class LPY_API ArgRefList {
public:
	ArgRefList(size_t i = 0) : __data(i) { }

	inline size_t size() const { return __data.size(); }
	inline bool empty() const { return __data.empty(); }
	inline void reserve(size_t s) { __data.reserve(s); }
    inline const LpyObject& operator[](size_t i) const { return __data[i].get(); }
    inline void push_back(const LpyObject& obj){ __data.push_back(PyObjRef(obj,true)); }
    inline void push_back_ref(const LpyObject& obj){ __data.push_back(PyObjRef(obj,false)); }
	inline void append(const ArgRefList& l) { __data.insert(__data.end(),l.__data.begin(),l.__data.end()); }
	inline void prepend(const ArgRefList& l) { __data.insert(__data.begin(),l.__data.begin(),l.__data.end()); }
    inline LpyObjectList toPyList() const {
        LpyObjectList res;
        for(std::vector<PyObjRef>::const_iterator it = __data.begin(); it != __data.end(); ++it)  addToList(res,it->get());
		return res;
	}

protected:
	std::vector<PyObjRef> __data;
};

inline LpyObjectList toPyList( const ArgRefList& obj ) { return obj.toPyList(); }
inline size_t len( const ArgRefList& obj ) { return obj.size(); }

#ifdef USE_PYTHON_LIST_COLLECTOR
typedef bp::list ArgList;
#else
#ifdef USE_OBJECTVEC_COLLECTOR
typedef LpyObjectDeque ArgList;
#else
typedef ArgRefList ArgList;
#endif
#endif


/*---------------------------------------------------------------------------*/

LPY_END_NAMESPACE

#endif
