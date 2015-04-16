#pragma once
#pragma warning( disable : 4996 )

#include <stdio.h>
#include <iostream>
#include <time.h>
#include <assert.h>

#include "array3d.h"
#include "glm/glm.hpp" 

class MACVelocityField
{
public:
    MACVelocityField();
    MACVelocityField(int x_voxels, int y_voxels, int z_voxels, double cell_size);
    ~MACVelocityField();

    double U(int i, int j, int k);
    double V(int i, int j, int k);
    double W(int i, int j, int k);
    double tempU(int i, int j, int k);
    double tempV(int i, int j, int k);
    double tempW(int i, int j, int k);

    void setU(int i, int j, int k, double val);
    void setV(int i, int j, int k, double val);
    void setW(int i, int j, int k, double val);
    void setTempU(int i, int j, int k, double val);
    void setTempV(int i, int j, int k, double val);
    void setTempW(int i, int j, int k, double val);
    void resetTemporaryVelocityField();
    void commitTemporaryVelocityFieldValues();

    void clear();
    void clearU();
    void clearV();
    void clearW();

    void randomizeValues();
    void randomizeValues(double min, double max);

    inline bool isIndexInRangeU(int i, int j, int k) {
        return i >= 0 && j >= 0 && k >= 0 && i < i_voxels + 1 && j < j_voxels && k < k_voxels;
    }
    inline bool isIndexInRangeV(int i, int j, int k) {
        return i >= 0 && j >= 0 && k >= 0 && i < i_voxels && j < j_voxels + 1 && k < k_voxels;
    }
    inline bool isIndexInRangeW(int i, int j, int k) {
        return i >= 0 && j >= 0 && k >= 0 && i < i_voxels && j < j_voxels && k < k_voxels + 1;
    }

    glm::vec3 evaluateVelocityAtCellCenter(int i, int j, int k);
    double evaluateVelocityMagnitudeAtCellCenter(int i, int j, int k);
    double evaluateVelocityMagnitudeSquaredAtCellCenter(int i, int j, int k);
    double evaluateMaximumVelocityMagnitude();

    glm::vec3 evaluateVelocityAtFaceCenterU(int i, int j, int k);
    glm::vec3 evaluateVelocityAtFaceCenterV(int i, int j, int k);
    glm::vec3 evaluateVelocityAtFaceCenterW(int i, int j, int k);

    glm::vec3 evaluateVelocityAtPosition(double x, double y, double z);
    glm::vec3 evaluateVelocityAtPosition(glm::vec3 pos);

    glm::vec3 velocityIndexToPositionU(int i, int j, int k);
    glm::vec3 velocityIndexToPositionV(int i, int j, int k);
    glm::vec3 velocityIndexToPositionW(int i, int j, int k);

private:
    void _initializeVelocityGrids();

    inline bool _isCellIndexInRange(int i, int j, int k) {
        return i >= 0 && j >= 0 && k >= 0 && i < i_voxels && j < j_voxels && k < k_voxels;
    }
    inline bool _isPositionInGrid(double x, double y, double z) {
        return x >= 0 && y >= 0 && z >= 0 && x <= dx*i_voxels && y <= dx*j_voxels && z <= dx*k_voxels;
    }

    double _default_out_of_range_value = 1.0;

    inline double _randomFloat(double min, double max) {
         return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min))); 
    }

    double _cubicInterpolate(double p[4], double x);
    double _bicubicInterpolate(double p[4][4], double x, double y);
    double _tricubicInterpolate(double p[4][4][4], double x, double y, double z);

    double _interpolateU(double x, double y, double z);
    double _interpolateV(double x, double y, double z);
    double _interpolateW(double x, double y, double z);

    void _positionToGridIndex(double x, double y, double z, int *i, int *j, int *k);
    void _gridIndexToPosition(int i, int j, int k, double *x, double *y, double *z);

    double dx = 0.1;
    int i_voxels = 10;
    int j_voxels = 10;
    int k_voxels = 10;

    Array3d<double> _u;
    Array3d<double> _v;
    Array3d<double> _w;
    Array3d<double> _temp_u;
    Array3d<double> _temp_v;
    Array3d<double> _temp_w;
    Array3d<bool> _is_set_u;
    Array3d<bool> _is_set_v;
    Array3d<bool> _is_set_w;
};

