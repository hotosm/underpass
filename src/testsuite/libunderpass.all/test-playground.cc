//
// Copyright (c) 2020, 2021, 2022, 2023 Humanitarian OpenStreetMap Team
//
// This file is part of Underpass.
//
//     Underpass is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     Underpass is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with Underpass.  If not, see <https://www.gnu.org/licenses/>.
//

#include <iostream>
#include <map>
#include <sstream>

std::map<std::string, std::string> parseTagsString(const std::string& input) {
    std::map<std::string, std::string> result;
    std::stringstream ss(input);
    std::string token;

    while (std::getline(ss, token, ',')) {
        // std::cout << token << std::endl;
        // Remove leading and trailing whitespaces
        // token.erase(token.find_first_not_of(" "), token.find_last_not_of(" ") + 1);
        // std::cout << token << std::endl;
        // Find the position of the arrow
        size_t arrowPos = token.find("=>");
        if (arrowPos != std::string::npos) {
            std::string key = token.substr(0, arrowPos);
            std::string value = token.substr(arrowPos + 2);
            result[key] = value;
        }
    }

    return result;
}

int main() {
    std::string input = "\"fee\"=>\"no\", \"access\"=>\"yes\"";
    std::map<std::string, std::string> resultMap = parseTagsString(input);

    // Iterate over the map and print the key-value pairs
    for (const auto& pair : resultMap) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }

    return 0;
}


// local Variables:
// mode: C++
// indent-tabs-mode: nil
// End:
