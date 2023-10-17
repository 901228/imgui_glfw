#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#define GL_SILENCE_DEPRECATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ImGui/imgui.h>

#define MULTIVIEWPORT

class MainWindow {

    // constructor
public:
    MainWindow();
    ~MainWindow();

    // main functions
private:
    GLFWwindow* window = nullptr;
    bool Init();
    void Run();
    void Destroy();
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
    const unsigned int SCR_WIDTH = 1000;
    const unsigned int SCR_HEIGHT = 800;

    const ImGuiWindowFlags flag = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus;
    const ImGuiWindowFlags topFlag = ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar;

    // variables for layout
private:
    float menubar_offsetY = 20;
    float main_width = 760;
    float main_height = SCR_HEIGHT;

    ImVec2 window_pos{ 0, 0 };

    // variables
private:
    bool isSettingPageOpened = false;

    float sliderFloat = 0;
    int sliderInt1 = 20;
    int sliderInt2 = 10;
};

#endif // !MAIN_WINDOW_H
