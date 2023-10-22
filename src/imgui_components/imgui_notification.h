#ifndef IMGUI_NOTIFICATION_H
#define IMGUI_NOTIFICATION_H
#pragma once

#include <vector>
#include <string>
#include <map>

#include <memory>
#include <stdexcept>

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include "font_awesome_5.h"
#include "fa_solid_900.h"

#define NOTIFICATION_PADDING_X 20.f           // Bottom-left X padding
#define NOTIFICATION_PADDING_Y 20.f           // Bottom-left Y padding
#define NOTIFICATION_PADDING_MESSAGE_Y 10.f   // Padding Y between each message
#define NOTIFICATION_FADE_IN_OUT_TIME 10      // Fade in and out duration
#define NOTIFICATION_FADE_IN_OUT_TIME 10      // Fade in and out duration
#define NOTIFICATION_DROP_TIME 8              // Hide for drop animation
#define NOTIFICATION_DEFAULT_DISMISS 90       // Auto dismiss after X frames (fps) (default, applied only of no data provided in constructors)
#define NOTIFICATION_DEFAULT_OPACITY 1.0f     // 0-1 Toast opacity
#define NOTIFICATION_DEFAULT_ROUNDING 5.0f    // 0-1 Toast opacity
#define NOTIFICATION_TOAST_FLAGS ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing

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

    inline ImGuiToastTypeConfig(ImGuiToastType type, ImVec4 color_light, ImVec4 color_dark, std::string icon, std::string title) : type(type), color_light(color_light), color_dark(color_dark), icon(icon), title(title) {};

    ImGuiToastType type;
    ImVec4 color_light;
    ImVec4 color_dark;
    std::string icon;
    std::string title;

    inline static const ImGuiToastTypeConfig None() { return { ImGuiToastType_None, { 0, 0, 0, 1 }, { 1, 1, 1, 1 }, "", "" }; }
    inline static const ImGuiToastTypeConfig Success() { return { ImGuiToastType_Success, { 0, 0.8, 0, 1 }, { 0, 1, 0, 1 }, ICON_FA_CHECK_CIRCLE, "Success" }; }
    inline static const ImGuiToastTypeConfig Warning() { return { ImGuiToastType_Warning, { 0.7, 0.7, 0.3, 1 }, { 1, 1, 0, 1 }, ICON_FA_EXCLAMATION_TRIANGLE, "Warning" }; }
    inline static const ImGuiToastTypeConfig Error() { return { ImGuiToastType_Error, { 1, 0, 0, 1 }, { 1, 0, 0, 1 }, ICON_FA_TIMES_CIRCLE, "Error" }; }
    inline static const ImGuiToastTypeConfig Info() { return { ImGuiToastType_Info, { 0, 0.6, 1, 1 }, { 0, 0.6, 1, 1 }, ICON_FA_INFO_CIRCLE, "Info" }; }

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
    ImGuiToastPhase_Drop,
    ImGuiToastPhase_Expired,
    ImGuiToastPhase_MAX
};

enum ImGuiToastPos_ {
    ImGuiToastPos_Top = 1,
    ImGuiToastPos_Middle = 1 << 1,
    ImGuiToastPos_Bottom = 1 << 2,
    ImGuiToastPos_Left = 1 << 3,
    ImGuiToastPos_Center = 1 << 4,
    ImGuiToastPos_Right = 1 << 5,

    ImGuiToastPos_TopLeft = ImGuiToastPos_Top | ImGuiToastPos_Left,
    ImGuiToastPos_TopCenter = ImGuiToastPos_Top | ImGuiToastPos_Center,
    ImGuiToastPos_TopRight = ImGuiToastPos_Top | ImGuiToastPos_Right,
    ImGuiToastPos_MiddleLeft = ImGuiToastPos_Middle | ImGuiToastPos_Left,
    ImGuiToastPos_MiddleCenter = ImGuiToastPos_Middle | ImGuiToastPos_Center,
    ImGuiToastPos_MiddleRight = ImGuiToastPos_Middle | ImGuiToastPos_Right,
    ImGuiToastPos_BottomLeft = ImGuiToastPos_Bottom | ImGuiToastPos_Left,
    ImGuiToastPos_BottomCenter = ImGuiToastPos_Bottom | ImGuiToastPos_Center,
    ImGuiToastPos_BottomRight = ImGuiToastPos_Bottom | ImGuiToastPos_Right,
    ImGuiToastPos_MAX = 1 << 6
};

