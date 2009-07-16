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

#include "matching.h"
#include "patternmodule.h"
#include "axialtree_manip.h"
#include "matching_tmpl.h"
#include <boost/python.hpp>
#include "argcollector_core.h"

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/
MatchingEngine::eModuleMatchingMethod 
MatchingEngine::ModuleMatchingMethod = MatchingEngine::eDefaultModuleMatching;

MatchingEngine::ModuleMatchingFuncType MatchingEngine::ModuleMatchingFunc = &MatchingImplementation::module_matching_with_star_and_valueconstraints;


void MatchingEngine::setModuleMatchingMethod(MatchingEngine::eModuleMatchingMethod method)
{ 
	ModuleMatchingMethod = method; 
	switch(ModuleMatchingMethod){
	case eMSimple:
		ModuleMatchingFunc = &MatchingImplementation::simple_module_matching; break;
	case eMWithStar:
		ModuleMatchingFunc = &MatchingImplementation::module_matching_with_star; break;
	default:
		ModuleMatchingFunc = &MatchingImplementation::module_matching_with_star_and_valueconstraints; break;
	}
}

MatchingEngine::eModuleMatchingMethod LPY::MatchingEngine::getModuleMatchingMethod()
{ return ModuleMatchingMethod; }


MatchingEngine::eStringMatchingMethod 
MatchingEngine::StringMatchingMethod = MatchingEngine::eDefaultStringMatching;

MatchingEngine::RightMatchingFuncType MatchingEngine::RightMatchingFunc = &MatchingImplementation::tree_right_match;
MatchingEngine::LeftMatchingFuncType MatchingEngine::LeftMatchingFunc   = &MatchingImplementation::tree_left_match;

void MatchingEngine::setStringMatchingMethod(MatchingEngine::eStringMatchingMethod method)
{ 
	StringMatchingMethod = method; 
	switch(StringMatchingMethod){
		case eString:
			RightMatchingFunc = &MatchingImplementation::string_right_match /*StringMatcher<GetNext>::match*/;
			LeftMatchingFunc = &StringReverseMatcher<GetPrevious>::match; break;
		case eMScaleAxialTree:
			RightMatchingFunc = &MatchingImplementation::mstree_right_match; 
			LeftMatchingFunc = &MatchingImplementation::mstree_left_match; break;
		case eMLevelAxialTree:
			RightMatchingFunc = &MatchingImplementation::mltree_right_match; 
			LeftMatchingFunc = &MatchingImplementation::mltree_left_match; break;
		case eAxialTree:
		default:
			RightMatchingFunc = &MatchingImplementation::tree_right_match;
			LeftMatchingFunc = &MatchingImplementation::tree_left_match; break;
	}
}

MatchingEngine::eStringMatchingMethod LPY::MatchingEngine::getStringMatchingMethod()
{ return StringMatchingMethod; }

/*---------------------------------------------------------------------------*/


bool MatchingEngine::module_match(const ParamModule& module, 
								  const PatternModule& pattern,
								  ArgList& params) 
{
	return (*ModuleMatchingFunc)(module,pattern,params);

}


bool MatchingEngine::match(AxialTree::const_iterator  matching_start,
						   AxialTree::const_iterator  string_end,
						   PatternString::const_iterator  pattern_begin,
						   PatternString::const_iterator  pattern_end,
						   AxialTree::const_iterator& matching_end,
						   AxialTree::const_iterator& last_matched,
						   ArgList& params) 
{
	return MatchingImplementation::string_exact_match(matching_start,string_end,
						                              pattern_begin,pattern_end,
						                              matching_end,last_matched, params);
}

bool MatchingEngine::reverse_match(AxialTree::const_iterator matching_start,
								   AxialTree::const_iterator  string_begin,
								   AxialTree::const_iterator  string_end,
								   PatternString::const_reverse_iterator  pattern_rbegin,
								   PatternString::const_reverse_iterator  pattern_rend,
								   AxialTree::const_iterator& matching_end,
								   ArgList& params)
{ 
	return MatchingImplementation::string_exact_reverse_match(matching_start,string_begin,string_end,
						                               pattern_rbegin,pattern_rend,
						                               matching_end,params);
}

