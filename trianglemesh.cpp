#include "trianglemesh.h"


TriangleMesh::TriangleMesh()
{
}

TriangleMesh::~TriangleMesh()
{
}

int TriangleMesh::numVertices() {
    return (int)vertices.size();
}

int TriangleMesh::numFaces() {
    return (int)triangles.size();
}

void TriangleMesh::clear() {
    vertices.clear();
    normals.clear();
    triangles.clear();
    _vertexTriangles.clear();
}

// method of loading OBJ from:
// http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/
// .obj must be a closed watertight mesh with triangle with either shared triangle
// vertices in correct winding order, or vertices with pre-computed vertex normals.
bool TriangleMesh::loadOBJ(std::string filename, glm::vec3 offset, double scale) {
    clear();

    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;
    std::vector<Triangle> temp_triangles;

    FILE * file;
    fopen_s(&file, filename.c_str(), "rb");
    if( file == NULL ){
        printf("Unable to open the OBJ file!\n");
        return false;
    }

    while( true ){
        char lineHeader[128];
        // read the first word of the line
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF) {
            break; // EOF = End Of File. Quit the loop.
        }
        
        if ( strcmp( lineHeader, "v" ) == 0 ){
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z );
            temp_vertices.push_back((float)scale*vertex + offset);
        } else if (strcmp( lineHeader, "vn" ) == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z );
            temp_normals.push_back(normal);
        } else if ( strcmp( lineHeader, "f" ) == 0 ) {
            long start = ftell(file);
            unsigned int vertexIndex[3];
            unsigned int uvIndex[3];
            unsigned int normalIndex[3];
            int matches = fscanf(file, "%d %d %d\n", &vertexIndex[0], &vertexIndex[1], &vertexIndex[2]);

            if (matches != 3){
                long diff = ftell(file) - start;
                fseek (file, -diff , SEEK_CUR);
                start = ftell(file);
                matches = fscanf(file, "%d//%d %d//%d %d//%d\n", &vertexIndex[0], &normalIndex[0], 
                                                                 &vertexIndex[1], &normalIndex[1],
                                                                 &vertexIndex[2], &normalIndex[2]);
                if (matches != 6) {
                    long diff = ftell(file) - start;
                    fseek (file, -diff , SEEK_CUR);
                    matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", 
                                              &vertexIndex[0], &normalIndex[0], &uvIndex[0],
                                              &vertexIndex[1], &normalIndex[1], &uvIndex[1],
                                              &vertexIndex[2], &normalIndex[2], &uvIndex[2]);

                    if (matches != 9) {
                        printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                        return false;
                    }
                }
            }

            Triangle t = Triangle(vertexIndex[0] - 1,
                                  vertexIndex[1] - 1,
                                  vertexIndex[2] - 1);
            temp_triangles.push_back(t);
        }
    }

    fclose(file);

    vertices.insert(vertices.end(), temp_vertices.begin(), temp_vertices.end());
    triangles.insert(triangles.end(), temp_triangles.begin(), temp_triangles.end());
    removeDuplicateTriangles();

    if (normals.size() == vertices.size()) {
        normals.clear();
        normals.insert(normals.end(), normals.begin(), normals.end());
    } else {
        updateVertexNormals();
    }

    return true;
}

void TriangleMesh::writeMeshToOBJ(std::string filename) {
    assert(normals.size() == vertices.size());

    std::ostringstream str;

    str << "# OBJ file format with ext .obj" << std::endl;
    str << "# vertex count = " << vertices.size() << std::endl;
    str << "# face count = " << triangles.size() << std::endl;

    glm::vec3 p;
    for (int i = 0; i < vertices.size(); i++) {
        p = vertices[i];
        str << "v " << p.x << " " << p.y << " " << p.z << std::endl;
    }

    glm::vec3 n;
    for (int i = 0; i < normals.size(); i++) {
        n = normals[i];
        str << "vn " << n.x << " " << n.y << " " << n.z << std::endl;
    }

    Triangle t;
    int v1, v2, v3;
    for (int i = 0; i < triangles.size(); i++) {
        t = triangles[i];
        v1 = t.tri[0] + 1;
        v2 = t.tri[1] + 1;
        v3 = t.tri[2] + 1;

        str << "f " << v1 << "//" << v1 << " " <<
            v2 << "//" << v2 << " " <<
            v3 << "//" << v3 << std::endl;
    }

    std::ofstream out(filename);
    out << str.str();
    out.close();
}

