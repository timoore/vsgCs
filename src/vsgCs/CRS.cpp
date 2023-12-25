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

/**
 * Lotsa stuff is from:
 * rocky c++
 * Copyright 2023 Pelican Mapping
 * MIT License
 */

#include "CRS.h"

#include "runtimeSupport.h"

#include <vsg/maths/transform.h>
#include <vsg/io/Logger.h>

#include <CesiumGeospatial/Ellipsoid.h>
#include <CesiumGeospatial/LocalHorizontalCoordinateSystem.h>

#include <proj.h>

#include <stdexcept>

namespace vsgCs
{
    void redirect_proj_log(void*, int, const char* msg)
    {
        if (msg)
        {
            vsg::info("PROJ says: " + std::string(msg));
        }
    }

    inline bool starts_with(const std::string& s, const char* pattern) {
        return s.rfind(pattern, 0) == 0;
    }

    inline bool contains(const std::string& s, const char* pattern) {
        return s.find(pattern) != std::string::npos;
    }


    //! Per thread proj threading context.
    thread_local PJ_CONTEXT* g_pj_thread_local_context = nullptr;

        //! Entry in the per-thread SRS data cache
    struct SRSEntry
    {
        PJ* pj = nullptr;
        PJ_TYPE horiz_crs_type;
        std::string wkt;
        std::string proj;
        CesiumGeospatial::Ellipsoid ellipsoid = CesiumGeospatial::Ellipsoid::WGS84;
        std::string error;
    };


    //! SRS data factory and PROJ main interface
    struct SRSFactory
    {
        std::unordered_map<std::string, SRSEntry> umap;
        static const std::string empty_string;

        //! destroy cache entries and threading context upon descope
        ~SRSFactory()
        {
            //Instance::log().debug << "SRSRepo: destructor in thread " << util::getCurrentThreadId()
            //    << " destroying " << size() << " objects" << std::endl;

            for (auto iter = umap.begin(); iter != umap.end(); ++iter)
            {
                if (iter->second.pj)
                {
                    proj_destroy(iter->second.pj);
                }
            }

            if (g_pj_thread_local_context)
                proj_context_destroy(g_pj_thread_local_context);
        }

        PJ_CONTEXT* threading_context()
        {
            if (g_pj_thread_local_context == nullptr)
            {
                g_pj_thread_local_context = proj_context_create();
                proj_log_func(g_pj_thread_local_context, nullptr, redirect_proj_log);
            }
            return g_pj_thread_local_context;
        }

        const std::string& get_error_message(const std::string& def)
        {
            auto iter = umap.find(def);
            if (iter == umap.end())
                return empty_string; // shouldn't happen
            else
                return iter->second.error;
        }

        //! retrieve or create a PJ projection object based on the provided definition string,
        //! which may be a proj string, a WKT string, an espg identifer, or a well-known alias
        //! like "spherical-mercator" or "wgs84".
        SRSEntry& get_or_create(const std::string& def)
        {
            auto ctx = threading_context();

            auto iter = umap.find(def);
            if (iter == umap.end())
            {
                PJ* pj = nullptr;

                std::string to_try = def;
                std::string ndef = toLower(def);

                //TODO: Think about epsg:4979, which is supposedly a 3D version of epsg::4326.

                // process some common aliases
                if (ndef == "wgs84" || ndef == "global-geodetic")
                    to_try = "EPSG:4979";
                else if (ndef == "spherical-mercator")
                    to_try = "EPSG:3785";
                else if (ndef == "geocentric" || ndef == "ecef")
                    to_try = "EPSG:4978";
                else if (ndef == "plate-carree" || ndef == "plate-carre")
                    to_try = "EPSG:32663";

                // try to determine whether this ia WKT so we can use the right create function
                auto wkt_dialect = proj_context_guess_wkt_dialect(ctx, to_try.c_str());
                if (wkt_dialect != PJ_GUESSED_NOT_WKT)
                {
                    pj = proj_create_from_wkt(ctx, to_try.c_str(), nullptr, nullptr, nullptr);
                }
                else
                {
                    // if it's a PROJ string, be sure to add the +type=crs
                    if (contains(ndef, "+proj"))
                    {
                        if (!contains(ndef, "proj=pipeline") && !contains(ndef, "type=crs"))
                        {
                            to_try += " +type=crs";
                        }
                    }
                    pj = proj_create(ctx, to_try.c_str());
                }

                // store in the cache (even if it failed)
                SRSEntry& new_entry = umap[def];
                new_entry.pj = pj;

                if (pj == nullptr)
                {
                    // store any error in the cache entry
                    auto err_no = proj_context_errno(ctx);
                    new_entry.error = proj_errno_string(err_no);
                }
                else
                {
                    // extract the type
                    PJ_TYPE type = proj_get_type(pj);
                    if (type == PJ_TYPE_COMPOUND_CRS)
                    {
                        PJ* horiz = proj_crs_get_sub_crs(ctx, pj, 0);
                        if (horiz)
                            new_entry.horiz_crs_type = proj_get_type(horiz);
                    }
                    else new_entry.horiz_crs_type = type;

                    // extract the ellipsoid parameters.
                    // if this fails, the entry will contain the default WGS84 ellipsoid, and that is ok.
                    PJ* ellps = proj_get_ellipsoid(ctx, pj);
                    if (ellps)
                    {
                        double semi_major, semi_minor, inv_flattening;
                        int is_semi_minor_computed;
                        proj_ellipsoid_get_parameters(ctx, ellps, &semi_major, &semi_minor, &is_semi_minor_computed, &inv_flattening);
                        proj_destroy(ellps);
                        new_entry.ellipsoid = CesiumGeospatial::Ellipsoid(semi_major, semi_major, semi_minor);
                    }

                    // extract the WKT
                    const char* wkt = proj_as_wkt(ctx, pj, PJ_WKT2_2019, nullptr);
                    if (wkt)
                        new_entry.wkt = wkt;

                    // extract the PROJ string
                    const char* proj = proj_as_proj_string(ctx, pj, PJ_PROJ_5, nullptr);
                    if (proj)
                        new_entry.proj = proj;
                }

                return new_entry;
            }
            else
            {
                return iter->second;
            }
        }

