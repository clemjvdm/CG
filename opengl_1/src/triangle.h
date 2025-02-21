#ifndef TRIANGLE_H
#define TRIANGLE_H

#endif // TRIANGLE_H

struct Vertex {
    float x;
    float y;
    float z;
    float red;
    float green;
    float blue;

    Vertex(float px, float py, float pz, float pred, float pgreen, float pblue) {
        x = px;
        y = py;
        z = pz;
        red = pred;
        blue = pblue;
        green = pgreen;
    }

};
