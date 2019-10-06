#ifndef VERTEX_H
#define VERTEX_H
// ////////////////////////////////////////////////////// Struct: Vertex //
struct Vertex {
    // ============================================= Public interface == //
    // ------------------------------------------------------ Data -- == //
    // ............................................... Position .. -- == //
    float x, y, z;
    // ................................................ Texture .. -- == //
    float u, v;
    // ------------------------------------------------- Behaviour -- == //
    // .............................................. Operators .. -- == //
    Vertex operator+(Vertex const &vertex) const {
        return {x + vertex.x,
                y + vertex.y,
                z + vertex.z,
                u + vertex.u,
                v + vertex.v};
    }
    Vertex operator/(float const divisor) const {
        return {x / divisor,
                y / divisor,
                z / divisor,
                u / divisor,
                v / divisor};
    }
};
// ///////////////////////////////////////////////////////////////////// //
#endif // VERTEX_H
