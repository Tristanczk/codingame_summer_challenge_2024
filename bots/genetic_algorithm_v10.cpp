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
#include <array>
#include <bitset>

// excluding skating for now
const int TURN_SIMULATED = 16;
const double INITIAL_MUTATION_RATE = 0.1;
const double DECAY_FACTOR = 0.01;
const u_int16_t POPULATION_SIZE = 300;
const u_int8_t OPPONENT_POPULATION_SIZE = 60;

// we use this order so that the u_int correspond to the number of space in hurdle race
std::map<int, std::string> intToAction = {{0, "UP"}, {1, "LEFT"}, {2, "DOWN"}, {3, "RIGHT"}};
std::map<int, char> intToLetter = {{0, 'U'}, {1, 'L'}, {2, 'D'}, {3, 'R'}};

static unsigned int g_seed;
inline void mySrand(int seed) { g_seed = seed; }
inline int myRandInt()
{
    g_seed = (214013 * g_seed + 2531011);
    return (g_seed >> 16) & 0x7FFF;
}
inline int myRandInt(int maxSize) { return myRandInt() % maxSize; }
inline int myRandInt(int a, int b) { return (a + myRandInt(b - a)); }

class Player
{
public:
    int8_t hurdle_position, hurdle_turn_finished, hurdle_stun, archery_x, archery_y, skating_position, skating_stun, skating_adjusted_position, skating_risk_level, diving_points, diving_combo;
    unsigned int score[4];
    unsigned int archery_score;

    Player() : hurdle_position(0), hurdle_turn_finished(100), hurdle_stun(0), archery_x(0), archery_y(0), skating_position(0), skating_stun(0), skating_adjusted_position(0), skating_risk_level(0), diving_points(0), diving_combo(0), archery_score(800)
    {
        for (int8_t i = 0; i < 4; ++i)
        {
            score[i] = 0;
        }
    }

    Player(const Player &other)
    {
        *this = other;
    }

    // void print()
    // {
    //     std::cerr << "Hurdle Position: " << (int)hurdle_position << std::endl;
    //     std::cerr << "Hurdle Turn Finished: " << (int)hurdle_turn_finished << std::endl;
    //     std::cerr << "Hurdle Stun: " << (int)hurdle_stun << std::endl;
    //     std::cerr << "Archery Score: " << archery_score << std::endl;
    //     std::cerr << "Archery X: " << (int)archery_x << std::endl;
    //     std::cerr << "Archery Y: " << (int)archery_y << std::endl;
    //     std::cerr << "Skating Position: " << (int)skating_position << std::endl;
    //     std::cerr << "Skating Stun: " << (int)skating_stun << std::endl;
    //     std::cerr << "Skating Risk Level: " << (int)skating_risk_level << std::endl;
    //     std::cerr << "Skating Adjusted Position: " << (int)skating_adjusted_position << std::endl;
    //     std::cerr << "Diving Points: " << (int)diving_points << std::endl;
    //     std::cerr << "Diving Combo: " << (int)diving_combo << std::endl;
    //     for (int8_t i = 0; i < 4; ++i)
    //     {
    //         std::cerr << "Score " << i << ": " << score[i] << std::endl;
    //     }
    // }

    Player &operator=(const Player &other)
    {
        if (this != &other)
        {
            hurdle_position = other.hurdle_position;
            hurdle_turn_finished = other.hurdle_turn_finished;
            hurdle_stun = other.hurdle_stun;
            archery_x = other.archery_x;
            archery_y = other.archery_y;
            skating_position = other.skating_position;
            skating_stun = other.skating_stun;
            skating_adjusted_position = other.skating_adjusted_position;
            skating_risk_level = other.skating_risk_level;
            diving_points = other.diving_points;
            diving_combo = other.diving_combo;
            archery_score = other.archery_score;
            for (int8_t i = 0; i < 4; ++i)
            {
                score[i] = other.score[i];
            }
        }
        return *this;
    }
};

class Game
{
public:
    std::string hurdle_field, archery_wind, skating_risk, diving_sequence;
    int8_t skating_remaining_turns;

    Game() : hurdle_field("GAME_OVER"), archery_wind("GAME_OVER"), skating_risk("GAME_OVER"), diving_sequence("GAME_OVER"), skating_remaining_turns(0) {}
};

class Solution
{
public:
    unsigned int solution;
    unsigned int score;

    Solution() : solution(0), score(0) {}

