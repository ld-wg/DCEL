#ifndef DCEL_H
#define DCEL_H

#include <vector>
#include <map>
#include <memory>
#include <utility>

// forward declarations
struct Vertex;
struct Face;
struct HalfEdge;

struct Point {
    int x, y;
    
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
    
    bool operator==(const Point& other) const;
    bool operator!=(const Point& other) const;
};

struct Vertex {
    Point position;
    int index;
    HalfEdge* incidentEdge;
    
    Vertex() : index(-1), incidentEdge(nullptr) {}
    Vertex(const Point& p, int idx) : position(p), index(idx), incidentEdge(nullptr) {}
};

struct Face {
    int index;
    HalfEdge* outerComponent;
    
    Face() : index(-1), outerComponent(nullptr) {}
    Face(int idx) : index(idx), outerComponent(nullptr) {}
};

struct HalfEdge {
    int index;
    Vertex* origin;
    HalfEdge* twin;
    Face* incidentFace;
    HalfEdge* next;
    HalfEdge* prev;
    
    HalfEdge() : index(-1), origin(nullptr), twin(nullptr), 
                 incidentFace(nullptr), next(nullptr), prev(nullptr) {}
    
    Vertex* destination() const;
    Point getSegmentStart() const;
    Point getSegmentEnd() const;
};

class DCEL {
private:
    struct EdgeKey {
        int from, to;
        EdgeKey(int f, int t) : from(std::min(f, t)), to(std::max(f, t)) {}
        bool operator<(const EdgeKey& other) const {
            return from < other.from || (from == other.from && to < other.to);
        }
    };

    std::vector<std::unique_ptr<Vertex>> vertices;
    std::vector<std::unique_ptr<Face>> faces;
    std::vector<std::unique_ptr<HalfEdge>> halfEdges;
    std::vector<std::vector<int>> faceVertexIndices;
    std::map<EdgeKey, std::pair<HalfEdge*, HalfEdge*>> edgeMap;

private:
    // conversao de indices: entrada usa 1-based, interno usa 0-based
    static int inputToInternal(int inputIndex) { return inputIndex - 1; }
    static int internalToOutput(int internalIndex) { return internalIndex + 1; }
    
    bool constructDCEL();
    bool createHalfEdges();
    void linkHalfEdgeChains();
    bool hasOpenEdges() const;
    bool isNonPlanarSubdivision() const;
    bool hasIntersectingFaces() const;

public:
    DCEL() = default;
    ~DCEL() = default;

    bool loadFromInput();
    void printDCEL() const;
    bool isValidDCEL() const;

    size_t getVertexCount() const { return vertices.size(); }
    size_t getFaceCount() const { return faces.size(); }
    size_t getHalfEdgeCount() const { return halfEdges.size(); }
    size_t getEdgeCount() const { return halfEdges.size() / 2; }
};

#endif // DCEL_H 