bool MatchingEngine::right_match(AxialTree::const_iterator  matching_start,
								 AxialTree::const_iterator  string_end,
								 PatternString::const_iterator  pattern_begin,
								 PatternString::const_iterator  pattern_end,
								 AxialTree::const_iterator  last_matched,
								 AxialTree::const_iterator& matching_end,
								 ArgList& params) 
{
	return (*RightMatchingFunc)(matching_start,string_end,
						        pattern_begin,pattern_end,
						        last_matched, matching_end,params);
/*	switch(StringMatchingMethod){
		case eString:
				return StringMatcher<GetNext>::
				match(matching_start, string_end, pattern_begin, pattern_end, last_matched, matching_end, params);
		case eMScaleAxialTree:
			return MatchingImplementation::mstree_right_match(matching_start,string_end,
						                              pattern_begin,pattern_end,
						                              last_matched, matching_end,params);
		case eMLevelAxialTree:
			return MatchingImplementation::mltree_right_match(matching_start,string_end,
						                              pattern_begin,pattern_end,
						                              last_matched, matching_end,params);
		case eAxialTree:
		default:
			return MatchingImplementation::tree_right_match(matching_start,string_end,
						                              pattern_begin,pattern_end,
						                              last_matched, matching_end,params);
	}*/
}

bool MatchingEngine::left_match(AxialTree::const_iterator  matching_start,
								AxialTree::const_iterator  string_begin,
								AxialTree::const_iterator  string_end,
								PatternString::const_reverse_iterator  pattern_rbegin,
								PatternString::const_reverse_iterator  pattern_rend,
								AxialTree::const_iterator& matching_end,
								ArgList& params) 
{
	return (*LeftMatchingFunc)(matching_start,string_begin,string_end,pattern_rbegin,pattern_rend,matching_end,params);
/*	switch(StringMatchingMethod){
		case eString:
				return StringReverseMatcher<GetPrevious>::
				match(matching_start,string_begin,string_end,pattern_rbegin,pattern_rend,matching_end,params);
		case eMScaleAxialTree:
			return MatchingImplementation::mstree_left_match(matching_start,string_begin,string_end,
						                                 pattern_rbegin,pattern_rend,
						                                 matching_end,params);
		case eMLevelAxialTree:
			return MatchingImplementation::mltree_left_match(matching_start,string_begin,string_end,
						                                 pattern_rbegin,pattern_rend,
						                                 matching_end,params);
		case eAxialTree:
		default:
			return MatchingImplementation::tree_left_match(matching_start,string_begin,string_end,
						                                 pattern_rbegin,pattern_rend,
						                                 matching_end,params);
	}*/
}

/*---------------------------------------------------------------------------*/

bool 
MatchingImplementation::simple_module_matching(const ParamModule& module, 
													 const PatternModule& pattern, 
													 ArgList& l)
{
  if( module.sameName(pattern) && module.argSize() == pattern.argSize()){
	  ArgsCollector::append_modargs(l,module.getParameterList());
	return true;
  }
  return false;
}

