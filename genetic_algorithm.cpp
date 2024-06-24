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
#include <unordered_set>
#include <random>
#include <climits>
#include <chrono>



const int TURN_SIMULATED = 16;
const double INITIAL_MUTATION_RATE = 0.1;
const double DECAY_FACTOR = 0.1;
const unsigned int ACTIONS_MAX = 16384; // equal to 4 ** TURN_SIMULATED but faster to hard code than use power

// we use this order so that the u_int correspond to the number of space in hurdle race
std::map<int, std::string> intToAction = {{0, "UP"}, {1, "LEFT"}, {2, "DOWN"}, {3, "RIGHT"}};
std::map<int, char> intToLetter = {{0, 'U'}, {1, 'L'}, {2, 'D'}, {3, 'R'}};


class Hurdle {
public:
    std::string field;
    int position, starting_position, stunDuration, turnFinished, optimalTurnFinished, optimalDistance;
    bool finished;

    Hurdle(int position, int stunDuration, const std::string &field)
        : field(field), position(position), starting_position(position), stunDuration(stunDuration), turnFinished(position >= 29 ? 50 : 0), finished(position >= 29) {
        std::tie(optimalTurnFinished, optimalDistance) = find_optimal(TURN_SIMULATED);
    }

    Hurdle(const Hurdle &other)
        : field(other.field), position(other.position), starting_position(other.starting_position),
          stunDuration(other.stunDuration), turnFinished(other.turnFinished),
          optimalTurnFinished(other.optimalTurnFinished),
          optimalDistance(other.optimalDistance),
          finished(other.finished) {}

    Hurdle& operator=(const Hurdle &other) {
        if (this != &other) {
            field = other.field;
            position = other.position;
            starting_position = other.starting_position;
            stunDuration = other.stunDuration;
            turnFinished = other.turnFinished;
            optimalTurnFinished = other.optimalTurnFinished;
            optimalDistance = other.optimalDistance;
            finished = other.finished;
        }
        return *this;
    }

    void playTurn(std::uint8_t action, int turn_nb, int turns_left) {
        if (position >= 29 || field == "GAME_OVER" || turn_nb > turns_left) return;
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
        return std::make_tuple(50, s);
    }

    double evaluate() {
        if (field == "GAME_OVER") return 0;
        if (finished) return optimalTurnFinished / static_cast<double>(turnFinished) * 100;
        if (optimalTurnFinished != 50) optimalDistance += (TURN_SIMULATED - optimalTurnFinished) * 2;
        return (position - starting_position) / static_cast<double>(optimalDistance - starting_position) * 100;
    }

    unsigned int evaluate_compete(Hurdle &oppo1, Hurdle &oppo2) {
        if (field == "GAME_OVER" || !finished) return 0;
        if (turnFinished <= oppo1.turnFinished && turnFinished <= oppo2.turnFinished) {
            return 3;
        } else if (turnFinished <= oppo1.turnFinished || turnFinished <= oppo2.turnFinished) {
            return 1;
        } else {
            return 0;
        }
    }
};

class Archery {
public:
    int x_coord;
    int y_coord;
    int remainingTurns;
    std::string windSpeed;
    unsigned int len_windSpeed;
    int dist_to_target;

    Archery(int x_coord, int y_coord, int remainingTurns, const std::string &windSpeed)
        : x_coord(x_coord), y_coord(y_coord), remainingTurns(remainingTurns), windSpeed(windSpeed), len_windSpeed(windSpeed.size()), dist_to_target(40) {}

    Archery(const Archery &other)
        : x_coord(other.x_coord), y_coord(other.y_coord), remainingTurns(other.remainingTurns),
          windSpeed(other.windSpeed), len_windSpeed(other.len_windSpeed), dist_to_target(other.dist_to_target) {}

    Archery& operator=(const Archery &other) {
        if (this != &other) {
            x_coord = other.x_coord;
            y_coord = other.y_coord;
            remainingTurns = other.remainingTurns;
            windSpeed = other.windSpeed;
            len_windSpeed = other.len_windSpeed;
            dist_to_target = other.dist_to_target;
        }
        return *this;
    }

