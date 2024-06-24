#pragma GCC optimize("O3")
#pragma GCC optimize("inline")
#pragma GCC optimize("omit-frame-pointer")
#pragma GCC optimize("unroll-loops")

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
#include <limits>
#include <cstdint>


const int TURN_SIMULATED = 4;
const unsigned int ACTIONS_MAX = 256; // equal to 4 ** TURN_SIMULATED but faster to hard code than use power

// we use this order so that the u_int correspond to the number of space in hurdle race
std::map<int, std::string> intToAction = {{0, "UP"}, {1, "LEFT"}, {2, "DOWN"}, {3, "RIGHT"}};
std::map<int, char> intToLetter = {{0, 'U'}, {1, 'L'}, {2, 'D'}, {3, 'R'}};


class Hurdle {
public:
    int position, starting_position, stunDuration, turnFinished, optimalTurnFinished, optimalDistance;
    bool finished;
    std::string field;

    Hurdle(int position, int stunDuration, const std::string &field)
        : position(position), starting_position(position), stunDuration(stunDuration), field(field), finished(position >= 29), turnFinished(position >= 29 ? -1 : 0) {
        std::tie(optimalTurnFinished, optimalDistance) = find_optimal(TURN_SIMULATED);
    }

    Hurdle(const Hurdle &other)
        : position(other.position), starting_position(other.starting_position),
          stunDuration(other.stunDuration), field(other.field),
          finished(other.finished), turnFinished(other.turnFinished),
          optimalTurnFinished(other.optimalTurnFinished),
          optimalDistance(other.optimalDistance) {}

    Hurdle& operator=(const Hurdle &other) {
        if (this != &other) {
            position = other.position;
            starting_position = other.starting_position;
            stunDuration = other.stunDuration;
            field = other.field;
            finished = other.finished;
            turnFinished = other.turnFinished;
            optimalTurnFinished = other.optimalTurnFinished;
            optimalDistance = other.optimalDistance;
        }
        return *this;
    }

    void playTurn(std::uint8_t action, int turn_nb) {
        if (position >= 29 || field == "GAME_OVER") return;
        if (stunDuration > 0) {
            stunDuration -= 1;
            return;
        }
        if (action == 0) {
            position += 2;
            if (position < 30 && field[position] == '#') {
                stunDuration = 2;
            }
        } else {
            for (int i = 0; i < action; ++i) {
                position += 1;
                if (position < 30 && field[position] == '#') {
                    stunDuration = 2;
                    break;
                }
            }
        }
        if (position >= 29) {
            finished = true;
            turnFinished = turn_nb;
        }
    }

    std::tuple<int, int> find_optimal(int nb_turn) {
        int s = position;
        if (s >= 29 || field == "GAME_OVER") return std::make_tuple(0, 29);
        for (int i = 0; i < nb_turn; ++i) {
            if (s + 1 < 30 && field[s + 1] == '#') {
                s += 2;
            } else if (s + 2 < 30 && field[s + 2] == '#') {
                s += 1;
            } else if (s + 3 < 30 && field[s + 3] == '#') {
                s += 2;
            } else {
                s += 3;
            }
            if (s >= 29) return std::make_tuple(i + 1, 29);
        }
        return std::make_tuple(-1, s);
    }

    double evaluate() {
        if (field == "GAME_OVER") return 0;
        if (finished) return optimalTurnFinished / static_cast<double>(turnFinished) * 100;
        if (optimalTurnFinished != -1) optimalDistance += (TURN_SIMULATED - optimalTurnFinished) * 2;
        return (position - starting_position) / static_cast<double>(optimalDistance - starting_position) * 100;
    }
};

class Archery {
public:
    int x_coord;
    int y_coord;
    int remainingTurns;
    std::string windSpeed;
    unsigned int len_windSpeed;

