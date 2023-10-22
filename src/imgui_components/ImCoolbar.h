/*
MIT License

Copyright (c) 2023 Stephane Cuillerdier (aka Aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>
#include <cmath>

typedef int ImCoolBarFlags;
enum ImCoolBarFlags_ {
    ImCoolBarFlags_None = 0,
    ImCoolBarFlags_Vertical = (1 << 0),
    ImCoolBarFlags_Horizontal = (1 << 1),
};

namespace ImGui {

    struct ImCoolBarConfig {

        ImVec2 anchor = ImVec2(-1.0f, -1.0f);
        float normal_size = 40.0f;
        float hovered_size = 60.0f;
        float anim_step = 0.15f;
        float effect_strength = 0.5f;
        ImCoolBarConfig(
            const ImVec2 vAnchor = ImVec2(-1.0f, -1.0f),
            const float& vNormalSize = 40.0f,
            const float& vHoveredSize = 60.0f,
            const float& vAnimStep = 0.15f,
            const float& vEffectStrength = 0.5f)
            :
            anchor(vAnchor),
            normal_size(vNormalSize),
            hovered_size(vHoveredSize),
            anim_step(vAnimStep),
            effect_strength(vEffectStrength)
        {}
    };
    IMGUI_API inline bool BeginCoolBar(const char* vLabel, ImCoolBarFlags vCBFlags = ImCoolBarFlags_Vertical, const ImCoolBarConfig& vConfig = {}, ImGuiWindowFlags vFlags = ImGuiWindowFlags_None) {
        ImGuiWindowFlags flags = vFlags |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_DockNodeHost |
            ImGuiWindowFlags_NoDocking;

        //FIXME: bg is not transparent
        bool res = ImGui::Begin(vLabel, nullptr, flags);
        if (!res) {
            ImGui::End();
        }
        else {
            ImGuiContext& g = *GImGui;
            ImGuiWindow* window = GetCurrentWindow();
            ImGuiViewport* viewport = ImGui::GetMainViewport();
            window->StateStorage.SetInt(window->GetID("##CoolBarItemIndex"), 0);
            window->StateStorage.SetInt(window->GetID("##CoolBarFlags"), vCBFlags);
            window->StateStorage.SetInt(window->GetID("##CoolBarID"), window->GetID(vLabel));
            window->StateStorage.SetFloat(window->GetID("##CoolBarAnchorX"), vConfig.anchor.x);
            window->StateStorage.SetFloat(window->GetID("##CoolBarAnchorY"), vConfig.anchor.y);
            window->StateStorage.SetFloat(window->GetID("##CoolBarNormalSize"), vConfig.normal_size);
            window->StateStorage.SetFloat(window->GetID("##CoolBarHoveredSize"), vConfig.hovered_size);
            window->StateStorage.SetFloat(window->GetID("##CoolBarEffectStrength"), vConfig.effect_strength);

            const auto& bar_size = ImGui::GetWindowSize();
            const float& offset_x = (viewport->Size.x - bar_size.x) * vConfig.anchor.x;
            const float& offset_y = (viewport->Size.y - bar_size.y) * vConfig.anchor.y;
            ImGui::SetWindowPos({ offset_x + viewport->Pos.x, offset_y + viewport->Pos.y });

            if (ImGui::IsMouseHoveringRect(window->Rect().Min, window->Rect().Max)) {
                const float& anim_scale = window->StateStorage.GetFloat(window->GetID("##CoolBarAnimScale"));
                if (anim_scale < 1.0f) {
                    window->StateStorage.SetFloat(window->GetID("##CoolBarAnimScale"), anim_scale + vConfig.anim_step);
                }
            }
            else {
                const float& anim_scale = window->StateStorage.GetFloat(window->GetID("##CoolBarAnimScale"));
                if (anim_scale > 0.0f) {
                    window->StateStorage.SetFloat(window->GetID("##CoolBarAnimScale"), anim_scale - vConfig.anim_step);
                }
            }
        }

        return res;
    }

    IMGUI_API inline void EndCoolBar() {

        ImGui::End();
    }

    IMGUI_API inline bool CoolBarItem() {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        const auto& idx = window->StateStorage.GetInt(window->GetID("##CoolBarItemIndex"));
        const auto& coolbar_id = window->StateStorage.GetInt(window->GetID("##CoolBarID"));
        const auto& coolbar_item_id = window->GetID(coolbar_id + idx + 1);
        const auto& current_item_size = window->StateStorage.GetFloat(coolbar_item_id);

        const auto& flags = window->StateStorage.GetInt(window->GetID("##CoolBarFlags"));
        const auto& anim_scale = window->StateStorage.GetFloat(window->GetID("##CoolBarAnimScale"));
        const auto& normal_size = window->StateStorage.GetFloat(window->GetID("##CoolBarNormalSize"));
        const auto& hovered_size = window->StateStorage.GetFloat(window->GetID("##CoolBarHoveredSize"));
        const auto& effect_strength = window->StateStorage.GetFloat(window->GetID("##CoolBarEffectStrength"));

        assert(normal_size > 0.0f);

        if (flags & ImCoolBarFlags_Horizontal) {
            if (idx) {
                ImGui::SameLine();
            }
        }

        float w = normal_size;
        ImGuiContext& g = *GImGui;

        // Can be Horizontal or Vertical, not both
        assert((flags & ImCoolBarFlags_Horizontal) || (flags & ImCoolBarFlags_Vertical));

#ifdef ENABLE_DEBUG
        char buf[256 + 1];
        const auto& color = ImGui::GetColorU32(ImVec4(0, 0, 1, 1));
#endif

        if (ImGui::IsMouseHoveringRect(window->Rect().Min, window->Rect().Max) || anim_scale > 0.0f) {
            if (flags & ImCoolBarFlags_Horizontal) {
                const float& btn_center_x = ImGui::GetCursorScreenPos().x + current_item_size * 0.5f;
                const float& diff_pos = (ImGui::GetMousePos().x - btn_center_x) / window->Size.x;
                w = normal_size + (hovered_size - normal_size) * (pow(cos(diff_pos * IM_PI * effect_strength), 12.0f)) * anim_scale;
                const float& anchor_y = window->StateStorage.GetFloat(window->GetID("##CoolBarAnchorY"));
                const float& bar_height = normal_size + hovered_size * anim_scale;
                float btn_offset_y = (bar_height - w) * anchor_y + g.Style.WindowPadding.y;
                ImGui::SetCursorPosY(btn_offset_y);
#ifdef ENABLE_DEBUG
                ImFormatString(buf, 256, "%.3f", diff_pos);
                const auto& bug_size = ImGui::CalcTextSize(buf);
                window->DrawList->AddText(ImVec2(btn_center_x - bug_size.x * 0.5f, window->Pos.y + window->Size.y * (1.0f - anchor_y)), color, buf);
                window->DrawList->AddLine(ImVec2(btn_center_x, window->Pos.y), ImVec2(btn_center_x, window->Pos.y + window->Size.y), color, 2.0f);
#endif
            }
            else {
                const float& btn_center_y = ImGui::GetCursorScreenPos().y + current_item_size * 0.5f;
                const float& diff_pos = (ImGui::GetMousePos().y - btn_center_y) / window->Size.y;
                w = normal_size + (hovered_size - normal_size) * (pow(cos(diff_pos * IM_PI * effect_strength), 12.0f)) * anim_scale;
                const float& anchor_x = window->StateStorage.GetFloat(window->GetID("##CoolBarAnchorX"));
                const float& bar_width = normal_size + hovered_size * anim_scale;
                float btn_offset_x = (bar_width - w) * anchor_x + g.Style.WindowPadding.x;
                ImGui::SetCursorPosX(btn_offset_x);
#ifdef ENABLE_DEBUG
                ImFormatString(buf, 256, "%.3f", diff_pos);
                const auto& bug_size = ImGui::CalcTextSize(buf);
                window->DrawList->AddText(ImVec2(window->Pos.x + window->Size.x * (1.0f - anchor_x), btn_center_y), color, buf);
                window->DrawList->AddLine(ImVec2(window->Pos.x, btn_center_y), ImVec2(window->Pos.x + window->Size.x, btn_center_y), color, 2.0f);
#endif
            }
        }

        window->StateStorage.SetInt(window->GetID("##CoolBarItemIndex"), idx + 1);
        window->StateStorage.SetFloat(coolbar_item_id, w);
        window->StateStorage.SetFloat(window->GetID("##CoolBarItemCurrentSize"), w);
        window->StateStorage.SetFloat(window->GetID("##CoolBarItemCurrentScale"), w / normal_size);

        return true;
    }

    IMGUI_API inline float GetCoolBarItemWidth() {

        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return 0.0f;
        return window->StateStorage.GetFloat(window->GetID("##CoolBarItemCurrentSize"));
    }

    IMGUI_API inline float GetCoolBarItemScale() {

        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems) return 0.0f;
        return window->StateStorage.GetFloat(window->GetID("##CoolBarItemCurrentScale"));
    }
} // namespace ImGui