    bool operator<(const Solution &other) const
    {
        return score < other.score;
    }

    bool operator>(const Solution &other) const
    {
        return score > other.score;
    }

    void print()
    {
        std::cerr << "Solution: " << solution << " || score: " << score << std::endl;
    }
};

void playturns(unsigned int solution, int8_t turns_left, Player &player, Game &game)
{
    for (uint8_t i = 0; i < TURN_SIMULATED; ++i)
    {
        if (turns_left < i + 1)
        {
            break;
        }
        int8_t action = solution & 3;
        // Hurdle
        if (player.hurdle_position < 29 && game.hurdle_field != "GAME_OVER")
        {
            if (player.hurdle_stun > 0)
                player.hurdle_stun -= 1;
            else if (action == 0)
            {
                player.hurdle_position += 2;
                if (player.hurdle_position < 29 && game.hurdle_field[player.hurdle_position] == '#')
                    player.hurdle_stun = 2;
            }
            else
            {
                for (int8_t j = 0; j < action; ++j)
                {
                    player.hurdle_position += 1;
                    if (player.hurdle_position < 29 && game.hurdle_field[player.hurdle_position] == '#')
                    {
                        player.hurdle_stun = 2;
                        break;
                    }
                }
            }
            if (player.hurdle_position >= 29)
            {
                player.hurdle_turn_finished = i + 1;
            }
        }
        // Archery
        if (game.archery_wind != "GAME_OVER" && i < game.archery_wind.size())
        {
            int8_t wind = game.archery_wind[i] - '0';
            if (action == 0)
            {
                player.archery_y -= wind;
            }
            else if (action == 1)
            {
                player.archery_x -= wind;
            }
            else if (action == 2)
            {
                player.archery_y += wind;
            }
            else
            {
                player.archery_x += wind;
            }
            if (player.archery_x < -20)
            {
                player.archery_x = -20;
            }
            else if (player.archery_x > 20)
            {
                player.archery_x = 20;
            }
            if (player.archery_y < -20)
            {
                player.archery_y = -20;
            }
            else if (player.archery_y > 20)
            {
                player.archery_y = 20;
            }
            if (i == game.archery_wind.size() - 1)
                player.archery_score = player.archery_x * player.archery_x + player.archery_y * player.archery_y;
        }
        // Skating (without other players for now)
        // if (game.skating_risk != "GAME_OVER" && i == 0)
        // {
        //     int riskIdx = game.skating_risk.find(intToLetter[action]);
        //     if (player.skating_stun > 0)
        //     {
        //         player.skating_stun -= 1;
        //     }
        //     else
        //     {
        //         if (riskIdx == 0)
        //         {
        //             player.skating_position += 1;
        //             if (player.skating_risk_level > 0)
        //             {
        //                 player.skating_risk_level -= 1;
        //             }
        //         }
        //         else if (riskIdx == 1)
        //         {
        //             player.skating_position += 2;
        //         }
        //         else if (riskIdx == 2)
        //         {
        //             player.skating_position += 2;
        //             player.skating_risk_level += 1;
        //         }
        //         else
        //         {
        //             player.skating_position += 3;
        //             player.skating_risk_level += 2;
        //         }
        //         if (player.skating_risk_level >= 5)
        //         {
        //             player.skating_stun = 2;
        //             player.skating_risk_level = 0;
        //         }
        //     }
        //     player.skating_adjusted_position = player.skating_position * 2 - 4 * player.skating_stun - player.skating_risk_level;
        // }
        // Diving
        if (game.diving_sequence != "GAME_OVER" && i < game.diving_sequence.size())
        {
            if (intToLetter[action] == game.diving_sequence[i])
            {
                player.diving_combo += 1;
            }
            else
            {
                player.diving_combo = 0;
            }
            player.diving_points += player.diving_combo;
        }
        solution >>= 2;
    }
}

unsigned int evaluateHurdle(Player &player, Game &game)
{
    if (game.hurdle_field == "GAME_OVER")
        return 0;
    unsigned int score = player.hurdle_position + player.hurdle_turn_finished < TURN_SIMULATED ? (TURN_SIMULATED - player.hurdle_turn_finished) * 3 : 0;
    return score * 100 / 29;
}

unsigned int evaluateArchery(Player &player, Game &game, int8_t turns_left)
{
    if (game.archery_wind == "GAME_OVER" || game.archery_wind.size() > static_cast<std::size_t>(turns_left))
        return 0;
    unsigned int score = player.archery_x * player.archery_x + player.archery_y * player.archery_y;
    return (800 - score) / 4;
}

