/*
Copyright (c) 2016 Ryan L. Guy

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
#include "fluidsimulation.h"


FluidSimulation::FluidSimulation() {
}

FluidSimulation::FluidSimulation(int isize, int jsize, int ksize, double dx) :
                                _isize(isize), _jsize(jsize), _ksize(ksize), _dx(dx),
                                _markerParticleRadius(pow(3*(_dx*_dx*_dx / 8.0) / (4*3.141592653), 1.0/3.0)),
                                _MACVelocity(_isize, _jsize, _ksize, _dx),
                                _materialGrid(_isize, _jsize, _ksize),
                                _fluidCellIndices(_isize, _jsize, _ksize),
                                _addedFluidCellQueue(_isize, _jsize, _ksize),
                                _levelset(_isize, _jsize, _ksize, _dx) {
}

FluidSimulation::FluidSimulation(FluidSimulationSaveState &state) {
    assert(state.isLoadStateInitialized());
    _initializeSimulationFromSaveState(state);
}

FluidSimulation::~FluidSimulation() {
    for (unsigned int i = 0; i < _fluidSources.size(); i++) {
        delete[] _fluidSources[i];
    }
}

/*******************************************************************************
    PUBLIC
********************************************************************************/

void FluidSimulation::run() {
    if (!_isSimulationInitialized) {
        _initializeSimulation();
    }
    _isSimulationRunning = true;
}

void FluidSimulation::pause() {
    if (!_isSimulationInitialized) {
        return;
    }

    _isSimulationRunning = !_isSimulationRunning;
}

void FluidSimulation::saveState() {
    saveState("savestates/autosave.state");
}

void FluidSimulation::saveState(std::string filename) {
    if (!_isSaveStateEnabled) {
        return;
    }

    FluidSimulationSaveState state;
    state.saveState(filename, this);
}

int FluidSimulation::getCurrentFrame() {
    return _currentFrame;
}

bool FluidSimulation::isCurrentFrameFinished() { 
    return _isCurrentFrameFinished; 
}

double FluidSimulation::getCellSize() { 
    return _dx; 
}

void FluidSimulation::getGridDimensions(int *i, int *j, int *k) { 
    *i = _isize; *j = _jsize; *k = _ksize; 
}

void FluidSimulation::getSimulationDimensions(double *w, double *h, double *d) { 
    *w = (double)_isize*_dx;
    *h = (double)_jsize*_dx;
    *d = (double)_ksize*_dx;
}

double FluidSimulation::getSimulationWidth() {  
    return (double)_isize*_dx; 
}

double FluidSimulation::getSimulationHeight() { 
    return (double)_jsize*_dx; 
}

double FluidSimulation::getSimulationDepth() {  
    return (double)_ksize*_dx; 
}

double FluidSimulation::getDensity() { 
    return _density; 
}

void FluidSimulation::setDensity(double p) { 
    assert(p > 0); _density = p; 
}

Material FluidSimulation::getMaterial(int i, int j, int k) { 
    return _materialGrid(i, j, k); 
}

void FluidSimulation::setMarkerParticleScale(double s) { 
    _markerParticleScale = s; 
}


void FluidSimulation::setSurfaceSubdivisionLevel(unsigned int n) {
    assert(n >= 1);
    _outputFluidSurfaceSubdivisionLevel = n;
}

void FluidSimulation::setNumSurfaceReconstructionPolygonizerSlices(int n) {
    assert(n >= 1);
    _numSurfaceReconstructionPolygonizerSlices = n;
}

void FluidSimulation::setMinimumPolyhedronTriangleCount(unsigned int n) {
    _minimumSurfacePolyhedronTriangleCount = n;
}

void FluidSimulation::enableSurfaceMeshOutput() {
    _isSurfaceMeshOutputEnabled = true;
}

void FluidSimulation::disableSurfaceMeshOutput() {
    _isSurfaceMeshOutputEnabled = false;
}

void FluidSimulation::enableIsotropicSurfaceReconstruction() {
    _isIsotropicSurfaceMeshReconstructionEnabled = true;
}

void FluidSimulation::disableIsotropicSurfaceReconstruction() {
    _isIsotropicSurfaceMeshReconstructionEnabled = false;
}

void FluidSimulation::enableAnisotropicSurfaceReconstruction() {
    _isAnisotropicSurfaceMeshReconstructionEnabled = true;
}

void FluidSimulation::disableAnisotropicSurfaceReconstruction() {
    _isAnisotropicSurfaceMeshReconstructionEnabled = false;
}


void FluidSimulation::enableDiffuseMaterialOutput() {
    _isDiffuseMaterialOutputEnabled = true;
    _isBubbleDiffuseMaterialEnabled = true;
    _isSprayDiffuseMaterialEnabled = true;
    _isFoamDiffuseMaterialEnabled = true;
}

void FluidSimulation::disableDiffuseMaterialOutput() {
    _isDiffuseMaterialOutputEnabled = false;
    _isBubbleDiffuseMaterialEnabled = false;
    _isSprayDiffuseMaterialEnabled = false;
    _isFoamDiffuseMaterialEnabled = false;
}

void FluidSimulation::enableBubbleDiffuseMaterial() {
    _isBubbleDiffuseMaterialEnabled = true;
    _isDiffuseMaterialOutputEnabled = true;
}

void FluidSimulation::enableSprayDiffuseMaterial() {
    _isSprayDiffuseMaterialEnabled = true;
    _isDiffuseMaterialOutputEnabled = true;
}

void FluidSimulation::enableFoamDiffuseMaterial() {
    _isFoamDiffuseMaterialEnabled = true;
    _isDiffuseMaterialOutputEnabled = true;
}

void FluidSimulation::disableBubbleDiffuseMaterial() {
    _isBubbleDiffuseMaterialEnabled = false;
}

void FluidSimulation::disableSprayDiffuseMaterial() {
    _isSprayDiffuseMaterialEnabled = false;
}

void FluidSimulation::disableFoamDiffuseMaterial() {
    _isFoamDiffuseMaterialEnabled = false;
}

void FluidSimulation::outputDiffuseMaterialAsSeparateFiles() {
    _isDiffuseMaterialOutputEnabled = true;
    _isDiffuseMaterialFilesSeparated = true;
}

void FluidSimulation::outputDiffuseMaterialAsSingleFile() {
    _isDiffuseMaterialOutputEnabled = true;
    _isDiffuseMaterialFilesSeparated = false;
}

void FluidSimulation::enableBrickOutput() {
    AABB brick = AABB(vmath::vec3(), _brickWidth, _brickHeight, _brickDepth);
    _fluidBrickGrid = FluidBrickGrid(_isize, _jsize, _ksize, _dx, brick);
    _isBrickOutputEnabled = true;
}

void FluidSimulation::enableBrickOutput(double width, double height, double depth) {
    assert(width > 0.0 && height > 0.0 && depth > 0.0);
    _brickWidth = width;
    _brickHeight = height;
    _brickDepth = depth;

    AABB brick = AABB(vmath::vec3(), _brickWidth, _brickHeight, _brickDepth);

    int i, j, k;
    _fluidBrickGrid.getGridDimensions(&i, &j, &k);
    if (i != _isize || j != _jsize || k != _ksize) {
        _fluidBrickGrid = FluidBrickGrid(_isize, _jsize, _ksize, _dx, brick);
    }
    _fluidBrickGrid.setBrickDimensions(brick);

    _isBrickOutputEnabled = true;
}

void FluidSimulation::disableBrickOutput() {
    _isBrickOutputEnabled = false;
}

void FluidSimulation::enableSaveState() {
    _isSaveStateEnabled = true;
}

void FluidSimulation::disableSaveState() {
    _isSaveStateEnabled = false;
}

void FluidSimulation::addBodyForce(double fx, double fy, double fz) { 
    addBodyForce(vmath::vec3(fx, fy, fz)); 
}

void FluidSimulation::addBodyForce(vmath::vec3 f) {
    _constantBodyForces.push_back(f);
}

void FluidSimulation::addBodyForce(vmath::vec3 (*fieldFunction)(vmath::vec3)) {
    _variableBodyForces.push_back(fieldFunction);
}

void FluidSimulation::resetBodyForces() {
    _constantBodyForces.clear();
    _variableBodyForces.clear();
}

void FluidSimulation::addImplicitFluidPoint(double x, double y, double z, double r) {
    addImplicitFluidPoint(vmath::vec3(x, y, z), r);
}

void FluidSimulation::addImplicitFluidPoint(vmath::vec3 p, double r) {
    _fluidPoints.push_back(FluidPoint(p, r));
}

void FluidSimulation::addFluidCuboid(double x, double y, double z, 
                                     double w, double h, double d) {
    addFluidCuboid(vmath::vec3(x, y, z), w, h, d);
}

void FluidSimulation::addFluidCuboid(vmath::vec3 p1, vmath::vec3 p2) {
    vmath::vec3 minp = vmath::vec3(fmin(p1.x, p2.x),
                               fmin(p1.y, p2.y), 
                               fmin(p1.z, p2.z));
    double width = fabs(p2.x - p1.x);
    double height = fabs(p2.y - p1.y);
    double depth = fabs(p2.z - p1.z);

    addFluidCuboid(minp, width, height, depth);
}

void FluidSimulation::addFluidCuboid(vmath::vec3 p, double w, double h, double d) {
    _fluidCuboids.push_back(FluidCuboid(p, w, h, d));
}


SphericalFluidSource* FluidSimulation::addSphericalFluidSource(vmath::vec3 pos, double r) {
    SphericalFluidSource *source = new SphericalFluidSource(pos, r);
    source->setID(_getUniqueFluidSourceID());

    _fluidSources.push_back(source);
    _sphericalFluidSources.push_back(source);
    return source;
}

SphericalFluidSource* FluidSimulation::addSphericalFluidSource(vmath::vec3 pos, double r, 
                                             vmath::vec3 velocity) {
    SphericalFluidSource *source = new SphericalFluidSource(pos, r, velocity);
    source->setID(_getUniqueFluidSourceID());

    _fluidSources.push_back(source);
    _sphericalFluidSources.push_back(source);
    return source;
}

CuboidFluidSource* FluidSimulation::addCuboidFluidSource(AABB bbox) {
    CuboidFluidSource *source = new CuboidFluidSource(bbox);
    source->setID(_getUniqueFluidSourceID());

    _fluidSources.push_back(source);
    _cuboidFluidSources.push_back(source);
    return source;
}

CuboidFluidSource* FluidSimulation::addCuboidFluidSource(AABB bbox, vmath::vec3 velocity) {
    CuboidFluidSource *source = new CuboidFluidSource(bbox, velocity);
    source->setID(_getUniqueFluidSourceID());

    _fluidSources.push_back(source);
    _cuboidFluidSources.push_back(source);
    return source;
}

void FluidSimulation::removeFluidSource(FluidSource *source) {
    bool isFound = false;
    for (unsigned int i = 0; i < _fluidSources.size(); i++) {
        if (source->getID() == _fluidSources[i]->getID()) {
            delete (_fluidSources[i]);
            _fluidSources.erase(_fluidSources.begin() + i);
            isFound = true;
            break;
        }
    }

    assert(isFound);

    isFound = false;
    for (unsigned int i = 0; i < _sphericalFluidSources.size(); i++) {
        if (source->getID() == _sphericalFluidSources[i]->getID()) {
            _sphericalFluidSources.erase(_sphericalFluidSources.begin() + i);
            isFound = true;
            break;
        }
    }

    for (unsigned int i = 0; i < _cuboidFluidSources.size(); i++) {
        if (source->getID() == _cuboidFluidSources[i]->getID()) {
            _cuboidFluidSources.erase(_cuboidFluidSources.begin() + i);
            isFound = true;
            break;
        }
    }

    assert(isFound);
}

void FluidSimulation::removeFluidSources() {
    for (unsigned int i = 0; i < _fluidSources.size(); i++) {
        delete (_fluidSources[i]);
    }
    _fluidSources.clear();
    _sphericalFluidSources.clear();
    _cuboidFluidSources.clear();
}

void FluidSimulation::addSolidCell(int i, int j, int k) {
    if (_isSimulationInitialized) { return; }
    assert(Grid3d::isGridIndexInRange(i, j, k, _isize, _jsize, _ksize));
    _materialGrid.setSolid(i, j, k);
}

void FluidSimulation::addSolidCell(GridIndex g) {
    addSolidCell(g.i, g.j, g.k);
}

void FluidSimulation::addSolidCells(std::vector<GridIndex> indices) {
    for (unsigned int i = 0; i < indices.size(); i++) {
        addSolidCell(indices[i]);
    }
}

void FluidSimulation::removeSolidCell(int i, int j, int k) {
    assert(Grid3d::isGridIndexInRange(i, j, k, _isize, _jsize, _ksize));

    // Cannot remove border cells
    if (Grid3d::isGridIndexOnBorder(i, j, k, _isize, _jsize, _ksize)) { 
        return; 
    }

    if (_materialGrid.isCellSolid(i, j, k)) {
        _materialGrid.setAir(i, j, k);
    }
}

