#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>

using namespace std;

struct Point {
    int x, y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
};

struct InputMesh {
    vector<Point> vertices;
    vector<vector<int>> faces;
    int nVertices, nFaces;
};

struct DCELVertex {
    Point p;
    int halfEdge;
};

struct DCELFace {
    int halfEdge;
};

struct DCELHalfEdge {
    int origin, twin, face, next, prev;
};

struct DCEL {
    vector<DCELVertex> vertices;
    vector<DCELFace> faces;
    vector<DCELHalfEdge> halfEdges;
    int nVertices, nEdges, nFaces;
    bool isValid;
};

class SVGDrawer {
private:
    double minX, maxX, minY, maxY;
    double width, height;
    double scale;
    double offsetX, offsetY;
    
public:
    SVGDrawer() : width(800), height(600) {}
    
    void calculateBounds(const vector<Point>& vertices) {
        if (vertices.empty()) return;
        
        minX = maxX = vertices[0].x;
        minY = maxY = vertices[0].y;
        
        for (const auto& v : vertices) {
            minX = min(minX, (double)v.x);
            maxX = max(maxX, (double)v.x);
            minY = min(minY, (double)v.y);
            maxY = max(maxY, (double)v.y);
        }
        
        // Add padding
        double padding = 50;
        double rangeX = maxX - minX;
        double rangeY = maxY - minY;
        
        if (rangeX == 0) rangeX = 1;
        if (rangeY == 0) rangeY = 1;
        
        scale = min((width - 2 * padding) / rangeX, (height - 2 * padding) / rangeY);
        
        offsetX = padding - minX * scale;
        offsetY = height - padding + minY * scale; // SVG Y is flipped
    }
    
    Point transform(const Point& p) {
        return Point(
            (int)(p.x * scale + offsetX),
            (int)(offsetY - p.y * scale)  // Flip Y coordinate
        );
    }
    
    void drawInputMesh(const InputMesh& mesh, const string& filename) {
        ofstream file(filename);
        
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        file << "<title>Input Mesh</title>\n";
        
        calculateBounds(mesh.vertices);
        
        // Draw faces
        for (size_t i = 0; i < mesh.faces.size(); i++) {
            const auto& face = mesh.faces[i];
            
            file << "<polygon points=\"";
            for (size_t j = 0; j < face.size(); j++) {
                Point p = transform(mesh.vertices[face[j] - 1]); // Convert from 1-indexed
                file << p.x << "," << p.y;
                if (j < face.size() - 1) file << " ";
            }
            file << "\" fill=\"rgba(" << (50 + i * 40) % 255 << "," << (100 + i * 60) % 255 << "," 
                 << (150 + i * 80) % 255 << ",0.3)\" stroke=\"black\" stroke-width=\"2\"/>\n";
        }
        
        // Draw vertices
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            Point p = transform(mesh.vertices[i]);
            file << "<circle cx=\"" << p.x << "\" cy=\"" << p.y << "\" r=\"5\" fill=\"red\"/>\n";
            file << "<text x=\"" << p.x + 8 << "\" y=\"" << p.y - 8 << "\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">" 
                 << (i + 1) << "</text>\n";
        }
        
        // Add legend
        file << "<text x=\"10\" y=\"30\" font-family=\"Arial\" font-size=\"16\" fill=\"black\" font-weight=\"bold\">Input Mesh</text>\n";
        file << "<text x=\"10\" y=\"50\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">Vertices: " << mesh.nVertices << "</text>\n";
        file << "<text x=\"10\" y=\"70\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">Faces: " << mesh.nFaces << "</text>\n";
        
        file << "</svg>\n";
        file.close();
        
