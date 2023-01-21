#include "Window.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <iostream>
#include "Input.hpp"

Window *Window::s_Instance = nullptr;

Window::Window(const char *title, unsigned int width, unsigned int height) {
    Window::s_Instance = this;
    m_Data.Width = width;
    m_Data.Height = height;

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_Native = glfwCreateWindow(m_Data.Width, m_Data.Height, title, nullptr, nullptr);
    glfwMakeContextCurrent(m_Native);
    glfwSetWindowUserPointer(m_Native, (void *)&m_Data);
    glfwSetInputMode(m_Native, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSetWindowCloseCallback(m_Native, [](GLFWwindow *window) {
        WnData *data = (WnData *)glfwGetWindowUserPointer(window);

        data->Running = false;
    });

    glfwSetWindowSizeCallback(m_Native, [](GLFWwindow *window, int width, int height) {
        WnData *data = (WnData *)glfwGetWindowUserPointer(window);

        data->Width = width;
        data->Height = height;
    });
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

Window::~Window() {
    glfwDestroyWindow(m_Native);
    glfwTerminate();
}

void Window::Update() {
    glViewport(0, 0, m_Data.Width, m_Data.Height);

    glfwPollEvents();
    glfwSwapBuffers(m_Native);
}