        //! fetch the projection type
        PJ_TYPE get_horiz_crs_type(const std::string& def)
        {
            return get_or_create(def).horiz_crs_type;
        }

        //! fetch the ellipsoid associated with an SRS definition
        //! that was previously created
        const CesiumGeospatial::Ellipsoid& get_ellipsoid(const std::string& def)
        {
            return get_or_create(def).ellipsoid;
        };

#if 0
        //! Get the computed bounds of a projection (or guess at them)
        const Box& get_bounds(const std::string& def)
        {
            auto ctx = threading_context();

            SRSEntry& entry = get_or_create(def);

            if (entry.pj == nullptr)
                return empty_box;

            if (entry.bounds.has_value())
                return entry.bounds.value();

            double west_lon, south_lat, east_lon, north_lat;
            if (proj_get_area_of_use(ctx, entry.pj, &west_lon, &south_lat, &east_lon, &north_lat, nullptr) &&
                west_lon > -1000)
            {
                // always returns lat/long, so transform back to this srs
                auto xform = get_or_create_operation("wgs84", def); // don't call proj_destroy on this
                PJ_COORD LL = proj_trans(xform, PJ_FWD, PJ_COORD{ west_lon, south_lat, 0.0, 0.0 });
                PJ_COORD UR = proj_trans(xform, PJ_FWD, PJ_COORD{ east_lon, north_lat, 0.0, 0.0 });
                entry.bounds = Box(LL.xyz.x, LL.xyz.y, UR.xyz.x, UR.xyz.y);
            }
            else
            {
                // We will have to make an educated guess.
                if (entry.horiz_crs_type == PJ_TYPE_GEOGRAPHIC_2D_CRS || entry.horiz_crs_type == PJ_TYPE_GEOGRAPHIC_3D_CRS)
                {
                    entry.bounds = Box(-180, -90, 180, 90);
                }
                else if (entry.horiz_crs_type != PJ_TYPE_GEOCENTRIC_CRS)
                {
                    if (contains(entry.proj, "proj=utm"))
                    {
                        if (contains(entry.proj, "+south"))
                            entry.bounds = Box(166000, 1116915, 834000, 10000000);
                        else
                            entry.bounds = Box(166000, 0, 834000, 9330000);
                    }

                    else if (contains(entry.proj, "proj=merc"))
                    {
                        // values found empirically 
                        entry.bounds = Box(-20037508.342789244, -20048966.104014594, 20037508.342789244, 20048966.104014594);
                    }

                    else if (contains(entry.proj, "proj=qsc"))
                    {
                        // maximum possible values, I think
                        entry.bounds = Box(
                            -entry.ellipsoid.semiMajorAxis(),
                            -entry.ellipsoid.semiMinorAxis(),
                            entry.ellipsoid.semiMajorAxis(),
                            entry.ellipsoid.semiMinorAxis());
                    }
                }
            }

            return entry.bounds.value();
        }
#endif

        const std::string& get_wkt(const std::string& def)
        {
            auto iter = umap.find(def);
            if (iter == umap.end())
            {
                get_or_create(def);
                iter = umap.find(def);
                if (iter == umap.end())
                    return empty_string;
            }
            return iter->second.wkt;
        }