        cout << "Input mesh drawn to: " << filename << endl;
    }
    
    void drawDCEL(const DCEL& dcel, const string& filename) {
        ofstream file(filename);
        
        file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
        file << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";
        file << "<title>DCEL Structure</title>\n";
        
        vector<Point> vertices;
        for (const auto& v : dcel.vertices) {
            vertices.push_back(v.p);
        }
        calculateBounds(vertices);
        
        // Draw half-edges
        for (size_t i = 0; i < dcel.halfEdges.size(); i++) {
            const auto& he = dcel.halfEdges[i];
            Point p1 = transform(dcel.vertices[he.origin - 1].p); // Convert from 1-indexed
            
            // Find destination vertex (origin of next half-edge)
            int nextHE = he.next - 1;
            Point p2 = transform(dcel.vertices[dcel.halfEdges[nextHE].origin - 1].p);
            
            // Draw edge
            file << "<line x1=\"" << p1.x << "\" y1=\"" << p1.y << "\" x2=\"" << p2.x << "\" y2=\"" << p2.y 
                 << "\" stroke=\"blue\" stroke-width=\"1.5\"/>\n";
            
            // Draw arrow to show direction
            double dx = p2.x - p1.x;
            double dy = p2.y - p1.y;
            double len = sqrt(dx*dx + dy*dy);
            if (len > 0) {
                dx /= len; dy /= len;
                double midX = p1.x + dx * len * 0.7;
                double midY = p1.y + dy * len * 0.7;
                double arrowSize = 8;
                
                file << "<polygon points=\"" 
                     << (midX + dx * arrowSize) << "," << (midY + dy * arrowSize) << " "
                     << (midX - dx * arrowSize + dy * arrowSize * 0.5) << "," << (midY - dy * arrowSize - dx * arrowSize * 0.5) << " "
                     << (midX - dx * arrowSize - dy * arrowSize * 0.5) << "," << (midY - dy * arrowSize + dx * arrowSize * 0.5)
                     << "\" fill=\"blue\"/>\n";
            }
            
            // Label half-edge
            double midX = (p1.x + p2.x) / 2;
            double midY = (p1.y + p2.y) / 2;
            file << "<text x=\"" << midX << "\" y=\"" << midY << "\" font-family=\"Arial\" font-size=\"10\" fill=\"darkblue\" text-anchor=\"middle\">" 
                 << (i + 1) << "</text>\n";
        }
        
        // Draw vertices
        for (size_t i = 0; i < dcel.vertices.size(); i++) {
            Point p = transform(dcel.vertices[i].p);
            file << "<circle cx=\"" << p.x << "\" cy=\"" << p.y << "\" r=\"6\" fill=\"red\" stroke=\"darkred\" stroke-width=\"2\"/>\n";
            file << "<text x=\"" << p.x + 10 << "\" y=\"" << p.y - 10 << "\" font-family=\"Arial\" font-size=\"12\" fill=\"black\" font-weight=\"bold\">" 
                 << (i + 1) << "</text>\n";
        }
        
        // Draw face labels
        for (size_t i = 0; i < dcel.faces.size(); i++) {
            // Find center of face by averaging vertices
            int startHE = dcel.faces[i].halfEdge - 1;
            int currentHE = startHE;
            vector<Point> faceVertices;
            
            do {
                faceVertices.push_back(dcel.vertices[dcel.halfEdges[currentHE].origin - 1].p);
                currentHE = dcel.halfEdges[currentHE].next - 1;
            } while (currentHE != startHE && faceVertices.size() < 20); // Safety check
            
            if (!faceVertices.empty()) {
                double centerX = 0, centerY = 0;
                for (const auto& v : faceVertices) {
                    centerX += v.x;
                    centerY += v.y;
                }
                centerX /= faceVertices.size();
                centerY /= faceVertices.size();
                
                Point center = transform(Point((int)centerX, (int)centerY));
                file << "<text x=\"" << center.x << "\" y=\"" << center.y << "\" font-family=\"Arial\" font-size=\"14\" fill=\"green\" font-weight=\"bold\" text-anchor=\"middle\">F" 
                     << (i + 1) << "</text>\n";
            }
        }
        
        // Add legend
        file << "<text x=\"10\" y=\"30\" font-family=\"Arial\" font-size=\"16\" fill=\"black\" font-weight=\"bold\">DCEL Structure</text>\n";
        file << "<text x=\"10\" y=\"50\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">Vertices: " << dcel.nVertices << "</text>\n";
        file << "<text x=\"10\" y=\"70\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">Edges: " << dcel.nEdges << "</text>\n";
        file << "<text x=\"10\" y=\"90\" font-family=\"Arial\" font-size=\"12\" fill=\"black\">Faces: " << dcel.nFaces << "</text>\n";
        file << "<text x=\"10\" y=\"110\" font-family=\"Arial\" font-size=\"12\" fill=\"blue\">Blue arrows: Half-edges</text>\n";
        file << "<text x=\"10\" y=\"130\" font-family=\"Arial\" font-size=\"12\" fill=\"red\">Red circles: Vertices</text>\n";
        file << "<text x=\"10\" y=\"150\" font-family=\"Arial\" font-size=\"12\" fill=\"green\">Green labels: Faces</text>\n";
        
        file << "</svg>\n";
        file.close();
        
        cout << "DCEL drawn to: " << filename << endl;
    }
};

