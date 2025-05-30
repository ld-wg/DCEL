#include "DCEL.h"
#include "geometry.h"
#include <iostream>
#include <cstdio>

// estrategia de indexacao:
// - entrada: indices baseados em 1 (vertices 1, 2, 3, ...)
// - interno: indices baseados em 0 (arrays 0, 1, 2, ...)  
// - saida: indices baseados em 1 (conversao de volta)

// point methods
bool Point::operator==(const Point& other) const {
    return x == other.x && y == other.y;
}

bool Point::operator!=(const Point& other) const {
    return !(*this == other);
}

// half-edge methods
Vertex* HalfEdge::destination() const {
    return twin ? twin->origin : nullptr;
}

Point HalfEdge::getSegmentStart() const {
    return origin ? origin->position : Point();
}

Point HalfEdge::getSegmentEnd() const {
    return destination() ? destination()->position : Point();
}

bool DCEL::loadFromInput() {
    // le cabecalho: numero de vertices e faces
    int nVertices, nFaces;
    if (scanf("%d %d", &nVertices, &nFaces) != 2) {
        return false;
    }
    
    // pre-aloca memoria para melhor performance
    vertices.reserve(nVertices);
    for (int i = 0; i < nVertices; i++) {
        int x, y;
        if (scanf("%d %d", &x, &y) != 2) {
            return false;
        }
        vertices.push_back(std::make_unique<Vertex>(Point(x, y), i));
    }
    
    faces.reserve(nFaces);
    faceVertexIndices.reserve(nFaces);
    
    // le cada face como sequencia de vertices ate encontrar quebra de linha
    for (int i = 0; i < nFaces; i++) {
        faces.push_back(std::make_unique<Face>(i));
        
        std::vector<int> faceVertices;
        int vertexIndex;
        
        while (scanf("%d", &vertexIndex) == 1) {
            faceVertices.push_back(inputToInternal(vertexIndex)); // converte entrada 1-based para interno 0-based
            if (getchar() == '\n') break;
        }
        
        faceVertexIndices.push_back(std::move(faceVertices));
    }
    
    return constructDCEL();
}

bool DCEL::constructDCEL() {
    if (!createHalfEdges()) {
        return false;
    }
    linkHalfEdgeChains();
    return true;
}

bool DCEL::createHalfEdges() {
    // indice para identificacao unica de cada half-edge
    int halfEdgeIndex = 0;
    
    // itera sobre cada face para criar suas half-edges
    for (size_t faceIdx = 0; faceIdx < faces.size(); faceIdx++) {
        const auto& faceVertices = faceVertexIndices[faceIdx];
        size_t numVertices = faceVertices.size();
        
        if (numVertices < 3) {
            continue;
        }
        
        // cria half-edges para cada par de vertices consecutivos da face
        for (size_t i = 0; i < numVertices; i++) {
            int fromIdx = faceVertices[i];
            int toIdx = faceVertices[(i + 1) % numVertices];
            
            if (fromIdx < 0 || fromIdx >= (int)vertices.size() ||
                toIdx < 0 || toIdx >= (int)vertices.size()) {
                continue;
            }
            
            // chave para identificar arestas gemeas (twins)
            EdgeKey key(fromIdx, toIdx);
            
            auto halfEdge = std::make_unique<HalfEdge>();
            halfEdge->index = halfEdgeIndex++;
            halfEdge->origin = vertices[fromIdx].get();
            halfEdge->incidentFace = faces[faceIdx].get();
            
            // garante que cada vertice tenha pelo menos uma half-edge incidente
            if (!vertices[fromIdx]->incidentEdge) {
                vertices[fromIdx]->incidentEdge = halfEdge.get();
            }
            
            // define a primeira half-edge como componente externo da face
            if (!faces[faceIdx]->outerComponent) {
                faces[faceIdx]->outerComponent = halfEdge.get();
            }
            
            HalfEdge* hePtr = halfEdge.get();
            halfEdges.push_back(std::move(halfEdge));
            
            // mapeia half-edges para encontrar twins posteriormente
            if (edgeMap.find(key) == edgeMap.end()) {
                edgeMap[key] = {hePtr, nullptr};
            } else {
                auto& edgePair = edgeMap[key];
                if (edgePair.first->origin->index == fromIdx) {
                    edgePair.second = hePtr;
                } else {
                    edgePair.second = edgePair.first;
                    edgePair.first = hePtr;
                }
            }
        }
    }
    
    // conecta as half-edges gemeas (twins) usando o mapeamento criado
    for (auto& [key, edgePair] : edgeMap) {
        if (edgePair.first && edgePair.second) {
            edgePair.first->twin = edgePair.second;
            edgePair.second->twin = edgePair.first;
        }
    }
    
    return true;
}

