#include "macvelocityfield.h"


MACVelocityField::MACVelocityField()
{
    _initializeVelocityGrids();
}

MACVelocityField::MACVelocityField(int x_voxels, int y_voxels, int z_voxels, double cell_size) :
                                   _i_voxels(x_voxels), _j_voxels(y_voxels), _k_voxels(z_voxels),
                                   _dx(cell_size) {

    _initializeVelocityGrids();
}


MACVelocityField::~MACVelocityField()
{

}

void MACVelocityField::_initializeVelocityGrids() {

    _u = Array3d<double>(_i_voxels + 1, _j_voxels, _k_voxels, 0.0);
    _v = Array3d<double>(_i_voxels, _j_voxels + 1, _k_voxels, 0.0);
    _w = Array3d<double>(_i_voxels, _j_voxels, _k_voxels + 1, 0.0);

    _temp_u = Array3d<double>(_i_voxels + 1, _j_voxels, _k_voxels, 0.0);
    _temp_v = Array3d<double>(_i_voxels, _j_voxels + 1, _k_voxels, 0.0);
    _temp_w = Array3d<double>(_i_voxels, _j_voxels, _k_voxels + 1, 0.0);

    _is_set_u = Array3d<bool>(_i_voxels + 1, _j_voxels, _k_voxels, false);
    _is_set_v = Array3d<bool>(_i_voxels, _j_voxels + 1, _k_voxels, false);
    _is_set_w = Array3d<bool>(_i_voxels, _j_voxels, _k_voxels + 1, false);
}

void MACVelocityField::clearU() {
    _u.fill(0.0);
}

void MACVelocityField::clearV() {
    _v.fill(0.0);
}

void MACVelocityField::clearW() {
    _w.fill(0.0);
}

void MACVelocityField::clear() {
    clearU();
    clearV();
    clearW();
}

void MACVelocityField::resetTemporaryVelocityField() {
    _temp_u.fill(0.0);
    _temp_v.fill(0.0);
    _temp_w.fill(0.0);
    _is_set_u.fill(false);
    _is_set_v.fill(false);
    _is_set_w.fill(false);
}

void MACVelocityField::commitTemporaryVelocityFieldValues() {
    for (int k = 0; k < _is_set_u.depth; k++) {
        for (int j = 0; j < _is_set_u.height; j++) {
            for (int i = 0; i < _is_set_u.width; i++) {
                if (_is_set_u(i, j, k)) {
                    setU(i, j, k, _temp_u(i, j, k));
                }
            }
        }
    }

    for (int k = 0; k < _is_set_v.depth; k++) {
        for (int j = 0; j < _is_set_v.height; j++) {
            for (int i = 0; i < _is_set_v.width; i++) {
                if (_is_set_v(i, j, k)) {
                    setV(i, j, k, _temp_v(i, j, k));
                }
            }
        }
    }

    for (int k = 0; k < _is_set_w.depth; k++) {
        for (int j = 0; j < _is_set_w.height; j++) {
            for (int i = 0; i < _is_set_w.width; i++) {
                if (_is_set_w(i, j, k)) {
                    setW(i, j, k, _temp_w(i, j, k));
                }
            }
        }
    }
}

void MACVelocityField::randomizeValues() {
    randomizeValues(-1.0, 1.0);
}

void MACVelocityField::randomizeValues(double min, double max) {
    for (int k = 0; k < _k_voxels; k++) {
        for (int j = 0; j < _j_voxels; j++) {
            for (int i = 0; i < _i_voxels + 1; i++) {
                _u.set(i, j, k, _randomFloat(min, max));
            }
        }
    }

    for (int k = 0; k < _k_voxels; k++) {
        for (int j = 0; j < _j_voxels + 1; j++) {
            for (int i = 0; i < _i_voxels; i++) {
                _v.set(i, j, k, _randomFloat(min, max));
            }
        }
    }

    for (int k = 0; k < _k_voxels + 1; k++) {
        for (int j = 0; j < _j_voxels; j++) {
            for (int i = 0; i < _i_voxels; i++) {
                _w.set(i, j, k, _randomFloat(min, max));
            }
        }
    }
}