void FluidSimulation::removeSolidCells(std::vector<vmath::vec3> indices) {
    for (unsigned int i = 0; i < indices.size(); i++) {
        removeSolidCell((int)indices[i].x, (int)indices[i].y, (int)indices[i].z);
    }
}

std::vector<vmath::vec3> FluidSimulation::getSolidCells() {
    std::vector<vmath::vec3> indices;
    for (int k = 1; k < _materialGrid.depth - 1; k++) {
        for (int j = 1; j < _materialGrid.height - 1; j++) {
            for (int i = 1; i < _materialGrid.width - 1; i++) {
                if (_materialGrid.isCellSolid(i, j, k)) {
                    indices.push_back(vmath::vec3(i, j, k));
                }
            }
        }
    }

    return indices;
}

std::vector<vmath::vec3> FluidSimulation::getSolidCellPositions() {
    std::vector<vmath::vec3> indices;
    for (int k = 1; k < _materialGrid.depth - 1; k++) {
        for (int j = 1; j < _materialGrid.height - 1; j++) {
            for (int i = 1; i < _materialGrid.width - 1; i++) {
                if (_materialGrid.isCellSolid(i, j, k)) {
                    indices.push_back(Grid3d::GridIndexToCellCenter(i, j, k, _dx));
                }
            }
        }
    }

    return indices;
}

void FluidSimulation::addFluidCell(int i, int j, int k) {
    assert(Grid3d::isGridIndexInRange(i, j, k, _isize, _jsize, _ksize));

    if (_materialGrid.isCellAir(i, j, k)) {
        _addedFluidCellQueue.push_back(i, j, k);
    }
}

void FluidSimulation::addFluidCell(GridIndex g) {
    addFluidCell(g.i, g.j, g.k);
}

void FluidSimulation::addFluidCells(GridIndexVector indices) {
    for (unsigned int i = 0; i < indices.size(); i++) {
        addFluidCell(indices[i]);
    }
}

unsigned int FluidSimulation::getNumMarkerParticles() {
    return _markerParticles.size();
}

std::vector<vmath::vec3> FluidSimulation::getMarkerParticlePositions() {
    return getMarkerParticlePositions(0, _markerParticles.size() - 1);
}

std::vector<vmath::vec3> FluidSimulation::getMarkerParticlePositions(int startidx, int endidx) {
    assert(startidx >= 0 && startidx < (int)_markerParticles.size());
    assert(endidx >= 0 && endidx < (int)_markerParticles.size());

    std::vector<vmath::vec3> particles;
    particles.reserve(endidx - startidx + 1);

    for (int i = startidx; i <= endidx; i++) {
        particles.push_back(_markerParticles[i].position);
    }

    return particles;
}

std::vector<vmath::vec3> FluidSimulation::getMarkerParticleVelocities() {
    return getMarkerParticleVelocities(0, _markerParticles.size() - 1);
}

std::vector<vmath::vec3> FluidSimulation::getMarkerParticleVelocities(int startidx, int endidx) {
    assert(startidx >= 0 && startidx < (int)_markerParticles.size());
    assert(endidx >= 0 && endidx < (int)_markerParticles.size());

    std::vector<vmath::vec3> velocities;
    velocities.reserve(endidx - startidx + 1);

    for (int i = startidx; i <= endidx; i++) {
        velocities.push_back(_markerParticles[i].velocity);
    }

    return velocities;
}

unsigned int FluidSimulation::getNumDiffuseParticles() {
    return _diffuseParticles.size();
}

std::vector<vmath::vec3> FluidSimulation::getDiffuseParticlePositions() {
    return getDiffuseParticlePositions(0, _diffuseParticles.size() - 1);
}

std::vector<vmath::vec3> FluidSimulation::getDiffuseParticlePositions(int startidx, int endidx) {
    assert(startidx >= 0 && startidx < (int)_diffuseParticles.size());
    assert(endidx >= 0 && endidx < (int)_diffuseParticles.size());

    std::vector<vmath::vec3> particles;
    particles.reserve(endidx - startidx + 1);

    for (int i = startidx; i <= endidx; i++) {
        particles.push_back(_diffuseParticles[i].position);
    }

    return particles;
}

std::vector<vmath::vec3> FluidSimulation::getDiffuseParticleVelocities() {
    return getDiffuseParticleVelocities(0, _diffuseParticles.size() - 1);
}

std::vector<vmath::vec3> FluidSimulation::getDiffuseParticleVelocities(int startidx, int endidx) {
    assert(startidx >= 0 && startidx < (int)_diffuseParticles.size());
    assert(endidx >= 0 && endidx < (int)_diffuseParticles.size());

    std::vector<vmath::vec3> velocities;
    velocities.reserve(endidx - startidx + 1);

    for (int i = startidx; i <= endidx; i++) {
        velocities.push_back(_diffuseParticles[i].velocity);
    }

    return velocities;
}

std::vector<float> FluidSimulation::getDiffuseParticleLifetimes(int startidx, int endidx) {
    assert(startidx >= 0 && startidx < (int)_diffuseParticles.size());
    assert(endidx >= 0 && endidx < (int)_diffuseParticles.size());

    std::vector<float> lifetimes;
    lifetimes.reserve(endidx - startidx + 1);

    for (int i = startidx; i <= endidx; i++) {
        lifetimes.push_back(_diffuseParticles[i].lifetime);
    }

    return lifetimes;
}

std::vector<float> FluidSimulation::getDiffuseParticleLifetimes() {
    return getDiffuseParticleLifetimes(0, _diffuseParticles.size() - 1);
}

void FluidSimulation::getDiffuseParticles(std::vector<DiffuseParticle> &dps) {
    dps.reserve(_diffuseParticles.size());
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        dps.push_back(_diffuseParticles[i]);
    }
}

Array3d<float> FluidSimulation::getDensityGrid() {
    return _fluidBrickGrid.getDensityGrid();
}

MACVelocityField* FluidSimulation::getVelocityField() { 
    return &_MACVelocity; 
}

LevelSet* FluidSimulation::getLevelSet() { 
    return &_levelset; 
};

TriangleMesh* FluidSimulation::getFluidSurfaceTriangles() {
    return &_surfaceMesh;
}

/********************************************************************************
    INITIALIZATION
********************************************************************************/
void FluidSimulation::_initializeSolidCells() {
    // fill borders with solid cells
    for (int j = 0; j < _jsize; j++) {
        for (int i = 0; i < _isize; i++) {
            _materialGrid.setSolid(i, j, 0);
            _materialGrid.setSolid(i, j, _ksize-1);
        }
    }

    for (int k = 0; k < _ksize; k++) {
        for (int i = 0; i < _isize; i++) {
            _materialGrid.setSolid(i, 0, k);
            _materialGrid.setSolid(i, _jsize-1, k);
        }
    }

    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize; j++) {
            _materialGrid.setSolid(0, j, k);
            _materialGrid.setSolid(_isize-1, j, k);
        }
    }
}

void FluidSimulation::_addMarkerParticlesToCell(GridIndex g) {
    _addMarkerParticlesToCell(g, vmath::vec3());
}

void FluidSimulation::_addMarkerParticlesToCell(GridIndex g, vmath::vec3 velocity) {
    double q = 0.25*_dx;
    vmath::vec3 c = Grid3d::GridIndexToCellCenter(g, _dx);

    vmath::vec3 points[] = {
        vmath::vec3(c.x - q, c.y - q, c.z - q),
        vmath::vec3(c.x + q, c.y - q, c.z - q),
        vmath::vec3(c.x + q, c.y - q, c.z + q),
        vmath::vec3(c.x - q, c.y - q, c.z + q),
        vmath::vec3(c.x - q, c.y + q, c.z - q),
        vmath::vec3(c.x + q, c.y + q, c.z - q),
        vmath::vec3(c.x + q, c.y + q, c.z + q),
        vmath::vec3(c.x - q, c.y + q, c.z + q)
    };

    double eps = 10e-6;
    double jitter = 0.25*_dx - eps;

    for (int idx = 0; idx < 8; idx++) {
        vmath::vec3 jit = vmath::vec3(_randomDouble(-jitter, jitter),
                                  _randomDouble(-jitter, jitter),
                                  _randomDouble(-jitter, jitter));

        vmath::vec3 p = points[idx] + jit;
        _markerParticles.push_back(MarkerParticle(p, velocity));
    }
}

void FluidSimulation::_addMarkerParticle(vmath::vec3 p) {
    _addMarkerParticle(p, vmath::vec3());
}

void FluidSimulation::_addMarkerParticle(vmath::vec3 p, vmath::vec3 velocity) {
    GridIndex g = Grid3d::positionToGridIndex(p, _dx);
    if (Grid3d::isGridIndexInRange(g, _isize, _jsize, _ksize) &&
        !_materialGrid.isCellSolid(g)) {
        _markerParticles.push_back(MarkerParticle(p, velocity));
    }
}

void FluidSimulation::_addMarkerParticles(std::vector<vmath::vec3> particles) {
    for (unsigned int i = 0; i < particles.size(); i++) {
        _addMarkerParticle(particles[i], vmath::vec3());
    }
}

void FluidSimulation::_calculateInitialFluidSurfaceScalarField(ImplicitSurfaceScalarField &field) {
    FluidPoint fp;
    for (unsigned int i = 0; i < _fluidPoints.size(); i++) {
        fp = _fluidPoints[i];
        field.addPoint(fp.position, fp.radius);
    }

    FluidCuboid fc;
    for (unsigned int i = 0; i < _fluidCuboids.size(); i++) {
        fc = _fluidCuboids[i];
        field.addCuboid(fc.bbox.position, fc.bbox.width, fc.bbox.height, fc.bbox.depth);
    }
}

void FluidSimulation::_getInitialFluidCellsFromScalarField(ImplicitSurfaceScalarField &field,
                                                           GridIndexVector &fluidCells) {
    Polygonizer3d polygonizer(&field);

    field.setMaterialGrid(_materialGrid);

    polygonizer.polygonizeSurface();
    _surfaceMesh = polygonizer.getTriangleMesh();
    _surfaceMesh.setGridDimensions(_isize, _jsize, _ksize, _dx);

    fluidCells.clear();
    _surfaceMesh.getCellsInsideMesh(fluidCells);
}


void FluidSimulation::_getPartiallyFilledFluidCellParticles(GridIndexVector &partialFluidCells,
                                                            ImplicitSurfaceScalarField &field,
                                                            std::vector<vmath::vec3> &partialParticles) {
    FluidMaterialGrid submgrid(_isize, _jsize, _ksize);
    GridIndex g;
    for (unsigned int i = 0; i < partialFluidCells.size(); i++) {
        g = partialFluidCells[i];
        submgrid.setFluid(g);
    }
    submgrid.setSubdivisionLevel(2);
    double subdx = 0.5 * _dx;

    double eps = 10e-6;
    double jitter = 0.25*_dx - eps;
    vmath::vec3 jit;

    vmath::vec3 c;
    for (int k = 0; k < submgrid.depth; k++) {
        for (int j = 0; j < submgrid.height; j++) {
            for (int i = 0; i < submgrid.width; i++) {
                if (!submgrid.isCellFluid(i, j, k)) {
                    continue;
                }

                c = Grid3d::GridIndexToCellCenter(i, j, k, subdx);
                if (field.isPointInside(c)) {
                    jit = vmath::vec3(_randomDouble(-jitter, jitter),
                              _randomDouble(-jitter, jitter),
                              _randomDouble(-jitter, jitter));

                    partialParticles.push_back(c + jit);
                }
            }
        }
    }
}

void FluidSimulation::_initializeMarkerParticles(GridIndexVector &fullFluidCells,
                                                 std::vector<vmath::vec3> &partialParticles) {
    _markerParticles.reserve(8*fullFluidCells.size() + partialParticles.size());
    GridIndex g;
    for (unsigned int i = 0; i < fullFluidCells.size(); i++) {
        g = fullFluidCells[i];
        _addMarkerParticlesToCell(g);
    }
    _addMarkerParticles(partialParticles);
}

void FluidSimulation::_initializeFluidCellIndices() {
    GridIndex g;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        g = Grid3d::positionToGridIndex(_markerParticles[i].position, _dx);
        if (!_materialGrid.isCellFluid(g)) {
            _materialGrid.setFluid(g);
            _fluidCellIndices.push_back(g);
        }
    }
}

void FluidSimulation::_initializeFluidMaterial() {

    ImplicitSurfaceScalarField field = ImplicitSurfaceScalarField(_isize + 1, _jsize + 1, _ksize + 1, _dx);
    _calculateInitialFluidSurfaceScalarField(field);

    GridIndexVector fluidCells(_isize, _jsize, _ksize);
    _getInitialFluidCellsFromScalarField(field, fluidCells);

    FluidMaterialGrid mgrid(_isize, _jsize, _ksize);
    for (unsigned int i = 0; i < fluidCells.size(); i++) {
        mgrid.setFluid(fluidCells[i]);
    }

    GridIndexVector fullFluidCells(_isize, _jsize, _ksize);
    GridIndexVector partialFluidCells(_isize, _jsize, _ksize);
    GridIndex g;
    for (unsigned int i = 0; i < fluidCells.size(); i++) {
        g = fluidCells[i];
        if (mgrid.isCellNeighbouringAir(g)) {
            partialFluidCells.push_back(g);
        } else {
            fullFluidCells.push_back(g);
        }
    }

    std::vector<vmath::vec3> partialParticles;
    _getPartiallyFilledFluidCellParticles(partialFluidCells, field, partialParticles);

    _initializeMarkerParticles(fullFluidCells, partialParticles);
    _initializeFluidCellIndices();
}