unsigned int evaluateSkating(Player &player, Game &game, int8_t turns_left)
{
    if (game.skating_risk == "GAME_OVER" || game.skating_remaining_turns > turns_left)
        return 0;
    unsigned int score = player.skating_adjusted_position + game.skating_remaining_turns * 2 - 2;
    return score * 100 / 30;
}

unsigned int evaluateDiving(Player &player, Game &game, int8_t turns_left)
{
    if (game.diving_sequence == "GAME_OVER" || game.diving_sequence.size() > static_cast<std::size_t>(turns_left))
        return 0;
    return player.diving_points * 200 / 105; // 105 is perfect score over 14 turns
}

unsigned int game_multiplier(int game_score)
{
    if (game_score == 0)
        return 100;
    return 50 / game_score;
}

unsigned int evaluate(unsigned int solution, int8_t turns_left, Player &playerCopy, Game &game)
{
    playturns(solution, turns_left, playerCopy, game);
    return evaluateHurdle(playerCopy, game) * game_multiplier(playerCopy.score[0]) + evaluateArchery(playerCopy, game, turns_left) * game_multiplier(playerCopy.score[1]) + evaluateSkating(playerCopy, game, turns_left) * game_multiplier(playerCopy.score[2]) + evaluateDiving(playerCopy, game, turns_left) * game_multiplier(playerCopy.score[3]);
}

unsigned int evaluate_compete_hurdle(int8_t turns_left, Player &player, Game &game, Player &oppo1, Player &oppo2)
{
    if (game.hurdle_field == "GAME_OVER" || player.hurdle_turn_finished > 16)
        return 0;
    if (player.hurdle_turn_finished > turns_left && oppo1.hurdle_turn_finished > turns_left && oppo2.hurdle_turn_finished > turns_left)
        return 0;
    if (player.hurdle_turn_finished <= oppo1.hurdle_turn_finished && player.hurdle_turn_finished <= oppo2.hurdle_turn_finished)
        return 3;
    else if (player.hurdle_turn_finished <= oppo1.hurdle_turn_finished || player.hurdle_turn_finished <= oppo2.hurdle_turn_finished)
        return 1;
    else
        return 0;
}

unsigned int evaluate_compete_archery(int8_t turns_left, Player &player, Game &game, Player &oppo1, Player &oppo2)
{
    if (game.archery_wind == "GAME_OVER" || game.archery_wind.size() > static_cast<std::size_t>(turns_left))
        return 0;
    if (player.archery_score <= oppo1.archery_score && player.archery_score <= oppo2.archery_score)
        return 3;
    else if (player.archery_score <= oppo1.archery_score || player.archery_score <= oppo2.archery_score)
        return 1;
    else
        return 0;
}

unsigned int evaluate_compete_skating(int8_t turns_left, Player &player, Game &game, Player &oppo1, Player &oppo2)
{
    if (game.skating_risk == "GAME_OVER" || game.skating_remaining_turns > turns_left)
        return 0;
    if (game.skating_remaining_turns == 1)
    {
        if (player.skating_position >= oppo1.skating_position && player.skating_position >= oppo2.skating_position)
            return 3;
        else if (player.skating_position >= oppo1.skating_position || player.skating_position >= oppo2.skating_position)
            return 1;
        else
            return 0;
    }
    else
    {
        if (player.skating_adjusted_position >= oppo1.skating_adjusted_position && player.skating_adjusted_position >= oppo2.skating_adjusted_position)
            return 2; // victory less valorized because race not ended
        else if (player.skating_adjusted_position >= oppo1.skating_adjusted_position || player.skating_adjusted_position >= oppo2.skating_adjusted_position)
            return 1;
        else
            return 0;
    }
}

unsigned int evaluate_compete_diving(int8_t turns_left, Player &player, Game &game, Player &oppo1, Player &oppo2)
{
    if (game.diving_sequence == "GAME_OVER" || game.diving_sequence.size() > static_cast<std::size_t>(turns_left))
        return 0;
    if (player.diving_points >= oppo1.diving_points && player.diving_points >= oppo2.diving_points)
        return 3;
    else if (player.diving_points >= oppo1.diving_points || player.diving_points >= oppo2.diving_points)
        return 1;
    else
        return 0;
}

