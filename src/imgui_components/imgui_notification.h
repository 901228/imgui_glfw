#ifndef IMGUI_NOTIFICATION_H
#define IMGUI_NOTIFICATION_H
#pragma once

#include <vector>
#include <string>

#include <memory>
#include <stdexcept>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#define NOTIFY_PADDING_X 20.f           // Bottom-left X padding
#define NOTIFY_PADDING_Y 20.f           // Bottom-left Y padding
#define NOTIFY_PADDING_MESSAGE_Y 10.f   // Padding Y between each message
#define NOTIFY_FADE_IN_OUT_TIME 10       // Fade in and out duration
#define NOTIFY_DEFAULT_DISMISS 90       // Auto dismiss after X frames (fps) (default, applied only of no data provided in constructors)
#define NOTIFY_DEFAULT_OPACITY 1.0f     // 0-1 Toast opacity
#define NOTIFY_TOAST_FLAGS ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoFocusOnAppearing

// #define NOTIFY_FORMAT(fn, format, ...)	if (format) { va_list args; va_start(args, format); fn(format, args, __VA_ARGS__); va_end(args); }

template<typename ... Args>
static std::string string_format(const std::string& format, Args ... args) {

    int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
    if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
    size_t size = static_cast<size_t>(size_s);
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args ...);
    return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
}

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

struct ImGuiToastTypeConfig {

    inline ImGuiToastTypeConfig(ImGuiToastType type, ImVec4 color, std::string icon, std::string title) : type(type), color(color), icon(icon), title(title) {};

    ImGuiToastType type;
    ImVec4 color;
    std::string icon;
    std::string title;

    //TODO: icons
    inline static const ImGuiToastTypeConfig None() { return { ImGuiToastType_None, { 255, 255, 255, 255 }, "", "None" }; }
    inline static const ImGuiToastTypeConfig Success() { return { ImGuiToastType_Success, { 0, 255, 0, 255 }, "", "Success" }; }
    inline static const ImGuiToastTypeConfig Warning() { return { ImGuiToastType_Warning, { 255, 255, 0, 255 }, "", "Warning" }; }
    inline static const ImGuiToastTypeConfig Error() { return { ImGuiToastType_Error, { 255, 0, 0, 255 }, "", "Error" }; }
    inline static const ImGuiToastTypeConfig Info() { return { ImGuiToastType_Info, { 0, 157, 255, 255 }, "", "Info" }; }

    inline static const ImGuiToastTypeConfig get(ImGuiToastType type) {

        switch (type) {
        case ImGuiToastType_None: { return None(); }
        case ImGuiToastType_Success: { return Success(); }
        case ImGuiToastType_Warning: { return Warning(); }
        case ImGuiToastType_Error: { return Error(); }
        case ImGuiToastType_Info: { return Info(); }
        }
        return None();
    }
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
    ImGuiToast(ImGuiToastType type, int dismiss_time = NOTIFY_DEFAULT_DISMISS) {

        IM_ASSERT(type < ImGuiToastType_MAX);

        this->type = type;
        this->dismiss_time = dismiss_time;
        this->creation_time = ImGui::timestamp;
        this->title = "";
        this->content = "";
    }

    template<typename ... Args>
    ImGuiToast(ImGuiToastType type, const char* format, Args ... args) : ImGuiToast(type, NOTIFY_DEFAULT_DISMISS) { this->setContent(format, args ...); }

    template<typename ... Args>
    ImGuiToast(ImGuiToastType type, int dismiss_time, const char* format, Args ... args) : ImGuiToast(type, dismiss_time) { this->setContent(format, args ...); }
    ~ImGuiToast() = default;

private:
    ImGuiToastType type = ImGuiToastType_None;
    std::string title;
    std::string content;
    int dismiss_time = NOTIFY_DEFAULT_DISMISS;
    int creation_time = 0;
    float opacity = NOTIFY_DEFAULT_OPACITY;

private:
public:
    template<typename ... Args>
    inline void setTitle(const char* format, Args ... args) { this->title = string_format(format, args ...); }

    template<typename ... Args>
    inline void setContent(const char* format, Args ... args) { this->content = string_format(format, args ...); }

