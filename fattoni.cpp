///* (C) 2012-13 Spoilerhead
#include <boost/program_options.hpp>
#include <cmath>
#include <iostream>
#include <Magick++.h>
#include <string>
#include <vector>

#include "SiliconBonkConfig.h"  //version numbers and more, generated by cmake

#include "blend_modes.h"
#include "colorspace.h"


#ifdef _OPENMP
#include <omp.h>
#endif

using namespace Magick;
using namespace std;

namespace po = boost::program_options;

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
    cout<<"FatToni Stand Alone "<<BONK_VERSION_MAJOR<<"."<<BONK_VERSION_MINOR<<"."<<BONK_VERSION_PATCH<<" "<<endl;
    cout<<"  Spoilerhead 2012-2013"<<endl;
    #ifdef _OPENMP
    cout<<"openMP enabled"<<endl;
    #endif
    cout<<"==========================="<<endl;

    
    //default options
    int parHH = 0;
    int parHS = 0;
    int parSH = 0;
    int parSS = 0;
    
    int parMid = 0;
    int parMix = 0;
    int parCont = 85;
    int parBaseH = 0;
    int parBaseS = 0;
    int parBaseL = 100;
    
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

        ("light-hue", po::value<int>(), "highlight hue [0..360]")
        ("light-sat", po::value<int>(), "highlight  saturation [0..100]")
        ("shadow-hue", po::value<int>(), "shadow hue [0..360]")
        ("shadow-sat", po::value<int>(), "shadow  saturation [0..100]")

        ("mix,m", po::value<int>(), "separation (mix) [0..100]")
        ("mids,M", po::value<int>(), "Mids [-100..100]")
        ("contrast,C", po::value<int>(), "Contrast [0..100]")

        ("base-hue", po::value<int>(), "base hue [0..360]")
        ("base-sat", po::value<int>(), "base  saturation [0..100]")
        ("base-luma", po::value<int>(), "base luminance [0..100]")

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
    if (vm.count("light-hue"))  {
        parHH = vm["light-hue"].as< int>();
    }
    if (vm.count("light-saturation"))  {
        parSH = vm["light-saturation"].as< int>();
    }
    if (vm.count("shadow-hue"))  {
        parHS = vm["shadow-hue"].as< int>();
    }
    if (vm.count("shadow-saturation"))  {
        parSS = vm["shadow-saturation"].as< int>();
    }
    
    if (vm.count("contrast"))  {
        parCont = vm["contrast"].as< int>();
    }
    if (vm.count("mids"))  {
        parMid = vm["mids"].as< int>();
    }
    if (vm.count("mix"))  {
        parMix = vm["mix"].as< int>();
    }
    
    if (vm.count("base-hue"))  {
        parBaseH = vm["base-hue"].as< int>();
    }
    if (vm.count("base-saturation"))  {
        parBaseS = vm["base-saturation"].as< int>();
    }
    if (vm.count("base-luma"))  {
        parBaseL = vm["base-luma"].as< int>();
    }

    if (vm.count("normalize"))  {
        parNormalize = true;
    }
    if (vm.count("denoise"))  {
        parReduceNoise = true;
        reduceNoiseOrder = (vm["denoise"].as< int>())/100.0;
    }

    
    if(verbose) {
        cout<<"Input File:              "<<inFile  <<endl;
        cout<<"Output File:             "<<outFile  <<endl;
        cout<<"Highlight Hue:           "<<parHH  <<endl;
        cout<<"Highlight Saturation:    "<<parSH  <<endl;
        cout<<"Shadow Hue:              "<<parHS  <<endl;
        cout<<"Shadow Saturation:       "<<parSS  <<endl;

        cout<<"Mix:                     "<<parMix  <<endl;
        cout<<"Midpoint:                "<<parMid  <<endl;
        cout<<"Contrast:                "<<parCont  <<endl;

        cout<<"Base Hue:                "<<parBaseH  <<endl;
        cout<<"Base Saturation:         "<<parBaseS  <<endl;
        cout<<"Base Luminance:          "<<parBaseL  <<endl;
    }
    
    //Sanity check:
    if(   (parHH<0) || (parHH>360)
       || (parSH<0) || (parSH>100)
       || (parHS<0) || (parHS>360)
       || (parSS<0) || (parSS>100)
       || (parMix<0) || (parMix>100)
       || (parCont<-100) || (parCont>1000)
       || (parMid<-100) || (parMid>100)
       || (parBaseH<0) || (parBaseH>360)
       || (parBaseS<0) || (parBaseS>100)
       || (parBaseL<0) || (parBaseL>100)
       
       || (reduceNoiseOrder<0.0) || (reduceNoiseOrder>5.0)
       
       
    ) {
        cerr<<"Invalid Parameter range specified"<<endl;
        return -3;
    }

    const float optHH = parHH/360.f;
	const float optSH = parSH/100.f;
	const float optHS = parHS/360.f;
	const float optSS = parSS/100.f;	
	const float optCont = parCont/100.f;
	const float optMid = parMid/200.f;
	const float optMix = parMix/100.f;
	
	const float optBaseH = parBaseH/360.f;
	const float optBaseS = parBaseS/100.f;
	const float optBaseL = parBaseL/100.f;
    
    
    //precompure params
    
    hsv_color highlight;
    highlight.sat   = optSH;
    highlight.hue   = optHH;
    highlight.val = 0.5f;


    hsv_color shadow;
    shadow.sat      = optSS;    
    shadow.hue      = optHS;
    shadow.val = 0.5f;
    
    hsv_color base;
    base.sat    = optBaseS;
    base.hue    = optBaseH;
    base.val    = optBaseL;
 
    
    rgb_color srgb = HSL2RGB(shadow);
    rgb_color hrgb = HSL2RGB(highlight);
    rgb_color basergb = HSL2RGB(base);

    
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
            //Compute position on the linear blend between the 2 tonings
            float luma = (rgb.r+rgb.g+rgb.b)/3.f; //pixel luminance
			float toneAlpha = clipf(luma+(optMid),0.f,1.f);
			toneAlpha = clipf(ContrastBerndFast(toneAlpha,0.5f*optMix),0.f,1.f);    //Mix creates a harder "edge"

            //compute toning color for this pixel
			rgb_color toning;
            toning.r = BLEND( hrgb.r, srgb.r, toneAlpha);
			toning.g = BLEND( hrgb.g, srgb.g, toneAlpha);
			toning.b = BLEND( hrgb.b, srgb.b, toneAlpha);
			
			/* test mid toning for grubernd rgb_color mrgb;
			mrgb.r = 1.0;
			mrgb.g=0.5f;
			mrgb.b=0.5f;
			
			
			toning.r = BLEND( hrgb.r, srgb.r, toneAlpha);
			toning.g = BLEND( hrgb.g, srgb.g, toneAlpha);
			toning.b = BLEND( hrgb.b, srgb.b, toneAlpha);
			
			toning.r = BLEND( mrgb.r, toning.r, 1.f-2.f*abs((toneAlpha-0.5f)));
			toning.g = BLEND( mrgb.g, toning.g, 1.f-2.f*abs((toneAlpha-0.5f)));
			toning.b = BLEND( mrgb.b, toning.b, 1.f-2.f*abs((toneAlpha-0.5f)));*/

			//--------------------------------

            //actuall toning phase, blend between overlay and mix, overlay is a hard version, mix a soft one
			rgb.r = BLEND ( OVERLAY( toning.r ,rgb.r), clipf(MIX( toning.r ,rgb.r),0.f,1.f),   optCont); //clip to avoid underflowing!
			rgb.g = BLEND ( OVERLAY( toning.g ,rgb.g), clipf(MIX( toning.g ,rgb.g),0.f,1.f),   optCont);
			rgb.b = BLEND ( OVERLAY( toning.b ,rgb.b), clipf(MIX( toning.b ,rgb.b),0.f,1.f),   optCont);

            //Multiply with the base color	
            rgb.r = MULTIPLY(basergb.r, rgb.r);
			rgb.g = MULTIPLY(basergb.g, rgb.g);
			rgb.b = MULTIPLY(basergb.b, rgb.b);
           
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