unsigned int evaluate_compete(unsigned int solution, int8_t turns_left, Player &playerCopy, Game &game, Player &oppo1, Player &oppo2)
{
    playturns(solution, turns_left, playerCopy, game);
    unsigned int hurdle_score = evaluate_compete_hurdle(turns_left, playerCopy, game, oppo1, oppo2);
    unsigned int archery_score = evaluate_compete_archery(turns_left, playerCopy, game, oppo1, oppo2);
    // unsigned int skating_score = evaluate_compete_skating(turns_left, playerCopy, game, oppo1, oppo2);
    unsigned int diving_score = evaluate_compete_diving(turns_left, playerCopy, game, oppo1, oppo2);

    return (hurdle_score + playerCopy.score[0]) * (archery_score + playerCopy.score[1]) * (diving_score + playerCopy.score[3]) * 1000 + (hurdle_score + archery_score + diving_score);
}

std::array<Solution, OPPONENT_POPULATION_SIZE> generate_opponent_initial_population(int8_t turns_left, Player &player, Game &game)
{
    std::array<Solution, OPPONENT_POPULATION_SIZE> population;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, UINT_MAX);
    for (int i = 0; i < OPPONENT_POPULATION_SIZE; ++i)
    {
        unsigned int solution = dis(gen);
        Player playerCopy = player;
        population[i].solution = solution;
        population[i].score = evaluate(solution, turns_left, playerCopy, game);
    }
    std::sort(population.begin(), population.end(), std::greater<Solution>());
    return population;
}

std::array<Solution, POPULATION_SIZE> generate_initial_population(int8_t turns_left, Player &player, Game &game, Player &oppo1, Player &oppo2)
{
    std::array<Solution, POPULATION_SIZE> population;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned int> dis(0, UINT_MAX);
    for (int i = 0; i < POPULATION_SIZE; ++i)
    {
        unsigned int solution = dis(gen);
        Player playerCopy = player;
        population[i].solution = solution;
        population[i].score = evaluate_compete(solution, turns_left, playerCopy, game, oppo1, oppo2);
    }
    std::sort(population.begin(), population.end(), std::greater<Solution>());
    return population;
}

std::pair<unsigned int, unsigned int> selectionOppo(const std::array<Solution, OPPONENT_POPULATION_SIZE> &population)
{
    unsigned int idx1 = myRandInt(OPPONENT_POPULATION_SIZE);
    unsigned int idx2;
    do
    {
        idx2 = myRandInt(OPPONENT_POPULATION_SIZE);
    } while (idx1 == idx2);

    return {population[idx1].solution, population[idx2].solution};
}

std::pair<unsigned int, unsigned int> weightedSelectionOppo(const std::array<Solution, OPPONENT_POPULATION_SIZE> &population)
{
    unsigned int cumulative_score = 0;
    for (int i = 0; i < OPPONENT_POPULATION_SIZE; ++i)
    {
        cumulative_score += population[i].score;
    }
    double random_value = static_cast<double>(myRandInt());
    unsigned int idx1 = 0, idx2 = POPULATION_SIZE - 1;

    double current_score = 0.0;
    for (int i = 0; i < OPPONENT_POPULATION_SIZE; ++i)
    {
        current_score += population[i].score * (32767.0 / cumulative_score);
        if (current_score >= random_value)
        {
            idx1 = i;
            break;
        }
    }

    do
    {
        random_value = static_cast<double>(myRandInt());
        current_score = 0.0;
        for (int i = 0; i < OPPONENT_POPULATION_SIZE; ++i)
        {
            current_score += population[i].score * (32767.0 / cumulative_score);
            if (current_score >= random_value)
            {
                idx2 = i;
                break;
            }
        }
    } while (idx1 == idx2);

    return {population[idx1].solution, population[idx2].solution};
}

std::pair<unsigned int, unsigned int> selection(const std::array<Solution, POPULATION_SIZE> &population)
{
    unsigned int idx1 = myRandInt(POPULATION_SIZE);
    unsigned int idx2;
    do
    {
        idx2 = myRandInt(POPULATION_SIZE);
    } while (idx1 == idx2);

    return {population[idx1].solution, population[idx2].solution};
}