bool MatchingImplementation::module_matching_with_star(const ParamModule& module, 
													   const PatternModule& pattern, 
									                   ArgList& l)
{
  if (pattern.isStar()){
	size_t s = pattern.argSize();
	if (s == 0) return true;
	if (s == 1) {
	  size_t s2 = module.argSize();
	  LsysVar v = pattern.getAt(0);
	  if(v.isArgs()) { 
		  ArgList arg;
		  ArgsCollector::append_arg(arg,boost::python::object(module.name()));
		  ArgsCollector::append_modargs(arg,module.getParameterList());
		  ArgsCollector::append_as_arg(l,arg);
		  return true; 
	  }
	  else {
		if( s2 == 0){ ArgsCollector::append_arg(l,boost::python::object(module.name()));return true; }
	    else return false;
	  }
	}
	else {
	  size_t s2 = module.argSize();
	  if (s2 < s-2) return false;
	  LsysVar v = pattern.getAt(s-1);
	  if(!v.isArgs()){
  		if (s2 != s-1) return false;
		ArgsCollector::append_arg(l,boost::python::object(module.name()));
		ArgsCollector::append_modargs(l,module.getParameterList());
		return true;
	  }
	  ArgsCollector::append_arg(l,boost::python::object(module.name()));
	  if(s != 2){
		for(int i = 0; i < s-2; i++)
		  ArgsCollector::append_arg_ref(l,module.getAt(i));
		if(s2 > s-2)ArgsCollector::append_arg(l,module.getSlice(s-2,s2));
		else ArgsCollector::append_arg(l,boost::python::list());
	  }
	  else ArgsCollector::append_arg(l,module.getPyArgs());
	  return true;
	}
  }
  else {
	if (!module.sameName(pattern)) return false;
	size_t s = pattern.argSize();
    size_t s2 = module.argSize();
	if (s2 < s-1) return false;
	if( s == 0) return s2 == 0;
	else{
	  LsysVar v = pattern.getAt(s-1);
	  if(!v.isArgs()) {
		if(s2 == s) { ArgsCollector::append_modargs(l,module.getParameterList()); return true; }
		else return false;
	  }
	  if(s!=1){
		for(int i = 0; i < s-1; i++)
		  ArgsCollector::append_arg_ref(l,module.getAt(i));
		if(s2 > s-1)ArgsCollector::append_arg(l,module.getSlice(s-1,s2));
		else ArgsCollector::append_arg(l,boost::python::list());
	  }
	  else ArgsCollector::append_arg(l,module.getPyArgs());
	  return true;
	}
	return true;
  }
}