bool triangleSort(const Triangle &a, const Triangle &b)
{
    if (a.tri[0]==b.tri[0]) {
        if (a.tri[1]==b.tri[1]) {
            return a.tri[2] < b.tri[2];
        } else {
            return a.tri[1] < b.tri[1];
        }
    } else {
        return a.tri[0] < b.tri[0];
    }
}

void TriangleMesh::removeDuplicateTriangles() {
    std::vector<Triangle> uniqueTriangles;

    std::sort(triangles.begin(), triangles.end(), triangleSort);
    Triangle last;
    for (int i = 0; i < triangles.size(); i++) {
        Triangle t = triangles[i];

        if (!_trianglesEqual(t, last)) {
            uniqueTriangles.push_back(t);
        }
        last = t;
    }

    triangles.clear();
    triangles.insert(triangles.end(), uniqueTriangles.begin(), uniqueTriangles.end());
}

void TriangleMesh::updateVertexNormals() {
    normals.clear();
    _updateVertexTriangles();
    
    std::vector<glm::vec3> facenormals;
    facenormals.reserve(triangles.size());
    Triangle t;
    glm::vec3 v1, v2;
    for (int i = 0; i < triangles.size(); i++) {
        t = triangles[i];

        v1 = vertices[t.tri[1]] - vertices[t.tri[0]];
        v2 = vertices[t.tri[2]] - vertices[t.tri[0]];
        glm::vec3 norm = glm::normalize(glm::cross(v1, v2));

        if (norm.x != norm.x || norm.y != norm.y || norm.y != norm.y ) {
            std::cout << "NaN: " << v1.x << " " << v1.y << " " << v1.z << std::endl;
            std::cout << "NaN: " << v2.x << " " << v2.y << " " << v2.z << std::endl;
        }

        facenormals.push_back(norm);
    }

    glm::vec3 n;
    for (int i = 0; i < _vertexTriangles.size(); i++) {
        n = glm::vec3(0.0, 0.0, 0.0);
        for (int j = 0; j < _vertexTriangles[i].size(); j++) {
            n += facenormals[_vertexTriangles[i][j]];
        }

        n = glm::normalize(n / (float)_vertexTriangles[i].size());
        normals.push_back(n);
    }
}

void TriangleMesh::getFaceNeighbours(Triangle t, std::vector<int> &n) {
    std::vector<int> vn;
    for (int i = 1; i < 3; i++) {
        vn = _vertexTriangles[t.tri[i]];
        n.insert(n.end(), vn.begin(), vn.end());
    }
}

bool TriangleMesh::_trianglesEqual(Triangle &t1, Triangle &t2) {
    return t1.tri[0] == t2.tri[0] &&
           t1.tri[1] == t2.tri[1] &&
           t1.tri[2] == t2.tri[2];
}

bool TriangleMesh::isNeighbours(Triangle t1, Triangle t2) {
    std::vector<int> n;
    getFaceNeighbours(t1, n);
    for (int i = 0; i < n.size(); i++) {
        if (_trianglesEqual(triangles[n[i]], t2)) {
            return true;
        }
    }

    return false;
}

void TriangleMesh::_updateVertexTriangles() {
    _vertexTriangles.clear();
    _vertexTriangles.reserve(vertices.size());

    for (int i = 0; i < vertices.size(); i++) {
        std::vector<int> triangles;
        triangles.reserve(14);  // 14 is the maximum number of adjacent triangles
                                // to a vertex
        _vertexTriangles.push_back(triangles);
    }

    Triangle t;
    for (int i = 0; i < triangles.size(); i++) {
        t = triangles[i];
        _vertexTriangles[t.tri[0]].push_back(i);
        _vertexTriangles[t.tri[1]].push_back(i);
        _vertexTriangles[t.tri[2]].push_back(i);
    }
}

GridIndex TriangleMesh::_positionToGridIndex(glm::vec3 p) {
    double invdx = 1.0 / _dx;
    return GridIndex((int)floor(p.x*invdx),
                     (int)floor(p.y*invdx),
                     (int)floor(p.z*invdx));
}

