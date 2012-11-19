

#include "chuck_dl.h"
#include "chuck_def.h"

#include <stdio.h>
#include <limits.h>
#include <math.h>

//
//  main.cpp
//  Wavemesh
//
//  Created by Bill Hudson on 6/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#define X_SIZE 25
#define Y_SIZE 25
#define DOWNSAMPLE 8


#include <iostream>
#include <math.h>

inline float clipf (float x,float min,float max) {
    if (x < min) return min;
    else if (x > max) return max;
    else return x;
}

inline float gauss (float x,float y) {
    return .5 * pow(2.71828183, - ( ( x * 3.1415926535 ) * ( x * 3.1415926535 ) * 0.5 ) 
               + (2.71828183 - ( ( y * 3.1415926535) * ( y * 3.1415926535 ) * 0.5 )));
}

class waveMesh {
    int V[X_SIZE][Y_SIZE];
    int up[X_SIZE][Y_SIZE][2];
    int right[X_SIZE][Y_SIZE][2];
    int down[X_SIZE][Y_SIZE][2];
    int left[X_SIZE][Y_SIZE][2];
    
    int curr; //switch between meshes
    int step;
    
public:
    waveMesh();
    void initMesh(int amp,int centre_x,int centre_y,int width);
    void initMeshWithArray(int array[X_SIZE][Y_SIZE]);
    void calcNext();
    void setMeshValue(int x,int y,int value);

    float returnValue(int x,int y);
    void print();
    
    void copyArray(int arrayFrom[X_SIZE][Y_SIZE],int arrayTo[X_SIZE][Y_SIZE][2]);
    void copyArray(int arrayFrom[X_SIZE][Y_SIZE],int arrayTo[X_SIZE][Y_SIZE]);
    
    void printUp();
    void printDown();
    void printLeft();
    void printRight();
};

waveMesh::waveMesh() {
    // array cleaning
    for (int i = 0;i<X_SIZE;i++) {
        for (int j = 0;j<Y_SIZE;j++) {
            V[i][j] = 0;
            up[i][j][0] = up[i][j][1] = 0;
            right[i][j][0] = right[i][j][1] = 0;
            down[i][j][0] = down[i][j][1] = 0;
            left[i][j][0] = left[i][j][1] = 0;
        }
    }
    
    curr = 2;
    step = DOWNSAMPLE;
}



void waveMesh::calcNext() {
    // first calculate central nodes.
    int i,j,Vtemp;
    if (step == 0) {
        step = DOWNSAMPLE;
        
        for (i = 1;i<X_SIZE-1;i++) {
            for (j = 1;j<Y_SIZE-1;j++) {
                // calculate V by adding neighbor outputs and dividing by two (== bitshift to right)
                if (curr != 2) {
                    V[i][j] = Vtemp = 
                    (down[i][j+1][!curr] + left[i+1][j][!curr] + right[i-1][j][!curr] + up[i][j-1][!curr]) >> 1;
                }
                else { Vtemp = V[i][j]; }
                
                // Calculate new outputs by taking V of current cell and subtracting incoming waves.
                up[i][j][curr] = Vtemp - down[i][j+1][!curr];
                right[i][j][curr] = Vtemp - left[i+1][j][!curr];
                down[i][j][curr] = Vtemp - up[i][j-1][!curr];
                left[i][j][curr] = Vtemp - right[i-1][j][!curr];
            }
        }
        if (curr == 2) curr = 0;
        
        
        // bovenkant
        j = 0;
        for (i = 1;i<X_SIZE-1;i++) {
            down[i][j][!curr] = -0.9 * V[i][j];
            V[i][j] = up[i][j-1][!curr];
        }
        // onderkant
        j = Y_SIZE-1;
        for (i = 1;i<X_SIZE-1;i++) {
            up[i][j][!curr] = -0.9 * V[i][j];
            V[i][j] = down[i][j+1][!curr];
        }
        // rechterkant
        i = X_SIZE-1;
        for (j = 1;j<Y_SIZE-1;j++) {
            left[i][j][!curr] = -0.9 * V[i][j];
            V[i][j] = right[i-1][j][!curr];
        }
        // linkerkant
        i = 0;
        for (j = 1;j<Y_SIZE-1;j++) {
            right[i][j][!curr] = -0.9  * V[i][j];
            V[i][j] = left[i+1][j][!curr];
        }
        curr = !curr;
    }
    step--;
}