std::pair<unsigned int, unsigned int> weightedSelection(const std::array<Solution, POPULATION_SIZE> &population)
{
    unsigned int cumulative_score = 0;
    for (int i = 0; i < POPULATION_SIZE; ++i)
    {
        cumulative_score += population[i].score;
    }
    double random_value = static_cast<double>(myRandInt());
    unsigned int idx1 = 0, idx2 = POPULATION_SIZE - 1;

    double current_score = 0.0;
    for (int i = 0; i < POPULATION_SIZE; ++i)
    {
        current_score += population[i].score * (32767.0 / cumulative_score);
        if (current_score >= random_value)
        {
            idx1 = i;
            break;
        }
    }

    do
    {
        random_value = static_cast<double>(myRandInt());
        current_score = 0.0;
        for (int i = 0; i < POPULATION_SIZE; ++i)
        {
            current_score += population[i].score * (32767.0 / cumulative_score);
            if (current_score >= random_value)
            {
                idx2 = i;
                break;
            }
        }
    } while (idx1 == idx2);

    return {population[idx1].solution, population[idx2].solution};
}

std::pair<unsigned int, unsigned int> crossover(unsigned int parent1, unsigned int parent2)
{
    unsigned int x = myRandInt(1, 16);
    unsigned int mask = (1 << x) - 1;
    unsigned int child1 = (parent1 & mask) | (parent2 & ~mask);
    unsigned int child2 = (parent2 & mask) | (parent1 & ~mask);
    return {child1, child2};
}

unsigned int mutation(unsigned int child, u_int16_t generation)
{
    double mutation_rate = INITIAL_MUTATION_RATE * std::exp(-DECAY_FACTOR * generation);

    for (unsigned int i = 0; i < 32; ++i)
    {
        int randval = myRandInt(10000);
        double probability = randval / 10000.0;
        if (probability < mutation_rate)
        {
            child ^= 1 << i;
        }
    }
    return child;
}

std::array<Solution, OPPONENT_POPULATION_SIZE * 2> generate_next_opponent_generation(const std::array<Solution, OPPONENT_POPULATION_SIZE> &population, u_int16_t cur_gen)
{
    std::array<Solution, OPPONENT_POPULATION_SIZE * 2> next_generation;

    std::unordered_set<unsigned int> unique_solutions;
    for (int i = 0; i < OPPONENT_POPULATION_SIZE; ++i)
    {
        next_generation[i] = population[i];
        unique_solutions.insert(population[i].solution);
    }
    unsigned int insert_idx = OPPONENT_POPULATION_SIZE;
    while (insert_idx < OPPONENT_POPULATION_SIZE * 2)
    {
        std::pair<unsigned int, unsigned int> parents = weightedSelectionOppo(population);
        std::pair<unsigned int, unsigned int> children = crossover(parents.first, parents.second);
        unsigned int child1 = mutation(children.first, cur_gen);
        unsigned int child2 = mutation(children.second, cur_gen);
        if (unique_solutions.find(child1) == unique_solutions.end())
        {
            next_generation[insert_idx].solution = child1;
            unique_solutions.insert(child1);
            ++insert_idx;
        }
        if (insert_idx >= OPPONENT_POPULATION_SIZE * 2)
            break;
        if (unique_solutions.find(child2) == unique_solutions.end())
        {
            next_generation[insert_idx].solution = child2;
            unique_solutions.insert(child2);
            ++insert_idx;
        }
    }
    return next_generation;
}

std::array<Solution, POPULATION_SIZE * 2> generate_next_generation(const std::array<Solution, POPULATION_SIZE> &population, u_int16_t cur_gen)
{
    std::array<Solution, POPULATION_SIZE * 2> next_generation;

    auto time_start = std::chrono::system_clock::now();

    std::unordered_set<unsigned int> unique_solutions;
    for (int i = 0; i < POPULATION_SIZE; ++i)
    {
        next_generation[i] = population[i];
        unique_solutions.insert(population[i].solution);
    }
    unsigned int insert_idx = POPULATION_SIZE;
    // std::cerr << "Copy of existing array: "  << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
    while (insert_idx < POPULATION_SIZE * 2)
    {
        std::pair<unsigned int, unsigned int> parents = weightedSelection(population);
        // std::cerr << "Selection: "  << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
        // std::cerr << "parents: " << parents.first << " " << parents.second << std::endl;
        std::pair<unsigned int, unsigned int> children = crossover(parents.first, parents.second);
        // std::cerr << "Crossover: "  << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
        // std::cerr << "children after crossover: " << children.first << " " << children.second << std::endl;
        unsigned int child1 = mutation(children.first, cur_gen);
        unsigned int child2 = mutation(children.second, cur_gen);
        // std::cerr << "Mutation: "  << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
        // std::cerr << "children after mutation: " << child1 << " " << child2 << std::endl;
        if (unique_solutions.find(child1) == unique_solutions.end())
        {
            next_generation[insert_idx].solution = child1;
            unique_solutions.insert(child1);
            ++insert_idx;
        }
        if (insert_idx >= POPULATION_SIZE * 2)
            break;
        if (unique_solutions.find(child2) == unique_solutions.end())
        {
            next_generation[insert_idx].solution = child2;
            unique_solutions.insert(child2);
            ++insert_idx;
        }
        // std::cerr << "end of a loop: "  << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
    }
    return next_generation;
}

