#define IMGUI_DEFINE_MATH_OPERATORS
#include "main_window.h"

#include <cstdio>
#include <string>
#include <set>

#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include "imgui_components/imgui_opengl.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

MainWindow::MainWindow(bool isMultiViewport) {

    isReady = Init(isMultiViewport);
    if (isReady) Run();
}

MainWindow::~MainWindow() {

    if (isReady) Destroy();
}

void MainWindow::glfw_error_callback(int error, const char* description) {

    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

void MainWindow::glfw_windowSize_callback(GLFWwindow* window, int width, int height) {

    MainWindow* mw = (MainWindow*)glfwGetWindowUserPointer(window);
    if (window == mw->window) mw->setScreenSize(width, height);
}

void MainWindow::setScreenSize(int width, int height) {

    SCR_WIDTH = width;
    SCR_HEIGHT = height;
}

void MainWindow::glfw_windowPos_callback(GLFWwindow* window, int xpos, int ypos) {

    MainWindow* mw = (MainWindow*)glfwGetWindowUserPointer(window);
    if (window == mw->window) mw->setScreenPos(xpos, ypos);
}

void MainWindow::setScreenPos(int xpos, int ypos) {

    // restore window when drag window from the top
    if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED) && !IsPosCorner(xpos, ypos)) {

        // get mouse pos
        ImVec2 mouse = GetMouseLocalPos();

        // get screen size
        ImVec2 screenSize = GetWindowSize();

        glfwRestoreWindow(window);

        // set window to cursor
        ImVec2 windowSize = GetWindowSize();

        // calculate the relative distance from lefttop of the window and the mouse cursor
        ImVec2 relativeDistance = (mouse / screenSize) * windowSize;

        // reset the anchors of dragging window
        draggingWindowAnchor = GetMouseGlobalPos() - relativeDistance;
        draggingMouseAnchor = ImGui::GetMousePos();
        glfwSetWindowPos(window, static_cast<int>(draggingWindowAnchor.x), static_cast<int>(draggingWindowAnchor.y));
    }
    else {

        // drag to the top
        if (!glfwGetWindowAttrib(window, GLFW_MAXIMIZED) && IsPosTop(GetMouseGlobalPos().y)) {

            readyToShowShadowOfMaximized = true;
        }
        else readyToShowShadowOfMaximized = false;
    }
}

ImVec2 MainWindow::GetWindowPos() const {

    int xpos, ypos;
    glfwGetWindowPos(window, &xpos, &ypos);

    return { static_cast<float>(xpos), static_cast<float>(ypos) };
}

ImVec2 MainWindow::GetWindowSize() const {

    int width, height;
    glfwGetWindowSize(window, &width, &height);

    return { static_cast<float>(width), static_cast<float>(height) };
}

ImVec2 MainWindow::GetMouseLocalPos() const {

    double cursorXpos, cursorYpos;
    glfwGetCursorPos(window, &cursorXpos, &cursorYpos);

    return { static_cast<float>(cursorXpos), static_cast<float>(cursorYpos) };
}

ImVec2 MainWindow::GetMouseGlobalPos() const {

    double cursorXpos, cursorYpos;
    glfwGetCursorPos(window, &cursorXpos, &cursorYpos);

    int xpos, ypos;
    glfwGetWindowPos(window, &xpos, &ypos);

    return { static_cast<float>(xpos + cursorXpos), static_cast<float>(ypos + cursorYpos) };
}

bool MainWindow::IsPosCorner(int xpos, int ypos) const {

    int monitorCounts;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCounts);
    for (int i = 0; i < monitorCounts; i++) {

        int monitorXpos, monitorYpos;
        glfwGetMonitorPos(monitors[i], &monitorXpos, &monitorYpos);
        if (xpos == monitorXpos && ypos == monitorYpos) return true;
    }

    return false;
}

bool MainWindow::IsPosTop(int ypos) const {

    int monitorCounts;
    GLFWmonitor** monitors = glfwGetMonitors(&monitorCounts);
    for (int i = 0; i < monitorCounts; i++) {

        int monitorXpos, monitorYpos;
        glfwGetMonitorPos(monitors[i], &monitorXpos, &monitorYpos);
        if (ypos == monitorYpos) return true;
    }

    return false;
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
        // glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
        glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

        // glfw window creation
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL with ImGui", nullptr, nullptr);
        if (window == nullptr) {

            fprintf(stderr, "Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }

        // set window icon
        GLFWimage image;
        image.pixels = stbi_load(PROJECT_DIR "/asset/icon/strawberries.png", &image.width, &image.height, 0, 4);
        if (image.pixels) {

            glfwSetWindowIcon(window, 1, &image);
            stbi_image_free(image.pixels);
        }
        glfwShowWindow(window);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync

        glfwSetWindowUserPointer(window, this);
        glfwSetWindowSizeCallback(window, glfw_windowSize_callback);
        glfwSetWindowPosCallback(window, glfw_windowPos_callback);
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
        glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX); // rgb is add, alpha is max
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

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
        io.ConfigViewportsNoTaskBarIcon = true;

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

    // monitors' info
    if (false) {

        int monitorCounts;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCounts);
        for (int i = 0; i < monitorCounts; i++) {

            float xscale, yscale;
            glfwGetMonitorContentScale(monitors[i], &xscale, &yscale);
            printf("scale: (%g, %g)\n", xscale, yscale);

            printf("name: %s\n", glfwGetMonitorName(monitors[i]));

            int pwidth, pheight;
            glfwGetMonitorPhysicalSize(monitors[i], &pwidth, &pheight);
            printf("phsical size: (%i, %i)\n", pwidth, pheight);

            int pxpos = 0, pypos = 0;
            glfwGetMonitorPos(monitors[i], &pxpos, &pypos);
            printf("pos: (%i, %i)\n", pxpos, pypos);

            int wwidth, wheight, wxpos, wypos;
            glfwGetMonitorWorkarea(monitors[i], &wxpos, &wypos, &wwidth, &wheight);
            printf("work size: (%i, %i)\n", wwidth, wheight);
            printf("work pos: (%i, %i)\n\n", wxpos, wypos);
        }
    }

    return true;
}

