#pragma once

struct GLFWwindow;
struct WnData {
    unsigned int Width, Height;
    bool Running = true;
};

class Window {
   public:
    Window(const char *title, unsigned int width, unsigned int height, unsigned int flags = 0);
    ~Window();

    static Window *Get() { return s_Instance; }
    GLFWwindow *GetNativeWindow() { return m_Native; }
    unsigned int GetWidth() { return m_Data.Width; }
    unsigned int GetHeight() { return m_Data.Height; }
    WnData GetWindowData() { return m_Data; }
    bool IsRunning() { return m_Data.Running; }

    void SetRunning(bool status) { m_Data.Running = status; }

    void Update();

   private:
    static Window *s_Instance;
    WnData m_Data;
    GLFWwindow *m_Native;
};