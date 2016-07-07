#include "../fluidsimulation.h"

#ifdef _WIN32
    #define EXPORTDLL __declspec(dllexport)
#else
    #define EXPORTDLL
#endif

#define FLUIDSIMULATION_SUCCESS 1
#define FLUIDSIMULATION_FAIL 0

extern "C" {
    EXPORTDLL FluidSimulation* FluidSimulation_new_from_empty(int *err) {
        FluidSimulation *fluidsim = nullptr;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            fluidsim = new FluidSimulation();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return fluidsim;
    }

    EXPORTDLL FluidSimulation* FluidSimulation_new_from_dimensions(
            int isize, int jsize, int ksize, double dx, int *err) {

        FluidSimulation *fluidsim = nullptr;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            fluidsim = new FluidSimulation(isize, jsize, ksize, dx);
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return fluidsim;
    }

    EXPORTDLL FluidSimulation* FluidSimulation_new_from_save_state(
            FluidSimulationSaveState *savestate, int *err) {

        FluidSimulation *fluidsim = nullptr;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            fluidsim = new FluidSimulation(*savestate);
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return fluidsim; 
    }

    EXPORTDLL void FluidSimulation_destroy(FluidSimulation* obj) {
        delete obj;
    }

    EXPORTDLL void FluidSimulation_initialize(FluidSimulation* obj, int *err) {
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->initialize();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }
    }

    EXPORTDLL int FluidSimulation_is_initialized(FluidSimulation* obj, int *err) {
        bool result = false;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            result = obj->isInitialized();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return result;
    }

    EXPORTDLL void FluidSimulation_update(FluidSimulation* obj, double dt, int *err) {
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->update(dt);
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }
    }

    EXPORTDLL void FluidSimulation_save_state(FluidSimulation* obj, char* filename, 
                                              int *err) {
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->saveState(std::string(filename));
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }
    }

    EXPORTDLL int FluidSimulation_get_current_frame(FluidSimulation* obj, int *err) {
        int result = 0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            result = obj->getCurrentFrame();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return result;
    }

    EXPORTDLL int FluidSimulation_is_current_frame_finished(FluidSimulation* obj, 
                                                            int *err) {
        bool result = false;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            result = obj->isCurrentFrameFinished();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return result;
    }

    EXPORTDLL double FluidSimulation_get_cell_size(FluidSimulation* obj, int *err) {
        double dx = 0.0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            dx = obj->getCellSize();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return dx;
    }

    EXPORTDLL void FluidSimulation_get_grid_dimensions(
            FluidSimulation* obj, int *i, int *j, int *k, int *err) {

        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->getGridDimensions(i, j, k);
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }
    }

    EXPORTDLL int FluidSimulation_get_grid_width(FluidSimulation* obj, int *err) {
        int isize = 0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            isize = obj->getGridWidth();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return isize;
    }

    EXPORTDLL int FluidSimulation_get_grid_height(FluidSimulation* obj, int *err) {
        int jsize = 0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            jsize = obj->getGridHeight();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return jsize;
    }

    EXPORTDLL int FluidSimulation_get_grid_depth(FluidSimulation* obj, int *err) {
        int ksize = 0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            ksize = obj->getGridDepth();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return ksize;
    }

    EXPORTDLL void FluidSimulation_get_simulation_dimensions(
            FluidSimulation* obj, 
            double *width, double *height, double *depth, int *err) {

        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->getSimulationDimensions(width, height, depth);
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }
    }

    EXPORTDLL double FluidSimulation_get_simulation_width(FluidSimulation* obj, 
                                                          int *err) {
        double width = 0.0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            width = obj->getSimulationWidth();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return width;
    }

    EXPORTDLL double FluidSimulation_get_simulation_height(FluidSimulation* obj, 
                                                           int *err) {
        double height = 0.0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            height = obj->getSimulationHeight();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return height;
    }

    EXPORTDLL double FluidSimulation_get_simulation_depth(FluidSimulation* obj, 
                                                          int *err) {
        double depth = 0.0;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            depth = obj->getSimulationDepth();
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }

        return depth;
    }

    EXPORTDLL void FluidSimulation_add_implicit_fluid_point(
            FluidSimulation* obj, 
            double x, double y, double z, double r, 
            int *err) {
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->addImplicitFluidPoint(x, y, z, r);
        } catch (...) {
            *err = FLUIDSIMULATION_FAIL;
        }
    }
}
