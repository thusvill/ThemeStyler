#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <string>
#include <vector>
#include <tuple>
#include <cstdlib>

class SystemTheme
{
public:
    static void apply_systemwide();
    static std::vector<std::tuple<std::string, int, int, int>> colors;
    static std::string first_name, last_name;
    static void updateNames(const std::string fname, const std::string &name);

private:
    static int color_distance(int r1, int g1, int b1, int r2, int g2, int b2);
    static std::tuple<int, int, int> hex_to_rgb(const std::string &hex);
    static std::string extract_accent_color(const std::string &pywal_file);
    static std::string find_least_similar_color(const std::tuple<int, int, int> &accent_rgb);
    static void apply_theme(const std::string &least_similar_color);
};