    void playTurn(std::uint8_t action, int turns_left) {
        if (windSpeed == "GAME_OVER" || turns_left < remainingTurns) return;
        if (remainingTurns == 0) {
            dist_to_target = x_coord * x_coord + y_coord * y_coord;
            return;
        } 
        int curWind = windSpeed[len_windSpeed - remainingTurns] - '0';
        if (action == 0) y_coord -= curWind;
        else if (action == 2) y_coord += curWind;
        else if (action == 1) x_coord -= curWind;
        else if (action == 3) x_coord += curWind;
        remainingTurns -= 1;
        if (x_coord < -20) x_coord = -20;
        else if (x_coord > 20) x_coord = 20;
        if (y_coord < -20) y_coord = -20;
        else if (y_coord > 20) y_coord = 20;
    }

    double evaluate(int turns_left) {
        if (windSpeed == "GAME_OVER" || turns_left < remainingTurns) return 0;
        return (800 - (x_coord * x_coord + y_coord * y_coord)) / 800.0 * 100;
    }

    unsigned int evaluate_compete(int turns_left, Archery &oppo1, Archery &oppo2) {
        if (windSpeed == "GAME_OVER" || turns_left < remainingTurns) return 0;
        if (dist_to_target >= oppo1.dist_to_target && dist_to_target >= oppo2.dist_to_target) {
            return 3;
        } else if (dist_to_target >= oppo1.dist_to_target || dist_to_target >= oppo2.dist_to_target) {
            return 1;
        } else {
            return 0;
        }
    }
};

class Skating {
public:
    int position, starting_position, adjusted_position, risk, starting_risk, stunDuration, remainingTurns;
    std::string curRisk;

    Skating(int position, int risk, int stunDuration, int remainingTurns, const std::string &curRisk)
        : position(position), starting_position(position), adjusted_position(position), risk(risk), starting_risk(risk), stunDuration(stunDuration), remainingTurns(remainingTurns), curRisk(curRisk) {}

    Skating(const Skating &other)
        : position(other.position), starting_position(other.starting_position), adjusted_position(other.adjusted_position),
          risk(other.risk), starting_risk(other.starting_risk),
          stunDuration(other.stunDuration), remainingTurns(other.remainingTurns),
          curRisk(other.curRisk) {}
    