    inline void setType(const ImGuiToastType type) { IM_ASSERT(type < ImGuiToastType_MAX); this->type = type; };

public:
    inline const std::string getTitle() const { return (this->title.empty() ? getDefaultTitle() : this->title); }
    inline const std::string getContent() const { return this->content; }

    inline const ImGuiToastTypeConfig getDefaultConfig() const { return ImGuiToastTypeConfig::get(this->type); }
    inline const std::string getDefaultTitle() const { return getDefaultConfig().title; }
    inline const std::string getTypeIcon() const { return getDefaultConfig().icon; }
    inline const ImVec4 getTypeColor() const { return getDefaultConfig().color; }

    inline const int getElapseTime() const { return ImGui::timestamp - this->creation_time; }
    inline const ImGuiToastPhase getPhase() const {

        const int& elapsed = getElapseTime();

        if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismiss_time + NOTIFY_FADE_IN_OUT_TIME) {
            return ImGuiToastPhase_Expired;
        }
        else if (elapsed > NOTIFY_FADE_IN_OUT_TIME + this->dismiss_time) {
            return ImGuiToastPhase_FadeOut;
        }
        else if (elapsed > NOTIFY_FADE_IN_OUT_TIME) {
            return ImGuiToastPhase_Wait;
        }
        else {
            return ImGuiToastPhase_FadeIn;
        }
    }
    inline const float getFadePercent() const {

        const ImGuiToastPhase phase = getPhase();
        const float elapsed = static_cast<float>(getElapseTime());

        if (phase == ImGuiToastPhase_FadeIn) {

            return (elapsed * NOTIFY_DEFAULT_OPACITY) / NOTIFY_FADE_IN_OUT_TIME;
        }
        else if (phase == ImGuiToastPhase_FadeOut) {

            return NOTIFY_DEFAULT_OPACITY - (((elapsed - NOTIFY_FADE_IN_OUT_TIME - this->dismiss_time) * NOTIFY_DEFAULT_OPACITY) / NOTIFY_FADE_IN_OUT_TIME);
        }

        return 1.0f * NOTIFY_DEFAULT_OPACITY;
    }
};

namespace ImGui {

    inline std::vector<ImGuiToast> notifications;

    inline void InsertNotification(const ImGuiToast& toast) {

        notifications.push_back(toast);
    }
    inline void RemoveNotification(int index) {

        notifications.erase(notifications.begin() + index);
    }

    inline void RenderNotifications() {

        const ImVec2 wrk_size = GetViewportPlatformMonitor(GetMainViewport())->WorkSize;
        float yy = NOTIFY_PADDING_Y;

        PushStyleVar(ImGuiStyleVar_WindowRounding, 5.f); // Round borders
        // PushStyleColor(ImGuiCol_WindowBg, ImVec4(43.f / 255.f, 43.f / 255.f, 43.f / 255.f, 100.f / 255.f)); // Background color
        for (int i = 0; i < notifications.size(); i++) {

            const auto& notification = notifications[i];

            const float opacity = notification.getFadePercent();
            PushStyleVar(ImGuiStyleVar_Alpha, opacity);

            if (Begin(string_format("TOAST%i", i).c_str(), 0, NOTIFY_TOAST_FLAGS)) {

                // set notification position & record y for the position of the next notification
                ImVec2 w_size = ImGui::GetWindowSize();
                ImGui::SetWindowPos({ wrk_size.x - w_size.x - NOTIFY_PADDING_X , wrk_size.y - w_size.y - yy }, ImGuiCond_Always);
                yy += w_size.y + NOTIFY_PADDING_MESSAGE_Y;

                // title
                ImGui::Text("%s", notification.getTitle().c_str());
                ImGui::Separator();

                // content
                ImGui::Text(notification.getContent().c_str());

            }
            End();

            PopStyleVar();
        }

        // PopStyleColor();
        PopStyleVar();

        timestamp++;
        std::erase_if(notifications, [](const ImGuiToast& i) { return (i.getPhase() == ImGuiToastPhase_Expired); });
    }
}

#endif // !IMGUI_NOTIFICATION_H
