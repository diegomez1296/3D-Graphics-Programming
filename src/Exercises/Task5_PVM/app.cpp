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

    std::vector<GLfloat> vertices = {
            -0.4f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -0.4f, 0.3f, 0.0f, 0.0f, 1.0f, 1.0f,
            0.1f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,

            //-0.4f, 0.3f, 0.0f, 1.0f, 0.0f, 0.0f,
            //0.1f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.1f, 0.3f, 0.0f, 0.0f, 1.0f, 1.0f,

            //0.1f, 0.3f, 0.0f, 1.0f, 0.0f, 0.0f,
            //-0.4f, 0.3f, 0.0f, 1.0f, 0.0f, 0.0f,
            -0.1f, 0.6f, 0.0f, 0.0f, 1.0f, 1.0f
    };

    GLuint v_buffer_handle;
    glGenBuffers(1, &v_buffer_handle);
    glBindBuffer(GL_ARRAY_BUFFER, v_buffer_handle);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //*********************


    std::vector<GLushort> indices = {
            0,1,2,1,2,3,3,1,4,5 // wypisujemy tyle elementów ile mamy wierzchołków
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
    GLuint ubo_handle[2];
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
    glViewport(0, 0, w, h);
    //*********************

    //PVM
    //glm::mat4 ViewMatrix = glm::translate(glm::mat4(), glm::vec3(-3.0f, 0.0f ,0.0f));
    glm::mat4 ModelMatrix = glm::mat4(1.0f);
    glm::mat4 ViewMatrix = glm::lookAt(
            glm::vec3(0.2f,0.5f,1.0), // the position of your camera, in world space
            glm::vec3(0.01f,0.2f,0.01f),   // where you want to look at, in world space
            glm::vec3(0.0f,1.0f,0.0f)       // probably glm::vec3(0,1,0), but (0,-1,0) would make you looking upside-down, which can be great too
    );
    glm::mat4 ProjectionMatrix = glm::perspective(
            glm::radians(90.0f), // The vertical Field of View, in radians: the amount of "zoom". Think "camera lens". Usually between 90° (extra wide) and 30° (quite zoomed in)
            4.0f / 3.0f,       // Aspect Ratio. Depends on the size of your window. Notice that 4/3 == 800/600 == 1280/960, sounds familiar ?
            0.1f,              // Near clipping plane. Keep as big as possible, or you'll get precision issues.
            100.0f             // Far clipping plane. Keep as little as possible.
    );
    glm::mat4 pvm = ProjectionMatrix * ViewMatrix *ModelMatrix;

    //PVM buffer
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &pvm[0]);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle[1]);
    //glBindBuffer(GL_ARRAY_BUFFER, 0);

// --------------------

    glEnable(GL_DEPTH_TEST);
    glUseProgram(program);
}

void SimpleShapeApplication::frame() {
    glBindVertexArray(vao_);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_SHORT, reinterpret_cast<GLvoid *>(0));
    glBindVertexArray(0);
}
