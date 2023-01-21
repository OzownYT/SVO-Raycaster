#include "Input.hpp"

#include "Window.hpp"
#include <GLFW/glfw3.h>

Input *Input::s_Instance = new Input;

bool Input::IsKeyPressed(int key) {
    auto window = Window::Get()->GetNativeWindow();
    return glfwGetKey(window, key);
}

bool Input::IsButtonPressed(int button) {
    auto window = Window::Get()->GetNativeWindow();
    return glfwGetKey(window, button);
}

glm::vec2 Input::GetMousePosition() {
    auto window = Window::Get()->GetNativeWindow();
    glm::dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    return pos;
}

glm::vec2 Input::GetMouseScroll() {
    auto window = Window::Get()->GetNativeWindow();
    glm::dvec2 offset;
}