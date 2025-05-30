#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "DCEL.h"  // for Point structure

namespace Geometry {
    enum class Orientation {
        COLINEAR,    // pontos sao colineares
        HORARIO,     // sentido horario (clockwise)
        ANTIHORARIO  // sentido anti-horario (counter-clockwise)
    };

    // retorna orientacao de tres pontos
    Orientation orientation(const Point& a, const Point& b, const Point& c);
    
    bool onSegment(const Point& p, const Point& q, const Point& r);
    
    bool segmentsIntersect(const Point& p1, const Point& q1, const Point& p2, const Point& q2);
}

#endif