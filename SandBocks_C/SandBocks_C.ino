#include <array>
#include <memory>
#include <random>
#include <cstdint>



/*
TODO list:
 - Finish the project rofl

 - colour needs to be thought out, I need the colour_mode to be seperate so that it can be stacked for multiples in Cell class. The Colour
 class should only really store the colour params. New base values will need to be acquired, and then new display values will need to be acquired. 
 These cannot just be references, they need to be unique per instance.


*/

namespace RandomUtils {
// Random engine and distribution seed
static std::random_device rd;
static std::mt19937 gen(rd());  // Mersenne Twister engine for random numbers

// Generate a random byte  in the range [min, max]
byte getRandomByte(byte min, byte max) {
  if (min == max) {
    return min;  // Prevent range issues
  }
  std::uniform_int_distribution<> dis(min, max - 1);
  return dis(gen);
}

// Generate a random boolean with a given true probability (0.0 to 1.0)
bool getRandomBool(float trueProbability = 0.5f) {
  std::bernoulli_distribution dis(trueProbability);
  return dis(gen);
}
};

enum ELEMENT_ID {
  AIR,
  SAND,
  WATER,
  ICE,
  STEAM,
  SNOW,
  SMOKE,
  FIRE,
  OIL,
  LAVA,
  STONE,
  WOOD,
  MOLTEN_METAL,
  METAL,
  BLOCK,

  // Special
  DESTRUCT,
  GROW,
  CLONE,
  COLD,
  HOT,
  CONDUCT,
  FLIP,
  BUG,

  NO_ELEMENT
};

enum STATE_ID {
  NO_STATE,
  BURNING,
  DECAY
};

constexpr byte width = 32;
constexpr byte height = 16;
//Cell grid[width][height];

class Grid {
public:
  byte width;
  byte height;
  std::array<std::array<Cell, width>, height> grid;
  //    _ = ["Cell(air, temperature=23)" for _ in range(height)]
  //    self.grid = [_ for _ in range(width)]
  Grid(byte width, byte height)
  : width(width), height(height) {}

  Cell& get(byte x, byte y) {
    return grid[x][y];
  }

};



struct coordinate_return {
  byte x;
  byte y;
};



enum DIRECTION {
  NONE = -1,
  False,
  True
};

enum COLOURMODE {
  SOLID,
  STATIC,
  NOISE,
  TEMPERATURE_DOWN,
  BLACKBODY
};

class Colour {
public:
  const std::array<byte, 2> param_red;
  const std::array<byte, 2> param_green;
  const std::array<byte, 2> param_blue;

  byte base_red;
  byte base_green;
  byte base_blue;

  COLOURMODE colour_mode;

  Colour(std::array<byte, 2> param_red, std::array<byte, 2> param_green, std::array<byte, 2> param_blue, COLOURMODE colour_mode=COLOURMODE::STATIC)
    : param_red(param_red), param_green(param_green), param_blue(param_blue), colour_mode(colour_mode) {
    set_base_colour(param_red, param_green, param_blue);
  }

  void set_base_colour(std::array<byte, 2> param_red, std::array<byte, 2> param_green, std::array<byte, 2> param_blue) {
    this->base_red = (param_red[1] == 0) ? param_red[0] : RandomUtils::getRandomByte(param_red[0], param_red[1]);
    this->base_green = (param_green[1] == 0) ? param_green[0] : RandomUtils::getRandomByte(param_green[0], param_green[1]);
    this->base_blue = (param_blue[1] == 0) ? param_blue[0] : RandomUtils::getRandomByte(param_blue[0], param_blue[1]);
  }
};

Particle* ELEMENT[] = {
  Gas(ELEMENT_ID::AIR, { { 0, 0 }, { 0, 0 }, { 0, 0 } }, { COLOURMODE::SOLID }, { NULL, NULL }, { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::NO_ELEMENT })
};

class Cell {
public:
  ELEMENT_ID element_id;
  std::unique_ptr<Colour> colour_data;  // Smart pointer for automatic memory management
  DIRECTION direction;
  STATE_ID state_id;
  bool flammable;
  int temperature;
  byte fuel;
  byte life;
  byte branches;
  ELEMENT_ID clone_element;

  Cell(ELEMENT_ID element_id, std::unique_ptr<Colour> colour_data, bool flammable, int temperature, byte fuel, byte life, byte branches, STATE_ID state_id=STATE_ID::NO_STATE, ELEMENT_ID clone_element_id=ELEMENT_ID::NO_ELEMENT)
    : element_id(element_id), colour_data(std::move(colour_data)), direction(ELEMENT[element_id].default_direction), state_id(state_id), flammable(flammable), temperature(temperature), fuel(fuel), life(life), branches(branches), clone_element(clone_element_id) {
    colour_data->set_base_colour(colour_data->param_red, colour_data->param_green, colour_data->param_blue);
  }

  // void update_colour() {
  //   if COLOURMODE::NOISE
  // }

