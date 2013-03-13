#ifndef siliconbonk_h
#define siliconbonk_h

#include "blend_modes.h"
#include "colorspace.h"

#ifndef M_PI
#define M_PI    3.14159265358979323846f
#endif

//redefine here for faster versions
#define FASTLOCAL static inline

#define SIN fastsin
#define COS fastcos


float FASTLOCAL LinearContrast(const float val, const float c) {
    return ((val-0.5f)*c)+0.5f; //linear contrast
}

// needs precomputed factor
float FASTLOCAL ContrastBerndFast(const float val, const float c) {
    return val-c*SIN(2*M_PI*val);
}

float FASTLOCAL precomputeContrastFactor(const float c) {
    return ((c)/4.4f);
}

float FASTLOCAL MidPoint(const float val, const float strength) {
//    return val + (  -strength*(val*val)+strength*val);
    return val + (  strength*(val-(val*val)));

}

//parameter setting for luminance to internal value
float FASTLOCAL paramLtoValue(const float v) {
    return 1.f+0.5f*(v/50.f);
}

#endif //siliconbonk_h
