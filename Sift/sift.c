#include "sift.h"
#include "img_io.h"

#undef DUMP_OCTAVE
#undef DUMP_SCALESPACE
#undef DUMP_DOG


// scale space
float scaleSpace_g[MAX_O][MAX_S][MAX_OCTAVE_SIZE];

// scale space: equivalent with dynamic allocation
/*for(i = 0; i < NUMBER_OF_OCTAVES; i++){
for(j = 0; j < NUMBER_OF_SCALES; j++){
scaleSpace[i][j] =
(float*)malloc(octavesW_g[i] * octavesH_g[i] * sizeof(float));
}
}*/

// difference of gaussians
float dog_g[MAX_O][MAX_S - 1][MAX_OCTAVE_SIZE];


void
sift(unsigned char* img,
          int w, int h, char* out_filename,
          unsigned char octaves[MAX_O*MAX_OCTAVE_SIZE],
          int* octavesW, int* octavesH,
          float scaleSpace[][MAX_S][MAX_OCTAVE_SIZE],
          float dog[][MAX_S-1][MAX_OCTAVE_SIZE],
		  pointList* keyPointList, int O, int S, unsigned char* r, unsigned char* g, unsigned char* b) {
  
    /************************
     * Common Variables
     ***********************/
    int i, j;
#if (defined DUMP_OCTAVE || defined DUMP_SCALESPACE || defined DUMP_DOG)
    // file name
    char tmpfile[10];
#endif
    /************************
     * Global Variables
     ***********************/
    // sigma at each scale
    float sigma0 = 1.6f;
    float sigmas[MAX_O][MAX_S];

    /************************
     * Variable computation
     ***********************/
    // gaussian sigmas at each scale
    for(i = 0; i < O; i++){
        for(j = 0; j < S; j++){
            sigmas[i][j] = sigma0 * pow(sqrt(2.0f), (float)j/(float)(S-3) );
        }
    }

#if VERBOSE == 1
    printf("Image size: %dx%d\n", w, h);
    printf("Number of octaves (O):%d\n", O);
    printf("Scales per octave (S):%d\n", S);
    for(i = 0; i < O; i++){
        printf("size of octave level %d: %dx%d\n", i, octavesW[i], octavesH[i]);
    }
#endif
    
    /************************
     * Step 1:
     * Build Octave Base by downsampling
     ***********************/
    buildOB(img, octaves, O, octavesW, octavesH);
#ifdef DUMP_OCTAVE
    for(i = 0; i < O; i++){
        sprintf(tmpfile,"%s%d%s", "OB", i, ".pgm");
        write_pgm(octaves[i], octavesW[i], octavesH[i], tmpfile);
    }
#endif

    /************************
     * Step 2:
     * Build Scale Space by Gaussian
     ***********************/

    buildSS(octaves, scaleSpace, O, S, octavesW, octavesH, sigmas);

#ifdef DUMP_SCALESPACE
    for(i = 0; i < O; i++){
        for(j = 0; j < S; j++){
            sprintf(tmpfile, "%s%d%d%s", "SS", i, j, ".pgm");
            write_float_pgm(scaleSpace[i][j],octavesW[i],octavesH[i],tmpfile,1);
        }
    }
#endif

    /************************
     * Step 3:
     * Difference of Gaussians (DoG)
     ***********************/
    DoG(dog, scaleSpace, O, S, octavesW, octavesH);
#ifdef DUMP_DOG
    for(i = 0; i < O; i++){
        for(j = 0; j < S-1; j++){
            sprintf(tmpfile,"%s%d%d%s", "DoG", i, j, ".pgm");
            write_float_pgm(dog[i][j], octavesW[i], octavesH[i], tmpfile, 2);
        }
    }
#endif


    /************************
     * Step 4:
     * Find extreme points
     ***********************/
    extreme(keyPointList, dog, O, S, octavesW, octavesH);

    /************************
     * Keypoint Orientation
     ***********************/
    orientation(keyPointList, scaleSpace, sigmas, octavesW);

    /************************
     * Render Results
     * Draw circle around keypoints
     ***********************/
    render(img, out_filename, octavesW, octavesH, keyPointList, sigmas,r,g,b);

}// sift()


void AppelSift(unsigned char* input, int xsize, int ysize, unsigned char* r, unsigned char* g, unsigned char* b)
{
	int i;
	// Octaves generated by down sampling
	// Careful, this memory allocation is far from optimal.
	// All octaves are given the same size as the original image
	// octavesW_g[i] * octavesH_g[i] is the actual memory necessary for octave i
	unsigned char octaves_g[MAX_O*MAX_OCTAVE_SIZE];

	// difference of gaussians: equivalent with dynamic allocation
	/*for(i = 0; i < NUMBER_OF_OCTAVES; i++){
	for(j = 0; j < NUMBER_OF_SCALES-1; j++){
	dog[i][j] =
	(float*)malloc( octavesW_g[i] * octavesH_g[i] * sizeof(float));
	}
	}*/

	/* Octaves: equivalent with dynamic allocation
	for(i = 0; i < NUMBER_OF_OCTAVES; i++){
	octaves[i] = (unsigned char*)
	malloc(octavesW[i] * octavesH[i] * sizeof(unsigned char));
	}*/


	// width of each octave
	int octavesW_g[MAX_O];
	// height of each octave
	int octavesH_g[MAX_O];

	// extreme points
	pointList keyPointList_g;

	// computing the width of octaves at each level
	octavesW_g[0] = xsize;
	for (i = 1; i < NUMBER_OF_OCTAVES; i++){
		octavesW_g[i] = octavesW_g[i - 1] / 2;
	}
	// computing the height of octaves at each level
	octavesH_g[0] = ysize;
	for (i = 1; i < NUMBER_OF_OCTAVES; i++){
		octavesH_g[i] = octavesH_g[i - 1] / 2;
	}

	// Setting the original number of detected points to 0
	keyPointList_g.size = 0;

	sift(input, xsize, ysize, "Resultats/A_Results.pgm", octaves_g, octavesW_g,
		octavesH_g, scaleSpace_g, dog_g, &keyPointList_g,
		NUMBER_OF_OCTAVES, NUMBER_OF_SCALES, r,g,b);
}