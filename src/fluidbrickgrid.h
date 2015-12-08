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
#ifndef FLUIDBRICKGRID_H
#define FLUIDBRICKGRID_H

#include <stdio.h>
#include <iostream>
#include <vector>

#include "array3d.h"
#include "aabb.h"
#include "levelset.h"
#include "trianglemesh.h"
#include "glm/glm.hpp"

struct Brick {
    float intensity = 0.0f;
    double isActive = false;

    Brick() : intensity(0.0f),
                isActive(false) {}

    Brick(float i) : intensity(i),
                        isActive(false) {}
};

class FluidBrickGrid
{
public:
    FluidBrickGrid();
    FluidBrickGrid(int isize, int jsize, int ksize, double dx, AABB brick);
    ~FluidBrickGrid();

    void getGridDimensions(int *i, int *j, int *k);
    void setBrickDimensions(double width, double height, double depth);
    void setBrickDimensions(AABB brick);
    Array3d<float> getDensityGrid();
    bool getBrickMesh(LevelSet &levelset, TriangleMesh &mesh);
    bool isBrickMeshReady();
    void update(LevelSet &levelset, std::vector<glm::vec3> &markerParticles, double dt);

private:

    struct DensityNode {
        float currentDensity = 0.0f;
        float targetDensity = 0.0f;
        float intensityVelocity = 0.0f;
    };

    

    void _initializeBrickGrid();
    void _updateDensityGrid(std::vector<glm::vec3> &particles, double dt);
    void _updateTargetDensities(std::vector<glm::vec3> &particles);
    void _updateDensities(double dt);
    void _updateDensity(int i, int j, int k, double dt);
    void _updateBrickGrid(LevelSet &levelset);
    float _getBrickIntensity(int i, int j, int k);
    bool _isBrickNextToActiveNeighbour(int i, int j, int k);
    void _postProcessBrickGrid();
    void _removeStrayBricks();
    void _removeSmallBrickStructures();
    void _mergeBrickGrids();
    void _getNewBrickLocations(Array3d<Brick> &b1, Array3d<Brick> &b2,
                               Array3d<bool> &newBricks);
    void _getbrickStructures(Array3d<Brick> &brickGrid, Array3d<bool> &newBricks, 
                         std::vector<std::vector<GridIndex>> &brickStructures);
    void _getConnectedBricks(int i, int j, int k, Array3d<Brick> &brickGrid,
                                                  Array3d<bool> &newBricks, 
                                                  std::vector<GridIndex> &connectedBricks);
    void _removeInvalidbrickStructures(Array3d<Brick> &brickCurrent,
                                   Array3d<Brick> &brickNext,
                                   std::vector<std::vector<GridIndex>> &brickStructures);
    bool _isBrickMassInBrickGrid(std::vector<GridIndex> &cells, Array3d<Brick> &brickGrid);
    void _removeBrickStructureFromBrickGrid(std::vector<GridIndex> &cells, Array3d<Brick> &brickGrid);

    int _isize = 0;
    int _jsize = 0;
    int _ksize = 0;
    double _dx = 0.0;

    AABB _brick;
    Array3d<DensityNode> _densityGrid;
    Array3d<Brick> _brickGrid;
    Array3d<Brick> _brickGridQueue[3];
    int _brickGridQueueSize = 0;
    Array3d<Brick> _currentBrickGrid;
    bool _isCurrentBrickGridReady = false;

    int _minParticleDensityCount = 0;
    int _maxParticleDensityCount = 8;
    float _maxIntensityVelocity = 10.0f;
    float _maxIntensityAcceleration = 10.0f;
    float _decelerationRadius = 0.05f;
    unsigned int _minNumberOfBricksInStructure = 20;

    int _numUpdates = 0;
};

#endif
