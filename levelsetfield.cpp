#include "levelsetfield.h"


LevelSetField::LevelSetField(int width, int height, int depth, double dx) :
                             SurfaceField(width, height, depth, dx),
                             _distanceField(Array3d<double>(width, height, depth, 1.0))
{
    surfaceThreshold = 0.0;
}

LevelSetField::LevelSetField()
{
}

LevelSetField::~LevelSetField()
{
}

void LevelSetField::clear() {
    _distanceField.fill(1.0);
}

double LevelSetField::getFieldValue(glm::vec3 p) {
    GridIndex g = _positionToGridIndex(p);
    glm::vec3 gp = _gridIndexToPosition(g);

    g = GridIndex(g.i - 1, g.j - 1, g.k - 1);
    
    double invdx = 1 / dx;
    double ix = (p.x - gp.x)*invdx;
    double iy = (p.y - gp.y)*invdx;
    double iz = (p.z - gp.z)*invdx;

    assert(ix >= 0 && ix <= 1 && iy >= 0 && iy <= 1 && iz >= 0 && iz <= 1);

    double points[4][4][4];
    GridIndex c;
    for (int pk = 0; pk < 4; pk++) {
        for (int pj = 0; pj < 4; pj++) {
            for (int pi = 0; pi < 4; pi++) {
                c = GridIndex(pi + g.i, pj + g.j, pk + g.k);
                if (_isCellIndexInRange(c)) {
                    points[pi][pj][pk] = _distanceField(c);
                } else {
                    points[pi][pj][pk] = 0.0;
                }
            }
        }
    }

    double val = _fasttricubicInterpolate(points, ix, iy, iz);

    double eps = 10e-6;
    if (isMaterialGridSet && val > surfaceThreshold && _isPointNearSolid(p)) {
        val = surfaceThreshold - eps;
    }

    return val;
}

void LevelSetField::setSignedDistanceField(Array3d<double> distField) {
    _distanceField = distField;
}

GridIndex LevelSetField::_positionToGridIndex(glm::vec3 p) {
    double invdx = 1.0 / dx;
    return GridIndex((int)floor(p.x*invdx),
                     (int)floor(p.y*invdx),
                     (int)floor(p.z*invdx));
}

glm::vec3 LevelSetField::_gridIndexToPosition(GridIndex g) {
    return glm::vec3((double)g.i*dx, (double)g.j*dx, (double)g.k*dx);
}

/* 
    Intopolation methods from http://www.paulinternet.nl/?page=bicubic

    - p is indexed in order by p[i][j][k]
    - x, y, z are in [0,1]
    - this function will interpolate the volume between point Index 1 and 2
*/
double LevelSetField::_fasttricubicInterpolate(double p[4][4][4], double x, double y, double z) {
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