double* MACVelocityField::getRawArrayU() {
    return _u.getRawArray();
}

double* MACVelocityField::getRawArrayV() {
    return _v.getRawArray();
}

double* MACVelocityField::getRawArrayW() {
    return _w.getRawArray();
}

double MACVelocityField::U(int i, int j, int k) {
    if (!isIndexInRangeU(i, j, k)) {
        return _default_out_of_range_value;
    }

    return _u(i, j, k);
}

double MACVelocityField::V(int i, int j, int k) {
    if (!isIndexInRangeV(i, j, k)) {
        return _default_out_of_range_value;
    }

    return _v(i, j, k);
}

double MACVelocityField::W(int i, int j, int k) {
    if (!isIndexInRangeW(i, j, k)) {
        return _default_out_of_range_value;
    }

    return _w(i, j, k);
}

double MACVelocityField::U(GridIndex g) {
    if (!isIndexInRangeU(g)) {
        return _default_out_of_range_value;
    }

    return _u(g);
}

double MACVelocityField::V(GridIndex g) {
    if (!isIndexInRangeV(g)) {
        return _default_out_of_range_value;
    }

    return _v(g);
}

double MACVelocityField::W(GridIndex g) {
    if (!isIndexInRangeW(g)) {
        return _default_out_of_range_value;
    }

    return _w(g);
}

double MACVelocityField::tempU(int i, int j, int k) {
    if (!isIndexInRangeU(i, j, k)) {
        return _default_out_of_range_value;
    }

    return _temp_u(i, j, k);
}

double MACVelocityField::tempV(int i, int j, int k) {
    if (!isIndexInRangeV(i, j, k)) {
        return _default_out_of_range_value;
    }

    return _temp_v(i, j, k);
}

double MACVelocityField::tempW(int i, int j, int k) {
    if (!isIndexInRangeW(i, j, k)) {
        return _default_out_of_range_value;
    }

    return _temp_w(i, j, k);
}

void MACVelocityField::setU(int i, int j, int k, double val) {
    if (!isIndexInRangeU(i, j, k)) {
        return;
    }

    _u.set(i, j, k, val);
}

void MACVelocityField::setV(int i, int j, int k, double val) {
    if (!isIndexInRangeV(i, j, k)) {
        return;
    }

    _v.set(i, j, k, val);
}

void MACVelocityField::setW(int i, int j, int k, double val) {
    if (!isIndexInRangeW(i, j, k)) {
        return;
    }

    _w.set(i, j, k, val);
}

void MACVelocityField::setU(Array3d<double> &ugrid) {
    assert(ugrid.width == _u.width && 
           ugrid.height == _u.height && 
           ugrid.depth == _u.depth);
    _u = ugrid;
}

void MACVelocityField::setV(Array3d<double> &vgrid) {
    assert(vgrid.width == _v.width && 
           vgrid.height == _v.height && 
           vgrid.depth == _v.depth);
    _v = vgrid;
}

void MACVelocityField::setW(Array3d<double> &wgrid) {
    assert(wgrid.width == _w.width && 
           wgrid.height == _w.height && 
           wgrid.depth == _w.depth);
    _w = wgrid;
}

void MACVelocityField::addU(int i, int j, int k, double val) {
    if (!isIndexInRangeU(i, j, k)) {
        return;
    }

    _u.add(i, j, k, val);
}

void MACVelocityField::addV(int i, int j, int k, double val) {
    if (!isIndexInRangeV(i, j, k)) {
        return;
    }

    _v.add(i, j, k, val);
}

void MACVelocityField::addW(int i, int j, int k, double val) {
    if (!isIndexInRangeW(i, j, k)) {
        return;
    }

    _w.add(i, j, k, val);
}

void MACVelocityField::setTempU(int i, int j, int k, double val) {
    if (!isIndexInRangeU(i, j, k)) {
        return;
    }

    _temp_u.set(i, j, k, val);
    _is_set_u.set(i, j, k, true);
}

void MACVelocityField::setTempV(int i, int j, int k, double val) {
    if (!isIndexInRangeV(i, j, k)) {
        return;
    }

    _temp_v.set(i, j, k, val);
    _is_set_v.set(i, j, k, true);
}

