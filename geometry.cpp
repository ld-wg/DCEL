#include "geometry.h"
#include <algorithm>

namespace Geometry {
    Orientation orientation(const Point& a, const Point& b, const Point& c) {
        long long val = (long long)(b.y - a.y) * (c.x - b.x) - 
                       (long long)(b.x - a.x) * (c.y - b.y);
        if (val == 0) return Orientation::COLINEAR;
        return (val > 0) ? Orientation::ANTIHORARIO : Orientation::HORARIO;
    }
    
    bool onSegment(const Point& p, const Point& q, const Point& r) {
        return (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
                q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y));
    }
    
    bool segmentsIntersect(const Point& p1, const Point& q1, 
                          const Point& p2, const Point& q2) {
        if (p1 == p2 || p1 == q2 || q1 == p2 || q1 == q2) {
            return false;
        }
        
        Orientation o1 = orientation(p1, q1, p2);
        Orientation o2 = orientation(p1, q1, q2);
        Orientation o3 = orientation(p2, q2, p1);
        Orientation o4 = orientation(p2, q2, q1);
        
        if (o1 != o2 && o3 != o4) {
            return true;
        }
        
        if (o1 == Orientation::COLINEAR && onSegment(p1, p2, q1)) return true;
        if (o2 == Orientation::COLINEAR && onSegment(p1, q2, q1)) return true;
        if (o3 == Orientation::COLINEAR && onSegment(p2, p1, q2)) return true;
        if (o4 == Orientation::COLINEAR && onSegment(p2, q1, q2)) return true;
        
        return false;
    }
} 