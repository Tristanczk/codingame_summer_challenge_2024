#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <tuple>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cstring>
#include <sstream>
#include <iomanip>
#include <iterator>


int main() {
    int player_idx;
    std::cin >> player_idx; std::cin.ignore();
    int nb_games;
    std::cin >> nb_games; std::cin.ignore();
    std::vector<std::string> actions = {"UP", "DOWN", "LEFT", "RIGHT"};

    // Game loop
    while (true) {
        std::vector<std::vector<int>> scores(3, std::vector<int>(4));

        for (int i = 0; i < 3; ++i) {
            std::string score_info;
            std::getline(std::cin >> std::ws, score_info);
            std::istringstream ss(score_info);
            std::vector<int> break_score((std::istream_iterator<int>(ss)), std::istream_iterator<int>());
            for (int j = 0; j < 4; ++j) {
                scores[i][j] = break_score[1 + j * 3] * 3 + break_score[2 + j * 3];
            }
        }


        for (int i = 0; i < nb_games; ++i) {
            std::string input;
            std::getline(std::cin >> std::ws, input);
            std::istringstream ss(input);
            std::vector<std::string> tokens((std::istream_iterator<std::string>(ss)), std::istream_iterator<std::string>());
            std::string gpu = tokens[0];
            int reg_0 = std::stoi(tokens[1]);
            int reg_1 = std::stoi(tokens[2]);
            int reg_2 = std::stoi(tokens[3]);
            int reg_3 = std::stoi(tokens[4]);
            int reg_4 = std::stoi(tokens[5]);
            int reg_5 = std::stoi(tokens[6]);
            int reg_6 = std::stoi(tokens[7]);
        }
        std::cout << "LEFT" << std::endl;
    }

    return 0;
}