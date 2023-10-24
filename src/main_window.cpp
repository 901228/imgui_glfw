#define IMGUI_DEFINE_MATH_OPERATORS
#include "main_window.h"

#include <cstdio>

#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include "imgui_components/imgui_opengl.h"

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
void hide_taskbar_icon(GLFWwindow* win) {

    auto hwnd = glfwGetWin32Window(win);
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_TOOLWINDOW);
    // SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
}
void show_taskbar_icon(GLFWwindow* win) {

    auto hwnd = glfwGetWin32Window(win);
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) ^ WS_EX_TOOLWINDOW);
}
void setupWindowStyleNative(GLFWwindow* win) {

    auto hwnd = glfwGetWin32Window(win);
    // SetWindowLong(hwnd, GWL_STYLE, WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_GROUP);
    // SetWindowLong(hwnd, GWL_EXSTYLE, WS_EX_ACCEPTFILES | WS_EX_APPWINDOW);
}

MainWindow::MainWindow(bool isMultiViewport) {

    isReady = Init(isMultiViewport);
    if (isReady) Run();
}

MainWindow::~MainWindow() {

    if (isReady) Destroy();
}

static void glfw_error_callback(int error, const char* description) {

    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

static void glfw_windowSize_callback(GLFWwindow* window, int width, int height) {

    MainWindow* mw = (MainWindow*)glfwGetWindowUserPointer(window);
    mw->setScreenSize(width, height);
}

void MainWindow::setScreenSize(int width, int height) {

    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

bool MainWindow::Init(bool isMultiViewport) {

    // glfw initialization
    {
        glfwSetErrorCallback(glfw_error_callback);

        if (!glfwInit()) {

            fprintf(stderr, "glfw init failed\n");
            return false;
        }
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

        // glfw window creation
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL with ImGui", nullptr, nullptr);
        if (window == nullptr) {

            fprintf(stderr, "Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        setupWindowStyleNative(window);
        // calculateWindowBorders();

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, glfw_windowSize_callback);
    }

    // glad initialization
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {

        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwDestroyWindow(window);
        glfwTerminate();
        return false;
    }

    // GL initiation
    {
        glEnable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // glEnable(GL_PROGRAM_POINT_SIZE);
    }

    // ImGui initialization
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        if (isMultiViewport)
            io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable MultiViewports

        // Setup Dear ImGui style
        ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {

            fprintf(stderr, "Failed to ImGui_ImplGlfw_InitForOpenGL\n");
            glfwDestroyWindow(window);
            glfwTerminate();
            return false;
        }
        if (!ImGui_ImplOpenGL3_Init("#version 410")) {

            fprintf(stderr, "Failed to ImGui_ImplOpenGL3_Init\n");
            glfwDestroyWindow(window);
            glfwTerminate();
            return false;
        }
    }

    return true;
}

void MainWindow::Run() {

    // render
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();
        HandleUserInput();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui components
        CreateImGuiComponents();
        // ImGui::ShowDemoWindow();
        ImGui::Render();

        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {

            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);

        if (shouldWindowMaximize) {

            shouldWindowMaximize = false;

            if (!glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) glfwMaximizeWindow(window);
            else glfwRestoreWindow(window);
        }
    }
}

void MainWindow::Destroy() {

    // OnDestroy
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
}

void MainWindow::CreateImGuiComponents() {

    window_pos = (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ? ImVec2(ImGui::GetMainViewport()->Pos) : ImVec2(0, 0));

    CreateMenuBar();
    CreateMainView();
    CreateControlPanel();
    CreateSettingPage();

    if (isDragging) {

        ImVec2 newWindowPos = draggingWindowAnchor + (ImGui::GetMousePos() - draggingMouseAnchor);

        glfwSetWindowPos(window, newWindowPos.x, newWindowPos.y);
    }
}

void MainWindow::HandleUserInput() {

    ImVec2 pos = ImGui::GetMousePos();

    //TODO: handle user inputs
}

void MainWindow::CreateMenuBar() {

    if (ImGui::BeginMainMenuBar()) {

        // calculate window height for other windows
        menubar_offsetY = ImGui::GetWindowHeight();
        main_height = SCR_HEIGHT - menubar_offsetY;

        // leadings (left)
        {
            if (ImGui::MenuItem("Settings")) {

                isSettingPageOpened = !isSettingPageOpened;
            }
            ImGui::Separator();
        }

        std::pair<ImVec2, ImVec2> menubarRect = { { ImGui::GetWindowPos().x + ImGui::GetCursorPosX(), ImGui::GetWindowPos().y }, ImGui::GetWindowPos() + ImGui::GetWindowSize() };
        float rightCursorPosX = ImGui::GetWindowSize().x;

        // trailings (right)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, { 0, 0, 0, 0 });

            // close button
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0.76, 0.16, 0.1, 1 });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0.91, 0.19, 0.12, 1 });
            ImVec2 buttonSize{ ImGui::GetWindowSize().y, ImGui::GetWindowSize().y };
            rightCursorPosX -= buttonSize.x;
            menubarRect.second.x -= buttonSize.x;
            ImGui::SetCursorPosX(rightCursorPosX);
            if (ImGui::Button("x", buttonSize)) {

                glfwSetWindowShouldClose(window, true);
            }
            ImGui::PopStyleColor(2);

            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0, 0, 0, 0.2 });
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0, 0, 0, 0.3 });
            {
                // maximize button
                rightCursorPosX -= buttonSize.x;
                menubarRect.second.x -= buttonSize.x;
                ImGui::SetCursorPosX(rightCursorPosX);
                if (ImGui::Button("O", buttonSize)) {

                    shouldWindowMaximize = true;
                }

                // minimize button
                rightCursorPosX -= buttonSize.x;
                menubarRect.second.x -= buttonSize.x;
                ImGui::SetCursorPosX(rightCursorPosX);
                if (ImGui::Button("-", buttonSize)) {

                    if (!glfwGetWindowAttrib(window, GLFW_ICONIFIED)) glfwIconifyWindow(window);
                    else glfwRestoreWindow(window);
                }
            }
            ImGui::PopStyleColor(2);

            ImGui::PopStyleColor();
        }

        // Handle dragging window
        {
            if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {

                if (!isMouseDown && ImGui::IsMouseHoveringRect(menubarRect.first, menubarRect.second) || isDragging) {

                    if (!isDragging) {

                        int w_xpos, w_ypos;
                        glfwGetWindowPos(window, &w_xpos, &w_ypos);
                        draggingWindowAnchor = { static_cast<float>(w_xpos), static_cast<float>(w_ypos) };
                        // printf("%i, %i\n", w_xpos, w_ypos);

                        draggingMouseAnchor = ImGui::GetMousePos();
                    }

                    isDragging = true;
                }

                isMouseDown = true;
            }
            else {

                isDragging = false;
                isMouseDown = false;
            }
        }

        ImGui::EndMainMenuBar();
    }
}