  // std::array<byte, 3> display_colour() {
  //   int r =
  //   return [r, g, b]
  // }
};

struct grid_return {
  byte x;
  byte y;
  Cell cell;
};

class Particle {
public:
  const String name;
  const Colour colour_params;
  const DIRECTION default_direction;

  Particle(String name, Colour colour_params, DIRECTION default_direction = DIRECTION::NONE)
    : name(name), colour_params(colour_params), default_direction(default_direction) {}

  virtual ~Particle() {}  // Virtual destructor for polymorphism
};

class Updatable : public Particle {
public:
  const std::array<int, 2> phase_change_temp;
  const std::array<ELEMENT_ID, 2> next_phase;
  const float thermal_conductivity;
  const DIRECTION default_direction;

  Updatable(String name, Colour colour_data, std::array<int, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, DIRECTION default_direction = DIRECTION::NONE)
    : Particle(name, colour_data), phase_change_temp(phase_change_temp), next_phase(next_phase), thermal_conductivity(thermal_conductivity), default_direction(default_direction) {}

  virtual ~Updatable() {}  // Virtual destructor for polymorphism

  // void act(Grid grid, byte x, byte y) {
  //   Cell cell = grid.get(x, y);
  //   int current_temp = cell.temperature;
  //   if (ELEMENT[cell.element_id].next_phase[0] != ELEMENT_ID::NO_ELEMENT && current_temp < ELEMENT[cell.element_id].phase_change_temp[0]) {
  //     cell.element_id = ELEMENT[cell.element_id].next_phase[0]]];
  //     cell.direction = ELEMENT[cell.element_id].default_direction;
  //     cell.update_colour_from_state_change();
  //     return
  //   } else if (ELEMENT[cell.element_id].next_phase[1] != ELEMENT_ID::NO_ELEMENT && current_temp > ELEMENT[cell.element_id].phase_change_temp[1]) {
  //     cell.element_id = ELEMENT[cell.element_id].next_phase[1]]];
  //     cell.direction = ELEMENT[cell.element_id].default_direction;
  //     cell.update_colour_from_state_change();
  //     return
  //   }

  //   if (ELEMENT[cell.element_id].next_phase[1] == ELEMENT_ID::NO_ELEMENT && cell.flammable && current_temp > ELEMENT[cell.element_id].phase_change_temp[1]) {
  //     cell.state_id = STATE::BURNING;
  //   }
  // }
};

class Solid : public Updatable {
public:
  float density;
  DIRECTION default_direction;

  Solid(String name, Colour colour_data, COLOURMODE colour_mode, std::array<int, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, float density, DIRECTION default_direction = DIRECTION::True)
    : Updatable(name, colour_data, phase_change_temp, next_phase, thermal_conductivity), density(density), default_direction(default_direction) {}

  // void move(Grid grid, byte x, byte y) {
  //   coordinate_return gaps[2] = grid.check_unimpeded(x, y);
  //   // TODO check unimpeded when the direct below space is free will only return an array of length 1. But now we have to explicitly set it to 3

  //   // Get length of valid entries
  //   byte length = 0;
  //   for (auto& gap : gaps) {
  //     if (gap.x == NULL) {
  //       break;
  //     }
  //     length++;
  //   }

  //   if (length == 1) {
  //     if (grid.can_swap_after_random(x, y, gaps[0].x, gaps[0].y)) {
  //       grid.swap(x, y, gaps[0].x, gaps[0].y)
  //     }
  //   } else if (length == 2) {
  //     byte index = RandomUtils::getRandomByte(0, length);
  //     if (grid.can_swap_after_random(x, y, gaps[index].x, gaps[index].y)) {
  //       grid.swap(x, y, gaps[index].x, gaps[index].y)
  //     }
  //   }
  };
};



/*

static const std::map<ELEMENT_ID, std::array<std::array<byte, 2>, 3>> element_params = {
    {ELEMENT_ID::WATER, {{ {0, 50}, {100, 150}, {200, 255} }}},  // Red, Green, Blue
    {ELEMENT_ID::AIR,   {{ {200, 255}, {200, 255}, {255, 255} }}},
    // Add more elements...
};

Particle* ELEMENT[] = {
    new Gas(
        ELEMENT_ID::AIR,
        element_params.at(ELEMENT_ID::AIR),     // Retrieve shared parameters for AIR
        COLOURMODE::SOLID,
        { ELEMENT_ID::NONE, ELEMENT_ID::NONE }
    ),
    new Gas(
        ELEMENT_ID::WATER,
        element_params.at(ELEMENT_ID::WATER),  // Retrieve shared parameters for WATER
        COLOURMODE::SOLID,
        { ELEMENT_ID::NONE, ELEMENT_ID::NONE }
    ),
    // Add more materials...
};

*/



enum STATE {
  // BURNING = ,
  // DECAY = ,
}

void setup() {
  // put your setup code here, to run once:
}

void loop() {
  // put your main code here, to run repeatedly:
}
