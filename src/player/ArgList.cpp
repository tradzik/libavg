//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//
//  Original author of this file is Nick Hebner (hebnern@gmail.com).
//

#include "ArgList.h"

#include "Node.h"

#include "../base/Logger.h"
#include "../base/Exception.h"
#include "../base/StringHelper.h"

#include <sstream>

using namespace std;

namespace avg {

ArgList::ArgList()
{
}

ArgList::ArgList(const ArgList& ArgTemplates, const xmlNodePtr xmlNode)
{
    copyArgsFrom(ArgTemplates);

    for(xmlAttrPtr prop = xmlNode->properties; prop; prop = prop->next)
    {
        string name = (char*)prop->name;
        string value = (char*)prop->children->content;
        setArgValue(name, value);
    }
}

ArgList::ArgList(const ArgList& ArgTemplates, const boost::python::dict& PyDict)
{
    // TODO: Check if all required args are being set.
    copyArgsFrom(ArgTemplates);
    boost::python::list keys = PyDict.keys();
    int nKeys = boost::python::len(keys);
    for(int i = 0; i < nKeys; i++)
    {
        boost::python::object keyObj = keys[i];
        boost::python::object valObj = PyDict[keyObj];
        
        boost::python::extract<string> keyStrProxy(keyObj);
        if (!keyStrProxy.check()) {
            throw Exception(AVG_ERR_INVALID_ARGS, "Argument name must be a string.");
        }
        string keyStr = keyStrProxy();

        setArgValue(keyStr, valObj);
    }
}

ArgList::~ArgList()
{
}

bool ArgList::hasArg(const std::string& sName) const
{
    ArgMap::const_iterator it = m_Args.find(sName);
    return (it != m_Args.end() && !(it->second->isDefault()));
}

const ArgBasePtr ArgList::getArg(const string& sName) const
{
    ArgMap::const_iterator valIt = m_Args.find(sName);
    if (valIt == m_Args.end()) {
        // TODO: The error message should mention line number and node type.
        throw Exception(AVG_ERR_INVALID_ARGS, string("Argument ")+sName+" is not valid.");
    }
    return valIt->second;
}

void ArgList::getOverlayedArgVal(DPoint* pResult, const string& sName, 
        const string& sOverlay1, const string& sOverlay2, const string& sID) const
{
    if (hasArg(sName)) {
        if (hasArg(sOverlay1) || hasArg(sOverlay2)) {
            throw (Exception(AVG_ERR_INVALID_ARGS,
                    string("Duplicate node arguments (")+sName+" and "+
                    sOverlay1+","+sOverlay2+") for node '"+sID+"'"));
        }
        *pResult = getArgVal<DPoint>(sName);
    }
}

const ArgMap& ArgList::getArgMap() const
{
    return m_Args;
}

void ArgList::setArg(const ArgBase& newArg)
{
    m_Args.insert(ArgMap::value_type(newArg.getName(), ArgBasePtr(newArg.createCopy())));
}

void ArgList::setArgs(const ArgList& Args)
{
    for(ArgMap::const_iterator it = Args.m_Args.begin(); 
            it != Args.m_Args.end(); it++)
    {
        m_Args.insert(*it);
    }
}
    
void ArgList::setMembers(Node * pNode) const
{
    for(ArgMap::const_iterator it = m_Args.begin(); it != m_Args.end(); it++)
    {
        const ArgBasePtr pCurArg = it->second;
        pCurArg->setMember(pNode);
    }
    pNode->setArgs(*this);
}

template<class T>
void setArgValue(Arg<T>* pArg, const std::string & sName, const boost::python::object& Value)
{
    boost::python::extract<T> valProxy(Value);
    if (!valProxy.check()) {
        throw Exception(AVG_ERR_INVALID_ARGS, "Type error in argument "+sName+": "
                +typeid(T).name()+" expected.");
    }
    pArg->setValue(valProxy());
}

void ArgList::setArgValue(const std::string & sName, const boost::python::object& Value)
{
    ArgBasePtr pArg = getArg(sName);
    Arg<string>* pStringArg = dynamic_cast<Arg<string>* >(&*pArg);
    Arg<int>* pIntArg = dynamic_cast<Arg<int>* >(&*pArg);
    Arg<double>* pDoubleArg = dynamic_cast<Arg<double>* >(&*pArg);
    Arg<float>* pFloatArg = dynamic_cast<Arg<float>* >(&*pArg);
    Arg<bool>* pBoolArg = dynamic_cast<Arg<bool>* >(&*pArg);
    Arg<DPoint>* pDPointArg = dynamic_cast<Arg<DPoint>* >(&*pArg);
    Arg<vector<double> >* pDVectorArg = dynamic_cast<Arg<vector<double> >* >(&*pArg);
    Arg<vector<DPoint> >* pDPointVectorArg = 
            dynamic_cast<Arg<vector<DPoint> >* >(&*pArg);
    if(pStringArg) {
        avg::setArgValue(pStringArg, sName, Value);
    } else if (pIntArg) {
        avg::setArgValue(pIntArg, sName, Value);
    } else if (pDoubleArg) {
        avg::setArgValue(pDoubleArg, sName, Value);
    } else if (pFloatArg) {
        avg::setArgValue(pFloatArg, sName, Value);
    } else if (pBoolArg) {
        avg::setArgValue(pBoolArg, sName, Value);
    } else if (pDPointArg) {
        avg::setArgValue(pDPointArg, sName, Value);
    } else if (pDVectorArg) {
        avg::setArgValue(pDVectorArg, sName, Value);
    } else if (pDPointVectorArg) {
        avg::setArgValue(pDPointVectorArg, sName, Value);
    } else {
        assert(false);
    }
}

void ArgList::setArgValue(const std::string & sName, const std::string & sValue)
{
    ArgBasePtr pArg = getArg(sName);
    Arg<string>* pStringArg = dynamic_cast<Arg<string>* >(&*pArg);
    Arg<int>* pIntArg = dynamic_cast<Arg<int>* >(&*pArg);
    Arg<double>* pDoubleArg = dynamic_cast<Arg<double>* >(&*pArg);
    Arg<float>* pFloatArg = dynamic_cast<Arg<float>* >(&*pArg);
    Arg<bool>* pBoolArg = dynamic_cast<Arg<bool>* >(&*pArg);
    Arg<DPoint>* pDPointArg = dynamic_cast<Arg<DPoint>* >(&*pArg);
    Arg<vector<double> >* pDVectorArg = dynamic_cast<Arg<vector<double> >* >(&*pArg);
    Arg<vector<DPoint> >* pDPointVectorArg = 
            dynamic_cast<Arg<vector<DPoint> >* >(&*pArg);

    if (pStringArg) {
        pStringArg->setValue(sValue);
    } else if (pIntArg) {
        pIntArg->setValue(stringToInt(sValue));
    } else if (pDoubleArg) {
        pDoubleArg->setValue(stringToDouble(sValue));
    } else if (pFloatArg) {
        pFloatArg->setValue(float(stringToDouble(sValue)));
    } else if (pBoolArg) {
        pBoolArg->setValue(stringToBool(sValue));
    } else if (pDPointArg) {
        pDPointArg->setValue(stringToDPoint(sValue));
    } else if (pDVectorArg) {
        vector<double> v;
        fromString(sValue, v);
        pDVectorArg->setValue(v);
    } else if (pDPointVectorArg) {
        vector<DPoint> v;
        fromString(sValue, v);
        pDPointVectorArg->setValue(v);
    } else {
        assert(false);
    }   
}

void ArgList::copyArgsFrom(const ArgList& ArgTemplates)
{
    for(ArgMap::const_iterator it = ArgTemplates.m_Args.begin();
            it != ArgTemplates.m_Args.end(); it++)
    {
        string sKey = it->first;
        ArgBasePtr pArg = ArgBasePtr(it->second->createCopy());
        m_Args[sKey] = pArg;
    }
}

}

