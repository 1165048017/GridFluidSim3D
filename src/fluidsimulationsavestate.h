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
#ifndef FLUIDSIMULATIONSAVESTATE_H
#define FLUIDSIMULATIONSAVESTATE_H

#include <vector>
#include <fstream>

#include "macvelocityfield.h"
#include "fluidmaterialgrid.h"
#include "array3d.h"
#include "vmath.h"

class FluidSimulation;

class FluidSimulationSaveState
{
public:
    FluidSimulationSaveState();
    ~FluidSimulationSaveState();

    void saveState(std::string filename, FluidSimulation *fluidsim);
    bool loadState(std::string filename);
    void closeState();

    void getGridDimensions(int *i, int *j, int *);
    double getCellSize();
    int getCurrentFrame();
    int getNumMarkerParticles();
    int getNumDiffuseParticles();
    int getNumSolidCells();
    std::vector<vmath::vec3> getMarkerParticlePositions();
    std::vector<vmath::vec3> getMarkerParticleVelocities();
    std::vector<vmath::vec3> getDiffuseParticlePositions();
    std::vector<vmath::vec3> getDiffuseParticleVelocities();
    std::vector<float> getDiffuseParticleLifetimes();
    std::vector<GridIndex> getSolidCells();
    std::vector<vmath::vec3> getMarkerParticlePositions(int startidx, int endidx);
    std::vector<vmath::vec3> getMarkerParticleVelocities(int startidx, int endidx);
    std::vector<vmath::vec3> getDiffuseParticlePositions(int startidx, int endidx);
    std::vector<vmath::vec3> getDiffuseParticleVelocities(int startidx, int endidx);
    std::vector<float> getDiffuseParticleLifetimes(int startidx, int endidx);
    std::vector<GridIndex> getSolidCells(int startidx, int endidx);
    bool isLoadStateInitialized();

private:

    void _writeInt(int *value, std::ofstream *state);
    void _writeDouble(double *value, std::ofstream *state);
    void _writeBinaryMarkerParticlePositions(FluidSimulation *_fluidsim,
                                             std::ofstream *state);
    void _writeBinaryMarkerParticleVelocities(FluidSimulation *_fluidsim,
                                              std::ofstream *state);
    void _writeBinaryDiffuseParticlePositions(FluidSimulation *_fluidsim,
                                             std::ofstream *state);
    void _writeBinaryDiffuseParticleVelocities(FluidSimulation *_fluidsim,
                                              std::ofstream *state);
    void _writeBinaryDiffuseParticleLifetimes(FluidSimulation *_fluidsim,
                                             std::ofstream *state);
    int _getNumSolidCells(FluidSimulation *sim);
    void _writeBinarySolidCellIndices(FluidSimulation *sim, std::ofstream *state);

    void _writeBinaryVector3f(std::vector<vmath::vec3> &vectors, std::ofstream *state);
    void _writeBinaryVectorf(std::vector<float> &floats, std::ofstream *state);
    void _writeBinaryVectorGridIndex(std::vector<GridIndex> &floats, 
                                     std::ofstream *state);

    void _setLoadStateFileOffset(unsigned int foffset);
    bool _readInt(int *value, std::ifstream *state);
    bool _readDouble(double *value, std::ifstream *state);
    bool _readParticleVectors(std::vector<vmath::vec3> &particles, 
                              int numParticles,
                              std::ifstream *state);
    bool _readParticleLifetimes(std::vector<float> &lifetimes, 
                                int numParticles,
                                std::ifstream *state);
    bool _readSolidCells(std::vector<GridIndex> &indices, 
                         int numIndices,
                        std::ifstream *state);

    bool _isLoadStateInitialized = false;
    int _width, _height, _depth;

    int _writeChunkSize = 50000;

    std::ifstream _loadState;

    int _isize = 0;
    int _jsize = 0;
    int _ksize = 0;
    double _dx = 0.0;
    int _currentFrame = 0;

    int _numMarkerParticles = 0;
    int _numDiffuseParticles = 0;
    int _numSolidCells = 0;

    unsigned int _mpPositionOffset = 0;
    unsigned int _mpVelocityOffset = 0;
    unsigned int _dpPositionOffset = 0;
    unsigned int _dpVelocityOffset = 0;
    unsigned int _dpLifetimeOffset = 0;
    unsigned int _solidCellOffset = 0;
    unsigned int _oefOffset = 0;
    unsigned int _currentOffset = 0;

};

#endif