void FluidSimulation::_initializeSimulation() {
    _initializeSolidCells();
    _initializeFluidMaterial();
    _initializeParticleAdvector();

    _isSimulationInitialized = true;
}

void FluidSimulation::_initializeFluidMaterialParticlesFromSaveState() {
    MarkerParticle p;
    GridIndex g;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        p = _markerParticles[i];
        g = Grid3d::positionToGridIndex(p.position, _dx);
        assert(!_materialGrid.isCellSolid(g));
        _materialGrid.setFluid(g);
    }

    _fluidCellIndices.clear();
    for (int k = 0; k < _materialGrid.depth; k++) {
        for (int j = 0; j < _materialGrid.height; j++) {
            for (int i = 0; i < _materialGrid.width; i++) {
                if (_materialGrid.isCellFluid(i, j, k)) {
                    _fluidCellIndices.push_back(i, j, k);
                }
            }
        }
    }
}

void FluidSimulation::_initializeMarkerParticlesFromSaveState(
                                        FluidSimulationSaveState &state) {

    int n = state.getNumMarkerParticles();
    _markerParticles.reserve(n);

    int chunksize = _loadStateReadChunkSize;
    int numRead = 0;

    std::vector<vmath::vec3> vectors;
    while (numRead < n) {
        int startidx = numRead;
        int endidx = numRead + chunksize - 1;
        if (endidx >= n) {
            endidx = n - 1;
        }

        vectors = state.getMarkerParticlePositions(startidx, endidx);
        for (unsigned int i = 0; i < vectors.size(); i++) {
            _markerParticles.push_back(MarkerParticle(vectors[i]));
        }

        numRead += vectors.size();
    }

    numRead = 0;
    while (numRead < n) {
        int startidx = numRead;
        int endidx = numRead + chunksize - 1;
        if (endidx >= n) {
            endidx = n - 1;
        }

        vectors = state.getMarkerParticleVelocities(startidx, endidx);
        for (unsigned int i = 0; i < vectors.size(); i++) {
            _markerParticles[startidx + i].velocity = vectors[i];
        }

        numRead += vectors.size();
    }

}

void FluidSimulation::_initializeDiffuseParticlesFromSaveState(
                                        FluidSimulationSaveState &state) {
    int n = state.getNumDiffuseParticles();
    _diffuseParticles.reserve(n);

    int chunksize = _loadStateReadChunkSize;
    int numRead = 0;

    std::vector<vmath::vec3> vectors;
    while (numRead < n) {
        int startidx = numRead;
        int endidx = numRead + chunksize - 1;
        if (endidx >= n) {
            endidx = n - 1;
        }

        vectors = state.getDiffuseParticlePositions(startidx, endidx);
        for (unsigned int i = 0; i < vectors.size(); i++) {
            _diffuseParticles.push_back(DiffuseParticle());
            _diffuseParticles[startidx + i].position = vectors[i];
        }

        numRead += vectors.size();
    }

    numRead = 0;
    while (numRead < n) {
        int startidx = numRead;
        int endidx = numRead + chunksize - 1;
        if (endidx >= n) {
            endidx = n - 1;
        }

        vectors = state.getDiffuseParticleVelocities(startidx, endidx);
        for (unsigned int i = 0; i < vectors.size(); i++) {
            _diffuseParticles[startidx + i].velocity = vectors[i];
        }

        numRead += vectors.size();
    }
    vectors.clear();
    vectors.shrink_to_fit();

    numRead = 0;
    std::vector<float> lifetimes;
    while (numRead < n) {
        int startidx = numRead;
        int endidx = numRead + chunksize - 1;
        if (endidx >= n) {
            endidx = n - 1;
        }

        lifetimes = state.getDiffuseParticleLifetimes(startidx, endidx);
        for (unsigned int i = 0; i < lifetimes.size(); i++) {
            _diffuseParticles[startidx + i].lifetime = lifetimes[i];
        }

        numRead += lifetimes.size();
    }

    _isDiffuseParticleTypesInitialized = false;
}

void FluidSimulation::_initializeDiffuseParticleTypes() {
    if (_isDiffuseParticleTypesInitialized) {
        return;
    }

    _updateDiffuseParticleTypes();
    _isDiffuseParticleTypesInitialized = true;
}

void FluidSimulation::_initializeSolidCellsFromSaveState(FluidSimulationSaveState &state) {
    int n = state.getNumSolidCells();
    int chunksize = _loadStateReadChunkSize;
    int numRead = 0;

    std::vector<GridIndex> indices;
    while (numRead < n) {
        int startidx = numRead;
        int endidx = numRead + chunksize - 1;
        if (endidx >= n) {
            endidx = n - 1;
        }

        indices = state.getSolidCells(startidx, endidx);
        for (unsigned int i = 0; i < indices.size(); i++) {
            addSolidCell(indices[i]);
        }

        numRead += indices.size();
    }
}

void FluidSimulation::_initializeSimulationFromSaveState(FluidSimulationSaveState &state) {
    state.getGridDimensions(&_isize, &_jsize, &_ksize);
    _dx = state.getCellSize();
    _currentFrame = state.getCurrentFrame();
    _currentBrickMeshFrame = _currentFrame;

    _MACVelocity = MACVelocityField(_isize, _jsize, _ksize, _dx);
    _materialGrid = FluidMaterialGrid(_isize, _jsize, _ksize);
    _levelset = LevelSet(_isize, _jsize, _ksize, _dx);
    _markerParticleRadius = pow(3*(_dx*_dx*_dx / 8.0) / (4*3.141592653), 1.0/3.0);
    _fluidCellIndices = GridIndexVector(_isize, _jsize, _ksize);
    _addedFluidCellQueue = GridIndexVector(_isize, _jsize, _ksize);

    _initializeSolidCellsFromSaveState(state);
    _initializeMarkerParticlesFromSaveState(state);
    _initializeDiffuseParticlesFromSaveState(state);
    _initializeFluidMaterialParticlesFromSaveState();

    _isFluidInSimulation = _fluidCellIndices.size() > 0;
    _isSimulationInitialized = true;
}

void FluidSimulation::_initializeParticleAdvector() {
    assert(_particleAdvector.initialize());
}

/********************************************************************************
    UPDATE FLUID CELLS
********************************************************************************/

int FluidSimulation::_getUniqueFluidSourceID() {
    int id = _uniqueFluidSourceID;
    _uniqueFluidSourceID++;
    return id;
}

void FluidSimulation::_removeMarkerParticlesFromCells(GridIndexVector &cells) {
    std::vector<bool> isRemoved;
    isRemoved.reserve(_markerParticles.size());

    MarkerParticle p;
    GridIndex g;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        p = _markerParticles[i];
        g = Grid3d::positionToGridIndex(p.position, _dx);
        isRemoved.push_back(_isIndexInList(g, cells));
    }

    _removeItemsFromVector(_markerParticles, isRemoved);
}

void FluidSimulation::_removeDiffuseParticlesFromCells(GridIndexVector &cells) {
    std::vector<bool> isRemoved;
    isRemoved.reserve(_diffuseParticles.size());

    DiffuseParticle p;
    GridIndex g;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        p = _diffuseParticles[i];
        g = Grid3d::positionToGridIndex(p.position, _dx);
        isRemoved.push_back(_isIndexInList(g, cells));
    }

    _removeItemsFromVector(_diffuseParticles, isRemoved);
}

void FluidSimulation::_addNewFluidCells(GridIndexVector &cells, 
                                        vmath::vec3 velocity) {

    _markerParticles.reserve(_markerParticles.size() + 8*cells.size());
    for (unsigned int i = 0; i < cells.size(); i++) {
        if (_materialGrid.isCellAir(cells[i])) {
            _addMarkerParticlesToCell(cells[i], velocity);
            _materialGrid.setFluid(cells[i]);
        }
    }
}

void FluidSimulation::_addNewFluidParticles(std::vector<vmath::vec3> &particles, vmath::vec3 velocity) {
    _markerParticles.reserve(_markerParticles.size() + particles.size());
    GridIndex g;
    for (unsigned int i = 0; i < particles.size(); i++) {
        g = Grid3d::positionToGridIndex(particles[i], _dx);
        _addMarkerParticle(particles[i], velocity);
        _materialGrid.setFluid(g);
    }
}

void FluidSimulation::_getNewFluidParticles(FluidSource *source, std::vector<vmath::vec3> &particles) {
    AABB bbox = source->getAABB();
    bbox = Grid3d::fitAABBtoGrid(bbox, _dx, _isize, _jsize, _ksize);

    GridIndex gmin, gmax;
    Grid3d::getGridIndexBounds(bbox, _dx, _isize, _jsize, _ksize, &gmin, &gmax);
    int iwidth = gmax.i - gmin.i + 1;
    int jheight = gmax.j - gmin.j + 1;
    int kdepth = gmax.k - gmin.k + 1;
    
    Array3d<bool> isInvalidCell = Array3d<bool>(iwidth, jheight, kdepth, true);
    GridIndexVector sourceCells = source->getCells(_materialGrid, _dx);
    GridIndex g;
    for (unsigned int i = 0; i < sourceCells.size(); i++) {
        g = sourceCells[i];
        g = GridIndex(g.i - gmin.i, g.j - gmin.j, g.k - gmin.k);
        isInvalidCell.set(g, false);
    }

    GridIndex subgridOffsets[8] = {
        GridIndex(0, 0, 0), GridIndex(0, 0, 1), GridIndex(0, 1, 0), GridIndex(0, 1, 1),
        GridIndex(1, 0, 0), GridIndex(1, 0, 1), GridIndex(1, 1, 0), GridIndex(1, 1, 1)
    };

    Array3d<bool> newParticleGrid = Array3d<bool>(2*iwidth, 2*jheight, 2*kdepth, true);
    GridIndex subg;
    for (int k = 0; k < isInvalidCell.depth; k++) {
        for (int j = 0; j < isInvalidCell.height; j++) {
            for (int i = 0; i < isInvalidCell.width; i++) {
                if (isInvalidCell(i, j, k)) {
                    for (int idx = 0; idx < 8; idx++) {
                        subg = subgridOffsets[idx];
                        newParticleGrid.set(2*i + subg.i, 2*j + subg.j, 2*k + subg.k, false);
                    }
                }
            }
        }
    }

    vmath::vec3 offset = Grid3d::GridIndexToPosition(gmin, _dx);
    vmath::vec3 p;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        if (bbox.isPointInside(_markerParticles[i].position)) {
            p = _markerParticles[i].position - offset;
            subg = Grid3d::positionToGridIndex(p, 0.5*_dx);
            newParticleGrid.set(subg, false);
        }
    }

    double eps = 10e-6;
    double jitter = 0.25*_dx - eps;
    vmath::vec3 jit;
    for (int k = 0; k < newParticleGrid.depth; k++) {
        for (int j = 0; j < newParticleGrid.height; j++) {
            for (int i = 0; i < newParticleGrid.width; i++) {
                if (newParticleGrid(i, j, k)) {
                    jit = vmath::vec3(_randomDouble(-jitter, jitter),
                                    _randomDouble(-jitter, jitter),
                                    _randomDouble(-jitter, jitter));

                    p = Grid3d::GridIndexToCellCenter(i, j, k, 0.5*_dx);
                    particles.push_back(p + offset + jit);
                }
            }
        }
    }
}

void FluidSimulation::_updateFluidSource(FluidSource *source) {
    if (source->isInflow()) {
        GridIndexVector newCells = source->getNewFluidCells(_materialGrid, _dx);
        vmath::vec3 velocity = source->getVelocity();
        if (newCells.size() > 0) {
            _addNewFluidCells(newCells, velocity);
        }

        std::vector<vmath::vec3> newParticles;
        _getNewFluidParticles(source, newParticles);
        if (newParticles.size() > 0) {
            _addNewFluidParticles(newParticles, velocity);
        }
    } else if (source->isOutflow()) {
        GridIndexVector cells = source->getFluidCells(_materialGrid, _dx);

        if (cells.size() > 0) {
            _removeMarkerParticlesFromCells(cells);
            _removeDiffuseParticlesFromCells(cells);
        }
    }
}

void FluidSimulation::_updateFluidSources() {
    for (unsigned int i = 0; i < _fluidSources.size(); i++) {
        _updateFluidSource(_fluidSources[i]);
    }
}

void FluidSimulation::_updateAddedFluidCellQueue() {
    vmath::vec3 velocity;
    _addNewFluidCells(_addedFluidCellQueue, velocity);
    _addedFluidCellQueue.clear();
    _addedFluidCellQueue.shrink_to_fit();
}

