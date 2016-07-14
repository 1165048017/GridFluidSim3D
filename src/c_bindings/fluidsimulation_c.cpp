#include "../fluidsimulation.h"

#ifdef _WIN32
    #define EXPORTDLL __declspec(dllexport)
#else
    #define EXPORTDLL
#endif

#define FLUIDSIMULATION_SUCCESS 1
#define FLUIDSIMULATION_FAIL 0

char FLUIDSIMULATION_ERROR_MESSAGE[4096];
void FluidSimulation_set_error_message(std::exception &ex) {
    std::string msg = ex.what();
    msg.copy(FLUIDSIMULATION_ERROR_MESSAGE, msg.length(), 0);
    FLUIDSIMULATION_ERROR_MESSAGE[msg.length()] = '\0';
}

void FluidSimulation_execute_method(FluidSimulation *obj,
                                    void (FluidSimulation::*funcptr)(void),
                                    int *err) {
    *err = FLUIDSIMULATION_SUCCESS;
    try {
        (obj->*funcptr)();
    } catch (std::exception &ex) {
        FluidSimulation_set_error_message(ex);
        *err = FLUIDSIMULATION_FAIL;
    }
}

template<typename T>
T FluidSimulation_execute_method(FluidSimulation *obj,
                                    T (FluidSimulation::*funcptr)(void),
                                    int *err) {
    *err = FLUIDSIMULATION_SUCCESS;
    T result = T();
    try {
        result = (obj->*funcptr)();
    } catch (std::exception &ex) {
        FluidSimulation_set_error_message(ex);
        *err = FLUIDSIMULATION_FAIL;
    }

    return result;
}

template<typename T>
void FluidSimulation_execute_method(FluidSimulation *obj,
                                   void (FluidSimulation::*funcptr)(T),
                                   double param,
                                   int *err) {
    *err = FLUIDSIMULATION_SUCCESS;
    try {
        (obj->*funcptr)(param);
    } catch (std::exception &ex) {
        FluidSimulation_set_error_message(ex);
        *err = FLUIDSIMULATION_FAIL;
    }
}

template<typename T>
void FluidSimulation_execute_method(FluidSimulation *obj,
                                   void (FluidSimulation::*funcptr)(T, T, T),
                                   T param1, T param2, T param3,
                                   int *err) {
    *err = FLUIDSIMULATION_SUCCESS;
    try {
        (obj->*funcptr)(param1, param2, param3);
    } catch (std::exception &ex) {
        FluidSimulation_set_error_message(ex);
        *err = FLUIDSIMULATION_FAIL;
    }
}

