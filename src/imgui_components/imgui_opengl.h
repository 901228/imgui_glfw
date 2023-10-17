#ifndef IMGUI_OPENGL_H
#define IMGUI_OPENGL_H
#pragma once

#include <ImGui/imgui.h>
#include <ImGui/imgui_internal.h>

#include <glad/glad.h>

#include <stack>

class OpenGLPanel {

private:
    GLuint frameBufferObject;
    GLuint texture_id;
    GLuint renderBufferObject;

private:
    inline void createFBO() {

        glGenFramebuffers(1, &frameBufferObject);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);

        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glReadBuffer(GL_NONE);

        glGenRenderbuffers(1, &renderBufferObject);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            fprintf(stderr, "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n");

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
    }

public:
    inline OpenGLPanel() {

        createFBO();
    }
    inline ~OpenGLPanel() = default;

    inline void OnResize(GLsizei width, GLsizei height) {

        this->width = static_cast<GLsizei>(width * sampleFactor);
        this->height = static_cast<GLsizei>(height * sampleFactor);
        // rescale FBO
        {
            glBindTexture(GL_TEXTURE_2D, texture_id);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture_id, 0);

            glBindRenderbuffer(GL_RENDERBUFFER, renderBufferObject);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, this->width, this->height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBufferObject);
        }
        glViewport(0, 0, this->width, this->height);
    }

    inline GLuint GetTextureId() { return this->texture_id; }

    inline void bind() {

        glViewport(0, 0, this->width, this->height);
        glBindFramebuffer(GL_FRAMEBUFFER, frameBufferObject);
    }
    inline void unbind() {

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

private:
    GLsizei width = 800;
    GLsizei height = 800;
    const GLfloat sampleFactor = 4;
};

namespace ImGui {

    inline ImPool<OpenGLPanel> OpenGLPanelData;
    inline std::stack<ImGuiID> ID_stack;

    bool BeginOpenGL(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false, ImGuiWindowFlags flags = 0) {

        int beginFlag = ImGui::BeginChild(str_id, size, border, flags);
        if (!beginFlag) return beginFlag;

        ID_stack.push(ImGui::GetID(str_id));

        OpenGLPanel* data = OpenGLPanelData.GetOrAddByKey(ID_stack.top());

        const GLsizei window_width = static_cast<GLsizei>(ImGui::GetContentRegionAvail().x);
        const GLsizei window_height = static_cast<GLsizei>(ImGui::GetContentRegionAvail().y);
        data->OnResize(window_width, window_height);

        data->bind();
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        return beginFlag;
    }

    void EndOpenGL() {

        OpenGLPanel* data = OpenGLPanelData.GetByKey(ID_stack.top());
        data->unbind();

        const GLsizei window_width = static_cast<GLsizei>(ImGui::GetContentRegionAvail().x);
        const GLsizei window_height = static_cast<GLsizei>(ImGui::GetContentRegionAvail().y);
        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImGui::GetWindowDrawList()->AddImage(
            (void*)(intptr_t)data->GetTextureId(),
            ImVec2(pos.x, pos.y),
            ImVec2(pos.x + window_width, pos.y + window_height),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );

        ID_stack.pop();
        ImGui::EndChild();

        if (!ID_stack.empty()) OpenGLPanelData.GetByKey(ID_stack.top())->bind();
    }
}

#endif // !IMGUI_OPENGL_H
