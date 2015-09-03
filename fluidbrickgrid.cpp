#include "fluidbrickgrid.h"

FluidBrickGrid::FluidBrickGrid() {
}

FluidBrickGrid::FluidBrickGrid(int isize, int jsize, int ksize, double dx, AABB brick) :
                                    _isize(isize), _jsize(jsize), _ksize(ksize), _dx(dx),
                                    _brick(brick),
                                    _densityGrid(isize, jsize, ksize) {
    _initializeBrickGrid();
}

FluidBrickGrid::~FluidBrickGrid() {
}

void FluidBrickGrid::getGridDimensions(int *i, int *j, int *k) {
    *i = _isize;
    *j = _jsize;
    *k = _ksize;
}

void FluidBrickGrid::setBrickDimensions(double width, double height, double depth) {
    _brick = AABB(glm::vec3(), width, height, depth);
    _initializeBrickGrid();
}

void FluidBrickGrid::setBrickDimensions(AABB brick) {
    _brick = brick;
    _initializeBrickGrid();
}

Array3d<float> FluidBrickGrid::getDensityGrid() {
    Array3d<float> grid = Array3d<float>(_isize, _jsize, _ksize);
    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize; i++) {
                grid.set(i, j, k, _densityGrid(i, j, k).currentDensity);
            }
        }
    }

    return grid;
}

bool FluidBrickGrid::getBrickMesh(LevelSet &levelset, TriangleMesh &mesh) {
    if (!_isCurrentBrickGridReady) {
        return false;
    }

    double bw = _brick.width;
    double bh = _brick.height;
    double bd = _brick.depth;
    double maxwidth = 2.0*fmax(bw, fmax(bh, bd));

    glm::vec3 coffset = glm::vec3(0.5*bw, 0.5*bh, 0.5*bd);

    glm::vec3 p;
    for (int k = 0;  k < _currentBrickGrid.depth; k++) {
        for (int j = 0;  j < _currentBrickGrid.height; j++) {
            for (int i = 0;  i < _currentBrickGrid.width; i++) {
                if (_currentBrickGrid(i, j, k).isActive) {
                    p = coffset + glm::vec3(i*bw, j*bh, k*bd);

                    if (levelset.getDistance(p) <= maxwidth) {
                        double intensity = _currentBrickGrid(i, j, k).intensity;
                        mesh.vertices.push_back(p);
                        mesh.vertexcolors.push_back(glm::vec3(intensity, intensity, intensity));
                    }
                }
            }
        }
    }

    return true;
}

bool FluidBrickGrid::isBrickMeshReady() {
    return _isCurrentBrickGridReady;
}

void FluidBrickGrid::_getNewBrickLocations(Array3d<Brick> &b1, Array3d<Brick> &b2, 
                                           Array3d<bool> &newBricks) {
    for (int k = 0;  k < b1.depth; k++) {
        for (int j = 0;  j < b1.height; j++) {
            for (int i = 0;  i < b1.width; i++) {
                if (!b1(i, j, k).isActive && b2(i, j, k).isActive) {
                    newBricks.set(i, j, k, true);
                }
            }
        }
    }
}

void FluidBrickGrid::_getConnectedBricks(int i, int j, int k, 
                                         Array3d<Brick> &brickGrid,
                                         Array3d<bool> &newBricks, 
                                         std::vector<GridIndex> &connectedBricks) {
    int bw = _brickGrid.width;
    int bh = _brickGrid.height;
    int bd = _brickGrid.depth;

    Array3d<bool> isProcessed = Array3d<bool>(bw, bh, bd, false);

    std::vector<GridIndex> queue;
    queue.push_back(GridIndex(i, j, k));
    newBricks.set(i, j, k, false);
    isProcessed.set(i, j, k, true);

    GridIndex nbs[6];
    GridIndex g, n;
    while (!queue.empty()) {
        g = queue.back();
        queue.pop_back();

        Grid3d::getNeighbourGridIndices6(g, nbs);
        for (int idx = 0; idx < 6; idx++) {
            n = nbs[idx];
            if (Grid3d::isGridIndexInRange(n, bw, bh, bd) && brickGrid(n).isActive &&
                                                             !isProcessed(n)) {
                queue.push_back(n);
                newBricks.set(n, false);
                isProcessed.set(n, true);
            }
        }

        connectedBricks.push_back(g);
    }
}

