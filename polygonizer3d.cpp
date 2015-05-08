#include "polygonizer3d.h"


Polygonizer3d::Polygonizer3d() : _isize(0), _jsize(0), _ksize(0), _dx(1),
                                 _vertexValues(Array3d<double>(0, 0, 0, 0.0)),
                                 _isVertexSet(Array3d<bool>(0, 0, 0, false))
{
}

Polygonizer3d::Polygonizer3d(int i_width, int j_height, int k_depth, double cellsize,
                             ImplicitField *field) : 
                             _isize(i_width), _jsize(j_height), _ksize(k_depth), _dx(cellsize),
                             _field(field),
                             _vertexValues(Array3d<double>(_isize+1, _jsize+1, _ksize+1, 0.0)),
                             _isVertexSet(Array3d<bool>(_isize+1, _jsize+1, _ksize+1, false))
{
    _isInitialized = true;
}


Polygonizer3d::~Polygonizer3d()
{
}

void Polygonizer3d::setInsideCellIndices(std::vector<GridIndex> indices) {
    _insideIndices.clear();
    _insideIndices.reserve(indices.size());
    for (int i = 0; i < indices.size(); i++) {
        _insideIndices.push_back(indices[i]);
    }
}

void Polygonizer3d::_getCellVertexIndices(int i, int j, int k, GridIndex vertices[8]){
    vertices[0] = GridIndex(i,   j,   k);
    vertices[1] = GridIndex(i+1, j,   k);
    vertices[2] = GridIndex(i+1, j,   k+1);
    vertices[3] = GridIndex(i,   j,   k+1);
    vertices[4] = GridIndex(i,   j+1, k);
    vertices[5] = GridIndex(i+1, j+1, k);
    vertices[6] = GridIndex(i+1, j+1, k+1);
    vertices[7] = GridIndex(i,   j+1, k+1);
}

glm::vec3 Polygonizer3d::_getVertexPosition(GridIndex v) {
    return _getVertexPosition(v.i, v.j, v.k);
}

glm::vec3 Polygonizer3d::_getVertexPosition(int i, int j, int k) {
    assert(_vertexValues.isIndexInRange(i, j, k));
    return glm::vec3((double)i*_dx, (double)j*_dx, (double)k*_dx);
}

double Polygonizer3d::_getVertexFieldValue(GridIndex v) {
    return _getVertexFieldValue(v.i, v.j, v.k);
}

double Polygonizer3d::_getVertexFieldValue(int i, int j, int k) {
    assert(_vertexValues.isIndexInRange(i, j, k));

    if (!_isVertexSet(i, j, k)) {
        glm::vec3 p = _getVertexPosition(i, j, k);
        _vertexValues.set(i, j, k, _field->getFieldValue(p));
    }

    return _vertexValues(i, j, k);;
}

void Polygonizer3d::polygonizeSurface() {
    _vertexValues.fill(0.0);
    _isVertexSet.fill(false);

    _surfaceCells = _findSurfaceCells();
}

bool Polygonizer3d::_isCellOutsideSurface(int i, int j, int k) {
    return _getCellSurfaceStatus(i, j, k) == 1;
}

bool Polygonizer3d::_isCellInsideSurface(int i, int j, int k) {
    return _getCellSurfaceStatus(i, j, k) == -1;
}

bool Polygonizer3d::_isCellOnSurface(int i, int j, int k) {
    return _getCellSurfaceStatus(i, j, k) == 0;
}

int Polygonizer3d::_getCellSurfaceStatus(int i, int j, int k) {
    GridIndex vertices[8];
    _getCellVertexIndices(i, j, k, vertices);

    bool hasInside = false;
    bool hasOutside = false;
    double thresh = _field->getSurfaceThreshold();
    for (int idx = 0; idx < 8; idx ++) {
        double val = _getVertexFieldValue(vertices[idx]);
        if (val > thresh) {
            hasInside = true;
        }
        else {
            hasOutside = true;
        }
    }

    if (hasInside && hasOutside) {
        return 0;
    } 
    else if (hasInside) {
        return -1;
    }
    else {
        return 1;
    }
}

void Polygonizer3d::_getNeighbourGridIndices6(GridIndex c, GridIndex n[6]) {
    n[0] = GridIndex(c.i - 1, c.j, c.k);
    n[1] = GridIndex(c.i + 1, c.j, c.k);
    n[2] = GridIndex(c.i, c.j - 1, c.k);
    n[3] = GridIndex(c.i, c.j + 1, c.k);
    n[4] = GridIndex(c.i, c.j, c.k - 1);
    n[5] = GridIndex(c.i, c.j, c.k + 1);
}

std::vector<GridIndex> Polygonizer3d::_processSeedCell(GridIndex seed, 
                                                       Array3d<bool> &isCellDone) {
    std::vector<GridIndex> seedSurfaceCells;

    isCellDone.set(seed.i, seed.j, seed.k, true);
    std::queue<GridIndex> queue;
    queue.push(seed);

    while (!queue.empty()) {
        GridIndex c = queue.front();
        queue.pop();

        GridIndex neighbours[6];
        _getNeighbourGridIndices6(c, neighbours);
        for (int idx = 0; idx < 6; idx++) {
            GridIndex n = neighbours[idx];
            if (_isCellIndexInRange(n.i, n.j, n.k) && 
                    !isCellDone(n.i, n.j, n.k) &&
                    _isCellOnSurface(n.i, n.j, n.k)) {
                isCellDone.set(n.i, n.j, n.k, true);
                queue.push(n);
            }
        }

        seedSurfaceCells.push_back(c);
    }

    return seedSurfaceCells;
}

std::vector<GridIndex> Polygonizer3d::_findSurfaceCells() {
    std::vector<GridIndex> surfaceCells;
    Array3d<bool> isCellDone = Array3d<bool>(_isize, _jsize, _ksize, false);

    for (int i = _insideIndices.size() - 1; i >= 0; i--) {
        GridIndex cell = _insideIndices[i];
        
        if (isCellDone(cell.i, cell.j, cell.k)) {
            continue;
        }

        while (_isCellIndexInRange(cell.i, cell.j, cell.k)) {

            if (_isCellOnSurface(cell.i, cell.j, cell.k)) {
                std::vector<GridIndex> seedSurfaceCells = _processSeedCell(cell, isCellDone);
                surfaceCells.insert(surfaceCells.end(), seedSurfaceCells.begin(), seedSurfaceCells.end());
                break;
            }

            // march left until cell surface is found or index is out of range
            isCellDone.set(cell.i, cell.j, cell.k, true);
            cell = GridIndex(cell.i - 1, cell.j, cell.k);
        }
    }

    return surfaceCells;
}
