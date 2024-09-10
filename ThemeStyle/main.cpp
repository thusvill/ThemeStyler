#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <vector>
// Include ImGui headers
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "file_manager.hpp"
#include "apply_system_theme.hpp"

// #include <leif/leif.h>
// #include <libnotify/notify.h>
#include <libnotify/notify.h>

std::vector<std::tuple<std::string, int, int, int>> SystemTheme::colors = {
    {"purple", 98, 100, 178},
    {"pink", 185, 125, 150},
    {"green", 34, 139, 89},
    {"blue", 70, 130, 180},
    {"red", 255, 69, 70},
    {"yellow", 255, 215, 30},
    {"black", 20, 20, 20}};

char fname[128] = "Graphite-";
char lname[128] = "-Dark-nord";
std::string SystemTheme::first_name = "Graphite-";
std::string SystemTheme::last_name = "-Dark-nord";

static bool show_name_context_menu = false;
static bool show_command_context_menu = false;
int current_style = 0;
bool show_add_item_popup = false;

float minButtonWidth = 100.0f;
float maxButtonWidth = 400.0f;

int buttons_per_column = 10;
bool send_notifications = false;
bool edit = false;
bool open_settings = false;

void DockSpace(bool *p_open)
{

  static bool opt_fullscreen = true;
  static bool opt_padding = false;
  static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

  // We are using the ImGuiWindowFlags_NoDocking flag to make the parent window
  // not dockable into, because it would be confusing to have two docking
  // targets within each others.
  ImGuiWindowFlags window_flags =
      ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
  if (opt_fullscreen)
  {
    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                    ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |=
        ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
  }
  else
  {
    dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
  }

  // When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render
  // our background and handle the pass-thru hole, so we ask Begin() to not
  // render a background.
  if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
    window_flags |= ImGuiWindowFlags_NoBackground;

  // Important: note that we proceed even if Begin() returns false (aka window
  // is collapsed). This is because we want to keep our DockSpace() active. If a
  // DockSpace() is inactive, all active windows docked into it will lose their
  // parent and become undocked. We cannot preserve the docking relationship
  // between an active window and an inactive docking, otherwise any change of
  // dockspace/settings would lead to windows being stuck in limbo and never
  // being visible.
  if (!opt_padding)
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::Begin("DockSpace Demo", p_open, window_flags);
  if (!opt_padding)
    ImGui::PopStyleVar();

  if (opt_fullscreen)
    ImGui::PopStyleVar(2);

  // Submit the DockSpace
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard |
                    ImGuiConfigFlags_NavEnableSetMousePos;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
  io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleFonts;
  io.ConfigFlags |= ImGuiConfigFlags_DpiEnableScaleViewports;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcons;
  // io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoMerge;
  io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
  io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
  if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
  {
    ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
  }

  ImGui::End();
}

class Style
{
public:
  std::string name;
  std::string command;
  bool is_dock_enabled;
  void Apply()
  {
    system(command.c_str())+" &";
    SystemTheme::apply_systemwide();
    if (is_dock_enabled)
    {
      system("gnome-extensions enable ubuntu-dock@ubuntu.com");
    }
    else
    {
      system("gnome-extensions disable ubuntu-dock@ubuntu.com");
    }

    // system("killall pulseaudio && pulseaudio &");
    sleep(3);
    system("mplayer -channels 8 "
           "/home/bios/Themes/ThemeStyler/ThemeStyle/wrong-answer.mp3 &");
    if (send_notifications)
    {
      notify_init("ThemeStyler");
      NotifyNotification *notification = notify_notification_new(
          "Theme Reloaded!", NULL, "process-completed-symbolic");
      notify_notification_show(notification, NULL);
      g_object_unref(G_OBJECT(notification));
      notify_uninit();
    }
  }
};
// Serialize vector of Style objects to JSON string
std::string serializeStyles(const std::vector<Style> &styles, int c_style)
{
  rapidjson::StringBuffer buffer;
  rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);

  writer.StartArray();

  writer.StartObject();
  writer.Key("currentStyle");
  writer.Int(c_style);
  writer.EndObject();

  writer.StartObject();
  writer.Key("buttonSize");
  writer.Double(maxButtonWidth);
  writer.EndObject();

  writer.StartObject();
  writer.Key("buttonPerColumn");
  writer.Int(buttons_per_column);
  writer.EndObject();

  writer.StartObject();
  writer.Key("send_notification");
  writer.Bool(send_notifications);
  writer.EndObject();

  if (SystemTheme::first_name.empty() || SystemTheme::last_name.empty())
  {
    SystemTheme::first_name = "Graphite-";
    SystemTheme::last_name = "-nord";
  }

  writer.StartObject();
  writer.Key("shellTheme_first_name");
  writer.String(SystemTheme::first_name.c_str());
  writer.EndObject();

  writer.StartObject();
  writer.Key("shellTheme_last_name");
  writer.String(SystemTheme::last_name.c_str());
  writer.EndObject();

  for (const auto &style : styles)
  {
    writer.StartObject();
    writer.Key("name");
    writer.String(stringToBase64(style.name).c_str());
    writer.Key("command");
    writer.String(stringToBase64(style.command).c_str());
    writer.Key("is_dock_enabled");
    writer.Bool(style.is_dock_enabled);
    writer.EndObject();
  }
  writer.EndArray();

  return buffer.GetString();
}