void FluidBrickGrid::_getbrickStructures(Array3d<Brick> &brickGrid, Array3d<bool> &newBricks, 
                                     std::vector<std::vector<GridIndex>> &brickStructures) {
    for (int k = 0;  k < newBricks.depth; k++) {
        for (int j = 0;  j < newBricks.height; j++) {
            for (int i = 0;  i < newBricks.width; i++) {
                if (newBricks(i, j, k)) {

                    std::vector<GridIndex> connectedBricks;
                    _getConnectedBricks(i, j, k, brickGrid, newBricks, connectedBricks);
                    brickStructures.push_back(connectedBricks);
                }
            }
        }
    }
}

bool FluidBrickGrid::_isBrickMassInBrickGrid(std::vector<GridIndex> &cells,
                                             Array3d<Brick> &brickGrid) {
    bool isInGrid = false;
    for (unsigned int i = 0; i < cells.size(); i++) {
        if (brickGrid(cells[i]).isActive) {
            isInGrid = true;
            break;
        }
    }

    return isInGrid;
}

void FluidBrickGrid::_removeBrickStructureFromBrickGrid(std::vector<GridIndex> &cells, 
                                                   Array3d<Brick> &brickGrid) {
    Brick *b;
    for (unsigned int i = 0; i < cells.size(); i++) {
        b = brickGrid.getPointer(cells[i]);
        b->isActive = false;
    }
}

void FluidBrickGrid::_removeInvalidbrickStructures(Array3d<Brick> &brickCurrent,
                                               Array3d<Brick> &brickNext,
                                               std::vector<std::vector<GridIndex>> &brickStructures) {
    for (unsigned int i = 0; i < brickStructures.size(); i++) {
        if (!_isBrickMassInBrickGrid(brickStructures[i], brickNext)) {
            _removeBrickStructureFromBrickGrid(brickStructures[i], brickCurrent);
        }
    }
}

void FluidBrickGrid::_removeStrayBricks() {
    Array3d<Brick> brickPrev = _brickGridQueue[0];
    Array3d<Brick> brickCurrent = _brickGridQueue[1];
    Array3d<Brick> brickNext = _brickGridQueue[2];

    int bw = _brickGrid.width;
    int bh = _brickGrid.height;
    int bd = _brickGrid.depth;
    Array3d<bool> newBricks = Array3d<bool>(bw, bh, bd, false);
    _getNewBrickLocations(brickPrev, brickCurrent, newBricks);

    std::vector<std::vector<GridIndex>> brickStructures;
    _getbrickStructures(brickCurrent, newBricks, brickStructures);
    _removeInvalidbrickStructures(brickCurrent, brickNext, brickStructures);

    _brickGridQueue[1] = brickCurrent;
}

void FluidBrickGrid::_removeSmallBrickStructures() {
    Array3d<Brick> brickCurrent = _brickGridQueue[1];
    Array3d<bool> allBricks = Array3d<bool>(brickCurrent.width,
                                            brickCurrent.height,
                                            brickCurrent.depth, false);

    for (int k = 0;  k < brickCurrent.depth; k++) {
        for (int j = 0;  j < brickCurrent.height; j++) {
            for (int i = 0;  i < brickCurrent.width; i++) {
                if (brickCurrent(i, j, k).isActive) {
                    allBricks.set(i, j, k, true);
                }
            }
        }
    }

    std::vector<std::vector<GridIndex>> brickStructures;
    _getbrickStructures(brickCurrent, allBricks, brickStructures);
    for (unsigned int i = 0; i < brickStructures.size(); i++) {
        if (brickStructures[i].size() < _minNumberOfBricksInStructure) {
            _removeBrickStructureFromBrickGrid(brickStructures[i], brickCurrent);
        }
    }

    _brickGridQueue[1] = brickCurrent;
}

