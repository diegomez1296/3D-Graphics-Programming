//
// Created by pbialas on 25.09.2020.
//

#include "app.h"

#include <iostream>
#include <vector>
#include <tuple>
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"

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
    } else { glUniformBlockBinding(program, u_matrixes_index, 1); }

    set_camera(new Camera);
    set_controler(new CameraControler(camera()));

//    glEnable(GL_CULL_FACE);
//    glFrontFace(GL_CW);
//    glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);

    //set_Pyramid(new Pyramid);
    pyramid_ = std::make_shared<Pyramid>();
    //moon_ = std::make_shared<Pyramid>();
    //satelite_ = std::make_shared<Pyramid>();
    //Pyramid p = *pyramid_;

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

    camera_->perspective((glm::pi<float>()/2.0f), (float)w/h, 0.1f, 100.0f);
    camera_->look_at(glm::vec3(0.0f,0.0f,12.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f));
    glViewport(0, 0, w, h);
    //*********************

    //PVM
    glm::mat4 ModelMatrix = glm::mat4(1.0f);

    glUseProgram(program);

    //Animations
    rotation_period = 4.0f;
    rotation_period_moon = 10.0f;
    rotation_period_sat = 2.0f;

    start_ = std::chrono::steady_clock::now();

    orbital_rotation_period = 20.0f;
    orbital_rotation_period_moon = 10.0f;
    orbital_rotation_period_sat = 2.0f;
    a=10.0f;
    b=8.0f;

    r_moon = 3.0f;
    r_sat = 1.5f;

    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
}

void SimpleShapeApplication::framebuffer_resize_callback(int w, int h) {
    Application::framebuffer_resize_callback(w, h);
    glViewport(0,0,w,h);
    camera_->perspective((camera_->get_fov()), (float)w/h, 0.1f, 100.0f);
}

glm::mat4 rotation(float elapsed_time, float rotation_period, const glm::vec3 &axis)
{
    auto rotation_angle = 2.0f * glm::pi<float>() * elapsed_time / rotation_period;
    return glm::rotate(glm::mat4(1.0f), rotation_angle, axis);
}

glm::mat4 orbital_motion(float elapsed_time, float a, float b, float orbital_rotation_period)
{
    auto orbital_rotation_angle = 2.0f * glm::pi<float>() * elapsed_time / orbital_rotation_period;
    auto x = a * std::cos(orbital_rotation_angle);
    auto y = b * std::sin(orbital_rotation_angle);
    auto T = glm::translate(glm::mat4(1.0f), glm::vec3{x,y,0});
    return T;
}

void SimpleShapeApplication::frame() {

    auto now = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::duration<float>>(now - start_);
    auto seconds = elapsed_time.count();

    //Obrot piramidy
    auto R_earth = rotation(seconds, rotation_period, glm::vec3(0.0f,0.0f,1.0f));
    auto O_earth = orbital_motion(seconds, a, b, orbital_rotation_period);

    auto PVM = camera_->projection() * camera_->view() * O_earth * R_earth;


    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle[1]);

    pyramid_->draw();

    //Ksiezyc
    auto S_moon = glm::scale(glm::mat4(1.0f), {0.5f, 0.5f, 0.5f});
    auto R_moon = rotation(seconds, rotation_period_moon, glm::vec3(0.0f,0.0f,1.0f));
    auto O_moon = orbital_motion(seconds, r_moon, r_moon, orbital_rotation_period_moon);

    PVM = camera_->projection() * camera_->view() * O_earth * O_moon * R_moon * S_moon;

    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle[1]);

    pyramid_->draw();

    //Satelita
    auto S_sat = glm::scale(glm::mat4(1.0f), {0.2f, 0.2f, 0.2f});
    auto R_sat = rotation(seconds, rotation_period_sat, glm::vec3(0.0f,0.0f,1.0f));
    auto O_sat = orbital_motion(seconds, r_sat, r_sat, orbital_rotation_period_sat);

    auto tilt_sat = glm::rotate(glm::mat4(1.0f), glm::pi<float>() / 2.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    PVM = camera_->projection() * camera_->view() * O_earth * tilt_sat * O_sat * R_sat * S_sat;

    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &PVM[0]);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo_handle[1]);

    pyramid_->draw();
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