void TriangleMesh::_getTriangleGridCellOverlap(Triangle t, std::vector<GridIndex> &cells) {
    std::vector<GridIndex> testcells;
    AABB tbbox = AABB(t, vertices);
    tbbox.getOverlappingGridCells(_dx, testcells);

    AABB cbbox = AABB(glm::vec3(0.0, 0.0, 0.0), _dx, _dx, _dx);
    for (int i = 0; i < testcells.size(); i++) {
        cbbox.position = _gridIndexToPosition(testcells[i]);
        if (cbbox.isOverlappingTriangle(t, vertices)) {
            cells.push_back(testcells[i]);
        }
    }
}

void TriangleMesh::_updateTriangleGrid() {
    _destroyTriangleGrid();
    _triGrid = Array3d<std::vector<int>>(_gridi, _gridj, _gridk);

    std::vector<GridIndex> cells;
    std::vector<int> *triVector;
    Triangle t;
    GridIndex g;
    for (int i = 0; i < triangles.size(); i++) {
        t = triangles[i];
        cells.clear();
        _getTriangleGridCellOverlap(t, cells);

        for (int j = 0; j < cells.size(); j++) {
            g = cells[j];
            triVector = _triGrid.getPointer(g);
            triVector->push_back(i);
        }
    }
}

void TriangleMesh::_destroyTriangleGrid() {
    std::vector<int> *tris;
    for (int k = 0; k < _triGrid.depth; k++) {
        for (int j = 0; j < _triGrid.height; j++) {
            for (int i = 0; i < _triGrid.width; i++) {
                tris = _triGrid.getPointer(i, j, k);
                tris->clear();
                tris->shrink_to_fit();
            }
        }
    }
    _triGrid = Array3d<std::vector<int> >();
}

void TriangleMesh::_getSurfaceCells(std::vector<GridIndex> &cells) {
    std::vector<int> *tris;
    for (int k = 0; k < _triGrid.depth; k++) {
        for (int j = 0; j < _triGrid.height; j++) {
            for (int i = 0; i < _triGrid.width; i++) {
                tris = _triGrid.getPointer(i, j, k);
                if (tris->size() > 0) {
                    cells.push_back(GridIndex(i, j, k));
                }
            }
        }
    }
}

void TriangleMesh::_getNeighbourGridIndices6(GridIndex g, GridIndex n[6]) {
    n[0] = GridIndex(g.i-1, g.j, g.k);
    n[1] = GridIndex(g.i+1, g.j, g.k);
    n[2] = GridIndex(g.i, g.j-1, g.k);
    n[3] = GridIndex(g.i, g.j+1, g.k);
    n[4] = GridIndex(g.i, g.j, g.k-1);
    n[5] = GridIndex(g.i, g.j, g.k+1);
}

void TriangleMesh::_floodfill(GridIndex g, Array3d<bool> &cells) {
    assert(_isCellIndexInRange(g));
    if (cells(g)) {
        return;
    }

    Array3d<bool> isCellDone = Array3d<bool>(_gridi, _gridj, _gridk, false);
    std::queue<GridIndex> queue;
    queue.push(g);
    isCellDone.set(g, true);

    GridIndex gp;
    GridIndex ns[6];
    while (!queue.empty()) {
        gp = queue.front();
        queue.pop();

        _getNeighbourGridIndices6(gp, ns);
        for (int i = 0; i < 6; i++) {
            if (_isCellIndexInRange(ns[i]) && !cells(ns[i]) && !isCellDone(ns[i])) {
                isCellDone.set(ns[i], true);
                queue.push(ns[i]);
            }
        }

        cells.set(gp, true);
    }
}

void TriangleMesh::getTrianglePosition(unsigned int index, glm::vec3 tri[3]) {
    assert(index < triangles.size());

    Triangle t = triangles[index];
    int size = vertices.size();
    assert(t.tri[0] < size && t.tri[1] < size && t.tri[2] < size);

    tri[0] = vertices[t.tri[0]];
    tri[1] = vertices[t.tri[1]];
    tri[2] = vertices[t.tri[2]];
}

glm::vec3 TriangleMesh::getTriangleNormal(unsigned int index) {
    assert(index < triangles.size());

    Triangle t = triangles[index];
    int size = vertices.size();
    assert(t.tri[0] < size && t.tri[1] < size && t.tri[2] < size);

    return glm::normalize(normals[t.tri[0]] + normals[t.tri[1]] + normals[t.tri[2]]);
}

