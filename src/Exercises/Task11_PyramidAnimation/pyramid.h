#pragma once

#include "glad/glad.h"


class Pyramid  {
public:
    Pyramid();
    ~Pyramid();
    void draw();

    Pyramid(const Pyramid& rhs) = delete;
    Pyramid &operator=(const Pyramid& rhs) = delete;

    void operator=(Pyramid&& rhs) = delete;
    Pyramid(Pyramid&& rhs) = delete;
private:
    GLuint vao_;
    GLuint buffer_[2];
};
