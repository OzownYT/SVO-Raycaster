#include "Input.hpp"

#include "Window.hpp"
#include <GLFW/glfw3.h>

Input *Input::s_Instance = new Input;

bool Input::IsKeyDown(int key) {
    auto window = Window::Get()->GetNativeWindow();
    return glfwGetKey(window, key);
}

bool Input::IsButtonDown(int button) {
    auto window = Window::Get()->GetNativeWindow();
    return glfwGetKey(window, button);
}

bool Input::IsKeyPressed(int key) {
    static bool held = false;
    auto window = Window::Get()->GetNativeWindow();
    if (glfwGetKey(window, key)) {
        if (held) {
            return false;
        }
        held = true;
        return true;
    } else {
        held = false;
        return false;
    }
}

bool Input::IsButtonPressed(int button) {
    static bool held = false;
    auto window = Window::Get()->GetNativeWindow();
    if (glfwGetMouseButton(window, button)) {
        if (held) {
            return false;
        }
        held = true;
        return true;
    } else {
        held = false;
        return false;
    }
}

glm::vec2 Input::GetMousePosition() {
    auto window = Window::Get()->GetNativeWindow();
    glm::dvec2 pos;
    glfwGetCursorPos(window, &pos.x, &pos.y);
    return pos;
}

glm::vec2 Input::GetMouseScroll() {
    auto window = Window::Get()->GetNativeWindow();
    glm::dvec2 offset(0);
    return offset;
}