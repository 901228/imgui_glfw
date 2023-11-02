#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ImGui/imgui.h>

#include <utility>

class MainWindow {

    // constructor
public:
    MainWindow(bool isMultiViewport = true);
    ~MainWindow();

    // main functions
private:
    GLFWwindow* window = nullptr;
    bool Init(bool isMultiViewport);
    void Destroy();

    void Run();
    bool isReady = false;

    // wrap glfw functions into ImVec2
private:
    ImVec2 GetWindowPos() const;
    ImVec2 GetWindowSize() const;
    ImVec2 GetMouseLocalPos() const;
    ImVec2 GetMouseGlobalPos() const;
    bool IsPosCorner(int xpos, int ypos) const;
    bool IsPosTop(int ypos) const;

    // imgui components
private:
    void CreateImGuiComponents();
    void HandleUserInput();

    void CreateMenuBar();
    void CreateMainView();
    void CreateControlPanel();
    void CreateSettingPage();

    // constants
private:
    unsigned int SCR_WIDTH = 1000;
    unsigned int SCR_HEIGHT = 800;

    const ImGuiWindowFlags flag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus;
    const ImGuiWindowFlags topFlag = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    // variables for layout
private:
    float menubar_offsetY = 20;
    // float main_width = 760;
#define main_width (SCR_WIDTH * 0.76f)
    float main_height = SCR_HEIGHT;

    ImVec2 window_pos{ 0, 0 };

    // for custom window title bar
private:
    bool isDraggingWindow = false;
    bool isDraggingMouseDown = false;
    ImVec2 draggingWindowAnchor;
    ImVec2 draggingMouseAnchor;
    void HandleTitleBarEvents();

    bool shouldWindowMaximize = false;
    bool readyToShowShadowOfMaximized = false;
    int shadowMonitor = 0;
    GLFWwindow* shadowWindow = nullptr;
    void RenderShadowWindow();

    // variables
private:
    bool isSettingPageOpened = false;

    float sliderFloat = 0;
    int sliderInt = 0;

    // exposed functions
public:
    static void glfw_error_callback(int error, const char* description);
    static void glfw_windowSize_callback(GLFWwindow* window, int width, int height);
    void setScreenSize(int width, int height);
    static void glfw_windowPos_callback(GLFWwindow* window, int xpos, int ypos);
    void setScreenPos(int xpos, int ypos);
};

#endif // !MAIN_WINDOW_H