std::array<Solution, OPPONENT_POPULATION_SIZE> restrict_population_opponent(int8_t turns_left, Player &player, Game &game, Player &oppo1, Player &oppo2, std::array<Solution, OPPONENT_POPULATION_SIZE * 2> &population)
{
    for (int i = 0; i < OPPONENT_POPULATION_SIZE * 2; ++i)
    {
        Player playerCopy = player;
        population[i].score = evaluate_compete(population[i].solution, turns_left, playerCopy, game, oppo1, oppo2);
    }
    std::sort(population.begin(), population.end(), std::greater<Solution>());
    std::array<Solution, OPPONENT_POPULATION_SIZE> best_population;
    std::copy(population.begin(), population.begin() + OPPONENT_POPULATION_SIZE, best_population.begin());

    return best_population;
}

std::array<Solution, POPULATION_SIZE> restrict_population(int8_t turns_left, Player &player, Game &game, Player &oppo1, Player &oppo2, std::array<Solution, POPULATION_SIZE * 2> &population)
{
    for (int i = 0; i < POPULATION_SIZE * 2; ++i)
    {
        Player playerCopy = player;
        population[i].score = evaluate_compete(population[i].solution, turns_left, playerCopy, game, oppo1, oppo2);
    }
    std::sort(population.begin(), population.end(), std::greater<Solution>());
    std::array<Solution, POPULATION_SIZE> best_population;
    std::copy(population.begin(), population.begin() + POPULATION_SIZE, best_population.begin());

    return best_population;
}