bool MatchingImplementation::module_matching_with_star_and_valueconstraints(
										  const ParamModule& module, 
									      const PatternModule& pattern, 
									      ArgList& l)
{ 
  if (pattern.isStar()){
	size_t s = pattern.argSize();
	if (s == 0) return true;
	if (s == 1) {
	  size_t s2 = module.argSize();
	  const LsysVar& v = pattern.getAt(0);
	  if(v.isArgs()) { 
			ArgList largs;
			if(v.isNamed() || v.hasCondition()){
				ArgsCollector::append_arg(largs,bp::object(module.name()));
				ArgsCollector::append_modargs(largs,module.getParameterList()); 
			}
			if(v.hasCondition() && !v.isCompatible(toPyList(largs)))return false;
			if(v.isNamed()) ArgsCollector::append_as_arg(l,largs); 
			return true; 
		}
		else {
		  if(s2 == 0){ 
			 boost::python::object largs(module.name()); 
			 if (!v.isCompatible(largs))return false;
			 if(v.isNamed()) ArgsCollector::append_arg(l,largs); 
			 return true; 
		  }
		  else return false;
	  }
	}
	else {
	  size_t s2 = module.argSize();
	  int beg = 0;
	  const LsysVar& v1 = pattern.getAt(0);
	  if(!v1.isNamed()){
		  if (s2 < s-1) return false;
		  if (!v1.isCompatible(module.getAt(0)))return false;
		  beg = 1;
	  }
	  else if (s2 < s-2) return false;
	  else {
		  boost::python::object no(module.name());
		  if (!v1.isCompatible(no))return false;
		  ArgsCollector::append_arg(l,no);
	  }

	  bool lastarg = false;
	  boost::python::object lastargval;
	  const LsysVar& v = pattern.getAt(s-1);
	  if(!v.isNamed()){
		  if (s2 != (s-1+beg)) return false;
		  if (!v.isCompatible(module.getAt(s-2+beg)))return false;
	  }
	  else {
		lastarg = true;
		if(beg == 1){
		  if(!v.isArgs()){
			if (s2 != s) return false;
			lastargval = module.getAt(s-1);
		  }
		  else {
			if (s2 < s-1) return false;
			else if (s2 == s-1) lastargval = boost::python::list();
			lastargval = module.getSlice(s-1,s2);
		  }
		}
		else {
		  if(!v.isArgs()){
			if (s2 != s-1) return false;
			lastargval = module.getAt(s-2);
		  }
		  else {
			if (s2 < s-2) return false;
			else {
			  if(s == 2){
				  lastargval = module.getPyArgs();
				  if(!v.isCompatible(lastargval))return false;
				  ArgsCollector::append_arg(l,lastargval); 
                  return true; 
			  }
			  else if (s2 == s-2) lastargval = boost::python::list();
			  else lastargval = module.getSlice(s-2,s2);
			}
		  }
		}
		if(!v.isCompatible(lastargval))return false;
	  }
	  for(size_t i = 1; i < s-1; i++){
	    const LsysVar& v = pattern.getAt(i);
	    if(!v.isCompatible(module.getAt(i-1+beg))) return false; 
		if(v.isNamed())ArgsCollector::append_arg_ref(l,module.getAt(i-1+beg));
	  }
	  if(lastarg)ArgsCollector::append_arg(l,lastargval);
	  return true;
	}
  }
  else {
	if (!module.sameName(pattern)) return false;
	int s = pattern.argSize();
    int s2 = module.argSize();
	if( s == 0) return s2 == 0;
	else {
	  bool lastarg = false;
	  boost::python::object lastargval;
	  const LsysVar& v = pattern.getAt(s-1);
	  if(!v.isNamed()){
		if (s2 != s) return false;
		if (!v.isCompatible(module.getAt(s-1)))return false;
	  }
	  else {
		lastarg = true;
		if(!v.isArgs()){
		  if (s2 != s) return false; 
		  else lastargval = module.getAt(s-1);
		}
		else {
		  if (s2 < s - 1) return false;
		  if(s == 1){ 
			  lastargval =  module.getPyArgs();
			  if(!v.isCompatible(lastargval))return false; 
			  ArgsCollector::append_arg(l,lastargval); return true; 
		  }
		  else if (s2 == s - 1) lastargval = boost::python::list();
		  else lastargval = module.getSlice(s-1,s2);
		}
		if(!v.isCompatible(lastargval)) return false; 
	  }
	  for(size_t i = 0; i < s-1; i++){
		const LsysVar& v = pattern.getAt(i);
		if(!v.isCompatible(module.getAt(i))) return false; 
		if(v.isNamed())ArgsCollector::append_arg_ref(l,module.getAt(i));
	  }
	  if(lastarg)ArgsCollector::append_arg(l,lastargval);
	}
	return true;
  }
}

/*---------------------------------------------------------------------------*/


bool MatchingImplementation::string_exact_match(AxialTree::const_iterator  matching_start,
						   AxialTree::const_iterator  string_end,
						   PatternString::const_iterator  pattern_begin,
						   PatternString::const_iterator  pattern_end,
						   AxialTree::const_iterator& matching_end,
						   AxialTree::const_iterator& last_matched,
						   ArgList& params) 
{
	return StringMatcher<>::
		match(matching_start, string_end, pattern_begin, pattern_end, matching_end, last_matched, params);
}

bool MatchingImplementation::string_exact_reverse_match(AxialTree::const_iterator matching_start,
								   AxialTree::const_iterator  string_begin,
								   AxialTree::const_iterator  string_end,
								   PatternString::const_reverse_iterator  pattern_rbegin,
								   PatternString::const_reverse_iterator  pattern_rend,
								   AxialTree::const_iterator& matching_end,
								   ArgList& params)
{ 
	return StringReverseMatcher<>::
		match(matching_start, string_begin, string_end, pattern_rbegin, pattern_rend, matching_end, params);
}

