//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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

#ifndef _LinearAnim_H_
#define _LinearAnim_H_

#include "../api.h"

#include "SimpleAnim.h"

namespace avg {

class AVG_API LinearAnim: public SimpleAnim {
public:
    LinearAnim(const bp::object& node, const std::string& sAttrName, 
            long long duration,
            const bp::object& pStartValue, 
            const bp::object& pEndValue, 
            bool bUseInt=false, 
            const bp::object& startCallback=bp::object(),
            const bp::object& stopCallback=bp::object(),
            const bp::object& abortCallback=bp::object());
    virtual ~LinearAnim();
    
protected:
    virtual float interpolate(float t);
    
private:
    float getStartPart(float start, float end, float cur);
};

AnimPtr fadeIn(const bp::object& node, long long duration, float max=1.0f,
        const bp::object& stopCallback=bp::object());

AnimPtr fadeOut(const bp::object& node, long long duration, 
        const bp::object& stopCallback=bp::object());

}

#endif 