void waveMesh::initMesh(int amp,int centre_x,int centre_y,int width) {
    int i,j;
    int radius = width>>1;
    if (radius == 0) radius = 1;
    for (i = centre_x-radius;i<centre_x+radius;i++) {
        for (j = centre_y-radius;j<centre_y+radius;j++) {
            if ((i > 0) && (i < X_SIZE) && (j > 0) && (j < Y_SIZE)) {
                    V[i][j] = gauss( (i/(float) radius), (j/(float) radius) );
            }
        }
    }
}

void waveMesh::initMeshWithArray(int array[X_SIZE][Y_SIZE]) {
    copyArray(array,V);
}

void waveMesh::setMeshValue(int x,int y,int value) {
    V[x][y] = value;
    curr = 2;
}

void waveMesh::copyArray(int arrayFrom[X_SIZE][Y_SIZE],int arrayTo[X_SIZE][Y_SIZE][2]) {
    int i,j;
    for (i = 1;i<X_SIZE-1;i++) {
        for (j = 1;j<Y_SIZE-1;j++) {
            arrayTo[i][j][1] = arrayFrom[i][j];
        }
    }
}

void waveMesh::copyArray(int arrayFrom[X_SIZE][Y_SIZE],int arrayTo[X_SIZE][Y_SIZE]) {
    int i,j;
    for (i = 0;i<X_SIZE;i++) {
        for (j = 0;j<Y_SIZE;j++) {
            arrayTo[i][j] = arrayFrom[i][j];
        }
    }
}