static std::set<unsigned int> transparentBackgroundSet;

void MainWindow::Run() {

    // render
    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        RenderShadowWindow();

        glfwMakeContextCurrent(window);
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
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {

            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }

        glfwSwapBuffers(window);

        HandleTitleBarEvents();
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

        // Handle mouse event
        {
            bool isMouseHovering = ImGui::IsMouseHoveringRect(menubarRect.first, menubarRect.second);
            // Handle double click titlebar maximize
            if (isMouseHovering && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) shouldWindowMaximize = true;
            // Handle dragging window
            else if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {

                if (!isDraggingMouseDown && isMouseHovering || isDraggingWindow) {

                    if (!isDraggingWindow) {

                        isDraggingMouseDown = true;

                        int w_xpos, w_ypos;
                        glfwGetWindowPos(window, &w_xpos, &w_ypos);
                        draggingWindowAnchor = { static_cast<float>(w_xpos), static_cast<float>(w_ypos) };

                        draggingMouseAnchor = ImGui::GetMousePos();
                    }

                    isDraggingWindow = true;
                }

                isDraggingMouseDown = true;
            }
            else {

                isDraggingWindow = false;
                isDraggingMouseDown = false;
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

void MainWindow::HandleTitleBarEvents() {

    if (shouldWindowMaximize) {

        shouldWindowMaximize = false;

        if (!glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) glfwMaximizeWindow(window);
        else glfwRestoreWindow(window);
    }

    if (isDraggingWindow) {

        ImVec2 newWindowPos = draggingWindowAnchor + (ImGui::GetMousePos() - draggingMouseAnchor);

        glfwSetWindowPos(window, newWindowPos.x, newWindowPos.y);
    }

    // maximize window while mouse release
    if (readyToShowShadowOfMaximized && ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {

        readyToShowShadowOfMaximized = false;
        shouldWindowMaximize = true;
    }
}

void MainWindow::RenderShadowWindow() {

    // show shadow
    if (readyToShowShadowOfMaximized) {

        int monitorCounts;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCounts);
        int monitorWidth = 0, monitorHeight = 0;
        int monitorXpos = 0, monitorYpos = 0;
        float monitorXscale = 0, monitorYscale = 0;
        ImVec2 mouse = GetMouseGlobalPos();
        int i;
        for (i = 0; i < monitorCounts; i++) {

            glfwGetMonitorWorkarea(monitors[i], &monitorXpos, &monitorYpos, &monitorWidth, &monitorHeight);
            glfwGetMonitorContentScale(monitors[i], &monitorXscale, &monitorYscale);
            if (mouse.x >= monitorXpos && mouse.x < monitorXpos + monitorWidth && mouse.y >= monitorYpos && mouse.y < monitorYpos + monitorHeight) {

                break;
            }
        }

        if (!shadowWindow) {

            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
            glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
            glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);

            // glfw window creation
            shadowWindow = glfwCreateWindow(monitorWidth / monitorXscale, monitorHeight / monitorYscale, "shadow", nullptr, nullptr);
            glfwSetWindowPos(shadowWindow, monitorXpos, monitorYpos);

            LONG ex_style = ::GetWindowLong(glfwGetWin32Window(shadowWindow), GWL_EXSTYLE);
            ex_style &= ~WS_EX_APPWINDOW;
            ex_style |= WS_EX_TOOLWINDOW;
            ::SetWindowLong(glfwGetWin32Window(shadowWindow), GWL_EXSTYLE, ex_style);
        }

        if (i != shadowMonitor) {

            glfwHideWindow(shadowWindow);

            shadowMonitor = i;
            glfwSetWindowPos(shadowWindow, monitorXpos, monitorYpos);
            glfwSetWindowSize(shadowWindow, monitorWidth, monitorHeight);
        }

        if (!glfwGetWindowAttrib(shadowWindow, GLFW_VISIBLE)) glfwShowWindow(shadowWindow);

        glfwMakeContextCurrent(shadowWindow);

        glClearColor(0.0f, 0.0f, 0.0f, 0.3f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // render opengl
        if (false) {
            glUseProgram(0);
            glColor3f(0.3f, 0.8f, 1.0f);
            glLineWidth(10);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glBegin(GL_QUADS);
            {
                glVertex2f(-1, 1);
                glVertex2f(1, 1);
                glVertex2f(1, -1);
                glVertex2f(-1, -1);
            }
            glEnd();
        }

        glfwSwapBuffers(shadowWindow);

        glfwFocusWindow(window);
    }
    else {

        if (shadowWindow) glfwHideWindow(shadowWindow);
    }
}