void FluidBrickGrid::_mergeBrickGrids() {
    Array3d<Brick> brickPrev = _brickGridQueue[0];
    Array3d<Brick> brickCurrent = _brickGridQueue[1];

    for (int k = 0;  k < brickPrev.depth; k++) {
        for (int j = 0;  j < brickPrev.height; j++) {
            for (int i = 0;  i < brickPrev.width; i++) {
                if (!brickPrev(i, j, k).isActive && brickCurrent(i, j, k).isActive) {
                    brickPrev.set(i, j, k, brickCurrent(i, j, k));
                }
            }
        }
    }

    _brickGridQueue[0] = brickPrev;
}

void FluidBrickGrid::_postProcessBrickGrid() {
    _removeStrayBricks();
    _removeSmallBrickStructures();
    _mergeBrickGrids();
}

void FluidBrickGrid::update(LevelSet &levelset, std::vector<glm::vec3> &particles, double dt) {
    _updateDensityGrid(particles, dt);
    _updateBrickGrid(levelset);

    if (_brickGridQueueSize == 3) {
        _postProcessBrickGrid();
    }

    if (_brickGridQueueSize == 3) {
        _currentBrickGrid = _brickGridQueue[0];
        _isCurrentBrickGridReady = true;

        _brickGridQueue[0] = _brickGridQueue[1];
        _brickGridQueue[1] = _brickGridQueue[2];
        _brickGridQueue[2] = _brickGrid;
    } else {
        _brickGridQueue[_brickGridQueueSize] = _brickGrid;
        _brickGridQueueSize++;
    }

    _numUpdates++;
}

void FluidBrickGrid::_initializeBrickGrid() {
    assert(_brick.width > 0.0);
    assert(_brick.height > 0.0);
    assert(_brick.depth > 0.0);

    double width = _isize*_dx;
    double height = _isize*_dx;
    double depth = _isize*_dx;
    int bi = (int)ceil(width / _brick.width);
    int bj = (int)ceil(height / _brick.height);
    int bk = (int)ceil(depth / _brick.depth);

    _brickGrid = Array3d<Brick>(bi, bj, bk);
    _numUpdates = 0;
}

void FluidBrickGrid::_updateDensityGrid(std::vector<glm::vec3> &particles, double dt) {
    _updateTargetDensities(particles);
    _updateDensities(dt);
}

void FluidBrickGrid::_updateTargetDensities(std::vector<glm::vec3> &points) {
    Array3d<int> _countGrid = Array3d<int>(_isize, _jsize, _ksize, 0);
    GridIndex g;
    for (unsigned int i = 0; i < points.size(); i++) {
        g = Grid3d::positionToGridIndex(points[i], _dx);
        if (Grid3d::isGridIndexInRange(g, _isize, _jsize, _ksize)) {
            _countGrid.add(g, 1);
        }
    }

    double min = (double)_minParticleDensityCount;
    double max = (double)_maxParticleDensityCount;
    DensityNode *node;
    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize; i++) {
                double density = ((double)_countGrid(i, j, k) - min) / (max - min);
                density = fmax(0.0, density);
                density = fmin(1.0, density);

                node = _densityGrid.getPointer(i, j, k);
                node->targetDensity = (float)density;

                if (_numUpdates == 0) {
                    node->currentDensity = (float)density;
                }
            }
        }
    }
}

