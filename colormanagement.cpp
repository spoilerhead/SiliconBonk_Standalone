///* (C) 2012-13 Spoilerhead
#include <cassert>
#include <iostream>
#include "colormanagement.h"

using namespace std;

WorkingSpace::WorkingSpace(Magick::Blob &inputProfile) {
    cmsHPROFILE imgProfile = NULL;
    
    if(inputProfile.length() != 0) {
        imgProfile = cmsOpenProfileFromMem(inputProfile.data(), inputProfile.length());
    } else {
        cout << "Fallback to sRGB profile" << endl;
        imgProfile = cmsCreate_sRGBProfile();   //use default sRGB profile if there's no other
    }
    if(imgProfile == NULL) {
        cout << "Failed to load profile" << endl;
    }
    
    
    //cmsHPROFILE labProfile = cmsCreateLab2Profile(NULL); //create LAB profile with D50 white
    const double ProPhotoRGB [3][3] = {
      {0.797675, 0.135192, 0.0313534},
      {0.288040, 0.711874, 0.000086},
      {0.000000, 0.000000, 0.825210}
    };

    cmsToneCurve* Gamma = cmsBuildGamma(NULL, 2.0); //gamma 2
    //cmsToneCurve* Gamma = cmsBuildGamma(NULL, 1.0); //gamma 1
    cmsToneCurve* Gamma3[3];
    Gamma3[0] = Gamma3[1] = Gamma3[2] = Gamma;

    if(Gamma == NULL) {
        cout << "Failed Gamma" << endl;
    }
    cmsHPROFILE workingSpace = cmsCreateRGBProfile(cmsD50_xyY(),  (cmsCIExyYTRIPLE*) ProPhotoRGB, Gamma3);
    if(workingSpace == NULL) {
        cout << "Failed to create working space" << endl;
    }                             
    cmsFreeToneCurve(Gamma);
    
    assert((QuantumDepth == 8) || (QuantumDepth == 16));    //ensure quantum depth of image magick
    const cmsUInt32Number format = (QuantumDepth == 16)?TYPE_RGBA_16:TYPE_RGBA_8;
    const cmsUInt32Number formatSingle = (QuantumDepth == 16)?TYPE_RGB_16:TYPE_RGB_8;
    
    toWorkspace = cmsCreateTransform(imgProfile, TYPE_RGB_FLT, workingSpace, TYPE_RGB_FLT, INTENT_PERCEPTUAL, 0);
    if(toWorkspace == NULL) {
        cout << "Failed to create toWorkspace" << endl;
    }  
    
    fromWorkspace = cmsCreateTransform(workingSpace, TYPE_RGB_FLT, imgProfile, /*TYPE_RGB_FLT*/formatSingle, INTENT_PERCEPTUAL, 0);
    if(fromWorkspace == NULL) {
        cout << "Failed to create fromWorkspace" << endl;
    }  
    
    /*toWorkspacePlane = cmsCreateTransform(imgProfile, format, workingSpace, format, INTENT_PERCEPTUAL, cmsFLAGS_NOOPTIMIZE);
    if(toWorkspacePlane == NULL) {
        cout << "Failed to create toWorkspace" << endl;
    }  
    fromWorkspacePlane = cmsCreateTransform(workingSpace, format, imgProfile, format, INTENT_PERCEPTUAL, cmsFLAGS_NOOPTIMIZE);
    if(fromWorkspacePlane == NULL) {
        cout << "Failed to create fromWorkspace" << endl;
    } 
    */ 

    //Close Profiles        
    cmsCloseProfile(imgProfile);
    cmsCloseProfile(workingSpace);
}

WorkingSpace::~WorkingSpace() {
    //free memory
    cmsDeleteTransform(toWorkspace);
    cmsDeleteTransform(fromWorkspace);
    cmsDeleteTransform(toWorkspacePlane);
    cmsDeleteTransform(fromWorkspacePlane);
}