bool MatchingImplementation::string_right_match(AxialTree::const_iterator  matching_start,
		                         AxialTree::const_iterator  string_end,
								 PatternString::const_iterator  pattern_begin,
								 PatternString::const_iterator  pattern_end,
							     AxialTree::const_iterator  last_matched,
								 AxialTree::const_iterator& matching_end,
								 ArgList& params)

{
	return StringMatcher<GetNext>::
		match(matching_start, string_end, pattern_begin, pattern_end, matching_end, last_matched, params);
}


bool MatchingImplementation::tree_right_match(AxialTree::const_iterator  matching_start,
								 AxialTree::const_iterator  string_end,
								 PatternString::const_iterator  pattern_begin,
								 PatternString::const_iterator  pattern_end,
						         AxialTree::const_iterator  last_matched,
								 AxialTree::const_iterator& matching_end,
								 ArgList& params) 
{
	return TreeRightMatcher<>::
		match(matching_start, string_end, pattern_begin, pattern_end, last_matched, matching_end, params);
}




bool MatchingImplementation::tree_left_match(AxialTree::const_iterator  matching_start,
								AxialTree::const_iterator  string_begin,
								AxialTree::const_iterator  string_end,
								PatternString::const_reverse_iterator  pattern_rbegin,
								PatternString::const_reverse_iterator  pattern_rend,
								AxialTree::const_iterator& matching_end,
								ArgList& params) 
{
	return TreeLeftMatcher<>::
		match(matching_start, string_begin, string_end, pattern_rbegin, pattern_rend, matching_end, params);
}


bool MatchingImplementation::mstree_left_match(AxialTree::const_iterator matching_start,
								   AxialTree::const_iterator  string_begin,
								   AxialTree::const_iterator  string_end,
								   PatternString::const_reverse_iterator  pattern_rbegin,
								   PatternString::const_reverse_iterator  pattern_rend,
								   AxialTree::const_iterator& matching_end,
								   ArgList& params)
{ 
	return TreeLeftMatcher<GetScalePredecessor>::
		match(matching_start, string_begin, string_end, pattern_rbegin, pattern_rend, matching_end, params);
}

bool MatchingImplementation::mltree_left_match(AxialTree::const_iterator matching_start,
								   AxialTree::const_iterator  string_begin,
								   AxialTree::const_iterator  string_end,
								   PatternString::const_reverse_iterator  pattern_rbegin,
								   PatternString::const_reverse_iterator  pattern_rend,
								   AxialTree::const_iterator& matching_end,
								   ArgList& params)
{ 
	return TreeLeftMatcher<GetLevelPredecessor>::
		match(matching_start, string_begin, string_end, pattern_rbegin, pattern_rend, matching_end, params);
}


bool MatchingImplementation::mstree_right_match(AxialTree::const_iterator  matching_start,
								 AxialTree::const_iterator  string_end,
								 PatternString::const_iterator  pattern_begin,
								 PatternString::const_iterator  pattern_end,
						         AxialTree::const_iterator  last_matched,
								 AxialTree::const_iterator& matching_end,
								 ArgList& params) 
{
	return TreeRightMatcher<GetScaleSuccessor>::
		match(matching_start, string_end, pattern_begin, pattern_end, last_matched, matching_end, params);
}

bool MatchingImplementation::mltree_right_match(AxialTree::const_iterator  matching_start,
								 AxialTree::const_iterator  string_end,
								 PatternString::const_iterator  pattern_begin,
								 PatternString::const_iterator  pattern_end,
						         AxialTree::const_iterator  last_matched,
								 AxialTree::const_iterator& matching_end,
								 ArgList& params) 
{
	return TreeRightMatcher<GetLevelSuccessor>::
		match(matching_start, string_end, pattern_begin, pattern_end, last_matched, matching_end, params);
}

LPY_END_NAMESPACE

