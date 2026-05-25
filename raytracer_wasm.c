#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <emscripten.h>
#include "raytracer/src/structs.h"
#include "raytracer/src/shadowRay.h"
#include "raytracer/src/textureLookUp.h"
#include "raytracer/src/raytracer.h"
#include "raytracer/src/files.h"

EMSCRIPTEN_KEEPALIVE
int render(const char* inputFileName){
    objects_t objects;
    camera_t camera;
    objects_init(&objects);

    FILE *inputFilePointer;
    FILE *outputFilePointer;

    int readerOutput;
    int writerOutput;

    char outputFileName[255] = "";
    char *token1;
    const char *delim = ".";

    camera.d = 5.0;

    // Viewing Coordinate System
    vector_t u, v, w;


    // Looks for input file and hands it off to fileReader
    // Checks for bad inputs
    inputFilePointer = fopen(inputFileName, "r");
    if (inputFilePointer == NULL){
        printf("File not found\n");
        return 1;
    }
    else {
        readerOutput = fileReader(inputFilePointer, &objects, &camera);
        fclose(inputFilePointer);
        if (readerOutput){
            return 1;
        }
         
    }

    camera.image = malloc(sizeof(vector_t) * (camera.width * camera.height));
    // u' = view_dir * up_dir, u = u' / ||u'||
    // w = u * v = (uy*vz-uz*vy, uz*vx-ux*vz, ux*vy-uy*vx)
    // ||v|| = (vx^2+vy^2+vz^2)^(1/2)
    float templength;
    crossProduct(&u, camera.vdir, camera.up);
    templength = vectorLength(u);
    u.x = u.x / templength; 
    u.y = u.y / templength; 
    u.z = u.z / templength; 

    crossProduct(&v, u, camera.vdir);
    templength = vectorLength(v);
    v.x = v.x / templength; 
    v.y = v.y / templength; 
    v.z = v.z / templength; 

    // ul = view_origin + d×n – (w/2)×u + (h/2)×v
    // ur = view_origin + d×n + (w/2)×u + (h/2)×v
    // ll = view_origin + d×n – (w/2)×u – (h/2)×v
    // lr = view_origin + d×n + (w/2)×u – (h/2)×v
    // w = 2*d*tan(1/2 * HFOV)
    // aspect_ratio = width / height
    // h = w / aspect_ratio
    vector_t tempV = v;
    vector_t tempU = u;
    float aspect_ratio = (float)camera.width / (float)camera.height;
    float tempAngle = camera.hfov * (3.14159265359 / 180);
    float window_width = 2 * camera.d * tanf(1.0/2.0 * tempAngle); 
    float window_height = window_width / aspect_ratio;
    vector_t n = camera.vdir;
    vectorMult(&n, 1 / vectorLength(n));
    vectorMult(&n, camera.d);
    

    // ul
    camera.ul = camera.eye;
    vectorMult(&tempU, -window_width * 0.5);    
    vectorMult(&tempV, window_height * 0.5);     
    vectorAddVector(&camera.ul, tempU);
    vectorAddVector(&camera.ul, tempV);
    vectorAddVector(&camera.ul, n);

    // Cleanup
    tempV = v;
    tempU = u;

    // ur
    camera.ur = camera.eye;
    vectorMult(&tempU, window_width/2.0);
    vectorMult(&tempV, window_height/2.0);
    vectorAddVector(&camera.ur, tempU);
    vectorAddVector(&camera.ur, tempV);
    vectorAddVector(&camera.ur, n);

    // Cleanup
    tempV = v;
    tempU = u;

    // ll
    camera.ll = camera.eye;
    vectorMult(&tempU, -window_width/2.0);
    vectorMult(&tempV, -window_height/2.0);
    vectorAddVector(&camera.ll, tempU);
    vectorAddVector(&camera.ll, tempV);
    vectorAddVector(&camera.ll, n);

    // Cleanup
    tempV = v;
    tempU = u;

    // lr
    camera.lr = camera.eye;
    vectorMult(&tempU, window_width/2.0);
    vectorMult(&tempV, -window_height/2.0);
    vectorAddVector(&camera.lr, tempU);
    vectorAddVector(&camera.lr, tempV);
    vectorAddVector(&camera.lr, n);

    // Creates a new file and hands it off to imageWriter
    // Checks for errors
    outputFilePointer = fopen("output.ppm", "w");
    if (!outputFilePointer){
        printf("Failed to create file\n");
        return 1;
    }

    writerOutput = imageWriter(outputFilePointer, &objects, &camera);
    fclose(outputFilePointer);
    free(camera.image);

    // Checks for an error from imageWriter
    if (writerOutput){
        return 1;
    }

    return 0;

}