// Deserialize JSON string to vector of Style objects
std::vector<Style> deserializeStyles(const std::string &jsonString)
{
  std::vector<Style> styles;
  rapidjson::Document document;
  document.Parse(jsonString.c_str());

  if (!document.IsArray())
  {
    std::cerr << "Error: Invalid JSON format\n";
    return styles;
  }

  // if (document.HasMember("currentStyle") && document["currentStyle"].IsInt())
  // {
  //     current_style = document["currentStyle"].GetInt();
  // }

  for (const auto &value : document.GetArray())
  {
    if (value.HasMember("shellTheme_first_name") && value.HasMember("shellTheme_last_name"))
    {
      SystemTheme::first_name = base64ToString(value["shellTheme_first_name"].GetString());
      SystemTheme::last_name = base64ToString(value["shellTheme_last_name"].GetString());
      CopyStringToCharArray(SystemTheme::first_name, fname, sizeof(fname));
      CopyStringToCharArray(SystemTheme::last_name, lname, sizeof(lname));
    }

    if (value.HasMember("name") && value.HasMember("command") &&
        value["name"].IsString() && value["command"].IsString() &&
        value.HasMember("is_dock_enabled"))
    {
      Style style;
      style.name = base64ToString(value["name"].GetString());
      style.command = base64ToString(value["command"].GetString());
      style.is_dock_enabled = value["is_dock_enabled"].GetBool();
      styles.push_back(style);
    }
    else if (value.HasMember("currentStyle") &&
             value["currentStyle"].IsInt())
    {
      current_style = value["currentStyle"].GetInt();
    }
    else if (value.HasMember("buttonSize") &&
             value["buttonSize"].IsDouble())
    {
      maxButtonWidth = value["buttonSize"].GetDouble();
    }
    else if (value.HasMember("buttonPerColumn") &&
             value["buttonPerColumn"].IsInt())
    {
      buttons_per_column = value["buttonPerColumn"].GetInt();
    }
    else if (value.HasMember("send_notification") &&
             value["send_notification"].IsBool())
    {
      send_notifications = value["send_notification"].GetBool();
    }
  }

  return styles;
}

// Save styles to JSON file
void saveStyles(const std::vector<Style> &styles, int c_style,
                const std::string &filename)
{
  // std::ofstream file(filename, std::ios::out | std::ios::trunc);
  // if (!file.is_open())
  // {

  //     size_t pos = filename.find_last_of("/\\");
  //     if (pos != std::string::npos)
  //     {
  //         // Extract the directory part
  //         std::string make_dir = "mkdir -p " + filename.substr(0, pos);
  //         system(make_dir.c_str());
  //         std::string chmodCommand = "chmod 777 " + filename.substr(0, pos);
  //         if (system(chmodCommand.c_str()) != 0)
  //         {
  //             std::cerr << "Error: Failed to change directory permissions" <<
  //             std::endl; return;
  //         }
  //     }

  //     // std::string touchCommand = "touch " + filename;
  //     // system(touchCommand.c_str());
  // }

  // // Write data to file
  // if (file.is_open())
  // {
  //     std::string jsonStr = serializeStyles(styles);
  //     file << jsonStr;
  //     file.close();
  // }
  // else
  // {
  //     std::cerr << "Error: Unable to open file for writing\n";
  // }
  std::cout << "Saving styles to " << filename << std::endl;
  writeFile(filename, serializeStyles(styles, c_style));
}