void MainWindow::CreateMainView() {

    ImGui::SetNextWindowPos({ window_pos.x, window_pos.y + menubar_offsetY });
    ImGui::SetNextWindowSize({ main_width, main_height });
    if (ImGui::Begin("main view", 0, flag)) {

        if (ImGui::BeginTabBar("main view tab bar")) {

            if (ImGui::BeginTabItem("OpenGL view")) {

                if (ImGui::BeginOpenGL("OpenGL", ImGui::GetContentRegionAvail(), false, flag)) {

                    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    glUseProgram(0);
                    glColor3f(0.3f, 0.8f, 1.0f);
                    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
                    glBegin(GL_TRIANGLES);
                    {
                        glVertex2f(-1, 1);
                        glVertex2f(1, 1);
                        glVertex2f(1, -1);
                    }
                    glEnd();

                    ImGui::EndOpenGL();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void MainWindow::CreateControlPanel() {

    ImGui::SetNextWindowPos({ window_pos.x + main_width, window_pos.y + menubar_offsetY });
    ImGui::SetNextWindowSize({ SCR_WIDTH - main_width, main_height });
    if (ImGui::Begin("controls", 0, flag)) {

        ImGui::NewLine();
        const char* text = "Controls";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text).x) * 0.5f);
        ImGui::Text(text);
        ImGui::NewLine();

        if (ImGui::Button("btn1", { ImGui::GetContentRegionAvail().x, 0 })) {

            printf("btn1\n");
        }
        if (ImGui::Button("Hello World!", { ImGui::GetContentRegionAvail().x, 0 })) {

            printf("Hello World!\n");
        }
        if (ImGui::Button("hide_taskbar_icon", { ImGui::GetContentRegionAvail().x, 0 })) {

            FreeConsole();
            hide_taskbar_icon(this->window);
        }
        if (ImGui::Button("show_taskbar_icon", { ImGui::GetContentRegionAvail().x, 0 })) {

            show_taskbar_icon(this->window);
        }
        if (ImGui::Button("print GWL_EXSTYLE", { ImGui::GetContentRegionAvail().x, 0 })) {

            printf("GetWindowLong(hwnd, GWL_EXSTYLE) = %08x\n", GetWindowLong(glfwGetWin32Window(this->window), GWL_EXSTYLE));
        }
        if (ImGui::Button("print GWL_STYLE", { ImGui::GetContentRegionAvail().x, 0 })) {

            printf("GetWindowLong(hwnd, GWL_STYLE)   = %08x\n", GetWindowLong(glfwGetWin32Window(this->window), GWL_STYLE));
        }

        ImGui::NewLine();
        const char* text2 = "sliderInt";
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(text2).x) * 0.5f);
        ImGui::Text(text2);
        ImGui::PushItemWidth(-1);
        ImGui::SliderInt("sliderInt", &sliderInt, 1, 20);

        ImGui::End();
    }
}

void MainWindow::CreateSettingPage() {

    if (isSettingPageOpened && ImGui::Begin("Settings Page", &isSettingPageOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::SliderFloat("sliderFloat", &sliderFloat, 0.001f, 0.01f, "%.3f");

        ImGui::End();
    }
}
