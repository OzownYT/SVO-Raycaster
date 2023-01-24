#pragma once
#include <glm/glm.hpp>

class Input {
   public:
    static bool IsKeyDown(int key);
    static bool IsButtonDown(int button);
    static bool IsKeyPressed(int key);
    static bool IsButtonPressed(int button);
    static glm::vec2 GetMousePosition();
    static glm::vec2 GetMouseScroll();

   private:
    static Input *s_Instance;
};