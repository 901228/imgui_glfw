#ifndef IMGUI_OPENGL_H
#define IMGUI_OPENGL_H
#pragma once

typedef int ImGuiWindowFlags;

namespace ImGui {

    bool BeginOpenGL(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
    void EndOpenGL();
}

#endif // !IMGUI_OPENGL_H