namespace ImGui { inline int timestamp = 0; }

class ImGuiToast {

public:
    ImGuiToast(ImGuiToastType type, int dismiss_time = NOTIFICATION_DEFAULT_DISMISS, ImGuiToastPos pos = ImGuiToastPos_BottomRight) {

        IM_ASSERT(type < ImGuiToastType_MAX);

        this->type = type;
        this->dismiss_time = dismiss_time;
        this->creation_time = ImGui::timestamp;
        this->title = "";
        this->content = "";
        this->pos = pos;
    }

    template<typename ... Args>
    ImGuiToast(ImGuiToastType type, const char* format, Args ... args) : ImGuiToast(type) { this->setContent(format, args ...); }

    template<typename ... Args>
    ImGuiToast(ImGuiToastType type, ImGuiToastPos pos, const char* format, Args ... args) : ImGuiToast(type, NOTIFICATION_DEFAULT_DISMISS, pos) { this->setContent(format, args ...); }

    template<typename ... Args>
    ImGuiToast(ImGuiToastType type, int dismiss_time, ImGuiToastPos pos, const char* format, Args ... args) : ImGuiToast(type, dismiss_time, pos) { this->setContent(format, args ...); }

    /*
    template<typename ... Args>
    ImGuiToast(ImGuiToastType type, int dismiss_time, const char* format, Args ... args) : ImGuiToast(type, dismiss_time) { this->setContent(format, args ...); }
    */

    ~ImGuiToast() = default;

private:
    ImGuiToastType type = ImGuiToastType_None;
    std::string title;
    std::string content;
    int dismiss_time = NOTIFICATION_DEFAULT_DISMISS;
    int creation_time = 0;
    float opacity = NOTIFICATION_DEFAULT_OPACITY;
    ImGuiToastPos pos = ImGuiToastPos_BottomRight;

private:
public:
    template<typename ... Args>
    inline void setTitle(const char* format, Args ... args) { this->title = string_format(format, args ...); }

    template<typename ... Args>
    inline void setContent(const char* format, Args ... args) { this->content = string_format(format, args ...); }

    inline void setType(ImGuiToastType type) { IM_ASSERT(type < ImGuiToastType_MAX); this->type = type; }
    inline void setPos(ImGuiToastPos pos) { IM_ASSERT(pos < ImGuiToastPos_MAX); this->pos = pos; }

public:
    inline const std::string getTitle() const { return (this->title.empty() ? getDefaultTitle() : this->title); }
    inline const std::string getContent() const { return this->content; }
    inline const ImGuiToastPos getPos() const { return this->pos; }

    inline const ImGuiToastTypeConfig getDefaultConfig() const { return ImGuiToastTypeConfig::get(this->type); }
    inline const std::string getDefaultTitle() const { return getDefaultConfig().title; }
    inline const std::string getTypeIcon() const { return getDefaultConfig().icon; }
    inline const ImVec4 getTypeColor() const {

        // use text color to determine icon color
        ImVec4 textColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);