void MACVelocityField::setTempW(int i, int j, int k, double val) {
    if (!isIndexInRangeW(i, j, k)) {
        return;
    }

    _temp_w.set(i, j, k, val);
    _is_set_w.set(i, j, k, true);
}

void MACVelocityField::addTempU(int i, int j, int k, double val) {
    if (!isIndexInRangeU(i, j, k)) {
        return;
    }

    _temp_u.add(i, j, k, val);
    _is_set_u.set(i, j, k, true);
}

void MACVelocityField::addTempV(int i, int j, int k, double val) {
    if (!isIndexInRangeV(i, j, k)) {
        return;
    }

    _temp_v.add(i, j, k, val);
    _is_set_v.set(i, j, k, true);
}

void MACVelocityField::addTempW(int i, int j, int k, double val) {
    if (!isIndexInRangeW(i, j, k)) {
        return;
    }

    _temp_w.add(i, j, k, val);
    _is_set_w.set(i, j, k, true);
}

glm::vec3 MACVelocityField::velocityIndexToPositionU(int i, int j, int k) {
    assert(isIndexInRangeU(i, j, k));

    double gx = (double)(i-1)*_dx;
    double gy = (double)j*_dx;
    double gz = (double)k*_dx;

    return glm::vec3(gx + _dx, gy + 0.5*_dx, gz + 0.5*_dx);
}

glm::vec3 MACVelocityField::velocityIndexToPositionV(int i, int j, int k) {
    assert(isIndexInRangeV(i, j, k));

    double gx = (double)i*_dx;
    double gy = (double)(j-1)*_dx;
    double gz = (double)k*_dx;

    return glm::vec3(gx + 0.5*_dx, gy + _dx, gz + 0.5*_dx);
}

glm::vec3 MACVelocityField::velocityIndexToPositionW(int i, int j, int k) {
    assert(isIndexInRangeW(i, j, k));

    double gx = (double)i*_dx;
    double gy = (double)j*_dx;
    double gz = (double)(k-1)*_dx;

    return glm::vec3(gx + 0.5*_dx, gy + 0.5*_dx, gz + _dx);
}

glm::vec3 MACVelocityField::evaluateVelocityAtCellCenter(int i, int j, int k) {
    assert(_isCellIndexInRange(i, j, k));

    double xavg = 0.5 * (U(i + 1, j, k) + U(i, j, k));
    double yavg = 0.5 * (V(i, j + 1, k) + V(i, j, k));
    double zavg = 0.5 * (W(i, j, k + 1) + W(i, j, k));

    return glm::vec3(xavg, yavg, zavg);
}

double MACVelocityField::evaluateVelocityMagnitudeSquaredAtCellCenter(int i, int j, int k) {
    assert(_isCellIndexInRange(i, j, k));

    double xavg = 0.5 * (U(i + 1, j, k) + U(i, j, k));
    double yavg = 0.5 * (V(i, j + 1, k) + V(i, j, k));
    double zavg = 0.5 * (W(i, j, k + 1) + W(i, j, k));

    return xavg*xavg + yavg*yavg + zavg*zavg;
}

double MACVelocityField::evaluateVelocityMagnitudeAtCellCenter(int i, int j, int k) {
    assert(_isCellIndexInRange(i, j, k));

    double mag = evaluateVelocityMagnitudeSquaredAtCellCenter(i, j, k);
    if (mag > 0.0) {
        return sqrt(mag);
    }
    else {
        return 0.0;
    }
}

double MACVelocityField::evaluateMaximumVelocityMagnitude() {
    double maxsq = 0.0;
    for (int k = 0; k < _k_voxels; k++) {
        for (int j = 0; j < _j_voxels; j++) {
            for (int i = 0; i < _i_voxels; i++) {
                
                double m = evaluateVelocityMagnitudeSquaredAtCellCenter(i, j, k);
                maxsq = fmax(maxsq, m);
            }
        }
    }

    double max = maxsq;
    if (maxsq > 0.0) {
        max = sqrt(maxsq);
    }

    return max;
}

