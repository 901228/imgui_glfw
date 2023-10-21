#include "main_window.h"

#include <cstdio>

#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>

#include "imgui_components/imgui_opengl.h"
#include "imgui_components/imgui_notification.h"

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
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

        // glfw window creation
        window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL with ImGui", nullptr, nullptr);
        if (window == nullptr) {

            fprintf(stderr, "Failed to create GLFW window\n");
            glfwTerminate();
            return false;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable vsync
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

    RenderNotifications();
}

void MainWindow::HandleUserInput() {

    ImVec2 pos = ImGui::GetMousePos();

    //TODO: handle user inputs
}

void MainWindow::CreateMenuBar() {

    if (ImGui::BeginMainMenuBar()) {

        menubar_offsetY = ImGui::GetWindowHeight();
        main_height = SCR_HEIGHT - menubar_offsetY;

        if (ImGui::MenuItem("Settings")) {

            isSettingPageOpened = !isSettingPageOpened;
        }
        ImGui::Separator();

        ImGui::EndMainMenuBar();
    }
}

void MainWindow::CreateMainView() {

    if (ImGui::Begin("main view", 0, flag)) {

        ImGui::SetWindowPos({ window_pos.x, window_pos.y + menubar_offsetY });
        ImGui::SetWindowSize({ main_width, main_height });

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

            if (ImGui::BeginTabItem("notification")) {

                if (ImGui::BeginChild("notification child")) {

                    if (ImGui::Button("Insert Notifications")) {

                        ImGui::InsertNotification({ ImGuiToastType_None, 90, "Hello None!" });
                        // ImGui::InsertNotification({ ImGuiToastType_Success, 90, "Hello World! This is a success! %s", "We can also format here:)" });
                        // ImGui::InsertNotification({ ImGuiToastType_Warning, 90, "Hello World! This is a warning! %d", 0x1337 });
                        // ImGui::InsertNotification({ ImGuiToastType_Error, 90, "Hello World! This is an error! 0x%X", 0xDEADBEEF });
                        // ImGui::InsertNotification({ ImGuiToastType_Info, 90, "Hello World! This is an info!" });
                        // ImGui::InsertNotification({ ImGuiToastType_Info, 90, "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation" });
                    }
                    if (ImGui::Button("Custom Toast")) {

                        ImGuiToast toast(ImGuiToastType_Success, 90); // <-- content can also be passed here as above
                        toast.setTitle("This is a %s title", "wonderful");
                        toast.setContent("This is a %s content", "beautiful");
                        ImGui::InsertNotification(toast);
                    }

                    ImGui::EndChild();
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::End();
    }
}

void MainWindow::CreateControlPanel() {

    if (ImGui::Begin("controls", 0, flag)) {

        ImGui::SetWindowPos({ window_pos.x + main_width, window_pos.y + menubar_offsetY });
        ImGui::SetWindowSize({ SCR_WIDTH - main_width, main_height });
        {
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
        }
        ImGui::End();
    }
}

void MainWindow::CreateSettingPage() {

    if (isSettingPageOpened && ImGui::Begin("Settings Page", &isSettingPageOpened, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::SliderFloat("sliderFloat", &sliderFloat, 0.001f, 0.01f, "%.3f");

        ImGui::End();
    }
}

void MainWindow::RenderNotifications() {

    // Render Notifications
    ImGui::RenderNotifications();
}