        //XXX: whether text color is dark or light
        if (textColor.x + textColor.y + textColor.z < 1.5f) return getDefaultConfig().color_light;
        else return getDefaultConfig().color_dark;
    }

    inline const int getElapseTime() const { return ImGui::timestamp - this->creation_time; }
    inline const ImGuiToastPhase getPhase() const {

        const int& elapsed = getElapseTime();

        if (elapsed > NOTIFICATION_FADE_IN_OUT_TIME + this->dismiss_time + NOTIFICATION_FADE_IN_OUT_TIME + NOTIFICATION_DROP_TIME) {
            return ImGuiToastPhase_Expired;
        }
        if (elapsed > NOTIFICATION_FADE_IN_OUT_TIME + this->dismiss_time + NOTIFICATION_FADE_IN_OUT_TIME) {
            return ImGuiToastPhase_Drop;
        }
        else if (elapsed > NOTIFICATION_FADE_IN_OUT_TIME + this->dismiss_time) {
            return ImGuiToastPhase_FadeOut;
        }
        else if (elapsed > NOTIFICATION_FADE_IN_OUT_TIME) {
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

            return (elapsed * NOTIFICATION_DEFAULT_OPACITY) / NOTIFICATION_FADE_IN_OUT_TIME;
        }
        else if (phase == ImGuiToastPhase_FadeOut) {

            return NOTIFICATION_DEFAULT_OPACITY - (((elapsed - NOTIFICATION_FADE_IN_OUT_TIME - this->dismiss_time) * NOTIFICATION_DEFAULT_OPACITY) / NOTIFICATION_FADE_IN_OUT_TIME);
        }
        else if (phase == ImGuiToastPhase_Drop) {

            return 0;
        }

        return NOTIFICATION_DEFAULT_OPACITY;
    }
    inline const float getRemainPercent() const {

        const ImGuiToastPhase phase = getPhase();
        const float elapsed = static_cast<float>(getElapseTime() - NOTIFICATION_FADE_IN_OUT_TIME - this->dismiss_time - NOTIFICATION_FADE_IN_OUT_TIME);

        if (phase == ImGuiToastPhase_Drop) {
            return 1.0f - (elapsed / NOTIFICATION_DROP_TIME);
        }

        return 1.0f;
    }
};

namespace ImGui {

    inline void NotificationFontInit() {

        // Merge icons into default tool font
        ImGuiIO& io = GetIO();
        io.Fonts->AddFontDefault();

        ImFontConfig config;
        const float fontSize = 13;
        config.MergeMode = true;
        // config.GlyphMinAdvanceX = fontSize; // Use if you want to make the icon monospaced
        config.FontDataOwnedByAtlas = false;
        static const ImWchar icon_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
        io.Fonts->AddFontFromMemoryTTF((void*)fa_solid_900, sizeof(fa_solid_900), fontSize, &config, icon_ranges);
    }

    inline std::vector<ImGuiToast> notifications;

    inline void InsertNotification(const ImGuiToast& toast) {

        notifications.push_back(toast);
    }
    inline void RemoveNotification(int index) {

        notifications.erase(notifications.begin() + index);
    }