void FluidSimulation::_updateFluidCells() {
    _updateAddedFluidCellQueue();
    _updateFluidSources();

    _materialGrid.setAir(_fluidCellIndices);
    _fluidCellIndices.clear();
    
    MarkerParticle p;
    GridIndex g;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        p = _markerParticles[i];
        g = Grid3d::positionToGridIndex(p.position, _dx);
        assert(!_materialGrid.isCellSolid(g));
        _materialGrid.setFluid(g);
    }

    int count = 0;
    for (int k = 0; k < _materialGrid.depth; k++) {
        for (int j = 0; j < _materialGrid.height; j++) {
            for (int i = 0; i < _materialGrid.width; i++) {
                if (_materialGrid.isCellFluid(i, j, k)) {
                    count++;
                }
            }
        }
    }

    _fluidCellIndices.reserve(count);
    for (int k = 0; k < _materialGrid.depth; k++) {
        for (int j = 0; j < _materialGrid.height; j++) {
            for (int i = 0; i < _materialGrid.width; i++) {
                if (_materialGrid.isCellFluid(i, j, k)) {
                    _fluidCellIndices.push_back(i, j, k);
                }
            }
        }
    }

}

/********************************************************************************
    FLUID SURFACE RECONSTRUCTION
********************************************************************************/

TriangleMesh FluidSimulation::_polygonizeSurface() {
    IsotropicParticleMesher mesher(_isize, _jsize, _ksize, _dx);

    double r = _markerParticleRadius*_markerParticleScale;
    return mesher.meshParticles(_markerParticles, _materialGrid, r);
}

void FluidSimulation::_reconstructFluidSurface() {
    _surfaceMesh = _polygonizeSurface();
    _surfaceMesh.removeMinimumTriangleCountPolyhedra(
                        _minimumSurfacePolyhedronTriangleCount);
}

/********************************************************************************
    UPDATE LEVEL SET
********************************************************************************/

void FluidSimulation::_updateLevelSetSignedDistance() {
    _levelset.setSurfaceMesh(_surfaceMesh);
    int numLayers = 12;
    _levelset.calculateSignedDistanceField(numLayers);
}

/********************************************************************************
    RECONSTRUCT OUTPUT FLUID SURFACE
********************************************************************************/

void FluidSimulation::_writeDiffuseMaterialToFile(std::string bubblefile,
                                                  std::string foamfile,
                                                  std::string sprayfile) {
    double eps = 1e-1;
    TriangleMesh bubbleMesh;
    TriangleMesh foamMesh;
    TriangleMesh sprayMesh;
    DiffuseParticle dp;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        dp = _diffuseParticles[i];

        if (_isVertexNearSolid(dp.position, eps)) {
            continue;
        }

        if (dp.type == DP_BUBBLE && _isBubbleDiffuseMaterialEnabled) {
            bubbleMesh.vertices.push_back(dp.position);
        } else if (dp.type == DP_FOAM && _isFoamDiffuseMaterialEnabled) {
            foamMesh.vertices.push_back(dp.position);
        } else if (dp.type == DP_SPRAY && _isSprayDiffuseMaterialEnabled) {
            sprayMesh.vertices.push_back(dp.position);
        }
    }

    if (_isBubbleDiffuseMaterialEnabled) {
        bubbleMesh.writeMeshToPLY(bubblefile);
    }
    if (_isFoamDiffuseMaterialEnabled) {
        foamMesh.writeMeshToPLY(foamfile);
    }
    if (_isSprayDiffuseMaterialEnabled) {
        sprayMesh.writeMeshToPLY(sprayfile);
    }
}

void FluidSimulation::_writeDiffuseMaterialToFile(std::string diffusefile) {
    double eps = 1e-1;
    TriangleMesh diffuseMesh;
    DiffuseParticle dp;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        dp = _diffuseParticles[i];

        if (_isVertexNearSolid(dp.position, eps)) {
            continue;
        }

        if (dp.type == DP_BUBBLE && _isBubbleDiffuseMaterialEnabled) {
            diffuseMesh.vertices.push_back(dp.position);
        } else if (dp.type == DP_FOAM && _isFoamDiffuseMaterialEnabled) {
            diffuseMesh.vertices.push_back(dp.position);
        } else if (dp.type == DP_SPRAY && _isSprayDiffuseMaterialEnabled) {
            diffuseMesh.vertices.push_back(dp.position);
        }
    }
    diffuseMesh.writeMeshToPLY(diffusefile);
}

void FluidSimulation::_writeBrickColorListToFile(TriangleMesh &mesh, 
                                                     std::string filename) {
    int binsize = 3*sizeof(int)*mesh.vertexcolors.size();
    char *storage = new char[binsize];

    int *colordata = new int[3*mesh.vertexcolors.size()];
    vmath::vec3 c;
    for (unsigned int i = 0; i < mesh.vertexcolors.size(); i++) {
        c = mesh.vertexcolors[i];
        colordata[3*i] = (int)(c.x*255.0);
        colordata[3*i + 1] = (int)(c.y*255.0);
        colordata[3*i + 2] = (int)(c.z*255.0);
    }
    memcpy(storage, colordata, 3*sizeof(int)*mesh.vertexcolors.size());
    delete[] colordata;
    
    std::ofstream erasefile;
    erasefile.open(filename, std::ofstream::out | std::ofstream::trunc);
    erasefile.close();

    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    file.write(storage, binsize);
    file.close();

    delete[] storage;
}

void FluidSimulation::_writeBrickTextureToFile(TriangleMesh &mesh, 
                                               std::string filename) {

    int bisize, bjsize, bksize;
    _fluidBrickGrid.getBrickGridDimensions(&bisize, &bjsize, &bksize);

    double bx = _fluidBrickGrid.getBrickAABB().width;
    double by = _fluidBrickGrid.getBrickAABB().height;
    double bz = _fluidBrickGrid.getBrickAABB().depth;

    Array3d<unsigned char> colorGrid(bisize, bjsize, bksize, (char)0);

    vmath::vec3 p, c;
    for (unsigned int i = 0; i < mesh.vertices.size(); i++) {
        p = mesh.vertices[i];
        c = mesh.vertexcolors[i];

        int bi = (int)(p.x / bx);
        int bj = (int)(p.y / by);
        int bk = (int)(p.z / bz);

        int coloridx = (int)(c.x*255);
        colorGrid.set(bi, bj, bk, (unsigned char)coloridx);
    }
    
    int binsize = sizeof(unsigned char)*bisize*bjsize*bksize;
    unsigned char *storage = new unsigned char[binsize];

    int offset = 0;
    for (int k = 0; k < colorGrid.depth; k++) {
        for (int j = 0; j < colorGrid.height; j++) {
            for (int i = 0; i < colorGrid.width; i++) {
                storage[offset] = colorGrid(i, j, k);
                offset++;
            }
        }
    }
    
    std::ofstream erasefile;
    erasefile.open(filename, std::ofstream::out | std::ofstream::trunc);
    erasefile.close();

    std::ofstream file(filename.c_str(), std::ios::out | std::ios::binary);
    file.write((char*)storage, binsize);
    file.close();

    delete[] storage;
}

void FluidSimulation::_writeBrickMaterialToFile(std::string brickfile,
                                                std::string colorfile,
                                                std::string texturefile) {
    TriangleMesh brickmesh;
    _fluidBrickGrid.getBrickMesh(_levelset, brickmesh);

    brickmesh.writeMeshToPLY(brickfile);
    _writeBrickColorListToFile(brickmesh, colorfile);
    _writeBrickTextureToFile(brickmesh, texturefile);
}

std::string FluidSimulation::_numberToString(int number) {
    std::ostringstream ss;
    ss << number;
    return ss.str();
}

void FluidSimulation::_writeSurfaceMeshToFile(TriangleMesh &isomesh,
                                              TriangleMesh &anisomesh) {

    std::string currentFrame = _numberToString(_currentFrame);
    currentFrame.insert(currentFrame.begin(), 6 - currentFrame.size(), '0');

    if (_isSurfaceMeshOutputEnabled) {
        if (_isIsotropicSurfaceMeshReconstructionEnabled) {
            isomesh.writeMeshToPLY("bakefiles/" + currentFrame + ".ply");
        }

        if (_isAnisotropicSurfaceMeshReconstructionEnabled) {
            anisomesh.writeMeshToPLY("bakefiles/anisotropic" + currentFrame + ".ply");
        }
    }

    if (_isDiffuseMaterialOutputEnabled) {
        if (!_isDiffuseParticleTypesInitialized) {
            _initializeDiffuseParticleTypes();
        }

        if (_isDiffuseMaterialFilesSeparated) {
            _writeDiffuseMaterialToFile("bakefiles/bubble" + currentFrame + ".ply",
                                        "bakefiles/foam" + currentFrame + ".ply",
                                        "bakefiles/spray" + currentFrame + ".ply");
        } else {
            _writeDiffuseMaterialToFile("bakefiles/diffuse" + currentFrame + ".ply");
        }
    }

    if (_isBrickOutputEnabled && _fluidBrickGrid.isBrickMeshReady()) {
        std::string currentBrickMeshFrame = _numberToString(_currentBrickMeshFrame);
        currentBrickMeshFrame.insert(currentBrickMeshFrame.begin(), 6 - currentBrickMeshFrame.size(), '0');

        _writeBrickMaterialToFile("bakefiles/brick" + currentBrickMeshFrame + ".ply", 
                                  "bakefiles/brickcolor" + currentBrickMeshFrame + ".data",
                                  "bakefiles/bricktexture" + currentBrickMeshFrame + ".data");
        _currentBrickMeshFrame++;
    }
}

bool FluidSimulation::_isVertexNearSolid(vmath::vec3 v, double eps) {
    GridIndex g = Grid3d::positionToGridIndex(v, _dx);
    if (_materialGrid.isCellSolid(g)) {
        return true;
    }
    
    // is v near the solid boundary?
    vmath::vec3 e = vmath::vec3(eps, eps, eps);
    if (g.i == 1 || g.i == _isize - 2 || 
        g.j == 1 || g.j == _jsize - 2 || 
        g.k == 1 || g.k == _ksize - 2) {

        vmath::vec3 min = Grid3d::GridIndexToPosition(1, 1, 1, _dx);
        vmath::vec3 max = Grid3d::GridIndexToPosition(_isize - 2, 
                                                    _jsize - 2, 
                                                    _ksize - 2, _dx);
        AABB bbox = AABB(min + e, max + vmath::vec3(_dx, _dx, _dx) - e);
        if (!bbox.isPointInside(v)) {
            return true;
        }
    }

    // is v near a solid cell?
    vmath::vec3 gp = Grid3d::GridIndexToPosition(g, _dx);
    AABB bbox = AABB(gp + e, gp + vmath::vec3(_dx, _dx, _dx) - e);
    if (bbox.isPointInside(v)) {
        return false;
    }

    vmath::vec3 ex(eps, 0.0, 0.0);
    vmath::vec3 ey(0.0, eps, 0.0);
    vmath::vec3 ez(0.0, 0.0, eps);

    vmath::vec3 points[26] {
        v-ex, v+ex, v-ey, v+ey, v-ez, v+ez, v-ex-ey, v-ex+ey,
        v+ex-ey, v+ex+ey, v-ex-ez, v-ex+ez, v+ex-ez, v+ex+ez, 
        v-ey-ez, v-ey+ez, v+ey-ez, v+ey+ez, v-ex-ey-ez,
        v-ex-ey+ez, v-ex+ey-ez, v-ex+ey+ez, v+ex-ey-ez,
        v+ex-ey+ez, v+ex+ey-ez, v+ex+ey+ez};
    
    int i, j, k;
    for (int idx = 0; idx < 26; idx++) {
        Grid3d::positionToGridIndex(points[idx], _dx, &i, &j, &k);
        if (_materialGrid.isCellSolid(i, j, k)) {
            return true;
        }
    }

    return false;
}

void FluidSimulation::_getSmoothVertices(TriangleMesh &mesh,
                                         std::vector<int> &smoothVertices) {
    double eps = 0.02*_dx;
    vmath::vec3 v;
    for (unsigned int i = 0; i < mesh.vertices.size(); i++) {
        v = mesh.vertices[i];
        if (!_isVertexNearSolid(v, eps)) {
            smoothVertices.push_back(i);
        }
    }
}

void FluidSimulation::_smoothSurfaceMesh(TriangleMesh &mesh) {
    std::vector<int> smoothVertices;
    _getSmoothVertices(mesh, smoothVertices);

    mesh.smooth(_surfaceReconstructionSmoothingValue, 
                _surfaceReconstructionSmoothingIterations,
                smoothVertices);
}

TriangleMesh FluidSimulation::_polygonizeIsotropicOutputSurface() {
    int slices = _numSurfaceReconstructionPolygonizerSlices;
    double r = _markerParticleRadius*_markerParticleScale;

    IsotropicParticleMesher mesher(_isize, _jsize, _ksize, _dx);
    mesher.setSubdivisionLevel(_outputFluidSurfaceSubdivisionLevel);
    mesher.setNumPolygonizationSlices(slices);

    return mesher.meshParticles(_markerParticles, _materialGrid, r);
}

TriangleMesh FluidSimulation::_polygonizeAnisotropicOutputSurface() {
    int slices = _numSurfaceReconstructionPolygonizerSlices;
    double r = _markerParticleRadius;

    AnisotropicParticleMesher mesher(_isize, _jsize, _ksize, _dx);
    mesher.setSubdivisionLevel(_outputFluidSurfaceSubdivisionLevel);
    mesher.setNumPolygonizationSlices(slices);

    return mesher.meshParticles(_markerParticles, _levelset, _materialGrid, r);
}

