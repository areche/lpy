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


#include "error.h"
#ifndef USING_PYTHON
#include <iostream>
#else
#include <boost/python.hpp>
#endif
#include <sstream>
#include <memory>

static std::shared_ptr<std::function<void(const std::string &)>> errorCallback = nullptr;
static std::shared_ptr<std::function<void(const std::string &)>> warningCallback = nullptr;

LPY_BEGIN_NAMESPACE

/*---------------------------------------------------------------------------*/

LPY_API void SetLsysErrorCallback(const std::function<void(const std::string &)> &callback)
{
	errorCallback = std::make_shared<std::function<void(const std::string &)>>(callback);
}

LPY_API void SetLsysWarningCallback(const std::function<void(const std::string &)> &callback)
{
	warningCallback = std::make_shared<std::function<void(const std::string &)>>(callback);
}

LPY_API void LsysError(const std::string& error)
{
	if (errorCallback)
        errorCallback->operator()(error);
	else
	{
#ifndef USING_PYTHON
        std::cerr << error;
#else
        PyErr_SetString(PyExc_ValueError, error.c_str());
        boost::python::throw_error_already_set();
#endif
	}
}

LPY_API void LsysError(const std::string& error,const std::string& filename, int lineno)
{
	if (!filename.empty() || lineno >=0){
        std::stringstream stream;
        stream << (filename.empty()?"<string>":filename) << ':' << lineno << ':' << error;
        LsysError(stream.str());
	}
    else
        LsysError(error);
}

LPY_API void LsysSyntaxError(const std::string& error)
{
    if (errorCallback)
        errorCallback->operator()(error);
	else
	{
#ifndef USING_PYTHON
        std::cerr << error;
#else
        PyErr_SetString(PyExc_SyntaxError, error.c_str());
        boost::python::throw_error_already_set();
#endif
	}
}

LPY_API void LsysSyntaxError(const std::string& error,const std::string& filename, int lineno)
{
	if (!filename.empty() || lineno >=0){
        std::stringstream stream;
        stream << (filename.empty()?"<string>":filename) << ':' << lineno << ':' << error;
        LsysSyntaxError(stream.str());
	}
    else
        LsysSyntaxError(error);
}

LPY_API void LsysWarning(const std::string& error)
{
    if (warningCallback)
        warningCallback->operator()(error + "\n");
    else {
#ifndef USING_PYTHON
        std::cout << error << std::endl;
#else
        PyErr_WarnEx(PyExc_Warning,error.c_str(),1);
#endif
    }
}

LPY_API void LsysWarning(const std::string& error,const std::string& filename, int lineno)
{
    if (warningCallback) {
        std::stringstream stream;
        stream << (filename.empty()?"<string>":filename) << ':' << lineno << ':' << error;
        warningCallback->operator()(stream.str());
    }
    else {
#ifndef USING_PYTHON
        std::cerr << filename << ":" << lineno << error;
#else
        PyErr_WarnExplicit(PyExc_Warning,error.c_str(),filename.empty()?"<string>":filename.c_str(),lineno,NULL,NULL);
#endif
    }
}

/*---------------------------------------------------------------------------*/

LPY_END_NAMESPACE

