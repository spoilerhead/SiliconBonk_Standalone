///* (C) 2012 Spoilerhead
//#include "siliconbonk.h"
#include <Magick++.h>
#include <iostream>
#include <boost/program_options.hpp>

#include "colorspace.h"
#include "blend_modes.h"
#include <vector>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace Magick;
using namespace std;

namespace po = boost::program_options;

#include <cmath>

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
static inline float paramLtoValue(const float v) {
    return 1.f+0.5f*(v/50.f);
}


int main (int argc, char **argv) {
    cout<<"Silicon Bonk Stand Alone X4"<<endl;
    cout<<"  Spoilerhead 2012"<<endl;
    #ifdef _OPENMP
    cout<<"openMP enabled"<<endl;
    #endif
    cout<<"==========================="<<endl;

    
    //default options
    bool parHighlights = false;
    int parHue = 0;
    int parL = -0;
    int parCont = 0;
    int parMid = 0;
    int parSat = 0;
    bool verbose = false;
    bool parNormalize = false;
    bool parReduceNoise = false;
    double reduceNoiseOrder = 0.f;
    
    string inFile("");
    string outFile("");


    // Declare the supported options.
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help,h", "produce help message")
        ("input,i", po::value<string>(), "input file")
        ("output,o", po::value<string>(), "output file, if none input file will be overwritten")
        ("hue,H", po::value<int>(), "hue [0..360]")
        ("luma,L", po::value<int>(), "luminance change [-100..100]")
        ("contrast,C", po::value<int>(), "Contrast [-100..1000]")
        ("mids,M", po::value<int>(), "Mids [-100..100]")
        ("saturation,S", po::value<int>(), "saturation [0..200]")
        ("preserve-details,d", "preserve details")
        
        ("normalize,n", "Normalize Image (Increase contrast by normalizing the pixel values to span the full range of color values)") //by magick++
        ("denoise,D", po::value<int>(), "reduce noise [0..500] (very very slow)")//by magick++
        ("verbose,v", "verbose")
        
    ;
    
    po::positional_options_description p;
    p.add("input", 1);
    p.add("output", 1);


    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).
          options(desc).positional(p).run(), vm);
    po::notify(vm);    

    if ((vm.count("help")) || argc ==1) {
        cout << desc << "\n";
        return 1;
    }
    
    if (vm.count("verbose"))  {
        verbose = true;
    }

    if (vm.count("input"))  {
        //cout << "input "  << vm["input"].as< string>() << "\n";
        inFile = vm["input"].as< string>();
    } else {
        cerr<<"needs input file"<<endl;
        return -2;
    }
    if (vm.count("output"))  {
        //cout << "output "  << vm["output"].as< string>() << "\n";
        outFile = vm["output"].as< string>();
    } else {
        outFile = inFile;
    }
    
    
    //options
    if (vm.count("hue"))  {
        parHue = vm["hue"].as< int>();
    }
    if (vm.count("luma"))  {
        parL = vm["luma"].as< int>();
    }
    if (vm.count("contrast"))  {
        parCont = vm["contrast"].as< int>();
    }
    if (vm.count("mids"))  {
        parMid = vm["mids"].as< int>();
    }
    if (vm.count("saturation"))  {
        parSat = vm["saturation"].as< int>();
    }
    if (vm.count("preserve-details"))  {
        parHighlights = true;//= vm["preserve-details"].as< bool>();
    }
    if (vm.count("normalize"))  {
        parNormalize = true;
    }
    if (vm.count("denoise"))  {
        parReduceNoise = true;
        reduceNoiseOrder = (vm["denoise"].as< int>())/100.0;
    }

    
    if(verbose) {
        cout<<"Input File:  "<<inFile  <<endl;
        cout<<"Output File: "<<outFile  <<endl;
        cout<<"Hue:         "<<parHue  <<endl;
        cout<<"Luminance:   "<<parL  <<endl;
        cout<<"Contrast:    "<<parCont  <<endl;
        cout<<"Midpoint:    "<<parMid  <<endl;
        cout<<"Saturation:  "<<parSat  <<endl;
        cout<<"Details:     "<<parHighlights  <<endl;
    }
    
    //Sanity check:
    if(   (parHue<0) || (parHue>360)
       || (parL<-100) || (parL>100)
       || (parCont<-100) || (parCont>1000)
       || (parMid<-100) || (parMid>100)
       || (parSat<0) || (parSat>200)
       || (reduceNoiseOrder<0.0) || (reduceNoiseOrder>5.0)
       
       
    ) {
        cerr<<"Invalid Parameter range specified"<<endl;
        return -3;
    }

    #if 0
    bool optEQenabled = false;
    #endif
    const bool optHighlights = parHighlights;
    const float optL = parL;

    
    
    const float optH  = parHue/360.f;
    float optContrast = parCont/100.f;
    float optMid    = parMid/100.f;
    float optSat    = parSat/100.f;
   
    #if 0
    const int numBands = 6;
    int optEQ[numBands] = {0,0,0,0,0,0};
    #endif
    
    
    //precompure params
    float optLinearContrast ;
    if(optContrast >1.f) {
	    optLinearContrast = 1.f+(2.f*(optContrast-1.f)*(optContrast-1.f)); //-1 to get everything > 1, +1 as +1 is equivalent to 0 (do nothing) x2.. boost, square to get a move even contrast change
	    optContrast = 1.f;
	} else {
    	optLinearContrast = (1.f+0.f);
    }
    const float optContrastInternal = precomputeContrastFactor(optContrast);//precompute
    const float optLinearContrastInternal = optLinearContrast;
   
    #if 0
    float optEQmod[numBands];
	
	for(int i = 0;i<numBands;i++) {
	    optEQmod[i] = paramLtoValue(optEQ[i]);
	    if(optEQ[i] != 0) optEQenabled = true; //check if at least one band is enabled
	}
	#endif
	
	
    const float optLmod = paramLtoValue(optL);
    
    
    
    InitializeMagick(NULL);
    

    try {
        if(verbose) cout<<"read..."<<endl;
        Image inImg(inFile.c_str());
        const size_t width = inImg.columns();
        const size_t height = inImg.rows();
        if(verbose) cout<<"Image dimension: "<<width<<" x "<<height<<endl;
        // to preserve exifs (?)
        try{Blob exifProfile1 = inImg.profile("TIFF");}catch(...){}
        try{Blob exifProfile2 = inImg.profile("8BIM");}catch(...){}
        try{Blob exifProfile3 = inImg.profile("IPTC");}catch(...){}
        
        
        inImg.modifyImage();    //ensure that theres only one reference to the image
        
        Pixels imgCache(inImg);    //allocate pixel cache
        PixelPacket * pixels;   //pointer to pixelpacket array
        pixels = imgCache.get(0,0,width,height);
      
        if(verbose) cout<<"processing..."<<endl;
        #pragma omp parallel for
        for(size_t i = 0; i < width*height ; i++) {
            rgb_color rgb;
            rgb.r = ITOF(pixels[i].red);
            rgb.g = ITOF(pixels[i].green);
            rgb.b = ITOF(pixels[i].blue);
            
            ///- core, directly from bibble-------------------------------------------------------------
            hsv_color hsv = RGB2HCLnew(rgb);                            //to hsv

            float alpha = (COS((hsv.hue-optH)*(2.f*M_PI)))*.5f;                 //cosinal hue difference
            alpha *= clipf(hsv.sat*(2.f-hsv.sat),0.f,1.f);                                     //effect scales with saturation, quadratic curve
            float valMod = optLmod*hsv.val;                                     //modified liminance value
            float valNew = BLEND(valMod,hsv.val,alpha);                         //blend depending on alpha
            
            
            //===== TEST: band based modification =======
            #if 0 //EQ is disabled while no parames for it exist
            if(optEQenabled) {
                float band      = hsv.hue*numBands;             //basic band
                int   loBand    = band;                         //lower band bin
                int   hiBand    = (loBand+1)%numBands;          //upper band bin
                float bandFract = band-loBand;                  //fractional part
                float newL      = optEQmod[loBand]*(1.f-bandFract) + optEQmod[hiBand]*bandFract;
                float alphaEQ   = clipf(hsv.sat*(2.f-hsv.sat),0.f,1.f);        //effect scales with saturation, quadratic curve
                float valModEQ    = newL*valNew;                //modified liminance value
                valNew    = BLEND(valModEQ,valNew,alphaEQ);     //blend depending on alpha
                //hsv.sat= clipf(BLEND(newL*hsv.sat,hsv.sat,alphaEQ),0.f,1.f);
       
            }
            #endif
            
            //===== END TEST ============================
            
                               //blend depending on alpha
            
            //if(optHighlights) valNew = (-0.2782f*valNew+1.191f)*valNew;                           //prevent highlight blowout. quickly fitted in matalb
            //x = 0    0.1000    0.2000    0.3000    0.4000    0.5000    0.6000    0.7000    0.8000    0.9000    1.0000
            //yn = 0.0400    0.1200    0.2100    0.3050    0.4020    0.5010    0.6100    0.6900    0.7700    0.8400    0.9100
            
            if(optHighlights) valNew = (((-0.4481f*valNew)+0.5489f)*valNew+0.7659f)*valNew+0.03891f;   //prevent highlight blowout. quickly fitted in matalb
            
            
            
            valNew = MidPoint(valNew,optMid);                                   //midpoint adjustment
             
            valNew = ContrastBerndFast(clipf(valNew,0.f,1.f),optContrastInternal);                     //contrast (bernds method
            valNew = LinearContrast(valNew,optLinearContrastInternal); //linear contrast
            
            hsv.val = valNew;                                                   //apply and set colors to 0

            hsv.sat *= optSat;
            hsv.val = clipf(hsv.val,0.f,1.f); 
            
            rgb = HCLnew2RGB(hsv);
           
            rgb.r = clipf(rgb.r,0.f,1.f);
            rgb.g = clipf(rgb.g,0.f,1.f);
            rgb.b = clipf(rgb.b,0.f,1.f);
            
            ///---------------------------------------------------------
            pixels[i].red = FTOI(rgb.r);
            pixels[i].green = FTOI(rgb.g);
            pixels[i].blue = FTOI(rgb.b);
        }
        imgCache.sync();
        
        if(parReduceNoise)  {
            if(verbose) cout<<"Denoising..."<<endl;
            inImg.reduceNoise(reduceNoiseOrder);
        }
        
        if(parNormalize)  {
            if(verbose) cout<<"Normalizing..."<<endl;
            inImg.normalize();
        }
        
        if(verbose) cout<<"writing..."<<endl;
        //imgCache.sync();

        inImg.write(outFile.c_str());
        
    } catch(Error& err) {
        cerr<<err.what()<<endl;
        cerr<<"error"<<endl;
    }
    catch(...) {
        cerr<<"unknown error"<<endl;
    }
    
            
    return (0);
}