void FluidSimulation::_updateBrickGrid(double dt) {
    std::vector<vmath::vec3> points;
    points.reserve(_markerParticles.size());
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        points.push_back(_markerParticles[i].position);
    }

    _fluidBrickGrid.update(_levelset, points, dt);
}

void FluidSimulation::_reconstructOutputFluidSurface(double dt) {
    
    TriangleMesh isomesh, anisomesh;
    if (_isSurfaceMeshOutputEnabled) {
        if (_isIsotropicSurfaceMeshReconstructionEnabled) {
            if (_outputFluidSurfaceSubdivisionLevel == 1) {
                isomesh = _surfaceMesh;
            } else {
                isomesh = _polygonizeIsotropicOutputSurface();
                isomesh.removeMinimumTriangleCountPolyhedra(
                            _minimumSurfacePolyhedronTriangleCount);
            }
            _smoothSurfaceMesh(isomesh);
        }

        if (_isAnisotropicSurfaceMeshReconstructionEnabled) {
            anisomesh = _polygonizeAnisotropicOutputSurface();
            anisomesh.removeMinimumTriangleCountPolyhedra(
                            _minimumSurfacePolyhedronTriangleCount);
            _smoothSurfaceMesh(anisomesh);
        }
    }

    if (_isBrickOutputEnabled) {
        _updateBrickGrid(dt);
    }

    _writeSurfaceMeshToFile(isomesh, anisomesh);
}

/********************************************************************************
    ADVECT FLUID VELOCITIES
********************************************************************************/

void FluidSimulation::_applyFluidSourceToVelocityField(FluidSource *source,
                                                       int dir,
                                                       Array3d<bool> &isValueSet,
                                                       Array3d<float> &field) {
    int U = 0; int V = 1; int W = 2;

    vmath::vec3 (*getFacePosition)(int, int, int, double);
    if (dir == U) {
        getFacePosition = &Grid3d::FaceIndexToPositionU;
    } else if (dir == V) {
        getFacePosition = &Grid3d::FaceIndexToPositionV;
    } else if (dir == W) {
        getFacePosition = &Grid3d::FaceIndexToPositionW;
    } else {
        return;
    }

    double speed = source->getVelocity()[dir];
    vmath::vec3 p;
    for (int k = 0; k < field.depth; k++) {
        for (int j = 0; j < field.height; j++) {
            for (int i = 0; i < field.width; i++) {

                if (!isValueSet(i, j, k)) {
                    continue;
                }

                p = getFacePosition(i, j, k, _dx);
                if (source->containsPoint(p)) {
                    field.set(i, j, k, speed);
                }
            }
        }
    }

}

void FluidSimulation::_computeVelocityScalarField(Array3d<float> &field, 
                                                  Array3d<bool> &isValueSet,
                                                  int dir) {
    int U = 0; int V = 1; int W = 2;

    ImplicitSurfaceScalarField grid = ImplicitSurfaceScalarField(field.width,
                                                                 field.height,
                                                                 field.depth, _dx);

    grid.enableWeightField();
    grid.setPointRadius(_dx); 

    vmath::vec3 offset;
    if (dir == U) {
        offset = vmath::vec3(0.0, 0.5*_dx, 0.5*_dx);
    } else if (dir == V) {
        offset = vmath::vec3(0.5*_dx, 0.0, 0.5*_dx);
    } else if (dir == W) {
        offset = vmath::vec3(0.5*_dx, 0.5*_dx, 0.0);
    } else {
        return;
    }

    MarkerParticle p;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        p = _markerParticles[i];
        grid.addPointValue(p.position - offset, p.velocity[dir]);
    }
    grid.applyWeightField();

    grid.getScalarField(field);
    grid.getSetScalarFieldValues(isValueSet);

    FluidSource *source;
    for (unsigned int i = 0; i < _fluidSources.size(); i++) {
        source = _fluidSources[i];
        if (source->isInflow() && source->isActive()) {
            _applyFluidSourceToVelocityField(source, dir, isValueSet, field);
        }
    }
}

void FluidSimulation::_advectVelocityFieldU() {
    _MACVelocity.clearU();

    Array3d<float> ugrid = Array3d<float>(_isize + 1, _jsize, _ksize, 0.0f);
    Array3d<bool> isValueSet = Array3d<bool>(_isize + 1, _jsize, _ksize, false);
    _computeVelocityScalarField(ugrid, isValueSet, 0);

    GridIndexVector extrapolationIndices(_isize + 1, _jsize, _ksize);
    for (int k = 0; k < ugrid.depth; k++) {
        for (int j = 0; j < ugrid.height; j++) {
            for (int i = 0; i < ugrid.width; i++) {
                if (_materialGrid.isFaceBorderingFluidU(i, j, k)) {
                    if (!isValueSet(i, j, k)) {
                        extrapolationIndices.push_back(i, j, k);
                    } else {
                        _MACVelocity.setU(i, j, k, ugrid(i, j, k));
                    }
                }
            }
        }
    }

    GridIndex g, n;
    GridIndex nb[26];
    double avg, weight;
    for (unsigned int i = 0; i < extrapolationIndices.size(); i++) {
        g = extrapolationIndices[i];
        Grid3d::getNeighbourGridIndices26(g, nb);

        avg = 0.0;
        weight = 0.0;
        for (int idx = 0; idx < 26; idx++) {
            n = nb[idx];
            if (ugrid.isIndexInRange(n) && fabs(ugrid(n)) > 0.0) {
                avg += ugrid(n);
                weight += 1.0;
            }
        }

        if (weight > 0.0) {
            _MACVelocity.setU(g, avg / weight);
        }
    }
}

void FluidSimulation::_advectVelocityFieldV() {
    _MACVelocity.clearV();

    Array3d<float> vgrid = Array3d<float>(_isize, _jsize + 1, _ksize, 0.0f);
    Array3d<bool> isValueSet = Array3d<bool>(_isize, _jsize + 1, _ksize, false);
    _computeVelocityScalarField(vgrid, isValueSet, 1);
    
    GridIndexVector extrapolationIndices(_isize, _jsize + 1, _ksize);
    for (int k = 0; k < vgrid.depth; k++) {
        for (int j = 0; j < vgrid.height; j++) {
            for (int i = 0; i < vgrid.width; i++) {
                if (_materialGrid.isFaceBorderingFluidV(i, j, k)) {
                    if (!isValueSet(i, j, k)) {
                        extrapolationIndices.push_back(i, j, k);
                    } else {
                        _MACVelocity.setV(i, j, k, vgrid(i, j, k));
                    }
                }
            }
        }
    }

    GridIndex g, n;
    GridIndex nb[26];
    double avg, weight;
    for (unsigned int i = 0; i < extrapolationIndices.size(); i++) {
        g = extrapolationIndices[i];
        Grid3d::getNeighbourGridIndices26(g, nb);

        avg = 0.0;
        weight = 0.0;
        for (int idx = 0; idx < 26; idx++) {
            n = nb[idx];
            if (vgrid.isIndexInRange(n) && isValueSet(n)) {
                avg += vgrid(n);
                weight += 1.0;
            }
        }

        if (weight > 0.0) {
            _MACVelocity.setV(g, avg / weight);
        }
    }
}

void FluidSimulation::_advectVelocityFieldW() {
    _MACVelocity.clearW();

    Array3d<float> wgrid = Array3d<float>(_isize, _jsize, _ksize + 1, 0.0f);
    Array3d<bool> isValueSet = Array3d<bool>(_isize, _jsize, _ksize + 1, 0.0f);
    _computeVelocityScalarField(wgrid, isValueSet, 2);
    
    GridIndexVector extrapolationIndices(_isize, _jsize, _ksize + 1);
    for (int k = 0; k < wgrid.depth; k++) {
        for (int j = 0; j < wgrid.height; j++) {
            for (int i = 0; i < wgrid.width; i++) {
                if (_materialGrid.isFaceBorderingFluidW(i, j, k)) {
                    if (!isValueSet(i, j, k)) {
                        extrapolationIndices.push_back(i, j, k);
                    } else {
                        _MACVelocity.setW(i, j, k, wgrid(i, j, k));
                    }
                }
            }
        }
    }

    GridIndex g, n;
    GridIndex nb[26];
    double avg, weight;
    for (unsigned int i = 0; i < extrapolationIndices.size(); i++) {
        g = extrapolationIndices[i];
        Grid3d::getNeighbourGridIndices26(g, nb);

        avg = 0.0;
        weight = 0.0;
        for (int idx = 0; idx < 26; idx++) {
            n = nb[idx];
            if (wgrid.isIndexInRange(n) && isValueSet(n)) {
                avg += wgrid(n);
                weight += 1.0;
            }
        }

        if (weight > 0.0) {
            _MACVelocity.setW(g, avg / weight);
        }
    }
}

void FluidSimulation::_advectVelocityField() {
    _advectVelocityFieldU();
    _advectVelocityFieldV();
    _advectVelocityFieldW();
}

/********************************************************************************
    APPLY BODY FORCES
********************************************************************************/

void FluidSimulation::_applyConstantBodyForces(double dt) {

    vmath::vec3 bodyForce;
    for (unsigned int i = 0; i < _constantBodyForces.size(); i++) {
        bodyForce += _constantBodyForces[i];
    }

    if (fabs(bodyForce.x) > 0.0) {
        for (int k = 0; k < _ksize; k++) {
            for (int j = 0; j < _jsize; j++) {
                for (int i = 0; i < _isize + 1; i++) {
                    if (_materialGrid.isFaceBorderingFluidU(i, j, k)) {
                        _MACVelocity.addU(i, j, k, bodyForce.x * dt);
                    }
                }
            }
        }
    }

    if (fabs(bodyForce.y) > 0.0) {
        for (int k = 0; k < _ksize; k++) {
            for (int j = 0; j < _jsize + 1; j++) {
                for (int i = 0; i < _isize; i++) {
                    if (_materialGrid.isFaceBorderingFluidV(i, j, k)) {
                        _MACVelocity.addV(i, j, k, bodyForce.y * dt);
                    }
                }
            }
        }
    }

    if (fabs(bodyForce.z) > 0.0) {
        for (int k = 0; k < _ksize + 1; k++) {
            for (int j = 0; j < _jsize; j++) {
                for (int i = 0; i < _isize; i++) {
                    if (_materialGrid.isFaceBorderingFluidW(i, j, k)) {
                        _MACVelocity.addW(i, j, k, bodyForce.z * dt);
                    }
                }
            }
        }
    }
}

void FluidSimulation::_applyVariableBodyForce(vmath::vec3 (*fieldFunction)(vmath::vec3),
                                              double dt) {

    vmath::vec3 p;
    vmath::vec3 bodyForce;
    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize + 1; i++) {
                if (_materialGrid.isFaceBorderingFluidU(i, j, k)) {
                    p = Grid3d::FaceIndexToPositionU(i, j, k, _dx);
                    bodyForce = fieldFunction(p);
                    _MACVelocity.addU(i, j, k, bodyForce.x * dt);
                }
            }
        }
    }


    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize + 1; j++) {
            for (int i = 0; i < _isize; i++) {
                if (_materialGrid.isFaceBorderingFluidV(i, j, k)) {
                    p = Grid3d::FaceIndexToPositionV(i, j, k, _dx);
                    bodyForce = fieldFunction(p);
                    _MACVelocity.addV(i, j, k, bodyForce.y * dt);
                }
            }
        }
    }


    for (int k = 0; k < _ksize + 1; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize; i++) {
                if (_materialGrid.isFaceBorderingFluidW(i, j, k)) {
                    Grid3d::FaceIndexToPositionU(i, j, k, _dx);
                    bodyForce = fieldFunction(p);
                    _MACVelocity.addW(i, j, k, bodyForce.z * dt);
                }
            }
        }
    }

}

void FluidSimulation::_applyVariableBodyForces(double dt) {
    vmath::vec3 (*fieldFunction)(vmath::vec3);
    for (unsigned int i = 0; i < _variableBodyForces.size(); i++) {
        fieldFunction = _variableBodyForces[i];
        _applyVariableBodyForce(fieldFunction, dt);
    }
}

void FluidSimulation::_applyBodyForcesToVelocityField(double dt) {
    _applyConstantBodyForces(dt);
    _applyVariableBodyForces(dt);
}


/********************************************************************************
    UPDATE PRESSURE GRID
********************************************************************************/

void FluidSimulation::_updatePressureGrid(Array3d<float> &pressureGrid, double dt) {
    PressureSolverParameters params;
    params.cellwidth = _dx;
    params.density = _density;
    params.deltaTime = dt;
    params.fluidCells = &_fluidCellIndices;
    params.materialGrid = &_materialGrid;
    params.velocityField = &_MACVelocity;
    params.logfile = &_logfile;

    VectorXd pressures(_fluidCellIndices.size());
    PressureSolver solver;
    solver.solve(params, pressures);

    GridIndex g;
    for (unsigned int idx = 0; idx < _fluidCellIndices.size(); idx++) {
        g = _fluidCellIndices[idx];
        pressureGrid.set(g, (float)pressures[idx]);
    }
}

