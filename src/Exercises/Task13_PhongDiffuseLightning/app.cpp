//
// Created by pbialas on 25.09.2020.
//

#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "memory"

#include "Application/utils.h"

void SimpleShapeApplication::init() {

    auto program = xe::create_program(std::string(PROJECT_DIR) + "/shaders/base_vs.glsl",
                                      std::string(PROJECT_DIR) + "/shaders/base_fs.glsl");
    if (!program) {
        std::cerr << "Cannot create program from " << std::string(PROJECT_DIR) + "/shaders/base_vs.glsl" << " and ";
        std::cerr << std::string(PROJECT_DIR) + "/shaders/base_fs.glsl" << " shader files" << std::endl;
    }

    auto u_matrixes_index = glGetUniformBlockIndex(program, "Matrixes");
    if (u_matrixes_index == GL_INVALID_INDEX)
    {
        std::cout << "Cannot find Matrixes uniform block in program" << std::endl;
    } else { glUniformBlockBinding(program, u_matrixes_index, 0); }

    auto u_light_index = glGetUniformBlockIndex(program, "Light");
    if (u_light_index == GL_INVALID_INDEX)
    {
        std::cout << "Cannot find Modifiers uniform block in program" << std::endl;
    } else { glUniformBlockBinding(program, u_light_index, 1); }

    set_camera(new Camera);
    set_controler(new CameraControler(camera()));

//    glEnable(GL_CULL_FACE);
//    glFrontFace(GL_CW);
//    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    pyramid_ = std::make_shared<Pyramid>();

    light_.position = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    light_.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    light_.a = glm::vec4(1.0f, 0.0f, 1.0f, 0.0f);
    light_.ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);

    //*********************

    //Uniform buffer

    glGenBuffers(2,ubo_handle);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle[0]);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + sizeof(glm::mat<3,4,glm::f32>), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, ubo_handle[0]);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle[1]);
    glBufferData(GL_UNIFORM_BUFFER, 16 * sizeof(float), nullptr, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle[1]);

    //glClearColor(0.81f, 0.81f, 0.8f, 1.0f);
    int w, h;
    std::tie(w, h) = frame_buffer_size();

    camera_->perspective((glm::pi<float>()/4.0), (float)w/h, 0.1f, 100.0f);
    camera_->look_at(glm::vec3(0.1f,0.1f,2.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,0.0f,1.0f));
    glViewport(0, 0, w, h);
    //*********************

    //PVM
    glm::mat4 ModelMatrix = glm::mat4(1.0f);

    glUseProgram(program);

    auto  u_diffuse_map_location = glGetUniformLocation(program,"diffuse_map");
    if(u_diffuse_map_location==-1) {
        std::cerr<<"Cannot find uniform diffuse_map\n";
    } else {
        glUniform1ui(u_diffuse_map_location,0);
    }
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0,0,w,h);
    camera_->perspective((camera_->get_fov()), (float)w/h, 0.1f, 100.0f);
}

void SimpleShapeApplication::frame() {

    pyramid_->draw();

    auto P = camera()->projection();
    auto VM = camera()->view();
    auto R = glm::mat3(VM);
    auto N = glm::transpose(glm::inverse(R));
    auto lightP_VM = camera_->view() * light_.position;

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &P[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &VM[0]);

    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), 4 + sizeof(float), &N[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + 4 * sizeof(float), 4 * sizeof(float), &N[1]);
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4) + 8 * sizeof(float), 4 * sizeof(float), &N[2]);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glBindBuffer(GL_UNIFORM_BUFFER, ubo_handle[1]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 4 * sizeof(float), &lightP_VM[0]);
    glBufferSubData(GL_UNIFORM_BUFFER, 4 * sizeof(float), 4 *sizeof(float), &light_.color);
    glBufferSubData(GL_UNIFORM_BUFFER, 8 * sizeof(float), 4 *sizeof(float), &light_.a);
    glBufferSubData(GL_UNIFORM_BUFFER, 12 * sizeof(float), 4 *sizeof(float), &light_.ambient);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void SimpleShapeApplication::mouse_button_callback(int button, int action, int mods) {
    Application::mouse_button_callback(button, action, mods);

    if (controler_) {
        double x, y;
        glfwGetCursorPos(window_, &x, &y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            controler_->LMB_pressed(x, y);

        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
            controler_->LMB_released(x, y);
    }

}

void SimpleShapeApplication::cursor_position_callback(double x, double y) {
    Application::cursor_position_callback(x, y);
    if (controler_) {
        controler_->mouse_moved(x, y);
    }
}

void SimpleShapeApplication::cleanup() {

    delete &pyramid_;
}