int main()
{
    int player_idx;
    std::cin >> player_idx;
    std::cin.ignore();
    int nb_games;
    std::cin >> nb_games;
    std::cin.ignore();

    int8_t turns_left = 100;
    mySrand(static_cast<int>(time(nullptr)));
    // Game loop
    while (true)
    {
        Player players[3];
        Game curGame;

        for (int i = 0; i < 3; ++i)
        {
            std::string score_info;
            std::getline(std::cin >> std::ws, score_info);
            std::istringstream ss(score_info);
            std::vector<int> break_score((std::istream_iterator<int>(ss)), std::istream_iterator<int>());
            for (int j = 0; j < 4; ++j)
            {
                players[i].score[j] = break_score[1 + j * 3] * 3 + break_score[2 + j * 3];
            }
        }
        auto time_start = std::chrono::system_clock::now();

        for (int i = 0; i < nb_games; ++i)
        {
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
            if (i == 0)
            {
                curGame.hurdle_field = gpu;
                players[0].hurdle_position = reg_0;
                players[1].hurdle_position = reg_1;
                players[2].hurdle_position = reg_2;
                players[0].hurdle_stun = reg_3;
                players[1].hurdle_stun = reg_4;
                players[2].hurdle_stun = reg_5;
            }
            else if (i == 1)
            {
                curGame.archery_wind = gpu;
                players[0].archery_x = reg_0;
                players[0].archery_y = reg_1;
                players[1].archery_x = reg_2;
                players[1].archery_y = reg_3;
                players[2].archery_x = reg_4;
                players[2].archery_y = reg_5;
            }
            else if (i == 2)
            {
                curGame.skating_risk = gpu;
                players[0].skating_position = reg_0;
                players[1].skating_position = reg_1;
                players[2].skating_position = reg_2;
                players[0].skating_stun = reg_3 < 0 ? -reg_3 : 0;
                players[1].skating_stun = reg_4 < 0 ? -reg_3 : 0;
                players[2].skating_stun = reg_5 < 0 ? -reg_3 : 0;
                players[0].skating_risk_level = reg_3 >= 0 ? reg_3 : 0;
                players[1].skating_risk_level = reg_4 >= 0 ? reg_4 : 0;
                players[2].skating_risk_level = reg_5 >= 0 ? reg_5 : 0;
                curGame.skating_remaining_turns = reg_6;
            }
            else
            {
                curGame.diving_sequence = gpu;
                players[0].diving_points = reg_0;
                players[1].diving_points = reg_1;
                players[2].diving_points = reg_2;
                players[0].diving_combo = reg_3;
                players[1].diving_combo = reg_4;
                players[2].diving_combo = reg_5;
            }
        }
        int oppo1_idx;
        int oppo2_idx;

        if (player_idx == 0)
        {
            oppo1_idx = 1;
            oppo2_idx = 2;
        }
        else if (player_idx == 1)
        {
            oppo1_idx = 0;
            oppo2_idx = 2;
        }
        else
        {
            oppo1_idx = 0;
            oppo2_idx = 1;
        }
        // std::cerr << "After getting inputs: " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;

        Player myself = players[player_idx];
        Player oppo1 = players[oppo1_idx];
        Player oppo2 = players[oppo2_idx];
        u_int16_t cur_generation = 1;
        std::array<Solution, OPPONENT_POPULATION_SIZE> oppo1_population = generate_opponent_initial_population(turns_left, myself, curGame);
        std::array<Solution, OPPONENT_POPULATION_SIZE> oppo2_population = generate_opponent_initial_population(turns_left, myself, curGame);
        Player bestOppo1 = oppo1;
        Player bestOppo2 = oppo2;
        playturns(oppo1_population[0].solution, turns_left, bestOppo1, curGame);
        playturns(oppo2_population[0].solution, turns_left, bestOppo2, curGame);
        std::array<Solution, POPULATION_SIZE> population = generate_initial_population(turns_left, myself, curGame, bestOppo1, bestOppo2);
        Player bestMyself = myself;
        playturns(population[0].solution, turns_left, bestMyself, curGame);
        // bestMyself.print();

        // std::cerr << "Before while loop: " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;

        while (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - time_start).count() < 45)
        {
            // for opponent1
            // std::cerr << "Beginning of while loop " << cur_generation << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
            std::array<Solution, OPPONENT_POPULATION_SIZE * 2> next_generation_oppo1 = generate_next_opponent_generation(oppo1_population, cur_generation);
            oppo1_population = restrict_population_opponent(turns_left, myself, curGame, bestMyself, bestOppo2, next_generation_oppo1);
            bestOppo1 = oppo1;
            playturns(oppo1_population[0].solution, turns_left, bestOppo1, curGame);

            // for opponent2
            // std::cerr << "Finished opponent 1 " << cur_generation << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
            std::array<Solution, OPPONENT_POPULATION_SIZE * 2> next_generation_oppo2 = generate_next_opponent_generation(oppo2_population, cur_generation);
            oppo2_population = restrict_population_opponent(turns_left, myself, curGame, bestMyself, bestOppo1, next_generation_oppo2);
            bestOppo2 = oppo2;
            playturns(oppo2_population[0].solution, turns_left, bestOppo2, curGame);

            // for myself
            // std::cerr << "Finished opponent 2 " << cur_generation << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
            std::array<Solution, POPULATION_SIZE * 2> next_generation = generate_next_generation(population, cur_generation);
            // std::cerr << "Calculation of next generation: "  << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
            population = restrict_population(turns_left, myself, curGame, bestOppo1, bestOppo2, next_generation);
            // std::cerr << "Calculation of restrict population: "  << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
            bestMyself = myself;
            playturns(population[0].solution, turns_left, bestMyself, curGame);
            // std::cerr << "End of while loop " << cur_generation << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::system_clock::now() - time_start).count() << " ns" << std::endl;
            // if (cur_generation == 1 || cur_generation % 10 == 0)
            //     std::cerr << "Best score: " << population[0].score << std::endl;
            ++cur_generation;
        }

        std::cerr << "Simulated generations: " << cur_generation << std::endl;
        // std::cerr << "Oppo 1:" << std::endl;
        // bestOppo1.print();
        // std::cerr << "Oppo 2:" << std::endl;
        // bestOppo2.print();
        // std::cerr << "Myself:" << std::endl;
        // bestMyself.print();

        // std::cerr << "Solution: " << std::bitset<32>(population[0].solution) << std::endl;
        std::cout << intToAction[population[0].solution & 3] << std::endl;
        --turns_left;
    }

    return 0;
}