/********************************************************************************
    APPLY PRESSURE TO VELOCITY FIELD
********************************************************************************/

void FluidSimulation::_applyPressureToFaceU(int i, int j, int k, 
                                            Array3d<float> &pressureGrid,
                                            MACVelocityField &tempMACVelocity, double dt) {
    double usolid = 0.0;   // solids are stationary
    double scale = dt / (_density * _dx);
    double invscale = 1.0 / scale;

    int ci = i - 1; int cj = j; int ck = k;

    double p0, p1;
    if (!_materialGrid.isCellSolid(ci, cj, ck) && !_materialGrid.isCellSolid(ci + 1, cj, ck)) {
        p0 = pressureGrid(ci, cj, ck);
        p1 = pressureGrid(ci + 1, cj, ck);
    } else if (_materialGrid.isCellSolid(ci, cj, ck)) {
        p0 = pressureGrid(ci + 1, cj, ck) - 
                invscale*(_MACVelocity.U(i, j, k) - usolid);
        p1 = pressureGrid(ci + 1, cj, ck);
    } else {
        p0 = pressureGrid(ci, cj, ck);
        p1 = pressureGrid(ci, cj, ck) +
                invscale*(_MACVelocity.U(i, j, k) - usolid);
    }

    double unext = _MACVelocity.U(i, j, k) - scale*(p1 - p0);
    tempMACVelocity.setU(i, j, k, unext);
}

void FluidSimulation::_applyPressureToFaceV(int i, int j, int k, 
                                            Array3d<float> &pressureGrid,
                                            MACVelocityField &tempMACVelocity, double dt) {
    double usolid = 0.0;   // solids are stationary
    double scale = dt / (_density * _dx);
    double invscale = 1.0 / scale;

    int ci = i; int cj = j - 1; int ck = k;

    double p0, p1;
    if (!_materialGrid.isCellSolid(ci, cj, ck) && !_materialGrid.isCellSolid(ci, cj + 1, ck)) {
        p0 = pressureGrid(ci, cj, ck);
        p1 = pressureGrid(ci, cj + 1, ck);
    }
    else if (_materialGrid.isCellSolid(ci, cj, ck)) {
        p0 = pressureGrid(ci, cj + 1, ck) -
            invscale*(_MACVelocity.V(i, j, k) - usolid);
        p1 = pressureGrid(ci, cj + 1, ck);
    }
    else {
        p0 = pressureGrid(ci, cj, ck);
        p1 = pressureGrid(ci, cj, ck) +
            invscale*(_MACVelocity.V(i, j, k) - usolid);
    }

    double vnext = _MACVelocity.V(i, j, k) - scale*(p1 - p0);
    tempMACVelocity.setV(i, j, k, vnext);
}

void FluidSimulation::_applyPressureToFaceW(int i, int j, int k, 
                                            Array3d<float> &pressureGrid,
                                            MACVelocityField &tempMACVelocity, double dt) {
    double usolid = 0.0;   // solids are stationary
    double scale = dt / (_density * _dx);
    double invscale = 1.0 / scale;

    int ci = i; int cj = j; int ck = k - 1;

    double p0, p1;
    if (!_materialGrid.isCellSolid(ci, cj, ck) && !_materialGrid.isCellSolid(ci, cj, ck + 1)) {
        p0 = pressureGrid(ci, cj, ck);
        p1 = pressureGrid(ci, cj, ck + 1);
    }
    else if (_materialGrid.isCellSolid(ci, cj, ck)) {
        p0 = pressureGrid(ci, cj, ck + 1) -
                invscale*(_MACVelocity.W(i, j, k) - usolid);
        p1 = pressureGrid(ci, cj, ck + 1);
    }
    else {
        p0 = pressureGrid(ci, cj, ck);
        p1 = pressureGrid(ci, cj, ck) +
                invscale*(_MACVelocity.W(i, j, k) - usolid);
    }

    double wnext = _MACVelocity.W(i, j, k) - scale*(p1 - p0);
    tempMACVelocity.setW(i, j, k, wnext);
}

void FluidSimulation::_commitTemporaryVelocityFieldValues(MACVelocityField &tempMACVelocity) {
    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize + 1; i++) {
                if (_materialGrid.isFaceBorderingFluidU(i, j, k)) {
                    _MACVelocity.setU(i, j, k, tempMACVelocity.U(i, j, k));
                }
            }
        }
    }

    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize + 1; j++) {
            for (int i = 0; i < _isize; i++) {
                if (_materialGrid.isFaceBorderingFluidV(i, j, k)) {
                    _MACVelocity.setV(i, j, k, tempMACVelocity.V(i, j, k));
                }
            }
        }
    }

    for (int k = 0; k < _ksize + 1; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize; i++) {
                if (_materialGrid.isFaceBorderingFluidW(i, j, k)) {
                    _MACVelocity.setW(i, j, k, tempMACVelocity.W(i, j, k));
                }
            }
        }
    }
}

void FluidSimulation::_applyPressureToVelocityField(Array3d<float> &pressureGrid, double dt) {
    MACVelocityField tempMACVelocity = MACVelocityField(_isize, _jsize, _ksize, _dx);

    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize + 1; i++) {
                if (_materialGrid.isFaceBorderingSolidU(i, j, k)) {
                    tempMACVelocity.setU(i, j, k, 0.0);
                }

                if (_materialGrid.isFaceBorderingFluidU(i, j, k) &&
                    !_materialGrid.isFaceBorderingSolidU(i, j, k)) {
                    _applyPressureToFaceU(i, j, k, pressureGrid, tempMACVelocity, dt);
                }
            }
        }
    }

    for (int k = 0; k < _ksize; k++) {
        for (int j = 0; j < _jsize + 1; j++) {
            for (int i = 0; i < _isize; i++) {
                if (_materialGrid.isFaceBorderingSolidV(i, j, k)) {
                    tempMACVelocity.setV(i, j, k, 0.0);
                }

                if (_materialGrid.isFaceBorderingFluidV(i, j, k) &&
                    !_materialGrid.isFaceBorderingSolidV(i, j, k)) {
                    _applyPressureToFaceV(i, j, k, pressureGrid, tempMACVelocity, dt);
                }
            }
        }
    }

    for (int k = 0; k < _ksize + 1; k++) {
        for (int j = 0; j < _jsize; j++) {
            for (int i = 0; i < _isize; i++) {
                if (_materialGrid.isFaceBorderingSolidW(i, j, k)) {
                    tempMACVelocity.setW(i, j, k, 0.0);
                }

                if (_materialGrid.isFaceBorderingFluidW(i, j, k) &&
                    !_materialGrid.isFaceBorderingSolidW(i, j, k)) {
                    _applyPressureToFaceW(i, j, k, pressureGrid, tempMACVelocity, dt);
                }
            }
        }
    }

    _commitTemporaryVelocityFieldValues(tempMACVelocity);
}

/********************************************************************************
    EXTRAPOLATE FLUID VELOCITIES
********************************************************************************/

void FluidSimulation::_extrapolateFluidVelocities(MACVelocityField &MACGrid) {
    int numLayers = (int)ceil(_CFLConditionNumber + 2);
    MACGrid.extrapolateVelocityField(_materialGrid, numLayers);
}

/********************************************************************************
    UPDATE DIFFUSE MATERIAL PARTICLES
********************************************************************************/

void FluidSimulation::_getMinMaxMarkerParticleSpeeds(double *min, double *max) {
    vmath::vec3 v;
    double sq;
    double minsq = std::numeric_limits<double>::infinity();
    double maxsq = 0.0;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        v = _markerParticles[i].velocity;
        sq = vmath::dot(v, v);

        if (sq < minsq) {
            minsq = sq;
        } else if (sq > maxsq) {
            maxsq = sq;
        }
    }

    *min = sqrt(minsq);
    *max = sqrt(maxsq);
}

double FluidSimulation::_getVelocityUpperBoundByPercentile(double pct) {
    if (pct > 1.0) {
        pct /= 100;
    }

    assert(pct >= 0.0 && pct <= 1.0);

    
    int nbins = 1000000;
    std::vector<int> particleCounts = std::vector<int>(nbins, 0);

    double minspeed, maxspeed;
    _getMinMaxMarkerParticleSpeeds(&minspeed, &maxspeed);

    if (minspeed == maxspeed) {
        return maxspeed;
    }

    double binsize = (maxspeed - minspeed) / (double)nbins;
    vmath::vec3 v;
    double s;
    int binidx;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        v = _markerParticles[i].velocity;
        s = vmath::length(v);

        binidx = (int)floor(((s - minspeed) / (maxspeed - minspeed)) * (double)nbins);
        if (binidx >= nbins) {
            binidx = nbins - 1;
        } else if (binidx < 0) {
            binidx = 0;
        }

        particleCounts[binidx]++;
    }

    int maxcount = (int)floor(pct*_markerParticles.size());
    int count = 0;
    double percentilespeed;
    for (int i = 0; i < nbins; i++) {
        percentilespeed = minspeed + (double)i*binsize;
        count += particleCounts[i];
        if (count >= maxcount) {
            break;
        }
    }

    return percentilespeed;
}

void FluidSimulation::_sortMarkerParticlePositions(std::vector<vmath::vec3> &surface, 
                                                   std::vector<vmath::vec3> &inside) {

    // Speeds above max percentile of marker particle speeds are considered too fast to
    // generate diffuse particles. Generating diffuse particles with high speeds could cause
    // diffuse simulation to explode.
    double maxspeedsq = _getVelocityUpperBoundByPercentile(
                            _markerParticleVelocityUpperBoundPercentile);

    maxspeedsq *= maxspeedsq;

    vmath::vec3 p;
    double speedsq;
    double width = _diffuseSurfaceNarrowBandSize * _dx;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {

        speedsq = vmath::dot(_markerParticles[i].velocity, _markerParticles[i].velocity);
        if (speedsq > maxspeedsq) {
            continue;
        }

        p = _markerParticles[i].position;
        if (_levelset.getDistance(p) < width) {
            surface.push_back(p);
        } else if (_levelset.isPointInInsideCell(p)) {
            inside.push_back(p);
        }
    }

}

double FluidSimulation::_getWavecrestPotential(vmath::vec3 p, vmath::vec3 *v) {

    GridIndex g = Grid3d::positionToGridIndex(p, _dx);
    if (!_materialGrid.isCellAir(g) && !_materialGrid.isCellNeighbouringAir(g)) {
        return 0.0;
    }

    *v = _getVelocityAtPosition(p);
    vmath::vec3 normal;
    double k = _levelset.getSurfaceCurvature(p, &normal);

    if (vmath::dot(vmath::normalize(*v), normal) < 0.6) {
        return 0.0;
    }

    k = fmax(k, _minWavecrestCurvature);
    k = fmin(k, _maxWavecrestCurvature);

    return (k - _minWavecrestCurvature) / 
           (_maxWavecrestCurvature - _minWavecrestCurvature);
}

double FluidSimulation::_getTurbulencePotential(vmath::vec3 p, TurbulenceField &tfield) {

    GridIndex g = Grid3d::positionToGridIndex(p, _dx);
    if (!_materialGrid.isCellFluid(g) && !_materialGrid.isCellNeighbouringFluid(g)) {
        return 0.0;
    }

    double t = tfield.evaluateTurbulenceAtPosition(p);

    t = fmax(t, _minTurbulence);
    t = fmin(t, _maxTurbulence);

    return (t - _minTurbulence) / 
           (_maxTurbulence - _minTurbulence);
}

double FluidSimulation::_getEnergyPotential(vmath::vec3 velocity) {
    double e = 0.5*vmath::dot(velocity, velocity);
    e = fmax(e, _minParticleEnergy);
    e = fmin(e, _maxParticleEnergy);

    return (e - _minParticleEnergy) / (_maxParticleEnergy - _minParticleEnergy);
}

void FluidSimulation::_getSurfaceDiffuseParticleEmitters(
                            std::vector<vmath::vec3> &surface, 
                            std::vector<DiffuseParticleEmitter> &emitters) {
    vmath::vec3 p;
    for (unsigned int i = 0; i < surface.size(); i++) {
        p = surface[i];

        vmath::vec3 velocity;
        double Iwc = _getWavecrestPotential(p, &velocity);
        double It = 0.0;

        if (Iwc > 0.0 || It > 0.0) {
            double Ie = _getEnergyPotential(velocity);
            if (Ie > 0.0) {
                emitters.push_back(DiffuseParticleEmitter(p, velocity, Ie, Iwc, It));
            }
        }
    }
}

void FluidSimulation::_getInsideDiffuseParticleEmitters(
                            std::vector<vmath::vec3> &inside, 
                            std::vector<DiffuseParticleEmitter> &emitters) {
    vmath::vec3 p;
    for (unsigned int i = 0; i < inside.size(); i++) {
        p = inside[i];
        double It = _getTurbulencePotential(p, _turbulenceField);

        if (It > 0.0) {
            vmath::vec3 velocity = _getVelocityAtPosition(p);
            double Ie = _getEnergyPotential(velocity);
            if (Ie > 0.0) {
                emitters.push_back(DiffuseParticleEmitter(p, velocity, Ie, 0.0, It));
            }
        }
    }
}