glm::vec3 MACVelocityField::evaluateVelocityAtFaceCenterU(int i, int j, int k) {
    assert(isIndexInRangeU(i, j, k));

    // Shift reference coordinate to the left. The formula used is for calculating
    // u(i+1/2, j, k). If we keep original (i,j,k) coordinate, then using the formula
    // would calculate u(i+3/2, j, k) instead. The same will be done for the V and W
    // faces, shifting back in the respective direction.
    i--;

    double vx = U(i+1, j, k);
    double vy = 0.25 * (V(i, j, k) + V(i, j+1, k) + V(i+1, j, k) + V(i+1, j+1, k));
    double vz = 0.25 * (W(i, j, k) + W(i, j, k+1) + W(i+1, j, k) + W(i+1, j, k+1));

    return glm::vec3(vx, vy, vz);
}

glm::vec3 MACVelocityField::evaluateVelocityAtFaceCenterV(int i, int j, int k) {
    assert(isIndexInRangeV(i, j, k));

    j--;

    double vx = 0.25 * (U(i, j, k) + U(i+1, j, k) + U(i, j+1, k) + U(i+1, j+1, k));
    double vy = V(i, j + 1, k);
    double vz = 0.25 * (W(i, j, k) + W(i, j, k+1) + W(i, j+1, k) + W(i, j+1, k+1));

    return glm::vec3(vx, vy, vz);
}

glm::vec3 MACVelocityField::evaluateVelocityAtFaceCenterW(int i, int j, int k) {
    assert(isIndexInRangeW(i, j, k));

    k--;

    double vx = 0.25 * (U(i, j, k) + U(i+1, j, k) + U(i, j, k+1) + U(i+1, j, k+1));
    double vy = 0.25 * (V(i, j, k) + V(i, j+1, k) + V(i, j, k+1) + V(i, j+1, k+1));
    double vz = W(i, j, k + 1);

    return glm::vec3(vx, vy, vz);
}