// Load styles from JSON file
std::vector<Style> loadStyles(const std::string &filename)
{
  std::vector<Style> styles;
  // std::ifstream file(filename);
  // if (file.is_open())
  // {
  //     std::string jsonString((std::istreambuf_iterator<char>(file)),
  //     std::istreambuf_iterator<char>()); styles =
  //     deserializeStyles(jsonString); file.close();
  // }
  // else
  // {
  //     std::cerr << "Error: Unable to open file for reading\n";
  // }

  std::string content = readFile(filename);
  if (!content.empty())
  {
    // File content successfully read, proceed with parsing or other operations
    styles = deserializeStyles(content);
  }
  else
  {
    // Error occurred while reading the file, handle it accordingly
    std::cerr << "Error: Unable to open file for reading" << std::endl;
  }

  return styles;
}

std::vector<Style> styles = {};

#define SAVE_FILE ".stylemanager/themes.json"

void CheckAndLoad()
{
  current_style = 0;
  std::string homeDir = getenv("HOME");
  if (!loadStyles(homeDir + "/" + SAVE_FILE).empty())
  {

    std::vector<Style> load_cache = loadStyles(homeDir + "/" + SAVE_FILE);
    std::cout << "loading from " << homeDir + "/" + SAVE_FILE << std::endl;
    for (auto style : load_cache)
    {
      styles.push_back(style);
    }
  }
  else
  {
    Style default_style;
    default_style.name = "Default";
    default_style.command = "#!/bin/bash";
    styles.push_back(default_style);
  }
}

void add_demo_syles()
{
  for (int i = 0; i < 10; i++)
  {
    Style newStyle;
    newStyle.name = "Style " + std::to_string(i);
    newStyle.command = "notify-send 'ThemeApplied: " + std::to_string(i) + "'";
    styles.push_back(newStyle);
  }
}