        //! retrieve or create a transformation object
        PJ* get_or_create_operation(const std::string& firstDef, const std::string& secondDef)
        {
            auto ctx = threading_context();

            PJ* pj = nullptr;
            std::string proj;
            std::string error;

            // make a unique identifer for the transformation
            std::string def = firstDef + "->" + secondDef;

            auto iter = umap.find(def);

            if (iter == umap.end())
            {
                PJ* p1 = get_or_create(firstDef).pj;
                PJ* p2 = get_or_create(secondDef).pj;
                if (p1 && p2)
                {
                    bool p1_is_crs = proj_is_crs(p1);
                    bool p2_is_crs = proj_is_crs(p2);
                    bool normalize_to_gis_coords = true;

                    PJ_TYPE p1_type = proj_get_type(p1);
                    PJ_TYPE p2_type = proj_get_type(p2);

                    if (p1_is_crs && p2_is_crs)
                    {
                        // Check for an illegal operation (PROJ 9.1.0). We do this because
                        // proj_create_crs_to_crs_from_pj() will succeed even though the operation will not
                        // process the Z input.
                        if (p1_type == PJ_TYPE_GEOGRAPHIC_2D_CRS && p2_type == PJ_TYPE_COMPOUND_CRS)
                        {
                            std::string warning = "Warning, \"" + def + "\" transforms from GEOGRAPHIC_2D_CRS to COMPOUND_CRS. Z values will be discarded. Use a GEOGRAPHIC_3D_CRS instead";
                            redirect_proj_log(nullptr, 0, warning.c_str());
                        }

                        // if they are both CRS's, just make a transform operation.
                        pj = proj_create_crs_to_crs_from_pj(ctx, p1, p2, nullptr, nullptr);

                        if (pj && proj_get_type(pj) != PJ_TYPE_UNKNOWN)
                        {
                            const char* pcstr = proj_as_proj_string(ctx, pj, PJ_PROJ_5, nullptr);
                            if (pcstr) proj = pcstr;
                        }
                    }

                    else if (p1_is_crs && !p2_is_crs)
                    {
                        PJ_TYPE type = proj_get_type(p1);

                        if (type == PJ_TYPE_GEOGRAPHIC_2D_CRS || type == PJ_TYPE_GEOGRAPHIC_3D_CRS)
                        {
                            proj =
                                "+proj=pipeline"
                                " +step +proj=unitconvert +xy_in=deg +xy_out=rad"
                                " +step " + std::string(proj_as_proj_string(ctx, p2, PJ_PROJ_5, nullptr));

                            pj = proj_create(ctx, proj.c_str());

                            normalize_to_gis_coords = false;
                        }
                        else
                        {
                            proj =
                                "+proj=pipeline"
                                " +step +proj=longlat +ellps=WGS84 +datum=WGS84 +no_defs +towgs84=0,0,0"
                                " +step +proj=unitconvert +xy_in=deg +xy_out=rad"
                                " +step " + std::string(proj_as_proj_string(ctx, p2, PJ_PROJ_5, nullptr)) +
                                " +step +proj=unitconvert +xy_in=rad +xy_out=deg";

                            pj = proj_create(ctx, proj.c_str());

                            normalize_to_gis_coords = false;
                        }
                    }

                    else if (!p1_is_crs && p2_is_crs)
                    {
                        proj =
                            "+proj=pipeline"
                            " +step +inv " + std::string(proj_as_proj_string(ctx, p1, PJ_PROJ_5, nullptr)) +
                            " +step " + std::string(proj_as_proj_string(ctx, p2, PJ_PROJ_5, nullptr)) +
                            " +step +proj=unitconvert +xy_in=rad +xy_out=deg";

                        pj = proj_create(ctx, proj.c_str());

                        normalize_to_gis_coords = false;
                    }

                    else
                    {
                        proj =
                            "+proj=pipeline"
                            " +step +inv " + std::string(proj_as_proj_string(ctx, p1, PJ_PROJ_5, nullptr)) +
                            " +step " + std::string(proj_as_proj_string(ctx, p2, PJ_PROJ_5, nullptr));

                        pj = proj_create(ctx, proj.c_str());

                        normalize_to_gis_coords = false;
                    }

                    // integrate forward and backward vdatum conversions if necessary.
                    if (pj && normalize_to_gis_coords)
                    {
                        // re-order the coordinates to GIS standard long=x, lat=y
                        pj = proj_normalize_for_visualization(ctx, pj);
                    }
                }

                if (!pj && error.empty())
                {
                    auto err_no = proj_context_errno(ctx);
                    if (err_no != 0)
                    {
                        error = proj_errno_string(err_no);
                        //Instance::log().warn << error << " (\"" << def << "\")" << std::endl;
                    }
                }

                auto& new_entry = umap[def];
                new_entry.pj = pj;
                new_entry.proj = proj;
                new_entry.error = error;
            }
            else
            {
                pj = iter->second.pj;
            }
            return pj;
        }
    };

