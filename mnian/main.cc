// No copyright

#include <string.h>

#include <fontawesome.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <thread>  // NOLINT(build/c++11)

#include <Tracy.hpp>

#include "mnian/app.h"
#include "mnian/registry.h"

#include "mnres/all.h"


#if defined(IMGUI_IMPL_OPENGL_ES2)
  #include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>


#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&  \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif


static void GlfwErrorCallback(int, const char* description) {
  TracyMessageLCS("GLFW error", tracy::Color::Magenta, 0);
  TracyMessageLCS(description, tracy::Color::Gray, 0);
}

int main(int, char**) {
  {
    ZoneScopedN("init GLFW");
    glfwSetErrorCallback(GlfwErrorCallback);
    if (!glfwInit()) return 1;
  }

  GLFWwindow* window;
  const char* glsl_version;
  {
    ZoneScopedN("setup Window");

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#   if defined(__APPLE__)
      glsl_version = "#version 150";
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#   else
      glsl_version = "#version 130";
      glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
      glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
      glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#   endif

    window = glfwCreateWindow(1280, 720, "mnian", NULL, NULL);
    if (window == NULL) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
  }

  {
    ZoneScopedN("setup ImGUI");

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    auto& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    {
      ZoneScopedN("build font");
      {  // M+ code
        static constexpr float kSize = 17.f;

        ImFontConfig config;
        config.FontDataOwnedByAtlas = false;
        snprintf(config.Name, sizeof(config.Name), "M+ code");

        io.Fonts->AddFontFromMemoryTTF(
            const_cast<uint8_t*>(mnian::res::font::kMplusCode),
            static_cast<int>(mnian::res::font::kMplusCodeSize),
            kSize, &config, io.Fonts->GetGlyphRangesJapanese());
      }
      {  // FontAwesome
        static constexpr float kSize = 13.5f;

        static const ImWchar range[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

        ImFontConfig config;
        config.FontDataOwnedByAtlas = false;
        config.MergeMode            = true;
        config.GlyphMinAdvanceX     = kSize;
        config.GlyphMaxAdvanceX     = kSize;
        snprintf(config.Name, sizeof(config.Name), "FontAwesome");

        io.Fonts->AddFontFromMemoryTTF(
            const_cast<uint8_t*>(mnian::res::font::kFontAwesome),
            static_cast<int>(mnian::res::font::kFontAwesomeSize),
            kSize, &config, range);
      }
      io.Fonts->Build();
    }

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
  }

  mnian::core::DeserializerRegistry reg;
  mnian::SetupDeserializerRegistry(&reg);

  mnian::App app(window, &reg);
  glfwShowWindow(window);

  tracy::SetThreadName("main");
  while (app.alive()) {
    FrameMarkStart("main");
    {
      ZoneScopedN("poll events");
      glfwPollEvents();
    }
    {
      ZoneScopedN("update");

      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      app.Update();
    }
    {
      ZoneScopedN("render display");
      ImGui::Render();

      int w, h;
      glfwGetFramebufferSize(window, &w, &h);
      glViewport(0, 0, w, h);

      glClear(GL_COLOR_BUFFER_BIT);
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    {
      ZoneScopedN("swap buffer");
      glfwSwapBuffers(window);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    FrameMarkEnd("main");
  }

  {
    ZoneScopedN("teardown ImGUI");
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }
  {
    ZoneScopedN("teardown GLFW");
    glfwDestroyWindow(window);
    glfwTerminate();
  }
  return 0;
}