    Archery(int x_coord, int y_coord, int remainingTurns, const std::string &windSpeed)
        : x_coord(x_coord), y_coord(y_coord), remainingTurns(remainingTurns), windSpeed(windSpeed), len_windSpeed(windSpeed.size()) {}

    Archery(const Archery &other)
        : x_coord(other.x_coord), y_coord(other.y_coord), remainingTurns(other.remainingTurns),
          windSpeed(other.windSpeed), len_windSpeed(other.len_windSpeed) {}

    Archery& operator=(const Archery &other) {
        if (this != &other) {
            x_coord = other.x_coord;
            y_coord = other.y_coord;
            remainingTurns = other.remainingTurns;
            windSpeed = other.windSpeed;
            len_windSpeed = other.len_windSpeed;
        }
        return *this;
    }

    void playTurn(std::uint8_t action) {
        if (remainingTurns == 0 || windSpeed == "GAME_OVER") return;
        int curWind = windSpeed[len_windSpeed - remainingTurns] - '0';
        if (action == 0) y_coord -= curWind;
        else if (action == 2) y_coord += curWind;
        else if (action == 1) x_coord -= curWind;
        else if (action == 3) x_coord += curWind;
        remainingTurns -= 1;
        if (x_coord < -20) x_coord = -20;
        if (x_coord > 20) x_coord = 20;
        if (y_coord < -20) y_coord = -20;
        if (y_coord > 20) y_coord = 20;
    }

    double evaluate(int turns_left) {
        if (windSpeed == "GAME_OVER" || turns_left < remainingTurns) return 0;
        return (800 - (x_coord * x_coord + y_coord * y_coord)) / 800.0 * 100;
    }
};

class Skating {
public:
    int position, starting_position, risk, starting_risk, stunDuration, remainingTurns;
    std::string curRisk;

    Skating(int position, int risk, int stunDuration, int remainingTurns, const std::string &curRisk)
        : position(position), starting_position(position), risk(risk), starting_risk(risk), stunDuration(stunDuration), remainingTurns(remainingTurns), curRisk(curRisk) {}

    Skating(const Skating &other)
        : position(other.position), starting_position(other.starting_position),
          risk(other.risk), starting_risk(other.starting_risk),
          stunDuration(other.stunDuration), remainingTurns(other.remainingTurns),
          curRisk(other.curRisk) {}
    
    Skating& operator=(const Skating &other) {
        if (this != &other) {
            position = other.position;
            starting_position = other.starting_position;
            risk = other.risk;
            starting_risk = other.starting_risk;
            stunDuration = other.stunDuration;
            remainingTurns = other.remainingTurns;
            curRisk = other.curRisk;
        }
        return *this;
    }

    void playTurn(std::uint8_t action, int turn_nb) {
        if (turn_nb > 0 || curRisk == "GAME_OVER") return;
        if (stunDuration > 0) {
            stunDuration -= 1;
            return;
        }
        int riskIdx = curRisk.find(intToLetter[action]);
        if (riskIdx == 0) {
            position += 1;
            if (risk > 0) risk -= 1;
        } else if (riskIdx == 1) {
            position += 2;
        } else if (riskIdx == 2) {
            position += 2;
            risk += 1;
        } else {
            position += 3;
            risk += 2;
        }
        if (risk >= 5) {
            stunDuration = 2;
            risk = 0;
        }
        remainingTurns -= 1;
    }

    double evaluate(int turns_left) {
        if (curRisk == "GAME_OVER" || turns_left < remainingTurns) return 0;
        int space_gain = position - starting_position;
        int risk_gain = risk - starting_risk;
        if (risk == 4) risk_gain *= 2;
        else if (risk == 3) risk_gain *= 1.5;
        if (risk_gain == -1 && risk <= 1) risk_gain = 0;
        return (space_gain * 1.5 - risk_gain - 3 * stunDuration) / 3.0 * 100;
    }
};

class Diving {
public:
    int curCombo, curPoints, remainingTurns, optimalPoints, optimalCombo;
    std::string moveSequence;
    unsigned int len_moveSequence;

