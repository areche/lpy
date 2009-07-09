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


#include "patternmodule.h"
#include "patternstring.h"
#include "lsyscontext.h"
#include "lpy_parser.h"
#include "matching.h"

LPY_USING_NAMESPACE
using namespace boost::python;

/*---------------------------------------------------------------------------*/


LsysVar::LsysVar(const std::string& n):
	__name(n),__conditionType(NoCondition){}

LsysVar::LsysVar(boost::python::object value):
	__name(), __pyvalue(value), __conditionType(EqualValueCondition) {}


std::string LsysVar::str() const
{ 
	if (__conditionType == FunctionalCondition) {
		std::string res(__name);
		res += " if ";
		res += __textualcondition;
		return res;
	}
	else if(__conditionType == EqualValueCondition) {
		return extract<std::string>(bp::str(__pyvalue));
	}
	return __name; 
}


std::string LsysVar::varname() const
{ if (__name.empty()) return __name;
  else if (__name[0] == '*') return std::string(__name.begin()+1,__name.end());
  else return __name;
}


bool LsysVar::isCompatible(const boost::python::object& value) const
{
	switch(__conditionType) {
		case EqualValueCondition:
			return value == __pyvalue;
		case FunctionalCondition:
			return bp::call<bool>(__pyvalue.ptr(),value);
		default:
			return true;
	}
}

void LsysVar::setCondition(const std::string& textualcondition, int lineno)
{
	std::string txt = "lambda "+varname()+" : "+textualcondition;
	if (lineno > 0){
		std::string decal ;
		for(size_t i = 0; i < lineno-1; ++i)
			decal += '\n';
		txt = decal + txt;
	}
	__pyvalue = LsysContext::current()->evaluate(txt);
	__textualcondition = textualcondition;
	__conditionType = FunctionalCondition;
}

/*---------------------------------------------------------------------------*/
PatternModule::PatternModule(): BaseType() { }

PatternModule::PatternModule(const std::string& name, int lineno): BaseType()
{
  std::vector<std::pair<size_t,std::string> > parsedstring = LpyParsing::parselstring(name,lineno);
  if(parsedstring.size() != 1)LsysError("Invalid query module "+name,"",lineno);
  setClass(parsedstring[0].first);
  __processPatternModule(parsedstring[0].second,lineno);
}

PatternModule::PatternModule(size_t classid, const std::string& argstr, int lineno):
	BaseType(classid) {
  __processPatternModule(argstr,lineno);
}

PatternModule::~PatternModule() { }



std::vector<std::string> PatternModule::getVarNames() const
{
  std::vector<std::string> res;
  if (isRepExp()) {
	extract<const PatternString&> t(getAt(0).getPyValue());
	if(t.check()) return t().getVarNames();
  }
  else if (isOr()) {
	  for(size_t i = 0; i < size(); i++){
		  extract<const PatternString&> v(getAt(i).getPyValue());
		  if(v.check()) {
		   std::vector<std::string> lres = v().getVarNames();
		   res.insert(res.end(),lres.begin(),lres.end());
		  }
	  }
  }
  else if (isGetModule()) {
	res.push_back(getAt(0).varname());
	extract<const PatternModule&> v(getAt(0).getPyValue());
	if(v.check()) {
	   std::vector<std::string> lres = v().getVarNames();
	   res.insert(res.end(),lres.begin(),lres.end());
	}
  }
  else {
	  for(size_t i = 0; i < size(); i++){
		  if(getAt(i).isNamed()) res.push_back(getAt(i).varname());
	  }
  }
  return res;
}

size_t PatternModule::getVarNb() const
{
  size_t res = 0;
  if (isRepExp()) {
	extract<const PatternString&> t(getAt(0).getPyValue());
	if(t.check()) return t().getVarNb();
  }
  else if (isOr()) {
	  for(size_t i = 0; i < size(); i++){
		  extract<const PatternString&> v(getAt(i).getPyValue());
		  if(v.check()) res += v().getVarNb();
	  }
  }
  else if (isGetModule()) {
	if(getAt(0).isNamed()) res += 1;
	extract<const PatternModule&> v(getAt(1).getPyValue());
	if(v.check()) res += v().getVarNb();
  }
  else {
	  for(size_t i = 0; i < size(); i++){
		  if(getAt(i).isNamed()) res+=1;
	  }
  }
  return res;
}


void PatternModule::__processPatternModule(const std::string& argstr, int lineno)
{
  if (getClass() == ModuleClass::RepExp) {
	  std::vector<std::string> args = LpyParsing::parse_arguments(argstr);
	  if (args.empty())LsysError("No Matching Pattern in RepExp module","",lineno);
	  else if (args.size() > 3) LsysError("Too much parameters in RepExp module","",lineno);
	  append(LsysVar(boost::python::object(PatternString(args[0],lineno))));
	  if (args.size() > 1) append(LsysVar(LsysContext::currentContext()->evaluate(args[1])));
	  if (args.size() == 3) append(LsysVar(LsysContext::currentContext()->evaluate(args[2])));
  }
  else if (getClass() == ModuleClass::Or) {
	  std::vector<std::string> args = LpyParsing::parse_arguments(argstr);
	  if (args.size() < 2)LsysError("Not enough parameters in Or module","",lineno);
	  for(size_t i = 0; i < args.size(); ++i){
		append(LsysVar(boost::python::object(PatternString(args[i],lineno))));
	  }
  }
  else if (getClass() == ModuleClass::GetModule) {
	  std::vector<std::string> args = LpyParsing::parse_arguments(argstr);
	  if (args.size() != 2)LsysError("Invalid construction to GetModule","",lineno);
	  std::pair<std::string,std::string> vartxt = LpyParsing::parse_variable(args[0],lineno);
	  if(LpyParsing::isValidVariableName(vartxt.first)){
		LsysVar var(vartxt.first);
		if(!vartxt.second.empty())var.setCondition(vartxt.second,lineno);
		append(var);
	  }
	  append(LsysVar(boost::python::object(PatternModule(args[1],lineno))));
  }
  else {
	  std::vector<std::string> args = LpyParsing::parse_arguments(argstr);
	  for(std::vector<std::string>::const_iterator itarg = args.begin(); itarg != args.end(); ++itarg){
		  bool notvar = false;
		  if (MatchingEngine::getModuleMatchingMethod() == MatchingEngine::eMWithStarNValueConstraint){
			  try {
				std::pair<std::string,std::string> vartxt = LpyParsing::parse_variable(*itarg,lineno);
				if(LpyParsing::isValidVariableName(vartxt.first)){
					LsysVar var(vartxt.first);
				    if(!vartxt.second.empty())var.setCondition(vartxt.second,lineno);
				    append(var);
				    notvar = true;
				}
			  }
			  catch (boost::python::error_already_set) {   PyErr_Clear(); /* PyErr_Print();*/ }
			  if (!notvar) {
			      object o = LsysContext::currentContext()->try_evaluate(*itarg);
			      if(o != object()){ append(o); notvar = true; }
			  }
		  }
		  if (!notvar){
			  if(LpyParsing::isValidVariableName(*itarg))
				  append(LsysVar(*itarg));
			  else LsysError(*itarg+" is invalid","",lineno);
		  }
	  }
  }
}

/*---------------------------------------------------------------------------*/