/* 
    Intopolation methods from http://www.paulinternet.nl/?page=bicubic

    - p is indexed in order by p[i][j][k]
    - x, y, z are in [0,1]
    - this function will interpolate the volume between point Index 1 and 2
*/
double MACVelocityField::_fasttricubicInterpolate(double p[4][4][4], double x, double y, double z) {
    double c00 = p[0][0][1] + 0.5 * x*(p[0][0][2] - p[0][0][0] + x*(2.0*p[0][0][0] - 5.0*p[0][0][1] + 4.0*p[0][0][2] - p[0][0][3] + x*(3.0*(p[0][0][1] - p[0][0][2]) + p[0][0][3] - p[0][0][0])));
    double c01 = p[0][1][1] + 0.5 * x*(p[0][1][2] - p[0][1][0] + x*(2.0*p[0][1][0] - 5.0*p[0][1][1] + 4.0*p[0][1][2] - p[0][1][3] + x*(3.0*(p[0][1][1] - p[0][1][2]) + p[0][1][3] - p[0][1][0])));
    double c02 = p[0][2][1] + 0.5 * x*(p[0][2][2] - p[0][2][0] + x*(2.0*p[0][2][0] - 5.0*p[0][2][1] + 4.0*p[0][2][2] - p[0][2][3] + x*(3.0*(p[0][2][1] - p[0][2][2]) + p[0][2][3] - p[0][2][0])));
    double c03 = p[0][3][1] + 0.5 * x*(p[0][3][2] - p[0][3][0] + x*(2.0*p[0][3][0] - 5.0*p[0][3][1] + 4.0*p[0][3][2] - p[0][3][3] + x*(3.0*(p[0][3][1] - p[0][3][2]) + p[0][3][3] - p[0][3][0])));

    double c10 = p[1][0][1] + 0.5 * x*(p[1][0][2] - p[1][0][0] + x*(2.0*p[1][0][0] - 5.0*p[1][0][1] + 4.0*p[1][0][2] - p[1][0][3] + x*(3.0*(p[1][0][1] - p[1][0][2]) + p[1][0][3] - p[1][0][0])));
    double c11 = p[1][1][1] + 0.5 * x*(p[1][1][2] - p[1][1][0] + x*(2.0*p[1][1][0] - 5.0*p[1][1][1] + 4.0*p[1][1][2] - p[1][1][3] + x*(3.0*(p[1][1][1] - p[1][1][2]) + p[1][1][3] - p[1][1][0])));
    double c12 = p[1][2][1] + 0.5 * x*(p[1][2][2] - p[1][2][0] + x*(2.0*p[1][2][0] - 5.0*p[1][2][1] + 4.0*p[1][2][2] - p[1][2][3] + x*(3.0*(p[1][2][1] - p[1][2][2]) + p[1][2][3] - p[1][2][0])));
    double c13 = p[1][3][1] + 0.5 * x*(p[1][3][2] - p[1][3][0] + x*(2.0*p[1][3][0] - 5.0*p[1][3][1] + 4.0*p[1][3][2] - p[1][3][3] + x*(3.0*(p[1][3][1] - p[1][3][2]) + p[1][3][3] - p[1][3][0])));

    double c20 = p[2][0][1] + 0.5 * x*(p[2][0][2] - p[2][0][0] + x*(2.0*p[2][0][0] - 5.0*p[2][0][1] + 4.0*p[2][0][2] - p[2][0][3] + x*(3.0*(p[2][0][1] - p[2][0][2]) + p[2][0][3] - p[2][0][0])));
    double c21 = p[2][1][1] + 0.5 * x*(p[2][1][2] - p[2][1][0] + x*(2.0*p[2][1][0] - 5.0*p[2][1][1] + 4.0*p[2][1][2] - p[2][1][3] + x*(3.0*(p[2][1][1] - p[2][1][2]) + p[2][1][3] - p[2][1][0])));
    double c22 = p[2][2][1] + 0.5 * x*(p[2][2][2] - p[2][2][0] + x*(2.0*p[2][2][0] - 5.0*p[2][2][1] + 4.0*p[2][2][2] - p[2][2][3] + x*(3.0*(p[2][2][1] - p[2][2][2]) + p[2][2][3] - p[2][2][0])));
    double c23 = p[2][3][1] + 0.5 * x*(p[2][3][2] - p[2][3][0] + x*(2.0*p[2][3][0] - 5.0*p[2][3][1] + 4.0*p[2][3][2] - p[2][3][3] + x*(3.0*(p[2][3][1] - p[2][3][2]) + p[2][3][3] - p[2][3][0])));

    double c30 = p[3][0][1] + 0.5 * x*(p[3][0][2] - p[3][0][0] + x*(2.0*p[3][0][0] - 5.0*p[3][0][1] + 4.0*p[3][0][2] - p[3][0][3] + x*(3.0*(p[3][0][1] - p[3][0][2]) + p[3][0][3] - p[3][0][0])));
    double c31 = p[3][1][1] + 0.5 * x*(p[3][1][2] - p[3][1][0] + x*(2.0*p[3][1][0] - 5.0*p[3][1][1] + 4.0*p[3][1][2] - p[3][1][3] + x*(3.0*(p[3][1][1] - p[3][1][2]) + p[3][1][3] - p[3][1][0])));
    double c32 = p[3][2][1] + 0.5 * x*(p[3][2][2] - p[3][2][0] + x*(2.0*p[3][2][0] - 5.0*p[3][2][1] + 4.0*p[3][2][2] - p[3][2][3] + x*(3.0*(p[3][2][1] - p[3][2][2]) + p[3][2][3] - p[3][2][0])));
    double c33 = p[3][3][1] + 0.5 * x*(p[3][3][2] - p[3][3][0] + x*(2.0*p[3][3][0] - 5.0*p[3][3][1] + 4.0*p[3][3][2] - p[3][3][3] + x*(3.0*(p[3][3][1] - p[3][3][2]) + p[3][3][3] - p[3][3][0])));

    double b0 = c01 + 0.5 * x*(c02 - c00 + x*(2.0*c00 - 5.0*c01 + 4.0*c02 - c03 + x*(3.0*(c01 - c02) + c03 - c00)));
    double b1 = c11 + 0.5 * x*(c12 - c10 + x*(2.0*c10 - 5.0*c11 + 4.0*c12 - c13 + x*(3.0*(c11 - c12) + c13 - c10)));
    double b2 = c21 + 0.5 * x*(c22 - c20 + x*(2.0*c20 - 5.0*c21 + 4.0*c22 - c23 + x*(3.0*(c21 - c22) + c23 - c20)));
    double b3 = c31 + 0.5 * x*(c32 - c30 + x*(2.0*c30 - 5.0*c31 + 4.0*c32 - c33 + x*(3.0*(c31 - c32) + c33 - c30)));

    return b1 + 0.5 * x*(b2 - b0 + x*(2.0*b0 - 5.0*b1 + 4.0*b2 - b3 + x*(3.0*(b1 - b2) + b3 - b0)));
}