extern "C" {
    EXPORTDLL FluidSimulation* FluidSimulation_new_from_empty(int *err) {
        FluidSimulation *fluidsim = nullptr;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            fluidsim = new FluidSimulation();
        } catch (std::exception &ex) {
            FluidSimulation_set_error_message(ex);
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
        } catch (std::exception &ex) {
            FluidSimulation_set_error_message(ex);
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
        } catch (std::exception &ex) {
            FluidSimulation_set_error_message(ex);
            *err = FLUIDSIMULATION_FAIL;
        }

        return fluidsim; 
    }

    EXPORTDLL void FluidSimulation_destroy(FluidSimulation* obj) {
        delete obj;
    }

    EXPORTDLL void FluidSimulation_initialize(FluidSimulation* obj, int *err) {
        FluidSimulation_execute_method(obj, &FluidSimulation::initialize, err);
    }

    EXPORTDLL int FluidSimulation_is_initialized(FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isInitialized, err
        );
    }

    EXPORTDLL void FluidSimulation_update(FluidSimulation* obj, double dt, int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::update, dt, err
        );
    }

    EXPORTDLL void FluidSimulation_save_state(FluidSimulation* obj, char* filename, 
                                              int *err) {
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->saveState(std::string(filename));
        } catch (std::exception &ex) {
            FluidSimulation_set_error_message(ex);
            *err = FLUIDSIMULATION_FAIL;
        }
    }

    EXPORTDLL int FluidSimulation_get_current_frame(FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getCurrentFrame, err
        );
    }

    EXPORTDLL int FluidSimulation_is_current_frame_finished(FluidSimulation* obj, 
                                                            int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isCurrentFrameFinished, err
        );
    }

    EXPORTDLL double FluidSimulation_get_cell_size(FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getCellSize, err
        );
    }

    EXPORTDLL void FluidSimulation_get_grid_dimensions(
            FluidSimulation* obj, int *i, int *j, int *k, int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::getGridDimensions, i, j, k, err
        );
    }

    EXPORTDLL int FluidSimulation_get_grid_width(FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getGridWidth, err
        );
    }

    EXPORTDLL int FluidSimulation_get_grid_height(FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getGridHeight, err
        );
    }

    EXPORTDLL int FluidSimulation_get_grid_depth(FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getGridDepth, err
        );
    }

    EXPORTDLL void FluidSimulation_get_simulation_dimensions(
            FluidSimulation* obj, 
            double *width, double *height, double *depth, int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::getSimulationDimensions, width, height, depth, err
        );
    }

    EXPORTDLL double FluidSimulation_get_simulation_width(FluidSimulation* obj, 
                                                          int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getSimulationWidth, err
        );
    }

    EXPORTDLL double FluidSimulation_get_simulation_height(FluidSimulation* obj, 
                                                           int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getSimulationHeight, err
        );
    }

    EXPORTDLL double FluidSimulation_get_simulation_depth(FluidSimulation* obj, 
                                                          int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getSimulationDepth, err
        );
    }

    EXPORTDLL double FluidSimulation_get_density(FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getDensity, err
        );
    }

    EXPORTDLL void FluidSimulation_set_density(FluidSimulation* obj, 
                                               double density,
                                               int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setDensity, density, err
        );
    }

    EXPORTDLL int FluidSimulation_get_material(FluidSimulation* obj, 
                                                int i, int j, int k, int *err) {
        Material m = Material::air;
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            m = obj->getMaterial(i, j, k);
        } catch (std::exception &ex) {
            FluidSimulation_set_error_message(ex);
            *err = FLUIDSIMULATION_FAIL;
        }

        return (int)m;
    }

    EXPORTDLL double FluidSimulation_get_marker_particle_scale(FluidSimulation* obj, 
                                                               int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getMarkerParticleScale, err
        );
    }

    EXPORTDLL void FluidSimulation_set_marker_particle_scale(FluidSimulation* obj, 
                                                             double scale,
                                                             int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setMarkerParticleScale, scale, err
        );
    }

    EXPORTDLL int FluidSimulation_get_surface_subdivision_level(FluidSimulation* obj, 
                                                                int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getSurfaceSubdivisionLevel, err
        );
    }

    EXPORTDLL void FluidSimulation_set_surface_subdivision_level(FluidSimulation* obj, 
                                                                 int level,
                                                                 int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setSurfaceSubdivisionLevel, level, err
        );
    }

    EXPORTDLL int FluidSimulation_get_num_polygonizer_slices(FluidSimulation* obj, 
                                                             int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getNumPolygonizerSlices, err
        );
    }

    EXPORTDLL void FluidSimulation_set_num_polygonizer_slices(FluidSimulation* obj, 
                                                              int numslices,
                                                              int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setNumPolygonizerSlices, numslices, err
        );
    }

    EXPORTDLL int FluidSimulation_get_min_polyhedron_triangle_count(FluidSimulation* obj, 
                                                                    int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getMinPolyhedronTriangleCount, err
        );
    }

    EXPORTDLL void FluidSimulation_set_min_polyhedron_triangle_count(FluidSimulation* obj, 
                                                                     int count,
                                                                     int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setMinPolyhedronTriangleCount, count, err
        );
    }

    EXPORTDLL void FluidSimulation_enable_surface_mesh_output(FluidSimulation* obj,
                                                              int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::enableSurfaceMeshOutput, err
        );
    }

    EXPORTDLL void FluidSimulation_disable_surface_mesh_output(FluidSimulation* obj,
                                                               int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::disableSurfaceMeshOutput, err
        );
    }

    EXPORTDLL int FluidSimulation_is_surface_mesh_output_enabled(FluidSimulation* obj,
                                                                 int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isSurfaceMeshOutputEnabled, err
        );
    }

    EXPORTDLL void FluidSimulation_enable_isotropic_surface_reconstruction(FluidSimulation* obj,
                                                                           int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::enableIsotropicSurfaceReconstruction, err
        );
    }

    EXPORTDLL void FluidSimulation_disable_isotropic_surface_reconstruction(FluidSimulation* obj,
                                                                            int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::disableIsotropicSurfaceReconstruction, err
        );
    }

    EXPORTDLL int FluidSimulation_is_isotropic_surface_reconstruction_enabled(FluidSimulation* obj,
                                                                              int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isIsotropicSurfaceReconstructionEnabled, err
        );
    }

    EXPORTDLL void FluidSimulation_enable_anisotropic_surface_reconstruction(FluidSimulation* obj,
                                                                             int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::enableAnisotropicSurfaceReconstruction, err
        );
    }

    EXPORTDLL void FluidSimulation_disable_anisotropic_surface_reconstruction(FluidSimulation* obj,
                                                                              int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::disableAnisotropicSurfaceReconstruction, err
        );
    }

    EXPORTDLL int FluidSimulation_is_anisotropic_surface_reconstruction_enabled(FluidSimulation* obj,
                                                                                int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isAnisotropicSurfaceReconstructionEnabled, err
        );
    }

    EXPORTDLL void FluidSimulation_enable_diffuse_material_output(FluidSimulation* obj,
                                                                  int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::enableDiffuseMaterialOutput, err
        );
    }

    EXPORTDLL void FluidSimulation_disable_diffuse_material_output(FluidSimulation* obj,
                                                                   int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::disableDiffuseMaterialOutput, err
        );
    }

    EXPORTDLL int FluidSimulation_is_diffuse_material_output_enabled(FluidSimulation* obj,
                                                                     int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isDiffuseMaterialOutputEnabled, err
        );
    }

    EXPORTDLL void FluidSimulation_enable_bubble_diffuse_material(FluidSimulation* obj,
                                                                  int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::enableBubbleDiffuseMaterial, err
        );
    }

    EXPORTDLL void FluidSimulation_disable_bubble_diffuse_material(FluidSimulation* obj,
                                                                   int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::disableBubbleDiffuseMaterial, err
        );
    }

    EXPORTDLL int FluidSimulation_is_bubble_diffuse_material_enabled(FluidSimulation* obj,
                                                                     int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isBubbleDiffuseMaterialEnabled, err
        );
    }

    EXPORTDLL void FluidSimulation_enable_spray_diffuse_material(FluidSimulation* obj,
                                                                  int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::enableSprayDiffuseMaterial, err
        );
    }

    EXPORTDLL void FluidSimulation_disable_spray_diffuse_material(FluidSimulation* obj,
                                                                   int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::disableSprayDiffuseMaterial, err
        );
    }

    EXPORTDLL int FluidSimulation_is_spray_diffuse_material_enabled(FluidSimulation* obj,
                                                                     int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isSprayDiffuseMaterialEnabled, err
        );
    }

    EXPORTDLL void FluidSimulation_enable_foam_diffuse_material(FluidSimulation* obj,
                                                                  int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::enableFoamDiffuseMaterial, err
        );
    }

    EXPORTDLL void FluidSimulation_disable_foam_diffuse_material(FluidSimulation* obj,
                                                                   int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::disableFoamDiffuseMaterial, err
        );
    }

    EXPORTDLL int FluidSimulation_is_foam_diffuse_material_enabled(FluidSimulation* obj,
                                                                     int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::isFoamDiffuseMaterialEnabled, err
        );
    }

    EXPORTDLL int FluidSimulation_get_max_num_diffuse_particles(FluidSimulation* obj,
                                                                int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getMaxNumDiffuseParticles, err
        );
    }

    EXPORTDLL void FluidSimulation_set_max_num_diffuse_particles(FluidSimulation* obj,
                                                                 int n, int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setMaxNumDiffuseParticles, n, err
        );
    }

    EXPORTDLL double FluidSimulation_get_diffuse_particle_wavecrest_emission_rate(
            FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getDiffuseParticleWavecrestEmissionRate, err
        );
    }

    EXPORTDLL void FluidSimulation_set_diffuse_particle_wavecrest_emission_rate(
            FluidSimulation* obj, double rate, int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setDiffuseParticleWavecrestEmissionRate, rate, err
        );
    }

    EXPORTDLL double FluidSimulation_get_diffuse_particle_turbulence_emission_rate(
            FluidSimulation* obj, int *err) {
        return FluidSimulation_execute_method(
            obj, &FluidSimulation::getDiffuseParticleTurbulenceEmissionRate, err
        );
    }

    EXPORTDLL void FluidSimulation_set_diffuse_particle_turbulence_emission_rate(
            FluidSimulation* obj, double rate, int *err) {
        FluidSimulation_execute_method(
            obj, &FluidSimulation::setDiffuseParticleTurbulenceEmissionRate, rate, err
        );
    }

    EXPORTDLL void FluidSimulation_add_implicit_fluid_point(
            FluidSimulation* obj, 
            double x, double y, double z, double r, 
            int *err) {
        *err = FLUIDSIMULATION_SUCCESS;
        try {
            obj->addImplicitFluidPoint(x, y, z, r);
        } catch (std::exception &ex) {
            FluidSimulation_set_error_message(ex);
            *err = FLUIDSIMULATION_FAIL;
        }
    }

    EXPORTDLL char* FluidSimulation_get_error_message() {
        return FLUIDSIMULATION_ERROR_MESSAGE;
    }
}