void DCEL::linkHalfEdgeChains() {
    // conecta as half-edges de cada face em uma cadeia circular ordenada
    for (const auto& face : faces) {
        HalfEdge* start = face->outerComponent;
        if (!start) continue;
        
        HalfEdge* current = start;
        // previne loops infinitos em casos mal formados
        std::vector<HalfEdge*> faceEdges;
        
        do {
            faceEdges.push_back(current);
            
            // busca a proxima half-edge que comeca onde a atual termina
            HalfEdge* next = nullptr;
            Vertex* targetVertex = current->destination();
            
            for (const auto& he : halfEdges) {
                if (he->incidentFace == face.get() && 
                    he->origin == targetVertex) {
                    next = he.get();
                    break;
                }
            }
            
            // estabelece conexoes bidirecionais entre half-edges consecutivas
            current->next = next;
            if (next) {
                next->prev = current;
            }
            current = next;
            
            // continua ate fechar o ciclo ou detectar problema
        } while (current && current != start && 
                 faceEdges.size() < halfEdges.size());
    }
}

bool DCEL::isValidDCEL() const {
    if (hasOpenEdges()) {
        printf("aberta\n");
        return false;
    }
    
    if (isNonPlanarSubdivision()) {
        printf("não subdivisão planar\n");
        return false;
    }
    
    if (hasIntersectingFaces()) {
        printf("superposta\n");
        return false;
    }
    
    return true;
}

bool DCEL::hasOpenEdges() const {
    // verifica se a malha esta fechada checando propriedades das half-edges
    for (const auto& he : halfEdges) {
        // half-edge sem twin indica aresta de fronteira (malha aberta)
        if (!he->twin) {
            return true;
        }
        
        // relacao twin deve ser simetrica (twin do twin eh a propria half-edge)
        if (he->twin->twin != he.get()) {
            return true;
        }
        
        // twins devem pertencer a faces diferentes (propriedade fundamental do dcel)
        if (he->incidentFace == he->twin->incidentFace) {
            return true;
        }
    }
    return false;
}

bool DCEL::isNonPlanarSubdivision() const {
    // conta quantas faces cada aresta geometrica toca para validar planaridade
    std::map<EdgeKey, int> edgeFaceCount;
    
    for (const auto& he : halfEdges) {
        if (!he->origin || !he->destination()) continue;
        
        // normaliza aresta para chave unica independente da direcao
        EdgeKey key(he->origin->index, he->destination()->index);
        edgeFaceCount[key]++;
    }
    
    // em subdivisao planar valida cada aresta deve tocar exatamente 2 faces
    for (const auto& [key, count] : edgeFaceCount) {
        if (count != 2) {
            return true;
        }
    }
    return false;
}

bool DCEL::hasIntersectingFaces() const {
    for (size_t i = 0; i < halfEdges.size(); i++) {
        for (size_t j = i + 1; j < halfEdges.size(); j++) {
            const auto& he1 = halfEdges[i];
            const auto& he2 = halfEdges[j];
            
            // pula half-edges gemeas (mesma aresta em direcoes opostas, exemplo: a->b e b->a)
            if (he1->twin == he2.get() || he2->twin == he1.get()) {
                continue;
            }
                        
            // caso 1: ambas half-edges comecam no mesmo vertice
            // exemplo: he1: a->b, he2: a->c (mesmo vertice)
            if (he1->origin == he2->origin) continue;
            
            // caso 2: inicio de he1 eh o fim de he2
            // exemplo: he1: a->b, he2: c->a
            if (he1->origin == he2->destination()) continue;
            
            // caso 3: fim de he1 eh o inicio de he2
            // exemplo: he1: a->b, he2: b->c
            if (he1->destination() == he2->origin) continue;
            
            // caso 4: ambas half-edges terminam no mesmo vertice
            // exemplo: he1: a->b, he2: c->b
            if (he1->destination() == he2->destination()) continue;
            
            if (Geometry::segmentsIntersect(
                    he1->getSegmentStart(), he1->getSegmentEnd(),
                    he2->getSegmentStart(), he2->getSegmentEnd())) {
                return true;
            }
        }
    }
    return false;
}

void DCEL::printDCEL() const {
    printf("%zu %zu %zu\n", getVertexCount(), getEdgeCount(), getFaceCount());
    
    // converte indices internos 0-based para saida 1-based
    for (const auto& vertex : vertices) {
        printf("%d %d %d\n", 
               vertex->position.x, 
               vertex->position.y,
               vertex->incidentEdge ? internalToOutput(vertex->incidentEdge->index) : 1);
    }
    
    for (const auto& face : faces) {
        printf("%d\n", face->outerComponent ? internalToOutput(face->outerComponent->index) : 1);
    }
    
    for (const auto& he : halfEdges) {
        printf("%d %d %d %d %d\n",
               he->origin ? internalToOutput(he->origin->index) : 1,
               he->twin ? internalToOutput(he->twin->index) : 1,
               he->incidentFace ? internalToOutput(he->incidentFace->index) : 1,
               he->next ? internalToOutput(he->next->index) : 1,
               he->prev ? internalToOutput(he->prev->index) : 1);
    }
} 