glm::vec3 TriangleMesh::getTriangleFaceDirection(unsigned int index) {
    assert(index < triangles.size());

    Triangle t = triangles[index];
    int size = vertices.size();
    assert(t.tri[0] < size && t.tri[1] < size && t.tri[2] < size);

    return normals[t.tri[0]] + normals[t.tri[1]] + normals[t.tri[2]];
}

glm::vec3 TriangleMesh::getTriangleCenter(unsigned int index) {
    assert(index < triangles.size());

    Triangle t = triangles[index];
    int size = vertices.size();
    assert(t.tri[0] < size && t.tri[1] < size && t.tri[2] < size);

    return (vertices[t.tri[0]] + vertices[t.tri[1]] + vertices[t.tri[2]]) / 3.0f;
}

bool TriangleMesh::_isOnTriangleEdge(double u, double v) {
    double eps = 10e-6*_dx;

    if (fabs(u) < eps) {
        return true;
    }

    if (fabs(v) < eps || fabs(u + v - 1.0) < eps) {
        return true;
    }

    return false;
}

bool TriangleMesh::_isTriangleInVector(int index, std::vector<int> &tris) {
    for (int i = 0; i < tris.size(); i++) {
        if (_trianglesEqual(triangles[index], triangles[tris[i]])) {
            return true;
        }
    }
    return false;
}

int TriangleMesh::_getIntersectingTrianglesInCell(GridIndex g, glm::vec3 p, glm::vec3 dir,
                                                  std::vector<int> &tris, bool *success) {
    if (_triGrid(g).size() == 0) {
        *success = true;
        return 0;
    }

    // There are cases where this method could return an incorrect number of
    // surface intersections. If a line intersects at exactly an edge or vertex,
    // the number of intersections could be counted incorrectly as 2 or 3.
    // If it is detected that a line has intersected with an edge or vertex,
    // mark *success as false and return 0
    std::vector<int> *indices = _triGrid.getPointer(g);
    glm::vec3 collision;
    glm::vec3 tri[3];
    double u, v;
    int numIntersections = 0;

    numIntersections = 0;
    for (int i = 0; i < indices->size(); i++) {
        getTrianglePosition(indices->at(i), tri);

        bool isIntersecting = Collision::lineIntersectsTriangle(p, dir, 
                                                                tri[0], tri[1], tri[2],
                                                                &collision, &u, &v);
        if (!isIntersecting) { continue; }

        if (_isOnTriangleEdge(u, v)) {
            *success = false;
            return 0;
        }

        if (!_isTriangleInVector(indices->at(i), tris)) {
            tris.push_back(indices->at(i));
            numIntersections++;
        }
    }

    *success = true;
    return numIntersections;
}

bool TriangleMesh::_isIntInVector(int v, std::vector<int> &ints) {
    for (int i = 0; i < ints.size(); i++) {
        if (ints[i] == v) {
            return true;
        }
    }
    return false;
}