void FluidBrickGrid::_updateDensity(int i, int j, int k, double dt) {
    DensityNode *node = _densityGrid.getPointer(i, j, k);

    float target = node->targetDensity;
    float current = node->currentDensity;
    float desired = target - current;
    if (fabs(desired) < _decelerationRadius) {
        float r = fabs(desired) / _decelerationRadius;
        float mag = r*_maxIntensityVelocity;
        desired = desired > 0.0f ? mag : -mag;
    } else {
        desired = desired > 0.0f ? _maxIntensityVelocity : -_maxIntensityVelocity;
    }

    float acc = desired - node->intensityVelocity;
    if (fabs(acc) > _maxIntensityAcceleration) {
        acc = acc > 0.0f ? _maxIntensityAcceleration : -_maxIntensityAcceleration;
    }

    node->intensityVelocity += acc*(float)dt;
    node->currentDensity += node->intensityVelocity*(float)dt;
    node->currentDensity = fmax(0.0f, node->currentDensity);
    node->currentDensity = fmin(1.0f, node->currentDensity);
}

void FluidBrickGrid::_updateDensities(double dt) {
    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize; i++) {
                _updateDensity(i, j, k, dt);
            }
        }
    }
}

float FluidBrickGrid::_getBrickIntensity(int i, int j, int k) {
    glm::vec3 pmin = glm::vec3(i*_brick.width, j*_brick.height, k*_brick.depth);
    glm::vec3 pmax = pmin + glm::vec3(_brick.width, _brick.height, _brick.depth);
    AABB bbox = AABB(pmin, pmax);
    GridIndex gmin, gmax;
    Grid3d::getGridIndexBounds(bbox, _dx, _isize, _jsize, _ksize, &gmin, &gmax);

    double avg = 0.0;
    double count = 0.0;
    double eps = 0.001;
    for (int k = gmin.k; k < gmax.k; k++) {
        for (int j = gmin.j; j < gmax.j; j++) {
            for (int i = gmin.i; i < gmax.i; i++) {
                double val = (double)_densityGrid(i, j, k).currentDensity;
                if (val > eps) {
                    avg += val;
                    count++;
                }
            }
        }
    }

    if (count == 0.0) {
        return 0.0;
    }

    return (float)(avg /= count);
}

bool FluidBrickGrid::_isBrickNextToActiveNeighbour(int i, int j, int k) {
    GridIndex nbs[6];
    GridIndex n;
    Grid3d::getNeighbourGridIndices6(i, j, k, nbs);
    bool hasNeighbour = false;
    for (int idx = 0; idx < 6; idx++) {
        n = nbs[idx];
        if (Grid3d::isGridIndexInRange(n, _brickGrid.width, _brickGrid.height, _brickGrid.depth) &&
                _brickGrid(n).isActive) {
            hasNeighbour = true;
            break;
        }
    }

    return hasNeighbour;
}

void FluidBrickGrid::_updateBrickGrid(LevelSet &levelset) {
    _brickGrid.fill(Brick());

    double bw = _brick.width;
    double bh = _brick.height;
    double bd = _brick.depth;

    glm::vec3 coffset = glm::vec3(0.5*bw, 0.5*bh, 0.5*bd);

    glm::vec3 p;
    for (int k = 0;  k < _brickGrid.depth; k++) {
        for (int j = 0;  j < _brickGrid.height; j++) {
            for (int i = 0;  i < _brickGrid.width; i++) {
                p = coffset + glm::vec3(i*bw, j*bh, k*bd);
                if (Grid3d::isPositionInGrid(p, _dx, _isize, _jsize, _ksize) && 
                        levelset.isPointInInsideCell(p)) {
                    float intensity = _getBrickIntensity(i, j, k);
                    Brick b = Brick(intensity);
                    b.isActive = true;
                    _brickGrid.set(i, j, k, b);
                }
            }
        }
    }

    Brick *b;
    GridIndex nbs[6];
    GridIndex n;
    for (int k = 0;  k < _brickGrid.depth; k++) {
        for (int j = 0;  j < _brickGrid.height; j++) {
            for (int i = 0;  i < _brickGrid.width; i++) {
                if (_brickGrid(i, j, k).isActive && !_isBrickNextToActiveNeighbour(i, j, k)) {
                    b = _brickGrid.getPointer(i, j, k);
                    b->isActive = false;
                }
            }
        }
    }
}