    inline void RenderNotifications() {

        ImVec2 wrk_size = (GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable ? GetViewportPlatformMonitor(GetMainViewport())->WorkSize : GetMainViewport()->Size);

        const float textWrapWidth = wrk_size.x / 4.0f;

        std::map<ImGuiToastPos, float> yy;
        yy[ImGuiToastPos_Top] = NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_TopLeft] = NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_TopCenter] = NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_TopRight] = NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_Middle] = -NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_MiddleLeft] = -NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_MiddleCenter] = -NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_MiddleRight] = -NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_Bottom] = NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_BottomLeft] = NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_BottomCenter] = NOTIFICATION_PADDING_Y;
        yy[ImGuiToastPos_BottomRight] = NOTIFICATION_PADDING_Y;

        // calculate y for middle
        {
            for (const auto& i : notifications) {

                if (i.getPos() & ImGuiToastPos_Middle) {

                    const float titleSizeY = (i.getTitle().empty() ? 0 : CalcTextSize(string_format("%s%s", i.getTypeIcon().c_str(), i.getTitle().c_str()).c_str(), 0, false, textWrapWidth).y);
                    const float contentSizeY = CalcTextSize(i.getContent().c_str(), 0, false, textWrapWidth).y;
                    yy[i.getPos()] += std::max(
                        titleSizeY + contentSizeY + GetStyle().WindowPadding.y * 2 + (i.getTitle().empty() ? 0 : GetStyle().ItemSpacing.y * 2),
                        GetStyle().WindowMinSize.y
                    ) + NOTIFICATION_PADDING_Y;
                }
            }

            yy[ImGuiToastPos_Middle] = (wrk_size.y - yy[ImGuiToastPos_Middle]) / 2.0f;
            yy[ImGuiToastPos_MiddleLeft] = (wrk_size.y - yy[ImGuiToastPos_MiddleLeft]) / 2.0f;
            yy[ImGuiToastPos_MiddleCenter] = (wrk_size.y - yy[ImGuiToastPos_MiddleCenter]) / 2.0f;
            yy[ImGuiToastPos_MiddleRight] = (wrk_size.y - yy[ImGuiToastPos_MiddleRight]) / 2.0f;
        }

        //FIXME: rounding not working because of multiviewport ?
        PushStyleVar(ImGuiStyleVar_WindowRounding, NOTIFICATION_DEFAULT_ROUNDING); // Round borders
        for (int i = 0; i < notifications.size(); i++) {

            const ImGuiToast& notification = notifications[i];

            const bool isTitleRendered = !notification.getTitle().empty();
            const float opacity = notification.getFadePercent();

            PushStyleVar(ImGuiStyleVar_Alpha, opacity);

            //FIXME: pre-compute error
            ImVec2 w_size;
            {
                // compute the size of the notification
                const ImVec2 titleTextSize = (isTitleRendered ? CalcTextSize(string_format("%s", notification.getTitle().c_str()).c_str(), 0, false, textWrapWidth) : ImVec2(0, 0));
                const ImVec2 iconSize = (isTitleRendered ? CalcTextSize(string_format("%s", notification.getTypeIcon().c_str()).c_str(), 0, false, textWrapWidth) : ImVec2(0, 0));
                const ImVec2 titleSize = { iconSize.x + GetStyle().ItemSpacing.x + titleTextSize.x, iconSize.y + titleTextSize.y };
                const ImVec2 contentSize = CalcTextSize(notification.getContent().c_str(), 0, false, textWrapWidth);
                w_size.x = std::max(
                    std::max(titleSize.x, contentSize.x) + GetStyle().WindowPadding.x * 2,
                    GetStyle().WindowMinSize.x
                );
                w_size.y = std::max(
                    titleSize.y + contentSize.y + GetStyle().WindowPadding.y * 2 + (isTitleRendered ? GetStyle().ItemSpacing.y * 2 : 0),
                    GetStyle().WindowMinSize.y
                );

                // system("cls");
                printf("cal: (%g, %g)\n", w_size.x, w_size.y);
                printf("%g\n%g\n", titleSize.y, contentSize.y);

                w_size.y *= notification.getRemainPercent();

                ImVec2 windowPos;
                if (notification.getPos() & ImGuiToastPos_Right) windowPos.x = wrk_size.x - w_size.x - NOTIFICATION_PADDING_X;
                else if (notification.getPos() & ImGuiToastPos_Center) windowPos.x = (wrk_size.x - w_size.x) / 2.0f;
                else if (notification.getPos() & ImGuiToastPos_Left) windowPos.x = NOTIFICATION_PADDING_X;

                if (notification.getPos() & ImGuiToastPos_Top) windowPos.y = yy[notification.getPos()];
                else if (notification.getPos() & ImGuiToastPos_Middle) windowPos.y = yy[notification.getPos()];
                else if (notification.getPos() & ImGuiToastPos_Bottom) windowPos.y = wrk_size.y - w_size.y - yy[notification.getPos()];

                // set notification positions
                SetNextWindowPos(windowPos, ImGuiCond_Always);
            }
            if (Begin(string_format("TOAST%i", i).c_str(), 0, NOTIFICATION_TOAST_FLAGS)) {

                printf("ans: (%g, %g)\n", GetWindowSize().x, GetWindowSize().y);

                PushTextWrapPos(textWrapWidth);

                if (isTitleRendered) {

                    // title
                    TextColored(notification.getTypeColor(), notification.getTypeIcon().c_str());
                    SameLine();
                    Text(notification.getTitle().c_str());
                    Separator();
                }

                // content
                Text(notification.getContent().c_str());

                PopTextWrapPos();
            }
            End();
            // record y for the position of the next notification
            yy[notification.getPos()] += w_size.y + NOTIFICATION_PADDING_MESSAGE_Y;

            PopStyleVar();
        }

        PopStyleVar();

        timestamp++;
        // std::erase_if(notifications, [](const ImGuiToast& i) { return (i.getPhase() == ImGuiToastPhase_Expired); });
    }
}

#endif // !IMGUI_NOTIFICATION_H
