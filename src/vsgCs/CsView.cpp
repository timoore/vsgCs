/* <editor-fold desc="MIT License">

Copyright(c) 2023 Timothy Moore

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

</editor-fold> */

#include "CsView.h"

#include "ViewDependentStateExt.h"
#include "vsgCs/runtimeSupport.h"
#include <vsg/app/WindowResizeHandler.h>

using namespace vsgCs;

// XXX Need to handle the case where the camera is installed after CsView is created.

CsView::CsView()
{
    viewDependentState = ViewDependentStateExt::create();
}

CsView::CsView(vsg::ref_ptr<vsg::Camera> in_camera,
               vsg::ref_ptr<vsg::Node> in_scenegraph)
    : Inherit(in_camera, in_scenegraph)
{
    auto vdlExt = ViewDependentStateExt::create();
    viewDependentState = vdlExt;
    vdlExt->update(*this);
}

void CsView::accept(vsg::Visitor& visitor)
{
    vsg::WindowResizeHandler* resizeHandler = dynamic_cast<vsg::WindowResizeHandler*>(&visitor);
    Inherit::accept(visitor);
    if (resizeHandler)
    {
        auto vdsExt = ref_ptr_cast<ViewDependentStateExt>(viewDependentState);
        if (vdsExt)
        {
            vdsExt->update(*this);
        }
    }
}
