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

#include "CsViewer.h"

#include "vsgCs/runtimeSupport.h"
#include "vsgCs/Tracing.h"

using namespace CsApp;

void CsViewer::handleEvents()
{
    // X Windows famously sends way too many motion events. So, if there is another MoveEvent in the
    // buffer, then skip the current motion event.
    for (auto itr = _events.begin(); itr != _events.end(); ++itr)
    {
        auto& vsg_event = *itr;
        auto eventAsMove = vsgCs::ref_ptr_cast<vsg::MoveEvent>(vsg_event);
        if (eventAsMove.valid() && next(itr) != _events.end())
        {
            auto nextEvent = vsgCs::ref_ptr_cast<vsg::MoveEvent>(*next(itr));
            if (nextEvent.valid() && nextEvent->mask == eventAsMove->mask)
            {
                continue;
            }
        }
        for (auto& handler : _eventHandlers)
        {
            vsg_event->accept(*handler);
        }
    }
}

bool CsViewer::advanceToNextFrame(double simulationTime)
{
    VSGCS_ZONESCOPED;
    return Inherit::advanceToNextFrame(simulationTime);
}