    Skating& operator=(const Skating &other) {
        if (this != &other) {
            position = other.position;
            starting_position = other.starting_position;
            adjusted_position = other.adjusted_position;
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
            adjusted_position = 2 * position - 4 * stunDuration - risk;
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
        adjusted_position = 2 * position - 4 * stunDuration - risk;
    }

    void playTurnCompete(std::uint8_t action, int turn_nb, Skating &oppo1, Skating &oppo2) {
        if (turn_nb > 0 || curRisk == "GAME_OVER") return;
        if (stunDuration > 0) {
            stunDuration -= 1;
            adjusted_position = 2 * position - 4 * stunDuration - risk;
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
        if (position % 10 == oppo1.position % 10 || position % 10 == oppo2.position % 10) {
           risk += 2;
        }
        if (risk >= 5) {
            stunDuration = 2;
            risk = 0;
        }
        adjusted_position = 2 * position - 4 * stunDuration - risk;
    }

    double evaluate(int turns_left) {
        if (curRisk == "GAME_OVER" || turns_left < remainingTurns) return 0;
        int space_gain = position - starting_position;
        int risk_gain = risk - starting_risk;
        if (risk == 4) risk_gain *= 2;
        else if (risk == 3) risk_gain *= 1.5;
        if (risk_gain == -1 && risk <= 1) risk_gain = 0;
        return std::max((space_gain * 1.5 - risk_gain - 3 * stunDuration) / 3.0 * 100, 0.0);
    }

    unsigned int evaluate_compete(int turns_left, Skating &oppo1, Skating &oppo2) {
        if (curRisk == "GAME_OVER" || turns_left < remainingTurns) return 0;
        if (remainingTurns == 0) {
            if (position >= oppo1.position && position >= oppo2.position) {
                return 3;
            } else if (position >= oppo1.position || position >= oppo2.position) {
                return 1;
            } else {
                return 0;
            }
        } else {
            if (adjusted_position >= oppo1.adjusted_position && adjusted_position >= oppo2.adjusted_position) {
                return 2; // less valorization to be first as the game is not finished
            } else if (adjusted_position >= oppo1.adjusted_position || adjusted_position >= oppo2.adjusted_position) {
                return 1;
            } else {
                return 0;
            }
        }
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
          remainingTurns(other.remainingTurns), optimalPoints(other.optimalPoints), 
          optimalCombo(other.optimalCombo), moveSequence(other.moveSequence), 
          len_moveSequence(other.len_moveSequence) {}

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

    void playTurn(std::uint8_t action, int turns_left) {
        if (remainingTurns == 0 || moveSequence == "GAME_OVER" || turns_left < remainingTurns) return;
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

    unsigned int evaluate_compete(int turns_left, Diving &oppo1, Diving &oppo2) {
        if (moveSequence == "GAME_OVER" || turns_left < remainingTurns) return 0;
        if (curPoints >= oppo1.curPoints && curPoints >= oppo2.curPoints) {
            return 3;
        } else if (curPoints >= oppo1.curPoints || curPoints >= oppo2.curPoints) {
            return 1;
        } else {
            return 0;
        }
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

    void playTurn(std::uint8_t action, int turn_nb, int turns_left) {
        hurdle.playTurn(action, turn_nb, turns_left);
        archery.playTurn(action, turns_left);
        skating.playTurn(action, turn_nb);
        diving.playTurn(action, turns_left);
    }

    void playTurnCompete(std::uint8_t action, int turn_nb, int turns_left, Player &oppo1, Player &oppo2) {
        hurdle.playTurn(action, turn_nb, turns_left);
        archery.playTurn(action, turns_left);
        skating.playTurnCompete(action, turn_nb, oppo1.skating, oppo2.skating);
        diving.playTurn(action, turns_left);
    }

    void playTurns(unsigned int actions, int turns_left) {
        for (int i = 0; i < TURN_SIMULATED; ++i) {
            playTurn(actions & 3, i + 1, turns_left);
			actions >>= 2;
        }
    }

    void playTurnsCompete(unsigned int actions, int turns_left, Player &oppo1, Player &oppo2) {
        for (int i = 0; i < TURN_SIMULATED; ++i) {
            playTurnCompete(actions & 3, i + 1, turns_left, oppo1, oppo2);
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

    unsigned int evaluate_compete(int turns_left, Player &oppo1, Player &oppo2) {
        unsigned int a = hurdle.evaluate_compete(oppo1.hurdle, oppo2.hurdle);
        unsigned int b = archery.evaluate_compete(turns_left, oppo1.archery, oppo2.archery);
        unsigned int c = skating.evaluate_compete(turns_left, oppo1.skating, oppo2.skating);
        unsigned int d = diving.evaluate_compete(turns_left, oppo1.diving, oppo2.diving);
        // std::cerr << a << " " << b << " " << c << " " << d << std::endl;
        // to handle equality, especially if a score is 0
        return (score[0] + a) * (score[1] + b) * (score[2] + c) * (score[3] + d) * 1000 + (a + b + c + d);
    }
};


std::vector<Hurdle> createHurdleArray(const std::string &gpu, int reg_0, int reg_1, int reg_2, int reg_3, int reg_4, int reg_5) {
    std::vector<Hurdle> hurdles;
    hurdles.push_back(Hurdle(reg_0, reg_3, gpu));
    hurdles.push_back(Hurdle(reg_1, reg_4, gpu));
    hurdles.push_back(Hurdle(reg_2, reg_5, gpu));
    return hurdles;
} 

std::vector<Archery> createArcheryArray(const std::string &gpu, int reg_0, int reg_1, int reg_2, int reg_3, int reg_4, int reg_5) {
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

std::vector<Diving> createDivingArray(const std::string &gpu, int reg_0, int reg_1, int reg_2, int reg_3, int reg_4, int reg_5) {
    std::vector<Diving> divings;
    divings.push_back(Diving(reg_3, reg_0, gpu.size(), gpu));
    divings.push_back(Diving(reg_4, reg_1, gpu.size(), gpu));
    divings.push_back(Diving(reg_5, reg_2, gpu.size(), gpu));
    return divings;
}

std::unordered_set<unsigned int> generate_random_population(unsigned int size) {
    std::unordered_set<unsigned int> population;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, UINT_MAX);

    while (population.size() < size) {
        population.insert(dis(gen));
    }

    return population;
}

std::tuple<std::vector<std::pair<unsigned int, unsigned int>>, Player> evaluate_solutions_gen1(std::unordered_set<unsigned int> &population, Player &player, int turns_left) {
    std::vector<std::pair<unsigned int, unsigned int>> scores;
    unsigned int cur_score = 0;
    Player best_player = player;

    for (auto it = population.begin(); it != population.end(); ++it) {
        Player copy_player = player;
        copy_player.playTurns(*it, turns_left);
        unsigned int score = static_cast<unsigned int>(copy_player.evaluate(turns_left));
        scores.push_back({score, *it});
        if (score > cur_score) {
            cur_score = score;
            best_player = copy_player;
        }
    }
    
    return std::make_tuple(scores, best_player);
}

std::tuple<std::vector<std::pair<unsigned int, unsigned int>>, Player, unsigned int> evaluate_solutions(std::unordered_set<unsigned int> &population, Player &player, int turns_left, Player &oppo1, Player &oppo2, unsigned int prev_best = 0, bool print = false) {
    std::vector<std::pair<unsigned int, unsigned int>> scores;
    (void) prev_best;
    unsigned int cur_score = 0;
    Player best_player = player;
    unsigned int best_solution = 0;

    for (auto it = population.begin(); it != population.end(); ++it) {
        Player copy_player = player;
        copy_player.playTurnsCompete(*it, turns_left, oppo1, oppo2);
        unsigned int score = copy_player.evaluate_compete(turns_left, oppo1, oppo2);
        // if (*it == prev_best && print) {
        //     std::cerr << "Score of previous best solution: " << score << std::endl;
        // }
        scores.push_back({score, *it});
        if (score > cur_score) {
            cur_score = score;
            best_player = copy_player;
            best_solution = *it;
        }
    }
    if (print) {
        std::cerr << "Best score: " << cur_score << " with solution: " << best_solution << std::endl;
    }

    return std::make_tuple(scores, best_player, best_solution);
}

bool compareByScore(const std::pair<unsigned int, unsigned int>& a, const std::pair<unsigned int, unsigned int>& b) {
    return a.first > b.first;
}


std::unordered_set<unsigned int> extractTopXSolutions(std::vector<std::pair<unsigned int, unsigned int>>& vec, unsigned int x) {
    if (x > vec.size()) {
        x = vec.size();
    }
    std::nth_element(vec.begin(), vec.begin() + x, vec.end(), compareByScore);
    std::unordered_set<unsigned int> top_solutions;
    for (size_t i = 0; i < x; ++i) {
        top_solutions.insert(vec[i].second);
    }
    return top_solutions;
}


// to review
std::pair<unsigned int, unsigned int> selection(std::vector<std::pair<unsigned int, unsigned int>> &scores) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, scores.size() - 1);
    unsigned int idx1 = dis(gen);
    unsigned int idx2 = dis(gen);

    return {scores[idx1].second, scores[idx2].second};
}

std::pair<unsigned int, unsigned int> weightedSelection(std::vector<std::pair<unsigned int, unsigned int>> &scores) {
    std::random_device rd;
    std::mt19937 gen(rd());

    std::vector<unsigned int> cumulative_scores(scores.size());
    cumulative_scores[0] = scores[0].first;
    for (size_t i = 1; i < scores.size(); ++i) {
        cumulative_scores[i] = cumulative_scores[i - 1] + scores[i].first;
    }

    auto select = [&]() -> unsigned int {
        std::uniform_int_distribution<unsigned int> dis(0, cumulative_scores.back() - 1);
        unsigned int random_value = dis(gen);

        auto it = std::lower_bound(cumulative_scores.begin(), cumulative_scores.end(), random_value);
        return std::distance(cumulative_scores.begin(), it);
    };

    unsigned int idx1 = select();
    unsigned int idx2;
    do {
        idx2 = select();
    } while (idx1 == idx2);

    return {scores[idx1].second, scores[idx2].second};
}

// to review
// std::pair<unsigned int, unsigned int> crossover(unsigned int parent1, unsigned int parent2) {
//     std::random_device rd;
//     std::mt19937 gen(rd());
//     std::uniform_int_distribution<unsigned int> dis(0, UINT_MAX);
//     unsigned int mask = dis(gen);
//     unsigned int child1 = (parent1 & mask) | (parent2 & ~mask);
//     unsigned int child2 = (parent2 & mask) | (parent1 & ~mask);
//     return {child1, child2};
// }

std::pair<unsigned int, unsigned int> crossover(unsigned int parent1, unsigned int parent2) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(1, 15);
    unsigned int x = dis(gen);
    unsigned int mask = (1 << x) - 1;
    unsigned int child1 = (parent1 & mask) | (parent2 & ~mask);
    unsigned int child2 = (parent2 & mask) | (parent1 & ~mask);
    return {child1, child2};
}

//  to review
unsigned int mutation(unsigned int child, unsigned int generation, double inital_rate, double decay_factor) {
    double mutation_rate = inital_rate * std::exp(-decay_factor * generation);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::bernoulli_distribution dis(mutation_rate);

    for (unsigned int i = 0; i < 32; ++i) {
        if (dis(gen)) {
            child ^= 1 << i;
        }
    }
    return child;
}

int main() {
    int player_idx;
    std::cin >> player_idx; std::cin.ignore();
    int nb_games;
    std::cin >> nb_games; std::cin.ignore();

	int turns_left = 100;
    // Game loop
    while (true) {
        auto time_start = std::chrono::system_clock::now();
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
                hurdles = createHurdleArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5);
            } else if (i == 1) {
                archeries = createArcheryArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5);
            } else if (i == 2) {
                skatings = createSkatingArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5, reg_6);
            } else {
                divings = createDivingArray(gpu, reg_0, reg_1, reg_2, reg_3, reg_4, reg_5);
            }
        }
        int oppo1_idx;
        int oppo2_idx;

