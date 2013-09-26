///* (C) 2012-13 Spoilerhead
#ifndef COLORMANAGEMENT_H
#define COLORMANAGEMENT_H

#include <lcms2.h>
#include <Magick++.h>

#include "colorspace.h"

class WorkingSpace {
    public:
    WorkingSpace(Magick::Blob &inputProfile);
    ~WorkingSpace();
    
    inline rgb_color toWorkingSpace(const rgb_colorUInt16 &in) {
        rgb_color rgb;
        rgb.r = ITOF(in.r);
        rgb.g = ITOF(in.g);
        rgb.b = ITOF(in.b);

        cmsDoTransform(toWorkspace, &rgb, &rgb, 1);
        return rgb;
    }
    inline rgb_color fromWorkingSpace(rgb_color &in) {
        rgb_color ret;
        cmsDoTransform(fromWorkspace, &in, &ret, 1);
        return ret;
    }

    inline void PlanetoWorkingSpace(Magick::PixelPacket *pixels, const size_t count) {
        cmsDoTransform(toWorkspacePlane, pixels, pixels, count);
    }
    
    inline void PlanefromWorkingSpace(Magick::PixelPacket *pixels, const size_t count) {
        cmsDoTransform(fromWorkspacePlane, pixels, pixels, count);
    }
    
    private:
    cmsHTRANSFORM toWorkspace;
    cmsHTRANSFORM fromWorkspace;
    cmsHTRANSFORM toWorkspacePlane;
    cmsHTRANSFORM fromWorkspacePlane;
};

#endif //COLORMANAGEMENT_H

