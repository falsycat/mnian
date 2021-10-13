// No copyright

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <thread>  // NOLINT(build/c++11)

#include <Tracy.hpp>

#include "mnian/app.h"
#include "mnian/registry.h"


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
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
  }

  mnian::core::DeserializerRegistry reg;
  {
    ZoneScopedN("setup DeserializerRegistry");
    mnian::SetupDeserializerRegistry(&reg);
  }

  mnian::App app(&reg);
  {
    ZoneScopedN("setup App");
  }

  while (!glfwWindowShouldClose(window)) {
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
    FrameMarkNamed("main");
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