void FluidSimulation::_shuffleDiffuseParticleEmitters(std::vector<DiffuseParticleEmitter> &emitters) {
    DiffuseParticleEmitter em;
    for (int i = emitters.size() - 2; i >= 0; i--) {
        int j = (rand() % (int)(i - 0 + 1));
        em = emitters[i];
        emitters[i] = emitters[j];
        emitters[j] = em;
    }
}

void FluidSimulation::_getDiffuseParticleEmitters(std::vector<DiffuseParticleEmitter> &emitters) {

    _levelset.calculateSurfaceCurvature();
    _turbulenceField.calculateTurbulenceField(&_MACVelocity, _fluidCellIndices);

    std::vector<vmath::vec3> surfaceParticles;
    std::vector<vmath::vec3> insideParticles;
    _sortMarkerParticlePositions(surfaceParticles, insideParticles);
    _getSurfaceDiffuseParticleEmitters(surfaceParticles, emitters);
    _getInsideDiffuseParticleEmitters(insideParticles, emitters);
    _shuffleDiffuseParticleEmitters(emitters);
}

int FluidSimulation::_getNumberOfEmissionParticles(DiffuseParticleEmitter &emitter,
                                                   double dt) {
    double wc = _wavecrestEmissionRate*emitter.wavecrestPotential;
    double t = _turbulenceEmissionRate*emitter.turbulencePotential;
    double n = emitter.energyPotential*(wc + t)*dt;

    if (n < 0.0) {
        return 0;
    }

    return (int)(n + 0.5);
}

void FluidSimulation::_emitDiffuseParticles(DiffuseParticleEmitter &emitter, double dt) {
    int n = _getNumberOfEmissionParticles(emitter, dt);

    if (_diffuseParticles.size() + n >= _maxNumDiffuseParticles) {
        n = _maxNumDiffuseParticles - _diffuseParticles.size();
    }

    if (n <= 0) {
        return;
    }

    float particleRadius = 4.0f*(float)_markerParticleRadius;
    vmath::vec3 axis = vmath::normalize(emitter.velocity);

    float eps = 10e-6f;
    vmath::vec3 e1;
    if (fabs(axis.x) - 1.0 < eps && fabs(axis.y) < eps && fabs(axis.z) < eps) {
        e1 = vmath::normalize(vmath::cross(axis, vmath::vec3(1.0, 0.0, 0.0)));
    } else {
        e1 = vmath::normalize(vmath::cross(axis, vmath::vec3(0.0, 1.0, 0.0)));
    }

    e1 = e1*(float)particleRadius;
    vmath::vec3 e2 = vmath::normalize(vmath::cross(axis, e1)) * (float)particleRadius;

    float Xr, Xt, Xh, r, theta, h, sinval, cosval, lifetime;
    vmath::vec3 p, v;
    GridIndex g;
    for (int i = 0; i < n; i++) {
        Xr = (float)(rand()) / (float)RAND_MAX;
        Xt = (float)(rand()) / (float)RAND_MAX;
        Xh = (float)(rand()) / (float)RAND_MAX;

        r = particleRadius*sqrt(Xr);
        theta = Xt*2.0f*3.141592653f;
        h = Xh*vmath::length((float)dt*emitter.velocity);
        sinval = sin(theta);
        cosval = cos(theta);

        p = emitter.position + r*cosval*e1 + r*sinval*e2 + h*axis;
        g = Grid3d::positionToGridIndex(p, _dx);
        if (!Grid3d::isGridIndexInRange(g, _isize, _jsize, _ksize) || 
                _materialGrid.isCellSolid(g)) {
            continue;
        }

        v = _getVelocityAtPosition(p);
        lifetime = (float)(emitter.energyPotential*_maxDiffuseParticleLifetime);
        lifetime = _randomDouble(0.5*lifetime, lifetime);
        _diffuseParticles.push_back(DiffuseParticle(p, v, lifetime));
    }
}

void FluidSimulation::_emitDiffuseParticles(std::vector<DiffuseParticleEmitter> &emitters,
                                            double dt) {
    for (unsigned int i = 0; i < emitters.size(); i++) {
        if (_diffuseParticles.size() >= _maxNumDiffuseParticles) {
            return;
        }

        _emitDiffuseParticles(emitters[i], dt);
    }
}

int FluidSimulation::_getDiffuseParticleType(DiffuseParticle &dp) {
    double foamDist = _maxFoamToSurfaceDistance*_dx;
    double dist = _levelset.getSignedDistance(dp.position);

    int type;
    if (dist > 0.0) {       // inside surface
        type = DP_BUBBLE;
    } else {                // outside surface
        if (fabs(dist) < foamDist) {
            type = DP_FOAM;
        } else {
            type = DP_SPRAY;
        }
    }

    if (type == DP_FOAM || type == DP_SPRAY) {
        GridIndex g = Grid3d::positionToGridIndex(dp.position, _dx);
        
        if (!_materialGrid.isCellAir(g) && !_materialGrid.isCellNeighbouringAir(g)) {
            type = DP_BUBBLE;
        }
    }

    return type;
}

void FluidSimulation::_updateDiffuseParticleTypes() {
    DiffuseParticle dp;
    int type;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        dp = _diffuseParticles[i];
        type = _getDiffuseParticleType(dp);
        _diffuseParticles[i].type = type;
    }
}

void FluidSimulation::_updateDiffuseParticleLifetimes(double dt) {
    double maxDist = _maxSprayToSurfaceDistance*_dx;

    DiffuseParticle dp;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        dp = _diffuseParticles[i];

        double modifier = 0.0;
        if (dp.type == DP_SPRAY) {
            modifier = _sprayParticleLifetimeModifier;
            if (_levelset.getDistance(dp.position) > maxDist) {
                modifier = _sprayParticleMaxDistanceLifetimeModifier;
            }
        } else if (dp.type == DP_BUBBLE) {
            modifier = _bubbleParticleLifetimeModifier;
        } else if (dp.type == DP_FOAM) {
            modifier = _foamParticleLifetimeModifier;
        }

        _diffuseParticles[i].lifetime = dp.lifetime - (float)(modifier*dt);
    }
}

void FluidSimulation::_getNextBubbleDiffuseParticle(DiffuseParticle &dp,
                                                    DiffuseParticle &nextdp,
                                                    vmath::vec3 bodyForce,
                                                    double dt) {
    vmath::vec3 vmac = _getVelocityAtPosition(dp.position);
    vmath::vec3 vbub = dp.velocity;
    vmath::vec3 bouyancyVelocity = (float)-_bubbleBouyancyCoefficient*bodyForce;
    vmath::vec3 dragVelocity = (float)_bubbleDragCoefficient*(vmac - vbub) / (float)dt;

    nextdp.velocity = dp.velocity + (float)dt*(bouyancyVelocity + dragVelocity);
    nextdp.position = dp.position + nextdp.velocity*(float)dt;
}

void FluidSimulation::_getNextSprayDiffuseParticle(DiffuseParticle &dp,
                                                   DiffuseParticle &nextdp,
                                                   vmath::vec3 bodyForce,
                                                   double dt) {
    float drag = -(float)_sprayDragCoefficient*vmath::dot(dp.velocity, dp.velocity);

    vmath::vec3 accforce = bodyForce;
    if (fabs(drag) > 0.0) {
        accforce += drag*vmath::normalize(dp.velocity);
    }

    nextdp.velocity = dp.velocity + accforce*(float)dt;
    nextdp.position = dp.position + nextdp.velocity*(float)dt;
}

void FluidSimulation::_getNextFoamDiffuseParticle(DiffuseParticle &dp,
                                                  DiffuseParticle &nextdp,
                                                  double dt) {
    vmath::vec3 v0 = _getVelocityAtPosition(dp.position);
    nextdp.velocity = v0;
    nextdp.position = _RK2(dp.position, v0, dt);
}

void FluidSimulation::_advanceDiffuseParticles(double dt) {

    vmath::vec3 bodyForce;
    for (unsigned int i = 0; i < _constantBodyForces.size(); i++) {
        bodyForce += _constantBodyForces[i];
    }

    DiffuseParticle dp, nextdp;
    GridIndex g;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        dp = _diffuseParticles[i];

        if (dp.type == DP_SPRAY) {
            _getNextSprayDiffuseParticle(dp, nextdp, bodyForce, dt);
        } else if (dp.type == DP_BUBBLE) {
            _getNextBubbleDiffuseParticle(dp, nextdp, bodyForce, dt);
        } else {
            _getNextFoamDiffuseParticle(dp, nextdp, dt);
        }

        g = Grid3d::positionToGridIndex(nextdp.position, _dx);

        if (_materialGrid.isCellSolid(g)) {
            nextdp.position = _resolveParticleSolidCellCollision(dp.position, nextdp.position);
        }

        _diffuseParticles[i].position = nextdp.position;
        _diffuseParticles[i].velocity = nextdp.velocity;
    }
}

void FluidSimulation::_removeDiffuseParticles() {
    Array3d<int> countGrid = Array3d<int>(_isize, _jsize, _ksize, 0);

    std::vector<bool> isRemoved;
    isRemoved.reserve(_diffuseParticles.size());

    DiffuseParticle dp;
    GridIndex g;
    double eps = 1e-9;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        dp = _diffuseParticles[i];

        if (dp.lifetime <= 0.0) {
            isRemoved.push_back(true);
            continue;
        }

        g = Grid3d::positionToGridIndex(dp.position, _dx);
        if (countGrid(g) >= _maxDiffuseParticlesPerCell) {
            isRemoved.push_back(true);
            continue;
        }
        countGrid.add(g, 1);

        if (dp.type == DP_SPRAY && vmath::length(dp.velocity) < eps) {
            isRemoved.push_back(true);
            continue;
        }

        isRemoved.push_back(false);
    }

    _removeItemsFromVector(_diffuseParticles, isRemoved);
}

void FluidSimulation::_updateDiffuseMaterial(double dt) {
    std::vector<DiffuseParticleEmitter> emitters;
    _getDiffuseParticleEmitters(emitters);
    _emitDiffuseParticles(emitters, dt);

    if (_diffuseParticles.size() == 0.0) {
        return;
    }

    _updateDiffuseParticleTypes();
    _updateDiffuseParticleLifetimes(dt);
    _advanceDiffuseParticles(dt);
    _removeDiffuseParticles();

    int spraycount = 0;
    int bubblecount = 0;
    int foamcount = 0;
    for (unsigned int i = 0; i < _diffuseParticles.size(); i++) {
        int type = _diffuseParticles[i].type;
        if (type == DP_SPRAY) {
            spraycount++;
        } else if (type == DP_BUBBLE) {
            bubblecount++;
        } else if (type == DP_FOAM) {
            foamcount++;
        }
    }

    _logfile.log("Num Diffuse Particles: ", (int)_diffuseParticles.size(), 1);
    _logfile.log("NUM SPRAY:  ", spraycount, 2);
    _logfile.log("NUM BUBBLE: ", bubblecount, 2);
    _logfile.log("NUM FOAM:   ", foamcount, 2);
}

/********************************************************************************
    UPDATE MARKER PARTICLE VELOCITIES
********************************************************************************/

void FluidSimulation::_updateRangeOfMarkerParticleVelocities(int startIdx, int endIdx) {
    int size = endIdx - startIdx + 1;
    std::vector<vmath::vec3> positions, vnew, vold;
    positions.reserve(size);
    vnew.reserve(size);
    vold.reserve(size);

    for (int i = startIdx; i <= endIdx; i++) {
        positions.push_back(_markerParticles[i].position);
    }

    _particleAdvector.tricubicInterpolate(positions, &_MACVelocity, vnew);
    _particleAdvector.tricubicInterpolate(positions, &_savedVelocityField, vold);

    vmath::vec3 vPIC, vFLIP, v;
    MarkerParticle mp;
    for (unsigned int i = 0; i < positions.size(); i++) {
        mp = _markerParticles[startIdx + i];

        vPIC = vnew[i];
        vFLIP = mp.velocity + vnew[i] - vold[i];

        v = (float)_ratioPICFLIP * vPIC + (float)(1 - _ratioPICFLIP) * vFLIP;
        _markerParticles[startIdx + i].velocity = v;
    }
}

void FluidSimulation::_updateMarkerParticleVelocities() {
    int n = _maxParticlesPerVelocityUpdate;
    for (int startidx = 0; startidx < (int)_markerParticles.size(); startidx += n) {
        int endidx = startidx + n - 1;
        endidx = fmin(endidx, _markerParticles.size() - 1);

        _updateRangeOfMarkerParticleVelocities(startidx, endidx);
    }
}

/********************************************************************************
    ADVANCE MARKER PARTICLES
********************************************************************************/

vmath::vec3 FluidSimulation::_getVelocityAtPosition(vmath::vec3 p) {
    return _MACVelocity.evaluateVelocityAtPosition(p);
}