        if (player_idx == 0) {
            oppo1_idx = 1;
            oppo2_idx = 2;
        } else if (player_idx == 1) {
            oppo1_idx = 0;
            oppo2_idx = 2;
        } else {
            oppo1_idx = 0;
            oppo2_idx = 1;
        }

        Player myself = Player(hurdles[player_idx], archeries[player_idx], skatings[player_idx], divings[player_idx], scores[player_idx]);
        Player oppo1 = Player(hurdles[oppo1_idx], archeries[oppo1_idx], skatings[oppo1_idx], divings[oppo1_idx], scores[oppo1_idx]);
        Player oppo2 = Player(hurdles[oppo2_idx], archeries[oppo2_idx], skatings[oppo2_idx], divings[oppo2_idx], scores[oppo2_idx]);

        std::unordered_set<unsigned int> initial_oppo1 = generate_random_population(20);
        std::unordered_set<unsigned int> initial_oppo2 = generate_random_population(20);
        std::unordered_set<unsigned int> initial_myself = generate_random_population(100);
        std::unordered_set<unsigned int> next_gen_oppo1;
        std::unordered_set<unsigned int> next_gen_oppo2;
        std::unordered_set<unsigned int> next_gen_myself;
        std::vector<std::pair<unsigned int, unsigned int>> oppo1_solutions;
        std::vector<std::pair<unsigned int, unsigned int>> oppo2_solutions;
        std::vector<std::pair<unsigned int, unsigned int>> myself_solutions;
        unsigned int cur_generation = 1;
        unsigned int best_solution = 0;
        unsigned int best_oppo1_solution = 0;
        unsigned int best_oppo2_solution = 0;
        Player best_player = myself;
        Player best_oppo1 = oppo1;
        Player best_oppo2 = oppo2;
        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_start).count() < 48) {
            if (cur_generation == 1) {
                std::tie(oppo1_solutions, best_oppo1) = evaluate_solutions_gen1(initial_oppo1, oppo1, turns_left);
                std::tie(oppo2_solutions, best_oppo2) = evaluate_solutions_gen1(initial_oppo2, oppo2, turns_left);
                std::tie(myself_solutions, best_player, best_solution) = evaluate_solutions(initial_myself, myself, turns_left, oppo1, oppo2);
            } else {
                std::tie(oppo1_solutions, best_oppo1, best_oppo1_solution) = evaluate_solutions(next_gen_oppo1, oppo1, turns_left, best_player, best_oppo2);
                std::tie(oppo2_solutions, best_oppo2, best_oppo2_solution) = evaluate_solutions(next_gen_oppo2, oppo2, turns_left, best_player, best_oppo1);
                std::tie(myself_solutions, best_player, best_solution) = evaluate_solutions(next_gen_myself, myself, turns_left, best_oppo1, best_oppo2, best_solution, true);
            }
            next_gen_oppo1.clear();
            next_gen_oppo2.clear();
            next_gen_myself.clear();
            next_gen_oppo1 = extractTopXSolutions(oppo1_solutions, 2);
            next_gen_oppo2 = extractTopXSolutions(oppo2_solutions, 2);
            next_gen_myself = extractTopXSolutions(myself_solutions, 10);
            while (next_gen_oppo1.size() < 20) {
                std::pair<unsigned int, unsigned int> parents = weightedSelection(oppo1_solutions);
                std::pair<unsigned int, unsigned int> childs = crossover(parents.first, parents.second);
                next_gen_oppo1.insert(mutation(childs.first, cur_generation, INITIAL_MUTATION_RATE, DECAY_FACTOR));
                next_gen_oppo1.insert(mutation(childs.second, cur_generation, INITIAL_MUTATION_RATE, DECAY_FACTOR));
            }
            while (next_gen_oppo2.size() < 20) {
                std::pair<unsigned int, unsigned int> parents = weightedSelection(oppo2_solutions);
                std::pair<unsigned int, unsigned int> childs = crossover(parents.first, parents.second);
                next_gen_oppo2.insert(mutation(childs.first, cur_generation, INITIAL_MUTATION_RATE, DECAY_FACTOR));
                next_gen_oppo2.insert(mutation(childs.second, cur_generation, INITIAL_MUTATION_RATE, DECAY_FACTOR));
            }
            while (next_gen_myself.size() < 100) {
                std::pair<unsigned int, unsigned int> parents = weightedSelection(myself_solutions);
                std::pair<unsigned int, unsigned int> childs = crossover(parents.first, parents.second);
                next_gen_myself.insert(mutation(childs.first, cur_generation, INITIAL_MUTATION_RATE, DECAY_FACTOR));
                next_gen_myself.insert(mutation(childs.second, cur_generation, INITIAL_MUTATION_RATE, DECAY_FACTOR));
            }
            cur_generation += 1;
        }
        
        std::cerr << "Simulated generations: " << cur_generation << std::endl;
        (void) best_oppo1_solution;
        (void) best_oppo2_solution;
        std::cout << intToAction[best_solution & 3] << std::endl;
        turns_left -= 1;
    }

    return 0;
}