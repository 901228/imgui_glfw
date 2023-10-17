#ifndef IMGUI_NOTIFICATION_H
#define IMGUI_NOTIFICATION_H
#pragma once

#include <vector>

typedef int ImGuiToastType;
typedef int ImGuiToastPhase;
typedef int ImGuiToastPos;

enum ImGuiToastType_ {

    ImGuiToastType_None,
    ImGuiToastType_Success,
    ImGuiToastType_Warning,
    ImGuiToastType_Error,
    ImGuiToastType_Info,
    ImGuiToastType_MAX
};

enum ImGuiToastPhase_ {

    ImGuiToastPhase_FadeIn,
    ImGuiToastPhase_Wait,
    ImGuiToastPhase_FadeOut,
    ImGuiToastPhase_Expired,
    ImGuiToastPhase_MAX
};

enum ImGuiToastPos_ {

    ImGuiToastPos_TopLeft,
    ImGuiToastPos_TopCenter,
    ImGuiToastPos_TopRight,
    ImGuiToastPos_BottomLeft,
    ImGuiToastPos_BottomCenter,
    ImGuiToastPos_BottomRight,
    ImGuiToastPos_Center,
    ImGuiToastPos_MAX
};

namespace ImGui { inline int timestamp = 0; }

class ImGuiToast {

public:
    ImGuiToast();
    ~ImGuiToast() = default;
};

namespace ImGui {

    inline std::vector<ImGuiToast> notifications;

    inline void InsertNotification(const ImGuiToast& toast) {

        notifications.push_back(toast);
    }
    inline void RemoveNotification(int index) {

        notifications.erase(notifications.begin() + index);
    }

    inline void RenderNotifications() {}
}

#endif // !IMGUI_NOTIFICATION_H