double MACVelocityField::_tricubicInterpolate(double p[4][4][4], double x, double y, double z) {
    assert(x >= 0 && x <= 1 && y >= 0 && y <= 1 && z >= 0 && z <= 1);

    double arr[4];
    arr[0] = _bicubicInterpolate(p[0], y, z);
    arr[1] = _bicubicInterpolate(p[1], y, z);
    arr[2] = _bicubicInterpolate(p[2], y, z);
    arr[3] = _bicubicInterpolate(p[3], y, z);
    return _cubicInterpolate(arr, x);
}

double MACVelocityField::_bicubicInterpolate(double p[4][4], double x, double y) {
    double arr[4];
    arr[0] = _cubicInterpolate(p[0], y);
    arr[1] = _cubicInterpolate(p[1], y);
    arr[2] = _cubicInterpolate(p[2], y);
    arr[3] = _cubicInterpolate(p[3], y);
    return _cubicInterpolate(arr, x);
}

double MACVelocityField::_cubicInterpolate(double p[4], double x) {
    return p[1] + 0.5 * x*(p[2] - p[0] + x*(2.0*p[0] - 5.0*p[1] + 4.0*p[2] - p[3] + x*(3.0*(p[1] - p[2]) + p[3] - p[0])));
}

double MACVelocityField::_interpolateU(double x, double y, double z) {
    if (!_isPositionInGrid(x, y, z)) {
        return 0.0;
    }

    // Velocity positions are staggered from grid cell positions. We need to find the minimal
    // U coordinate (of the 8 that form a cube) that encloses point (x, y, z) so that we have
    // a reference to construct a 4x4x4 array of U values for a tricubic interpolation
    int i, j, k;
    double gx, gy, gz;
    _positionToGridIndex(x, y, z, &i, &j, &k);
    _gridIndexToPosition(i, j, k, &gx, &gy, &gz);

    double refx = gx;
    double refy = gy - 0.5*_dx;
    double refz = gz - 0.5*_dx;
    int refi = i;
    int refj = j - 1;
    int refk = k - 1;

    double cy = gy + 0.5*_dx;
    double cz = gz + 0.5*_dx;

    if (y >= cy) {
        refj++;
        refy += _dx;
    }

    if (z >= cz) {
        refk++;
        refz += _dx;
    }

    double inv_dx = 1 / _dx;
    double ix = (x - refx)*inv_dx;
    double iy = (y - refy)*inv_dx;
    double iz = (z - refz)*inv_dx;

    assert(ix >= 0 && ix < 1 && iy >= 0 && iy < 1 && iz >= 0 && iz < 1);

    refi--; refj--; refk--;
    double points[4][4][4];
    for (int pk = 0; pk < 4; pk++) {
        for (int pj = 0; pj < 4; pj++) {
            for (int pi = 0; pi < 4; pi++) {
                if (_u.isIndexInRange(pi, pj, pk)) {
                    points[pi][pj][pk] = U(pi + refi, pj + refj, pk + refk);
                } else {
                    points[pi][pj][pk] = 0;
                }
            }
        }
    }

    return _fasttricubicInterpolate(points, ix, iy, iz);
}

