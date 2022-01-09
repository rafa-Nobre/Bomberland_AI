#include "game_state.hpp"
#include "a_star.cpp"

#include <string>
#include <stdlib.h>
#include <iostream>
#include <random>

const std::vector<std::string> _actions = {"up", "left", "right", "down", "bomb", "detonate"};

class Agent {
private:
  static std::string my_agent_id;
  static std::vector<std::string> my_units;

  static int tick;

  static void on_game_tick(int tick, const json& game_state);
  static std::string pathFinder(int tick, const json& game_state);
public:
  static void run();
};

std::string Agent::my_agent_id;
std::vector<std::string> Agent::my_units;

int Agent::tick;

void Agent::run() 
{
  const char* connection_string = getenv ("GAME_CONNECTION_STRING");

  if (connection_string == NULL) 
  {
    connection_string = "ws://127.0.0.1:3000/?role=agent&agentId=agentId&name=defaultName";
  }
  std::cout << "The current connection_string is: " << connection_string << std::endl;

  GameState::connect(connection_string); 
  GameState::set_game_tick_callback(on_game_tick);
  GameState::handle_messages();
}

void Agent::on_game_tick(int tick_nr, const json& game_state) 
{
  DEBUG("*************** on_game_tick");
  DEBUG(game_state.dump());
  TEST(tick_nr, game_state.dump());
  
  tick = tick_nr;
  std::cout << "Tick #" << tick << std::endl;
  if (tick == 1)
  {
    my_agent_id = game_state["connection"]["agent_id"];
    my_units = game_state["agents"][my_agent_id]["unit_ids"].get<std::vector<std::string>>();
  }

  srand(1234567 * (my_agent_id == "a" ? 1 : 0) + tick * 100 + 13);

  // send each (alive) unit a random action
  for (const auto& unit_id: my_units)
  {
    const json& unit = game_state["unit_state"][unit_id];
    if (unit["hp"] <= 0) continue;
    //std::string action = _actions[rand() % _actions.size()];
    std::string action = pathFinder(tick, game_state);
    std::cout << "action: " << unit_id << " -> " << action << std::endl;

    if (action == "up" || action == "left" || action == "right" || action == "down")
    {
      GameState::send_move(action, unit_id);
    }
    else if (action == "bomb")
    {
      GameState::send_bomb(unit_id);
    }
    else if (action == "detonate")
    {
      const json& entities = game_state["entities"];
      for (const auto& entity: entities) 
      {
        if (entity["type"] == "b" && entity["unit_id"] == unit_id) 
        {
          int x = entity["x"], y = entity["y"];
          GameState::send_detonate(x, y, unit_id);
          break;
        }
      }
    }
    else 
    {
      std::cerr << "Unhandled action: " << action << " for unit " << unit_id << std::endl;
    }
  }
}

std::string Agent::pathFinder(int tick, const json& game_state) {

  std::string matrizMapa[15][15];
  for (int i = 0; i < 15; i++){
    for (int j = 0; j < 15; j++){
      matrizMapa[i][j] = {"#"};
    }
  }

  //montagem da matriz
  const json& entities = game_state["entities"];
  for (const auto& entity: entities) {
    int x = entity["x"];
    int y = entity["y"];
    if (entity["type"] == "a") {
      matrizMapa[x][y] = "A";
    } else if (entity["type"] == "b") {
      matrizMapa[x][y] = "B";
    } else if (entity["type"] == "x") {
      matrizMapa[x][y] = "X";
    } else if (entity["type"] == "bp") {
      matrizMapa[x][y] = "P";
    } else if (entity["type"] == "m") {
      matrizMapa[x][y] = "M";
    } else if (entity["type"] == "o") {
      matrizMapa[x][y] = "O";
    } else if (entity["type"] == "w") {
      matrizMapa[x][y] = "W";
    }
  }

  for (int i = 0; i < 15; i++){
    for (int j = 0; j < 15; j++){
      std::cout << matrizMapa[i][j];
    }
    std::cout << std::endl;
  }
  std::cout << std::endl;
  
  /*GridWithWeights grid = make_diagram4();
  GridLocation start{1, 4}, goal{8, 3};
  std::unordered_map<GridLocation, GridLocation> came_from;
  std::unordered_map<GridLocation, double> cost_so_far;
  a_star_search(grid, start, goal, came_from, cost_so_far);
  draw_grid(grid, nullptr, &came_from, nullptr, &start, &goal);
  std::cout << '\n';
  std::vector<GridLocation> path = reconstruct_path(start, goal, came_from);
  draw_grid(grid, nullptr, nullptr, &path, &start, &goal);
  std::cout << '\n';
  draw_grid(grid, &cost_so_far, nullptr, nullptr, &start, &goal);*/
  
  return _actions[rand() % _actions.size()];
}

int main()
{
  Agent::run();
}
