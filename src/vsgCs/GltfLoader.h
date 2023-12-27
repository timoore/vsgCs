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

#pragma once

#include "Export.h"
#include "RuntimeEnvironment.h"

#include <CesiumAsync/Future.h>
#include <CesiumGltfReader/GltfReader.h>

#include <vsg/io/ReaderWriter.h>

namespace vsgCs
{

    class VSGCS_EXPORT GltfLoader : public vsg::Inherit<vsg::ReaderWriter, GltfLoader>
    {
    public:
        GltfLoader(vsg::ref_ptr<RuntimeEnvironment> in_env = RuntimeEnvironment::get());
        vsg::ref_ptr<vsg::Object>
            read(const vsg::Path& /*filename*/, vsg::ref_ptr<const vsg::Options> = {}) const override;
    protected:
        struct ReadGltfResult
        {
            vsg::ref_ptr<vsg::Node> node;
            std::vector<std::string> errors;
        };
        CesiumAsync::Future<ReadGltfResult> loadGltfNode(std::string uri) const;
        vsg::ref_ptr<RuntimeEnvironment> env;
        CesiumGltfReader::GltfReader reader;
    };
}
