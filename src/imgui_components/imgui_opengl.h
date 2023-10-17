#ifndef IMGUI_OPENGL_H
#define IMGUI_OPENGL_H
#pragma once

typedef int ImGuiWindowFlags;

namespace ImGui {

    bool BeginOpenGL(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0);
    void EndOpenGL();
}

#endif // !IMGUI_OPENGL_H
