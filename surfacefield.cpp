/*
Copyright (c) 2015 Ryan L. Guy

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgement in the product documentation would be
   appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/
#include "surfacefield.h"


SurfaceField::SurfaceField()
{
}

SurfaceField::SurfaceField(int w, int h, int d, double cell_size) :
                             i_width(w), j_height(h), k_depth(d), dx(cell_size),
                             width((double)w*cell_size), 
                             height((double)h*cell_size), 
                             depth((double)d*cell_size)
{
}

SurfaceField::~SurfaceField()
{
}

void SurfaceField::setMaterialGrid(Array3d<int> matGrid) {
    materialGrid = matGrid;
    isMaterialGridSet = true;
}

void SurfaceField::setMaterialGrid() {
    materialGrid = Array3d<int>();
    isMaterialGridSet = false;
}

void SurfaceField::clear() {
}

double SurfaceField::getFieldValue(glm::vec3 p) {
    return 0.0;
}

bool SurfaceField::_isPointNearSolid(glm::vec3 p) {
    double eps = 10e-9;

    glm::vec3 x = glm::vec3(eps, 0.0, 0.0);
    glm::vec3 y = glm::vec3(0.0, eps, 0.0);
    glm::vec3 z = glm::vec3(0.0, 0.0, eps);

    int i, j, k;
    Grid3d::positionToGridIndex(p, dx, &i, &j, &k);
    if (materialGrid(i, j, k) == 2) {
        return true;
    }

    glm::vec3 points[26];
    points[0] = p - x;
    points[1] = p + x;
    points[2] = p - y;
    points[3] = p + y;
    points[4] = p - z;
    points[5] = p + z;
    points[6] = p - x - y;
    points[7] = p - x + y;
    points[8] = p + x - y;
    points[9] = p + x + y;
    points[10] = p - x - z;
    points[11] = p - x + z;
    points[12] = p + x - z;
    points[13] = p + x + z;
    points[14] = p - y - z;
    points[15] = p - y + z;
    points[16] = p + y - z;
    points[17] = p + y + z;
    points[18] = p - x - y - z;
    points[19] = p - x - y + z;
    points[20] = p - x + y - z;
    points[21] = p - x + y + z;
    points[22] = p + x - y - z;
    points[23] = p + x - y + z;
    points[24] = p + x + y - z;
    points[25] = p + x + y + z;
    
    for (int idx = 0; idx < 26; idx++) {
        Grid3d::positionToGridIndex(points[idx], dx, &i, &j, &k);
        if (materialGrid(i, j, k) == 2) {
            return true;
        }
    }

    return false;
}

bool SurfaceField::isInside(glm::vec3 p) {
    return getFieldValue(p) > surfaceThreshold;
}

bool SurfaceField::isOutside(glm::vec3 p) {
    return getFieldValue(p) <= surfaceThreshold;
}
