//
// Created by pbialas on 05.08.2020.
//


#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <memory>
#include <chrono>


#include "Application/application.h"
#include "Application/utils.h"

#include "glad/glad.h"
#include "camera.h"
#include "camera_controler.h"
#include "pyramid.h"

class SimpleShapeApplication : public xe::Application {
public:
    SimpleShapeApplication(int width, int height, std::string title, int major = 4, int minor = 1) :
            Application(width, height, title, major, minor) {}

    void init() override;;

    void frame() override;

    void framebuffer_resize_callback(int w, int h) override;

    void mouse_button_callback(int button, int action, int mods) override;

    void cursor_position_callback(double x, double y) override;

    void set_camera(Camera *camera) { camera_ = camera; }
    Camera *camera() { return camera_; }
    ~SimpleShapeApplication() {
        if (camera_) {
            delete camera_;
        }

//        if(pyramid_) {
//            delete pyramid_;
//        }
    }

    void set_controler(CameraControler *controler) { controler_ = controler; }

    void scroll_callback(double xoffset, double yoffset) override {
        Application::scroll_callback(xoffset, yoffset);
        camera()->zoom(yoffset / 30.0f);
    }

    //void set_Pyramid(Pyramid *pyramid) { pyramid_ = std::make_shared<Pyramid>(); }
    //Pyramid *pyramid() { return pyramid_; }

private:
    GLuint vao_;

    //GLuint u_pvm_buffer_;
    GLuint ubo_handle[2];

    Camera *camera_;
    CameraControler *controler_;

    std::shared_ptr<Pyramid> pyramid_;

    std::chrono::steady_clock::time_point start_;
    float rotation_period;
    float orbital_rotation_period, x, y, a, b;

    std::shared_ptr<Pyramid> moon_;
    float r_moon;
    float rotation_period_moon;
    float orbital_rotation_period_moon;

    std::shared_ptr<Pyramid> satelite_;
    float r_sat;
    float rotation_period_sat;
    float orbital_rotation_period_sat;
};