void waveMesh::print() {
    int i,j;
    for (i = 0;i<X_SIZE;i++) {
        for (j = 0;j<Y_SIZE;j++) {
            if (V[i][j] == 0) printf("    ");
            else printf("%+.3d ",V[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void waveMesh::printUp() {
    printf("printing up \n");
    int i,j;
    for (i = 0;i<X_SIZE;i++) {
        for (j = 0;j<Y_SIZE;j++) {
            printf("%.4d ",up[i][j][curr]);
        }
        printf("\n");
    }
    printf("\n");
}

void waveMesh::printDown() {
    printf("printing down \n");
    int i,j;
    for (i = 0;i<X_SIZE;i++) {
        for (j = 0;j<Y_SIZE;j++) {
            printf("%.4d ",down[i][j][curr]);
        }
        printf("\n");
    }
    printf("\n");
}

void waveMesh::printRight() {
    printf("printing right \n");
    int i,j;
    for (i = 0;i<X_SIZE;i++) {
        for (j = 0;j<Y_SIZE;j++) {
            printf("%.4d ",right[i][j][curr]);
        }
        printf("\n");
    }
    printf("\n");
}

void waveMesh::printLeft() {
    printf("printing left \n");
    int i,j;
    for (i = 0;i<X_SIZE;i++) {
        for (j = 0;j<Y_SIZE;j++) {
            printf("%.4d ",left[i][j][curr]);
        }
        printf("\n");
    }
    printf("\n");
}

float waveMesh::returnValue(int x,int y) {
    if (x < 0 || x > (X_SIZE-1)) x = 0;
    if (y < 0 || y > (Y_SIZE-1)) y = 0;
    return clipf(V[x][y] / 32768.0,-1.,1.); 
}

/*int main (int argc, const char * argv[])
{
    int initArray[X_SIZE][Y_SIZE] = { 
        { 0,0,0,0,0,0,0,0,0 } , 
        { 0,0,0,0,0,0,0,0,0 } ,
        { 0,0,0,0,0,0,0,0,0 } ,
        { 0,0,0,0,10,0,0,0,0 } ,
        { 0,0,0,10,1000,10,0,0,0,0 } ,
        { 0,0,0,0,10,0,0,0,0 } ,
        { 0,0,0,0,0,0,0,0,0 } ,
        { 0,0,0,0,0,0,0,0,0 } ,
        { 0,0,0,0,0,0,0,0,0 } ,
    };
    
    waveMesh *aMesh;
    aMesh = new waveMesh;
    aMesh->initMeshWithArray(initArray);
    
    int T = 1000;
    
    while(T--) {
        printf("%d \n",T);
        aMesh->print();
        
         aMesh->printUp();
         aMesh->printDown();
         aMesh->printRight();
         aMesh->printLeft();
         
        aMesh->calcNext();
        
    }
    
    // insert code here...
    return 0;
}
*/



CK_DLL_CTOR(twodmesh_ctor);
CK_DLL_DTOR(twodmesh_dtor);

CK_DLL_TICK(twodmesh_tick);
CK_DLL_MFUN(twodmesh_setMeshValue);
CK_DLL_MFUN(twodmesh_excite);

t_CKINT twodmesh_data_offset = 0;


struct twodmeshData
{
    waveMesh * mesh;
};


CK_DLL_QUERY(twodmesh)
{
    QUERY->setname(QUERY, "Twodmesh");
    
    QUERY->begin_class(QUERY, "Twodmesh", "UGen");
    
    QUERY->add_ctor(QUERY, twodmesh_ctor);
    QUERY->add_dtor(QUERY, twodmesh_dtor);
    
    QUERY->add_ugen_func(QUERY, twodmesh_tick, NULL, 1, 1);
    
    QUERY->add_mfun(QUERY, twodmesh_setMeshValue, "float", "setMeshValue"); // 
    QUERY->add_arg(QUERY, "int", "x");
    QUERY->add_arg(QUERY, "int", "y");
    QUERY->add_arg(QUERY, "float", "value");
    
    QUERY->add_mfun(QUERY, twodmesh_excite, "float", "excite"); // 
    QUERY->add_arg(QUERY, "int", "x");
    QUERY->add_arg(QUERY, "int", "y");
    QUERY->add_arg(QUERY, "float", "value");
    QUERY->add_arg(QUERY, "int", "radius");
    
    twodmesh_data_offset = QUERY->add_mvar(QUERY, "int", "@data", false);
    
    QUERY->end_class(QUERY);
    
    return TRUE;
}


CK_DLL_CTOR(twodmesh_ctor)
{
    OBJ_MEMBER_INT(SELF, twodmesh_data_offset) = 0;
    
    twodmeshData * tmdata = new twodmeshData;
    
    printf("hallo");
    
    tmdata->mesh = new waveMesh;
    
    OBJ_MEMBER_INT(SELF, twodmesh_data_offset) = (t_CKINT) tmdata;
}

CK_DLL_DTOR(twodmesh_dtor)
{
    twodmeshData * tmdata = (twodmeshData *) OBJ_MEMBER_INT(SELF, twodmesh_data_offset);
   
    if(tmdata)
    {
        if (tmdata->mesh) 
        {
            delete tmdata->mesh;
            tmdata->mesh = NULL;
        }
        delete tmdata;
        
        OBJ_MEMBER_INT(SELF, twodmesh_data_offset) = 0;
        tmdata = NULL;
    }
}

CK_DLL_TICK(twodmesh_tick)
{
    twodmeshData * tmdata = (twodmeshData *) OBJ_MEMBER_INT(SELF,twodmesh_data_offset); 
    tmdata->mesh->calcNext();
    *out = tmdata->mesh->returnValue(5,5);
    //*out = 0.1;
}

CK_DLL_MFUN(twodmesh_setMeshValue)
{
    twodmeshData * tmdata = (twodmeshData *) OBJ_MEMBER_INT(SELF, twodmesh_data_offset);
    int x = GET_NEXT_INT(ARGS);
    int y = GET_NEXT_INT(ARGS);
    int value = floor(clipf(GET_NEXT_FLOAT(ARGS),-1,1) * 32767);
    tmdata->mesh->setMeshValue(x,y,value);
    RETURN->v_float = (t_CKFLOAT) value;
}

CK_DLL_MFUN(twodmesh_excite)
{
    twodmeshData * tmdata = (twodmeshData *) OBJ_MEMBER_INT(SELF, twodmesh_data_offset);
    int x = GET_NEXT_INT(ARGS);
    int y = GET_NEXT_INT(ARGS);
    int value = floor(clipf(GET_NEXT_FLOAT(ARGS),-1,1) * 32767);
    tmdata->mesh->initMesh(value,x,y,GET_NEXT_INT(ARGS));
    RETURN->v_float = (t_CKFLOAT) value;
}