    Diving(int curCombo, int curPoints, int remainingTurns, const std::string &moveSequence)
        : curCombo(curCombo), curPoints(curPoints), remainingTurns(remainingTurns), moveSequence(moveSequence), len_moveSequence(moveSequence.size()) {
        std::tie(optimalPoints, optimalCombo) = find_optimal(TURN_SIMULATED);
    }

    Diving(const Diving &other)
        : curCombo(other.curCombo), curPoints(other.curPoints),
          remainingTurns(other.remainingTurns), moveSequence(other.moveSequence),
          optimalPoints(other.optimalPoints), optimalCombo(other.optimalCombo), len_moveSequence(other.len_moveSequence) {}

    Diving& operator=(const Diving &other) {
        if (this != &other) {
            curCombo = other.curCombo;
            curPoints = other.curPoints;
            remainingTurns = other.remainingTurns;
            moveSequence = other.moveSequence;
            optimalPoints = other.optimalPoints;
            optimalCombo = other.optimalCombo;
            len_moveSequence = other.len_moveSequence;
        }
        return *this;
    }

    void playTurn(std::uint8_t action) {
        if (remainingTurns == 0 || moveSequence == "GAME_OVER") return;
        if (intToLetter[action] == moveSequence[len_moveSequence - remainingTurns]) {
            curCombo += 1;
        } else {
            curCombo = 0;
        }
        curPoints += curCombo;
    }

    std::tuple<int, int> find_optimal(int nb_turn) {
        int points = curPoints;
        int combo = curCombo;
        for (int i = 0; i < std::min(nb_turn, remainingTurns); ++i) {
            combo += 1;
            points += combo;
        }
        return std::make_tuple(points, combo);
    }

    double evaluate(int turns_left) {
        if (moveSequence == "GAME_OVER" || turns_left < remainingTurns) return 0;
        double optimal = (optimalPoints + optimalCombo * 2) * 0.5;
        return (curPoints + curCombo * 2) / optimal * 100;
    }
};

// Game multiplier function
int game_multiplier(int game_score) {
    if (game_score == 0) return 100;
    return 50 / game_score;
}

class Player {
public:
    Hurdle hurdle;
    Archery archery;
    Skating skating;
    Diving diving;
    std::vector<int> score;

    Player(Hurdle hurdle, Archery archery, Skating skating, Diving diving, std::vector<int> score)
        : hurdle(hurdle), archery(archery), skating(skating), diving(diving), score(score) {}

    Player(const Player &other)
    : hurdle(other.hurdle), archery(other.archery), skating(other.skating), diving(other.diving), score(other.score) {}

    Player& operator=(const Player &other) {
        if (this != &other) {
            hurdle = other.hurdle;
            archery = other.archery;
            skating = other.skating;
            diving = other.diving;
            score = other.score;
        }
        return *this;
    }

    void playTurn(std::uint8_t action, int turn_nb) {
        hurdle.playTurn(action, turn_nb);
        archery.playTurn(action);
        skating.playTurn(action, turn_nb);
        diving.playTurn(action);
    }

    void playTurns(unsigned int actions) {
        for (int i = 0; i < TURN_SIMULATED; ++i) {
            playTurn(actions & 3, i + 1);
			actions >>= 2;
        }
    }

    double evaluate(int turns_left) {
        // double a = hurdle.evaluate();
        // double b = archery.evaluate(turns_left);
        // double c = skating.evaluate(turns_left);
        // double d = diving.evaluate(turns_left);
        // int e = game_multiplier(score[0]);
        // int f = game_multiplier(score[1]);
        // int g = game_multiplier(score[2]);
        // int h = game_multiplier(score[3]);
        return hurdle.evaluate() * game_multiplier(score[0]) + archery.evaluate(turns_left) * game_multiplier(score[1]) + skating.evaluate(turns_left) * game_multiplier(score[2]) + diving.evaluate(turns_left) * game_multiplier(score[3]);
    }
};


