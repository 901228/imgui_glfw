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

    // sub functions
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
    bool isDragging = false;
    bool isMouseDown = false;
    ImVec2 draggingWindowAnchor;
    ImVec2 draggingMouseAnchor;

    bool shouldWindowMaximize = false;

    void HandleTitleBarEvents();

    // variables
private:
    bool isSettingPageOpened = false;

    float sliderFloat = 0;
    int sliderInt = 0;

    // exposed functions
public:
    void setScreenSize(int width, int height);
};

#endif // !MAIN_WINDOW_H
