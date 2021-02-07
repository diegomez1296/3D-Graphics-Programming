//
// Created by pbialas on 25.09.2020.
//

#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Application/utils.h"

void SimpleShapeApplication::init() {

    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glCullFace(GL_BACK);

    auto program = xe::create_program(std::string(PROJECT_DIR) + "/shaders/base_vs.glsl",
                                      std::string(PROJECT_DIR) + "/shaders/base_fs.glsl");
    if (!program) {
        std::cerr << "Cannot create program from " << std::string(PROJECT_DIR) + "/shaders/base_vs.glsl" << " and ";
        std::cerr << std::string(PROJECT_DIR) + "/shaders/base_fs.glsl" << " shader files" << std::endl;
    }

    auto u_modifiers_index = glGetUniformBlockIndex(program, "Modifiers");
    if (u_modifiers_index == GL_INVALID_INDEX)
    {
        std::cout << "Cannot find Modifiers uniform block in program" << std::endl;
    } else { glUniformBlockBinding(program, u_modifiers_index, 0); }

    auto u_matrixes_index = glGetUniformBlockIndex(program, "Matrixes");
    if (u_matrixes_index == GL_INVALID_INDEX)
    {
        std::cout << "Cannot find Matrixes uniform block in program" << std::endl;
    } else { glUniformBlockBinding(program, u_matrixes_index, 1); }

    glEnable(GL_DEPTH_TEST);

    std::vector<GLfloat> vertices = {
            -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f,
            0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, //LightBlue

            0.5f, 0.0f, -0.5f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, //Blue

            -0.5f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.5f, 0.0f, -0.5f, 1.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, //Red

            -0.5f, 0.0f, -0.5f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.0f, 0.5f, 0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f //Green
    };

    GLuint v_buffer_handle;
    glGenBuffers(1, &v_buffer_handle);
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //*********************


    std::vector<GLushort> indices = {
            6,7,8,0,9,2,1,10,11, 3,4,5// wypisujemy tyle elementów ile mamy wierzchołków
    };

    GLuint idx_buffer_handle;
    glGenBuffers(1,&idx_buffer_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buffer_handle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_); //*
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, idx_buffer_handle);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), reinterpret_cast<GLvoid *>(3 * sizeof(GLfloat)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    //*********************


    float strength = 0.9f;
    float light[3] = {0.7, 0.7, 0.7};

    //Uniform buffer

    glGenBuffers(2,ubo_handle);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle[0]);
    glBufferData(GL_UNIFORM_BUFFER, 8 * sizeof(float), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float),&strength);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(float), 3 * sizeof(float),light);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_handle[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle[1]);


    glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    int w, h;
    std::tie(w, h) = frame_buffer_size();

    aspect_ = (float)w/h;
    fov_ = glm::pi<float>()/4.0;
    near_ = 0.1f;
    far_ = 100.0f;
    P_ = glm::perspective(fov_, aspect_, near_, far_);
    V_ = glm::lookAt(glm::vec3(1.0f,1.0f,1.0f), glm::vec3(0.0f,0.3f,0.05f), glm::vec3(0.0f,1.0f,0.0f));
    glViewport(0, 0, w, h);
    //*********************

    //PVM
    glm::mat4 ModelMatrix = glm::mat4(1.0f);
//    glm::mat4 ViewMatrix = glm::lookAt(
//            glm::vec3(1.0f,1.0f,1.0f), // the position of your camera, in world space
//            glm::vec3(0.0f,0.3f,0.05f),   // where you want to look at, in world space
//            glm::vec3(0.0f,1.0f,0.0f)       // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
//    );
//    glm::mat4 ProjectionMatrix = glm::perspective(
//            glm::radians(90.0f), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
//            4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
//            0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
//            100.0f             // Far clipping plane. Keep as little as possible.
//    );
    glm::mat4 pvm = P_ * V_ * ModelMatrix;


    //PVM buffer
//    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
//    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &pvm[0]);
//    glBindBuffer(GL_UNIFORM_BUFFER, 0);
//    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle[1]);

// --------------------


    glUseProgram(program);

    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0,0,w,h);
    aspect_ = (float) w / h;
    P_ = glm::perspective(fov_, aspect_, near_, far_);
}

void SimpleShapeApplication::frame() {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(0));
    glBindVertexArray(0);

    auto PVM = P_ * V_;

    //glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle[1]);
}