std::vector<Hurdle> createHurdleArray(const std::string &gpu, int reg_0, int reg_1, int reg_2, int reg_3, int reg_4, int reg_5, int reg_6) {
    std::vector<Hurdle> hurdles;
    hurdles.push_back(Hurdle(reg_0, reg_3, gpu));
    hurdles.push_back(Hurdle(reg_1, reg_4, gpu));
    hurdles.push_back(Hurdle(reg_2, reg_5, gpu));
    return hurdles;
} 

std::vector<Archery> createArcheryArray(const std::string &gpu, int reg_0, int reg_1, int reg_2, int reg_3, int reg_4, int reg_5, int reg_6) {
    std::vector<Archery> archeries;
    archeries.push_back(Archery(reg_0, reg_1, gpu.size(), gpu));
    archeries.push_back(Archery(reg_2, reg_3, gpu.size(), gpu));
    archeries.push_back(Archery(reg_4, reg_5, gpu.size(), gpu));
    return archeries;
} 

std::vector<Skating> createSkatingArray(const std::string &gpu, int reg_0, int reg_1, int reg_2, int reg_3, int reg_4, int reg_5, int reg_6) {
    std::vector<Skating> skatings;
    skatings.push_back(Skating(reg_0, reg_3 > 0 ? reg_3 : 0, reg_3 < 0 ? -reg_3 : 0, reg_6, gpu));
    skatings.push_back(Skating(reg_1, reg_4 > 0 ? reg_4 : 0, reg_4 < 0 ? -reg_4 : 0, reg_6, gpu));
    skatings.push_back(Skating(reg_2, reg_5 > 0 ? reg_5 : 0, reg_5 < 0 ? -reg_5 : 0, reg_6, gpu));
    return skatings;
}

std::vector<Diving> createDivingArray(const std::string &gpu, int reg_0, int reg_1, int reg_2, int reg_3, int reg_4, int reg_5, int reg_6) {
    std::vector<Diving> divings;
    divings.push_back(Diving(reg_3, reg_0, gpu.size(), gpu));
    divings.push_back(Diving(reg_4, reg_1, gpu.size(), gpu));
    divings.push_back(Diving(reg_5, reg_2, gpu.size(), gpu));
    return divings;
}

int main() {
    int player_idx;
    std::cin >> player_idx; std::cin.ignore();
    int nb_games;
    std::cin >> nb_games; std::cin.ignore();

	int turns_left = 100;
    // Game loop
    while (true) {
        double timestamp = std::clock();
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

        std::vector<Hurdle> hurdles;
        std::vector<Archery> archeries;
        std::vector<Skating> skatings;
        std::vector<Diving> divings;

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
            if (i == 0) {
                hurdles = createHurdleArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_6);
            } else if (i == 1) {
                archeries = createArcheryArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_6);
            } else if (i == 2) {
                skatings = createSkatingArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_6);
            } else {
                divings = createDivingArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_6);
            }
        }

        std::vector<Player> players;

        for (int i = 0; i < 3; ++i) {
            players.push_back(Player(hurdles[i], archeries[i], skatings[i], divings[i], scores[i]));
        }

        Player myself = players[player_idx];
        std::pair<double, unsigned int> best_score = {std::numeric_limits<double>::lowest(), 0};

        for (unsigned int i = 0; i < ACTIONS_MAX; ++i) {
			Player copy_of_myself = myself;
			copy_of_myself.playTurns(i);
			double score = copy_of_myself.evaluate(turns_left);
			if (score > best_score.first) {
				best_score = {score, i};
			}
        }
        std::cerr << (std::clock() - timestamp) / CLOCKS_PER_SEC << std::endl;
        std::cout << intToAction[best_score.second & 3] << std::endl;
        turns_left -= 1;
    }

    return 0;
}