InputMesh readInput() {
    InputMesh mesh;
    
    cin >> mesh.nVertices >> mesh.nFaces;
    
    mesh.vertices.resize(mesh.nVertices);
    for (int i = 0; i < mesh.nVertices; i++) {
        cin >> mesh.vertices[i].x >> mesh.vertices[i].y;
    }
    
    mesh.faces.resize(mesh.nFaces);
    string line;
    getline(cin, line); // consume newline
    
    for (int i = 0; i < mesh.nFaces; i++) {
        getline(cin, line);
        stringstream ss(line);
        int vertex;
        while (ss >> vertex) {
            mesh.faces[i].push_back(vertex);
        }
    }
    
    return mesh;
}

DCEL runMalhaAndParseDCEL(const InputMesh& mesh) {
    DCEL dcel;
    dcel.isValid = false;
    
    // Write input to temporary file
    ofstream tempInput("temp_input.txt");
    tempInput << mesh.nVertices << " " << mesh.nFaces << "\n";
    for (const auto& v : mesh.vertices) {
        tempInput << v.x << " " << v.y << "\n";
    }
    for (const auto& face : mesh.faces) {
        for (size_t i = 0; i < face.size(); i++) {
            tempInput << face[i];
            if (i < face.size() - 1) tempInput << " ";
        }
        tempInput << "\n";
    }
    tempInput.close();
    
    // Run malha program
    int result = system("./malha < temp_input.txt > temp_output.txt 2>&1");
    
    if (result == 0) {
        // Read output
        ifstream output("temp_output.txt");
        string firstLine;
        getline(output, firstLine);
        
        // Check if it's an error message
        if (firstLine == "aberta" || firstLine == "não subdivisão planar" || firstLine == "superposta") {
            cout << "Mesh validation failed: " << firstLine << endl;
            output.close();
            system("rm -f temp_input.txt temp_output.txt");
            return dcel;
        }
        
        // Parse DCEL output
        stringstream ss(firstLine);
        ss >> dcel.nVertices >> dcel.nEdges >> dcel.nFaces;
        
        dcel.vertices.resize(dcel.nVertices);
        dcel.faces.resize(dcel.nFaces);
        dcel.halfEdges.resize(dcel.nEdges * 2);
        
        // Read vertices
        for (int i = 0; i < dcel.nVertices; i++) {
            output >> dcel.vertices[i].p.x >> dcel.vertices[i].p.y >> dcel.vertices[i].halfEdge;
        }
        
        // Read faces
        for (int i = 0; i < dcel.nFaces; i++) {
            output >> dcel.faces[i].halfEdge;
        }
        
        // Read half-edges
        for (int i = 0; i < dcel.nEdges * 2; i++) {
            output >> dcel.halfEdges[i].origin >> dcel.halfEdges[i].twin 
                   >> dcel.halfEdges[i].face >> dcel.halfEdges[i].next >> dcel.halfEdges[i].prev;
        }
        
        dcel.isValid = true;
        output.close();
    }
    
    // Clean up temporary files
    system("rm -f temp_input.txt temp_output.txt");
    
    return dcel;
}

int main() {
    cout << "Reading input mesh..." << endl;
    InputMesh mesh = readInput();
    
    SVGDrawer drawer;
    
    // Draw input mesh
    drawer.drawInputMesh(mesh, "input_mesh.svg");
    
    // Try to generate and draw DCEL
    cout << "Running malha to generate DCEL..." << endl;
    DCEL dcel = runMalhaAndParseDCEL(mesh);
    
    if (dcel.isValid) {
        cout << "DCEL is valid! Drawing DCEL structure..." << endl;
        drawer.drawDCEL(dcel, "dcel_structure.svg");
        
        cout << "\nFiles generated:" << endl;
        cout << "- input_mesh.svg: Original mesh visualization" << endl;
        cout << "- dcel_structure.svg: DCEL structure visualization" << endl;
    } else {
        cout << "DCEL is not valid. Only input mesh was drawn." << endl;
        cout << "\nFile generated:" << endl;
        cout << "- input_mesh.svg: Original mesh visualization" << endl;
    }
    
    cout << "\nOpen the SVG files in a web browser to view the drawings." << endl;
    
    return 0;
} 