vmath::vec3 FluidSimulation::_RK2(vmath::vec3 p0, vmath::vec3 v0, double dt) {
    vmath::vec3 k1 = v0;
    vmath::vec3 k2 = _getVelocityAtPosition(p0 + (float)(0.5*dt)*k1);
    vmath::vec3 p1 = p0 + (float)dt*k2;

    return p1;
}

vmath::vec3 FluidSimulation::_RK3(vmath::vec3 p0, vmath::vec3 v0, double dt) {
    vmath::vec3 k1 = v0;
    vmath::vec3 k2 = _getVelocityAtPosition(p0 + (float)(0.5*dt)*k1);
    vmath::vec3 k3 = _getVelocityAtPosition(p0 + (float)(0.75*dt)*k2);
    vmath::vec3 p1 = p0 + (float)(dt/9.0f)*(2.0f*k1 + 3.0f*k2 + 4.0f*k3);

    return p1;
}

vmath::vec3 FluidSimulation::_RK4(vmath::vec3 p0, vmath::vec3 v0, double dt) {
    vmath::vec3 k1 = v0;
    vmath::vec3 k2 = _getVelocityAtPosition(p0 + (float)(0.5*dt)*k1);
    vmath::vec3 k3 = _getVelocityAtPosition(p0 + (float)(0.5*dt)*k2);
    vmath::vec3 k4 = _getVelocityAtPosition(p0 + (float)dt*k3);
    
    vmath::vec3 p1 = p0 + (float)(dt/6.0f)*(k1 + 2.0f*k2 + 2.0f*k3 + k4);

    return p1;
}

vmath::vec3 FluidSimulation::_resolveParticleSolidCellCollision(vmath::vec3 p0, vmath::vec3 p1) {
    
    GridIndex g1 = Grid3d::positionToGridIndex(p0, _dx);
    GridIndex g2 = Grid3d::positionToGridIndex(p1, _dx);
    assert(!_materialGrid.isCellSolid(g1));
    assert(_materialGrid.isCellSolid(g2));

    GridIndex voxel;
    bool foundVoxel = Collision::getLineSegmentVoxelIntersection(p0, p1, _dx,
                                                                 _materialGrid, 
                                                                 &voxel);
    if (!foundVoxel) {
        return p0;
    }

    vmath::vec3 raynorm = vmath::normalize(p1 - p0);
    vmath::vec3 vpos = Grid3d::GridIndexToPosition(voxel, _dx);
    AABB bbox(vpos, _dx, _dx, _dx);

    vmath::vec3 cpoint;
    bool foundCollision = Collision::rayIntersectsAABB(p0, raynorm, bbox, &cpoint);

    if (!foundCollision) {
        return p0;
    }

    vmath::vec3 resolvedPosition = cpoint - (float)(0.05*_dx)*raynorm;

    GridIndex gr = Grid3d::positionToGridIndex(resolvedPosition, _dx);
    if (_materialGrid.isCellSolid(gr)) {
        return p0;
    }
    
    return resolvedPosition;
}

void FluidSimulation::_advanceRangeOfMarkerParticles(int startIdx, int endIdx, 
                                                     double dt) {
    assert(startIdx <= endIdx);

    std::vector<vmath::vec3> positions;
    positions.reserve(endIdx - startIdx + 1);
    for (int i = startIdx; i <= endIdx; i++) {
        positions.push_back(_markerParticles[i].position);
    }

    std::vector<vmath::vec3> output;
    _particleAdvector.advectParticlesRK4(positions, &_MACVelocity, dt, 
                                         output);

    MarkerParticle mp;
    vmath::vec3 nextp;
    GridIndex g;
    for (unsigned int i = 0; i < output.size(); i++) {
        mp = _markerParticles[startIdx + i];
        nextp = output[i];

        g = Grid3d::positionToGridIndex(nextp, _dx);
        if (_materialGrid.isCellSolid(g)) {
            nextp = _resolveParticleSolidCellCollision(mp.position, nextp);
        }

        _markerParticles[startIdx + i].position = nextp;
    }
}

void FluidSimulation::_shuffleMarkerParticleOrder() {
    MarkerParticle mi;
    for (int i = _markerParticles.size() - 2; i >= 0; i--) {
        int j = (rand() % (int)(i - 0 + 1));
        mi = _markerParticles[i];
        _markerParticles[i] = _markerParticles[j];
        _markerParticles[j] = mi;
    }
}

void FluidSimulation::_removeMarkerParticles() {
    double maxspeed = (_CFLConditionNumber*_dx) / _minTimeStep;
    double maxspeedsq = maxspeed*maxspeed;

    Array3d<int> countGrid = Array3d<int>(_isize, _jsize, _ksize, 0);
    _shuffleMarkerParticleOrder();

    std::vector<bool> isRemoved;
    isRemoved.reserve(_markerParticles.size());

    MarkerParticle mp;
    GridIndex g;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        mp = _markerParticles[i];

        double speedsq = vmath::dot(mp.velocity, mp.velocity);
        if (speedsq > maxspeedsq) {
            isRemoved.push_back(true);
            continue;
        }

        g = Grid3d::positionToGridIndex(mp.position, _dx);
        if (countGrid(g) >= _maxMarkerParticlesPerCell) {
            isRemoved.push_back(true);
            continue;
        }
        countGrid.add(g, 1);

        isRemoved.push_back(false);
    }

    _removeItemsFromVector(_markerParticles, isRemoved);
}

void FluidSimulation::_advanceMarkerParticles(double dt) {

    int n = _maxParticlesPerAdvection;
    for (int startidx = 0; startidx < (int)_markerParticles.size(); startidx += n) {
        int endidx = startidx + n - 1;
        endidx = fmin(endidx, _markerParticles.size() - 1);

        _advanceRangeOfMarkerParticles(startidx, endidx, dt);
    }

    _removeMarkerParticles();
}

/********************************************************************************
    TIME STEP
********************************************************************************/

void FluidSimulation::_stepFluid(double dt) {

    _simulationTime += dt;

    StopWatch timer1 = StopWatch();
    StopWatch timer2 = StopWatch();
    StopWatch timer3 = StopWatch();
    StopWatch timer4 = StopWatch();
    StopWatch timer5 = StopWatch();
    StopWatch timer6 = StopWatch();
    StopWatch timer7 = StopWatch();
    StopWatch timer8 = StopWatch();
    StopWatch timer9 = StopWatch();
    StopWatch timer10 = StopWatch();
    StopWatch timer11 = StopWatch();
    StopWatch timer12 = StopWatch();
    StopWatch timer13 = StopWatch();

    _logfile.separator();
    _logfile.timestamp();
    _logfile.newline();
    _logfile.log("Frame: ", _currentFrame, 0);
    _logfile.log("StepTime: ", dt, 4);
    _logfile.newline();

    timer1.start();

    timer2.start();
    _updateFluidCells();
    timer2.stop();

    _logfile.log("Update Fluid Cells:          \t", timer2.getTime(), 4);
    _logfile.log("Num Fluid Cells: \t", (int)_fluidCellIndices.size(), 4, 1);
    _logfile.log("Num Marker Particles: \t", (int)_markerParticles.size(), 4, 1);

    timer3.start();
    _reconstructFluidSurface();
    timer3.stop();

    _logfile.log("Reconstruct Fluid Surface:  \t", timer3.getTime(), 4);

    timer4.start();
    _updateLevelSetSignedDistance();
    timer4.stop();

    _logfile.log("Update Level set:           \t", timer4.getTime(), 4);

    timer5.start();
    if (_isLastTimeStepForFrame) {
        _reconstructOutputFluidSurface(_frameTimeStep);
    }
    timer5.stop();

    _logfile.log("Reconstruct Output Surface: \t", timer5.getTime(), 4);

    timer6.start();
    _advectVelocityField();
    _savedVelocityField = _MACVelocity;
    _extrapolateFluidVelocities(_savedVelocityField);
    timer6.stop();

    _logfile.log("Advect Velocity Field:       \t", timer6.getTime(), 4);

    timer7.start();
    _applyBodyForcesToVelocityField(dt);
    timer7.stop();

    _logfile.log("Apply Body Forces:           \t", timer7.getTime(), 4);

    {
        timer8.start();
        Array3d<float> pressureGrid = Array3d<float>(_isize, _jsize, _ksize, 0.0f);
        _updatePressureGrid(pressureGrid, dt);
        timer8.stop();

        _logfile.log("Update Pressure Grid:        \t", timer8.getTime(), 4);

        timer9.start();
        _applyPressureToVelocityField(pressureGrid, dt);
        timer9.stop();

        _logfile.log("Apply Pressure:              \t", timer9.getTime(), 4);
    }

    timer10.start();
    _extrapolateFluidVelocities(_MACVelocity);
    timer10.stop();

    _logfile.log("Extrapolate Fluid Velocities:\t", timer10.getTime(), 4);

    timer11.start();
    if (_isDiffuseMaterialOutputEnabled) {
        _updateDiffuseMaterial(dt);
    }
    timer11.stop();

    _logfile.log("Update Diffuse Material:     \t", timer11.getTime(), 4);

    timer12.start();
    _updateMarkerParticleVelocities();
    _savedVelocityField = MACVelocityField();
    
    timer12.stop();

    _logfile.log("Update PIC/FLIP velocities   \t", timer12.getTime(), 4);

    timer13.start();
    _advanceMarkerParticles(dt);
    timer13.stop();

    _logfile.log("Advance Marker Particles:    \t", timer13.getTime(), 4);

    timer1.stop();

    double totalTime = floor(timer1.getTime()*1000.0) / 1000.0;
    _realTime += totalTime;
    _logfile.newline();
    _logfile.log("Update Time:           \t", totalTime, 3, 1);
    _logfile.newline();

    double p2 = floor(1000 * timer2.getTime() / totalTime) / 10.0;
    double p3 = floor(1000 * timer3.getTime() / totalTime) / 10.0;
    double p4 = floor(1000 * timer4.getTime() / totalTime) / 10.0;
    double p5 = floor(1000 * timer5.getTime() / totalTime) / 10.0;
    double p6 = floor(1000 * timer6.getTime() / totalTime) / 10.0;
    double p7 = floor(1000 * timer7.getTime() / totalTime) / 10.0;
    double p8 = floor(1000 * timer8.getTime() / totalTime) / 10.0;
    double p9 = floor(1000 * timer9.getTime() / totalTime) / 10.0;
    double p10 = floor(1000 * timer10.getTime() / totalTime) / 10.0;
    double p11 = floor(1000 * timer11.getTime() / totalTime) / 10.0;
    double p12 = floor(1000 * timer12.getTime() / totalTime) / 10.0;
    double p13 = floor(1000 * timer13.getTime() / totalTime) / 10.0;

    _logfile.log("---Percentage Breakdown---", "");
    _logfile.log("Update Fluid Cells:          \t", p2, 3);
    _logfile.log("Reconstruct Fluid Surface    \t", p3, 3);
    _logfile.log("Update Level Set:            \t", p4, 3);
    _logfile.log("Reconstruct Output Surface   \t", p5, 3);
    _logfile.log("Advect Velocity Field:       \t", p6, 3);
    _logfile.log("Apply Body Forces:           \t", p7, 3);
    _logfile.log("Update Pressure Grid:        \t", p8, 3);
    _logfile.log("Apply Pressure:              \t", p9, 3);
    _logfile.log("Extrapolate Fluid Velocities:\t", p10, 3);
    _logfile.log("Update Diffuse Material:     \t", p11, 3);
    _logfile.log("Update PIC/FLIP Velocity:    \t", p12, 3);
    _logfile.log("Advance Marker Particles:    \t", p13, 3);
    _logfile.newline();

    _logfile.log("Simulation time: ", _simulationTime, 3);
    _logfile.log("Real time: ", _realTime, 2);
    _logfile.newline();
    _logfile.write();
}

double FluidSimulation::_getMaximumMarkerParticleSpeed() {
    double maxsq = 0.0;
    MarkerParticle mp;
    for (unsigned int i = 0; i < _markerParticles.size(); i++) {
        double distsq = vmath::dot(mp.velocity, mp.velocity);
        if (distsq > maxsq) {
            maxsq = distsq;
        }
    }

    return sqrt(maxsq);
}

double FluidSimulation::_calculateNextTimeStep() {
    double maxu = _getMaximumMarkerParticleSpeed();
    double timeStep = _CFLConditionNumber*_dx / maxu;

    timeStep = (double)fmax((float)_minTimeStep, (float)timeStep);
    timeStep = (double)fmin((float)_maxTimeStep, (float)timeStep);

    return timeStep;
}

void FluidSimulation::update(double dt) {
    if (!_isSimulationRunning || !_isSimulationInitialized) {
        return;
    }
    _isCurrentFrameFinished = false;

    _frameTimeStep = dt;

    saveState();

    _currentTimeStep = 0;
    double timeleft = dt;
    while (timeleft > 0.0) {
        double timestep = _calculateNextTimeStep();
        if (timeleft - timestep < 0.0) {
            timestep = timeleft;
        }
        timeleft -= timestep;

        double eps = 10e-9;
        _isLastTimeStepForFrame = fabs(timeleft) < eps;

        _currentDeltaTime = timestep;
        _stepFluid(timestep);

        _currentTimeStep++;
    }
    _currentFrame++;

    _isCurrentFrameFinished = true;
}