double MACVelocityField::_interpolateV(double x, double y, double z) {
    if (!_isPositionInGrid(x, y, z)) {
        return 0.0;
    }

    int i, j, k;
    double gx, gy, gz;
    _positionToGridIndex(x, y, z, &i, &j, &k);
    _gridIndexToPosition(i, j, k, &gx, &gy, &gz);

    double refx = gx - 0.5*_dx;
    double refy = gy;
    double refz = gz - 0.5*_dx;
    int refi = i - 1;
    int refj = j;
    int refk = k - 1;

    double cx = gx + 0.5*_dx;
    double cz = gz + 0.5*_dx;

    if (x >= cx) {
        refi++;
        refx += _dx;
    }

    if (z >= cz) {
        refk++;
        refz += _dx;
    }

    double inv_dx = 1 / _dx;
    double ix = (x - refx)*inv_dx;
    double iy = (y - refy)*inv_dx;
    double iz = (z - refz)*inv_dx;

    assert(ix >= 0 && ix < 1 && iy >= 0 && iy < 1 && iz >= 0 && iz < 1);

    refi--; refj--; refk--;
    double points[4][4][4];
    for (int pk = 0; pk < 4; pk++) {
        for (int pj = 0; pj < 4; pj++) {
            for (int pi = 0; pi < 4; pi++) {
                if (_v.isIndexInRange(pi, pj, pk)) {
                    points[pi][pj][pk] = V(pi + refi, pj + refj, pk + refk);
                } else {
                    points[pi][pj][pk] = 0;
                }
            }
        }
    }

    return _fasttricubicInterpolate(points, ix, iy, iz);
}

double MACVelocityField::_interpolateW(double x, double y, double z) {
    if (!_isPositionInGrid(x, y, z)) {
        return 0.0;
    }

    int i, j, k;
    double gx, gy, gz;
    _positionToGridIndex(x, y, z, &i, &j, &k);
    _gridIndexToPosition(i, j, k, &gx, &gy, &gz);

    double refx = gx - 0.5*_dx;
    double refy = gy - 0.5*_dx;
    double refz = gz;
    int refi = i - 1;
    int refj = j - 1;
    int refk = k;

    double cx = gx + 0.5*_dx;
    double cy = gy + 0.5*_dx;

    if (x >= cx) {
        refi++;
        refx += _dx;
    }

    if (y >= cy) {
        refj++;
        refy += _dx;
    }

    double inv_dx = 1 / _dx;
    double ix = (x - refx)*inv_dx;
    double iy = (y - refy)*inv_dx;
    double iz = (z - refz)*inv_dx;

    assert(ix >= 0 && ix < 1 && iy >= 0 && iy < 1 && iz >= 0 && iz < 1);

    refi--; refj--; refk--;
    double points[4][4][4];
    for (int pk = 0; pk < 4; pk++) {
        for (int pj = 0; pj < 4; pj++) {
            for (int pi = 0; pi < 4; pi++) {
                if (_w.isIndexInRange(pi, pj, pk)) {
                    points[pi][pj][pk] = W(pi + refi, pj + refj, pk + refk);
                } else {
                    points[pi][pj][pk] = 0;
                }
            }
        }
    }

    return _fasttricubicInterpolate(points, ix, iy, iz);
}

void MACVelocityField::_positionToGridIndex(double x, double y, double z, int *i, int *j, int *k) {
    assert(_isPositionInGrid(x, y, z));

    double inv_dx = 1.0 / _dx;
    *i = fminf((int)floor(x*inv_dx), _i_voxels);
    *j = fminf((int)floor(y*inv_dx), _j_voxels);
    *k = fminf((int)floor(z*inv_dx), _k_voxels);
}

void MACVelocityField::_gridIndexToPosition(int i, int j, int k, double *x, double *y, double *z) {
    assert(_isCellIndexInRange(i, j, k));

    *x = (double)i*_dx;
    *y = (double)j*_dx;
    *z = (double)k*_dx;
}

glm::vec3 MACVelocityField::evaluateVelocityAtPosition(glm::vec3 pos) {
    return evaluateVelocityAtPosition(pos.x, pos.y, pos.z);
}

glm::vec3 MACVelocityField::evaluateVelocityAtPosition(double x, double y, double z) {
    if (!_isPositionInGrid(x, y, z)) {
        return glm::vec3(0.0, 0.0, 0.0);
    }

    double xvel = _interpolateU(x, y, z);
    double yvel = _interpolateV(x, y, z);
    double zvel = _interpolateW(x, y, z);

    return glm::vec3(xvel, yvel, zvel);
}