#include "apply_system_theme.hpp"


int SystemTheme::color_distance(int r1, int g1, int b1, int r2, int g2, int b2)
{
    return std::pow(r1 - r2, 2) + std::pow(g1 - g2, 2) + std::pow(b1 - b2, 2);
}

// Function to convert a hex color code to RGB
std::tuple<int, int, int> SystemTheme::hex_to_rgb(const std::string &hex)
{
    int r = std::stoi(hex.substr(1, 2), nullptr, 16);
    int g = std::stoi(hex.substr(3, 2), nullptr, 16);
    int b = std::stoi(hex.substr(5, 2), nullptr, 16);
    return std::make_tuple(r, g, b);
}

// Function to extract the accent color from the pywal output file
std::string SystemTheme::extract_accent_color(const std::string &pywal_file)
{
    std::ifstream file(pywal_file);
    if (!file.is_open())
    {
        std::cerr << "Error: The pywal output file '" << pywal_file << "' does not exist." << std::endl;
        exit(1);
    }

    std::string line;
    int line_number = 0;
    while (std::getline(file, line))
    {
        line_number++;
        if (line_number == 2)
        {
            return line;
        }
    }
    return "";
}

// Function to find the least similar color
std::string SystemTheme::find_least_similar_color(const std::tuple<int, int, int> &accent_rgb)
{
    

    int min_distance = 999999;
    std::string least_similar_color;

    for (const auto &color : colors)
    {
        const auto &[name, r, g, b] = color;
        int distance = color_distance(std::get<0>(accent_rgb), std::get<1>(accent_rgb), std::get<2>(accent_rgb), r, g, b);
        if (distance < min_distance)
        {
            least_similar_color = name;
            min_distance = distance;
        }
    }

    return least_similar_color;
}

// Function to apply themes based on color
void SystemTheme::apply_theme(const std::string &least_similar_color)
{
    std::string app_theme = first_name + least_similar_color + last_name;
    std::string command = "gsettings set org.gnome.desktop.interface gtk-theme '" + app_theme + "'";
    system(command.c_str());

    // Set shell theme to match the application theme
    std::string shell_theme = app_theme;
    command = "gsettings set org.gnome.shell.extensions.user-theme name '" + app_theme + "'";
    system(command.c_str());

    // Copy theme files for gtk-4.0 configuration
    command = "cp -rv ~/.local/share/themes/" + app_theme + "/gtk-4.0/** ~/.config/gtk-4.0";
    system(command.c_str());

    std::cout << "Applied GNOME Shell theme: " << shell_theme << std::endl;

    // Set icon theme
    std::string icon_theme;
    if (least_similar_color == "yellow")
    {
        icon_theme = "Reversal";
    }
    else
    {
        icon_theme = "Reversal-" + least_similar_color + "-dark";
    }
    command = "gsettings set org.gnome.desktop.interface icon-theme '" + icon_theme + "'";
    system(command.c_str());

    if (least_similar_color == "black")
    {
        system("gsettings set org.gnome.shell.extensions.user-theme name 'Graphite-Dark-nord'");
        system("gsettings set org.gnome.desktop.interface gtk-theme 'Graphite-Dark-nord'");
        system("gsettings set org.gnome.desktop.interface icon-theme 'Reversal-black'");
    }

    std::cout << "Applied application themes: " << app_theme << std::endl;
    std::cout << "Applied icon theme: " << icon_theme << std::endl;
}

void SystemTheme::apply_systemwide()
{
    std::string pywal_file = "/home/bios/.cache/wal/colors";

    // Extract the accent color hex code
    std::string accent_color_hex = extract_accent_color(pywal_file);
    std::cout << "Accent Color Hex: " << accent_color_hex << std::endl;

    // Convert the hex color code to RGB
    auto accent_rgb = hex_to_rgb(accent_color_hex);

    // Find the least similar color
    std::string least_similar_color = find_least_similar_color(accent_rgb);

    if (!least_similar_color.empty())
    {
        apply_theme(least_similar_color);
    }
    else
    {
        std::cout << "No specific theme condition met. Exiting without applying themes." << std::endl;
        
    }

}

void SystemTheme::updateNames(const std::string fname, const std::string &name)
{
    first_name = fname;
    last_name = name;
}