bool TriangleMesh::_isCellInsideMesh(const GridIndex g) {
    // count how many intersections between point and edge of grid
    // even intersections: outside
    // odd intersections: inside
    assert(_isCellIndexInRange(g));
    assert(_triGrid(g).size() == 0);

    // Add a random jitter to the center position of the cell.
    // If the line position is exactly in the center, intersections
    // will be more likely to occur on triangle edges and the method
    // _getIntersectingTrianglesInCell method will choose to safely fail.
    // The likeliness of edge intersections is due to symmetries in the 
    // polygonization method. 
    double jit = 0.1*_dx;
    glm::vec3 jitter = glm::vec3(_randomFloat(-jit, jit),
                                 _randomFloat(-jit, jit),
                                 _randomFloat(-jit, jit));

    glm::vec3 p = _gridIndexToPosition(g) + 0.5f*glm::vec3(_dx, _dx, _dx) + jitter;
    glm::vec3 dir = glm::vec3(1.0, 0.0, 0.0);
    

    std::vector<int> allIntersections;
    std::vector<int> leftIntersections;
    std::vector<int> rightIntersections;
    std::vector<int> intersections;
    GridIndex n = GridIndex(g.i - 1, g.j, g.k);
    while (_isCellIndexInRange(n)) {
        intersections.clear();
        bool success;
        int num = _getIntersectingTrianglesInCell(n, p, dir, intersections, &success);
        if (!success) {
            std::cout << "Error finding cell intersections: " <<
                          n.i << " " << n.j << " " << n.k << std::endl;
            return false;
        }

        for (int i = 0; i < intersections.size(); i++) {
            int idx = intersections[i];
            if (!_isIntInVector(idx, allIntersections) && !_isTriangleInVector(idx, allIntersections)) {
                leftIntersections.push_back(idx);
                allIntersections.push_back(idx);
            }
        }
        n = GridIndex(n.i - 1, n.j, n.k);
    }

    n = GridIndex(g.i + 1, g.j, g.k);
    while (_isCellIndexInRange(n)) {
        intersections.clear();
        bool success;
        int num = _getIntersectingTrianglesInCell(n, p, dir, intersections, &success);
        
        if (!success) {
            std::cout << "Error finding cell intersections: " <<
                          n.i << " " << n.j << " " << n.k << std::endl;
            return false;
        }

        for (int i = 0; i < intersections.size(); i++) {
            int idx = intersections[i];
            if (!_isIntInVector(idx, allIntersections) && !_isTriangleInVector(idx, allIntersections)) {
                rightIntersections.push_back(idx);
                allIntersections.push_back(idx);
            }
        }
        n = GridIndex(n.i + 1, n.j, n.k);
    }

    assert(leftIntersections.size() % 2 == rightIntersections.size() % 2);

    return leftIntersections.size() % 2 == 1;
}

void TriangleMesh::getCellsInsideMesh(std::vector<GridIndex> &cells) {
    if (_gridi == 0 || _gridj == 0 || _gridk == 0) {
        return;
    }

    // find all cells that are on the surface boundary.
    // Iterate through surface cells and test if any of their
    // 6 neighbours are inside the mesh. If a cell is inside the mesh,
    // floodfill that region.

    _updateTriangleGrid();

    std::vector<GridIndex> surfaceCells;
    _getSurfaceCells(surfaceCells);

    Array3d<bool> insideCellGrid = Array3d<bool>(_gridi, _gridj, _gridk, false);
    insideCellGrid.set(surfaceCells, true);

    GridIndex neighbours[6];
    GridIndex n;
    for (int i = 0; i < surfaceCells.size(); i++) {
        _getNeighbourGridIndices6(surfaceCells[i], neighbours);
        for (int j = 0; j < 6; j++) {
            n = neighbours[j];
            if (_isCellIndexInRange(n) && !insideCellGrid(n) && _isCellInsideMesh(n)) {
                _floodfill(n, insideCellGrid);
                break;
            }
        }
    }
    
    for (int k = 0; k < _triGrid.depth; k++) {
        for (int j = 0; j < _triGrid.height; j++) {
            for (int i = 0; i < _triGrid.width; i++) {
                if (insideCellGrid(i, j, k)) {
                    cells.push_back(GridIndex(i, j, k));
                }
            }
        }
    }
    
    _destroyTriangleGrid();
}

void TriangleMesh::_smoothTriangleMesh(double value) {
    std::vector<glm::vec3> newvertices;
    newvertices.reserve(vertices.size());

    glm::vec3 v;
    glm::vec3 nv;
    glm::vec3 avg;
    Triangle t;
    for (int i = 0; i < vertices.size(); i++) {

        avg = glm::vec3(0.0, 0.0, 0.0);
        for (int j = 0; j < _vertexTriangles[i].size(); j++) {
            t = triangles[_vertexTriangles[i][j]];
            if (t.tri[0] != i) {
                avg += vertices[t.tri[0]];
            }
            if (t.tri[1] != i) {
                avg += vertices[t.tri[1]];
            }
            if (t.tri[2] != i) {
                avg += vertices[t.tri[2]];
            }
        }

        avg /= ((float)_vertexTriangles[i].size()*2.0);
        v = vertices[i];
        nv = v + (float)value * (avg - v);
        newvertices.push_back(nv);
    }

    vertices = newvertices;
}

void TriangleMesh::smooth(double value, int iterations) {
    value = value < 0.0 ? 0.0 : value;
    value = value > 1.0 ? 1.0 : value;

    _vertexTriangles.clear();
    _updateVertexTriangles();

    for (int i = 0; i < iterations; i++) {
        _smoothTriangleMesh(value);
    }

    _vertexTriangles.clear();
}