    const std::string SRSFactory::empty_string;

    // create an SRS repo per thread since proj is not thread safe.
    thread_local SRSFactory g_srs_factory;

    class CRS::ConversionOperation
    {
    public:
        virtual vsg::dvec3 getECEF(const vsg::dvec3& coord) = 0;
        virtual vsg::dmat4 getENU(const vsg::dvec3& coord) = 0;

    };

    // A no-op CRS. Either the coordinates are ECEF (x, y, z), or there isn't actually a globe.
    class EPSG4978 : public CRS::ConversionOperation
    {
    public:
        vsg::dvec3 getECEF(const vsg::dvec3& coord) override
        {
            return coord;
        }

        vsg::dmat4 getENU(const vsg::dvec3& coord) override
        {
            return vsg::translate(coord);
        }
    };

    // Bog-standard WGS84 longitude, latitude, height to ECEF
    class EPSG4979 : public CRS::ConversionOperation
    {
    public:
        vsg::dvec3 getECEF(const vsg::dvec3& coord) override;
        vsg::dmat4 getENU(const vsg::dvec3& coord) override;
    };


    vsg::dvec3 EPSG4979::getECEF(const vsg::dvec3& coord)
    {
        using namespace CesiumGeospatial;
        auto glmvec = Ellipsoid::WGS84.cartographicToCartesian(Cartographic(coord.x, coord.y, coord.z));
        return glm2vsg(glmvec);
    }

    vsg::dmat4 EPSG4979::getENU(const vsg::dvec3& coord)
    {
        using namespace CesiumGeospatial;
        LocalHorizontalCoordinateSystem localSys(Cartographic(coord.x, coord.y, coord.z));
        vsg::dmat4 localMat = glm2vsg(localSys.getLocalToEcefTransformation());
        return localMat;
    }

    // The meat of vsgCs::CRS: conversion operations implemented by PROJ. The actual PROJ operation
    // will need a thread context.

    class ProjConversion : public CRS::ConversionOperation
    {
    public:
        ProjConversion(const std::string& in_sourceCRS)
            : sourceCRS(in_sourceCRS)
        {
        }

        std::string sourceCRS;

        vsg::dvec3 getECEF(const vsg::dvec3& coord) override
        {
            auto handle = g_srs_factory.get_or_create_operation(sourceCRS, "ecef");
            if (!handle)
            {
                throw std::runtime_error("can't create conversion from " + sourceCRS + " to ECEF.");
            }
            proj_errno_reset((PJ*)handle);
            PJ_COORD out = proj_trans((PJ*)handle, PJ_FWD, PJ_COORD{ coord.x, coord.y, coord.z });
            int err = proj_errno((PJ*)handle);
            vsg::dvec3 result(0.0, 0.0, 0.0);
            if (err != 0)
            {
                throw std::runtime_error(std::string("PROJ error: ") + proj_errno_string(err));
            }
            else
            {
                result.set(out.xyz.x, out.xyz.y, out.xyz.z);
            }
            return result;
        }

        vsg::dmat4 getENU(const vsg::dvec3& coord) override
        {
            using namespace CesiumGeospatial;
            vsg::dvec3 origin = getECEF(coord);
            auto& ellipsoid = g_srs_factory.get_ellipsoid(sourceCRS);
            LocalHorizontalCoordinateSystem lhcs(vsg2glm(origin),
                                                 LocalDirection::East, LocalDirection::North,
                                                 LocalDirection::Up,
                                                 1.0, ellipsoid);
            return glm2vsg(lhcs.getLocalToEcefTransformation());
        }
    };

    CRS::CRS(const std::string& name)
    {
        if (name == "epsg:4978" || name == "null")
        {
            _converter = std::make_shared<EPSG4978>();
        }
        if (name == "epsg:4979" || name == "wgs84")
        {
            _converter = std::make_shared<EPSG4979>();
        }
        else
        {
            _converter = std::make_shared<ProjConversion>(name);
        }
    }

    vsg::dvec3 CRS::getECEF(const vsg::dvec3& coord)
    {
        return _converter->getECEF(coord);
    }

    vsg::dmat4 CRS::getENU(const vsg::dvec3& coord)
    {
        return _converter->getENU(coord);
    }
}