int main(int argc, char *argv[])
{
  if (argc >= 2)
  {
    std::cout << "Applying style: " << argv[1] << std::endl;
    std::string argument = argv[1];

    if (argument == "apply")
    {

      CheckAndLoad();
      styles[current_style].Apply();

      return 0;
    }
  }
  // add_demo_syles();
  CheckAndLoad();

  // Initialize GLFW
  glfwInit();
  GLFWwindow *window = glfwCreateWindow(800, 600, "StylerManager", NULL, NULL);
  // lf_init_glfw(800, 600, window);
  glfwMakeContextCurrent(window);
  glfwSwapInterval(0); // Enable vsync

  // Initialize glad
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    return -1;
  }

  // Setup ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  // DockSpace(nullptr);
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");

  bool editing = false;
  bool editing_popup = false;

  // Main loop
  while (!glfwWindowShouldClose(window))
  {
    // lf_begin();

    // lf_text("Hello, Leif!");

    // lf_end();

    glfwPollEvents();

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::StyleColorsVectorVertex();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 9;
    style.ChildRounding = 7;
    style.FrameRounding = 7;
    style.PopupRounding = 7;
    style.ScrollbarRounding = 9;
    style.GrabRounding = 8;
    style.TabRounding = 7;

    ImGui::DockSpaceOverViewport(NULL, ImGuiDockNodeFlags_PassthruCentralNode);
    // ImGui test window
    ImGui::Begin("Menu");
    if (ImGui::Button("Reload"))
    {
      styles.clear();
      CheckAndLoad();
    }

    // Get window width
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);

    float maxButtonSize = static_cast<float>(windowWidth) / buttons_per_column;

    // Clamp the button size between the minimum and maximum values
    float buttonSize =
        std::clamp(maxButtonSize, minButtonWidth, maxButtonWidth);

    // Calculate the number of columns based on window width and button size
    int numColumns = std::max(1, static_cast<int>(windowWidth / buttonSize));

    // Create a grid layout with dynamic number of columns
    ImGui::Columns(numColumns, nullptr, false);

    // Add items to the grid
    for (int i = 0; i < styles.size();
         ++i) // Adjust the number of items according to your needs
    {

      if (ImGui::Button(styles[i].name.c_str(),
                        ImVec2(buttonSize, buttonSize)))
      {
        // styles[i].Apply();
        // saveStyles(styles, SAVE_FILE);
        current_style = i;
        editing = false;
        std::cout << "Current Style: " << current_style << std::endl;
      }
      ImGui::NextColumn();
    }

    // End the grid layout
    ImGui::Columns(1);

    ImGui::End();

    ImGui::Begin("Style Editor");
    char name[1024];
    char command[1024];
    bool dock_enabled;
    if (!editing)
    {
      const std::string &styleName = styles[current_style].name;
      const std::string &styleCommand = styles[current_style].command;
      strncpy(name, styleName.c_str(), sizeof(name) - 1);
      if (!styleCommand.empty())
      {
        strncpy(command, styleCommand.c_str(), sizeof(command) - 1);
      }
      else
      {
        strncpy(command, "#!/bin/bash", sizeof(command) - 1);
      }
      name[sizeof(name) - 1] = '\0';

      dock_enabled = styles[current_style].is_dock_enabled;
      if (styleName.size() < sizeof(name))
      {
        strncpy(name, styleName.c_str(), sizeof(name) - 1);
        name[sizeof(name) - 1] = '\0'; // Ensure null-termination
      }
      editing = true;
    }

    ImGui::InputTextWithHint("Style name", "Name your style", name,
                             sizeof(name));

    ImGui::InputTextMultiline("Command", command, sizeof(command));

    ImGui::Checkbox("Show Dock", &dock_enabled);
    if (ImGui::Button("Save"))
    {
      styles[current_style].name = std::string(name);
      styles[current_style].command = std::string(command);
      styles[current_style].is_dock_enabled = dock_enabled;
      saveStyles(styles, current_style, SAVE_FILE);
      styles.clear();
      CheckAndLoad();
      editing = false;
    }
    if (ImGui::Button("Apply"))
    {
      styles[current_style].name = std::string(name);
      styles[current_style].command = std::string(command);
      styles[current_style].is_dock_enabled = dock_enabled;
      styles[current_style].Apply();
      saveStyles(styles, current_style, SAVE_FILE);
      styles.clear();
      CheckAndLoad();
      editing = false;
    }
    if (ImGui::Button("Delete"))
    {
      styles.erase(styles.begin() + current_style);
      saveStyles(styles, current_style, SAVE_FILE);
      styles.clear();
      CheckAndLoad();
      editing = false;
    }

    if (ImGui::Button("Add New Item"))
    {
      show_add_item_popup =
          true; // Set this variable to true when you want to show the popup
    }

    if (ImGui::Button("Settings"))
    {
      if (open_settings)
      {
        open_settings = false;
      }
      else
      {
        open_settings = true;
      }
    }
    // Inside the ImGui rendering loop:
    if (show_add_item_popup)
    {
      ImGui::OpenPopup("Add New Style"); // Open the popup window
    }
    if (ImGui::BeginPopupModal("Add New Style", &show_add_item_popup))
    {
      std::cout << "showing popup" << std::endl;
      Style newStyle;
      char new_name[1024];    // Direct initialization
      char new_command[1024]; // Direct initialization
      bool new_dock_style;
      if (!editing_popup)
      {
        strncpy(new_name, "New Style", sizeof(new_name) - 1);
        strncpy(new_command, styles[current_style].command.c_str(),
                sizeof(new_command) - 1);
        // new_dock_style = styles[current_style].is_dock_enabled;
        editing_popup = true;
      }

      // ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);
      // ImGui::Text("New Style");
      // ImGui::PopFont();

      // ImGui::InputTextWithHint("New Style name", "New Style", new_name,
      // sizeof(new_name)); ImGui::InputTextMultiline("New Command",
      // new_command, sizeof(new_command));

      // Declare flags to track whether the context menus are open

      // Declare flags to track whether the context menus are open

      // Inside the ImGui window or wherever you want to display the input
      // fields
      if (ImGui::InputTextWithHint("New Style name", "New Style", new_name,
                                   sizeof(new_name)))
      {
        // Update clipboard when input text changes
        ImGui::SetClipboardText(new_name);
      }

      if (ImGui::IsItemHovered() &&
          ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      {
        show_name_context_menu = true;
      }

      // Open the context menu when the flag is true
      if (show_name_context_menu)
      {
        ImGui::OpenPopup("Name Context Menu");
      }

      if (ImGui::BeginPopup("Name Context Menu"))
      {
        if (ImGui::MenuItem("Paste"))
        {
          const char *clipboardText = ImGui::GetClipboardText();
          if (clipboardText != nullptr)
          {
            // Paste clipboard text into the active input field
            strncpy(new_name, clipboardText, sizeof(new_name) - 1);
          }
          show_name_context_menu = false;
        }
        ImGui::EndPopup();
      }

      if (ImGui::InputTextMultiline("New Command", new_command,
                                    sizeof(new_command)))
      {
        // Update clipboard when input text changes
        ImGui::SetClipboardText(new_command);
      }
      ImGui::Checkbox("Show Dock: ", &new_dock_style);

      if (ImGui::IsItemHovered() &&
          ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      {
        show_command_context_menu = true;
      }

      // Open the context menu when the flag is true
      if (show_command_context_menu)
      {
        ImGui::OpenPopup("Command Context Menu");
      }

      if (ImGui::BeginPopup("Command Context Menu"))
      {
        if (ImGui::MenuItem("Paste"))
        {
          const char *clipboardText = ImGui::GetClipboardText();
          if (clipboardText != nullptr)
          {
            // Paste clipboard text into the active input field
            strncpy(new_command, clipboardText, sizeof(new_command) - 1);
          }
          show_command_context_menu = false;
        }
        ImGui::EndPopup();
      }

      if (ImGui::Button("Add"))
      {
        newStyle.name = std::string(new_name);
        newStyle.command = std::string(new_command);
        newStyle.is_dock_enabled = new_dock_style;
        styles.push_back(newStyle);
        saveStyles(styles, current_style, SAVE_FILE);
        editing_popup = false;
        ImGui::CloseCurrentPopup();
        ImGui::CloseCurrentPopup();
      }

      ImGui::EndPopup(); // End the popup window
    }

    ImGui::End();

    if (open_settings)
    {
      ImGui::Begin("Settings");
      
      ImGui::DragFloat("Style Scale", &maxButtonWidth, 5.0f, 120.0f, 900.0f);
      ImGui::DragInt("Styles per column", &buttons_per_column, 1.0f, 1, 30);
      ImGui::Checkbox(": Send notification after operation",
                      &send_notifications);
      ImGui::Checkbox(": Show ImguiEditor", &edit);
      ImGui::Separator();
      ImGui::Text("Theme Settings");
      ImGui::PushItemWidth(std::max(ImGui::CalcTextSize(fname).x + ImGui::GetStyle().FramePadding.x * 2.0f, 50.0f));
      ImGui::InputText("##FirstName", fname, IM_ARRAYSIZE(fname));
      ImGui::PopItemWidth();
      ImGui::SameLine();
      ImGui::Text("ACCENT COLOR");
      ImGui::SameLine();
      ImGui::PushItemWidth(std::max(ImGui::CalcTextSize(lname).x + ImGui::GetStyle().FramePadding.x * 2.0f, 50.0f));
      ImGui::InputText("##LastName", lname, IM_ARRAYSIZE(lname));
      ImGui::PopItemWidth();
      SystemTheme::updateNames(std::string(fname), std::string(lname));
      ImGui::End();
      if (edit)
      {
        ImGui::Begin("Imgui Editor");
        ImGui::ShowStyleEditor();
        ImGui::End();
      }
    }

    // Rendering
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.0f, 0.0f, 0.0f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // Swap buffers
    glfwSwapBuffers(window);
  }

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  // lf_terminate();
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
