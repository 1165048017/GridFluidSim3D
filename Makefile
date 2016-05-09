OPENCLINCLUDEPATH=-I"C:\Program Files (x86)\Intel\OpenCL SDK\include"
OPENCLLIBPATH=-L"C:\Program Files (x86)\Intel\OpenCL SDK\lib\x86"

OPTIMIZE=-O3
CXXFLAGS=$(OPTIMIZE) $(OPENCLINCLUDEPATH) -std=c++11 -Wall
LDFLAGS=
LDLIBS=$(OPENCLLIBPATH) -lOpenCL

.PHONY: all clean

SOURCEPATH=src
SOURCES=$(SOURCEPATH)/aabb.cpp \
		$(SOURCEPATH)/anisotropicparticlemesher.cpp \
		$(SOURCEPATH)/clscalarfield.cpp \
		$(SOURCEPATH)/collision.cpp \
		$(SOURCEPATH)/cuboidfluidsource.cpp \
		$(SOURCEPATH)/diffuseparticlesimulation.cpp \
		$(SOURCEPATH)/fluidbrickgrid.cpp \
		$(SOURCEPATH)/fluidbrickgridsavestate.cpp \
		$(SOURCEPATH)/fluidmaterialgrid.cpp \
		$(SOURCEPATH)/fluidsimulation.cpp \
		$(SOURCEPATH)/fluidsimulationsavestate.cpp \
		$(SOURCEPATH)/fluidsource.cpp \
		$(SOURCEPATH)/gridindexkeymap.cpp \
		$(SOURCEPATH)/gridindexvector.cpp \
		$(SOURCEPATH)/implicitpointprimitive.cpp \
		$(SOURCEPATH)/implicitsurfacescalarfield.cpp \
		$(SOURCEPATH)/interpolation.cpp \
		$(SOURCEPATH)/isotropicparticlemesher.cpp \
		$(SOURCEPATH)/levelset.cpp \
		$(SOURCEPATH)/logfile.cpp \
		$(SOURCEPATH)/macvelocityfield.cpp \
		$(SOURCEPATH)/main.cpp \
		$(SOURCEPATH)/particleadvector.cpp \
		$(SOURCEPATH)/polygonizer3d.cpp \
		$(SOURCEPATH)/pressuresolver.cpp \
		$(SOURCEPATH)/spatialpointgrid.cpp \
		$(SOURCEPATH)/sphericalfluidsource.cpp \
		$(SOURCEPATH)/stopwatch.cpp \
		$(SOURCEPATH)/trianglemesh.cpp \
		$(SOURCEPATH)/turbulencefield.cpp \
		$(SOURCEPATH)/vmath.cpp

OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=fluidsim

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) 
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LDLIBS) -o $@

clean:
	$(RM) $(SOURCEPATH)/*.o $(EXECUTABLE)
