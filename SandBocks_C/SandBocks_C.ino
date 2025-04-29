#include <array>
#include <memory>
#include <random>
#include <cstdint>

class Cell;
class Grid;
class Particle;
class Updatable;
class Solid;

extern Particle* ELEMENT[];

/*
TODO list:
 - Finish the project rofl

 - colour needs to be thought out, I need the colour_mode to be seperate so that it can be stacked for multiples in Cell class. The Colour
 class should only really store the colour params. New base values will need to be acquired, and then new display values will need to be acquired. 
 These cannot just be references, they need to be unique per instance.


*/

// Enum
enum class ELEMENT_ID {
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

  NO_ELEMENT,

  ELEMENT_ID_ENUM_COUNT  //Required for setting up the element array
};

constexpr int ELEMENT_COUNT = static_cast<byte>(ELEMENT_ID::ELEMENT_ID_ENUM_COUNT);

enum class STATE_ID {
  NO_STATE,
  BURNING,
  DECAY
};

enum class DIRECTION {
  NONE = 0,
  False = -1,
  True = 1
};

enum class COLOURMODE {
  SOLID,
  STATIC,
  NOISE,
  TEMPERATURE_DOWN,
  BLACKBODY
};

enum class PARTICLETYPE {
  PARTICLE,
  UPDATABLE,
  SOLID,
  LIQUID,
  GAS,

  // more types...
};

// Struct

struct coordinate_return {
  byte x;
  byte y;
};

// Random
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

// classes
class Colour {
public:
  const std::array<byte, 2> param_red;
  const std::array<byte, 2> param_green;
  const std::array<byte, 2> param_blue;

  byte base_red;
  byte base_green;
  byte base_blue;

  COLOURMODE colour_mode;

  Colour(std::array<byte, 2> param_red, std::array<byte, 2> param_green, std::array<byte, 2> param_blue, COLOURMODE colour_mode = COLOURMODE::STATIC)
    : param_red(param_red), param_green(param_green), param_blue(param_blue), colour_mode(colour_mode) {
    set_base_colour(param_red, param_green, param_blue);
  }

  void set_base_colour(std::array<byte, 2> param_red, std::array<byte, 2> param_green, std::array<byte, 2> param_blue) {
    this->base_red = (param_red[1] == 0) ? param_red[0] : RandomUtils::getRandomByte(param_red[0], param_red[1]);
    this->base_green = (param_green[1] == 0) ? param_green[0] : RandomUtils::getRandomByte(param_green[0], param_green[1]);
    this->base_blue = (param_blue[1] == 0) ? param_blue[0] : RandomUtils::getRandomByte(param_blue[0], param_blue[1]);
  }
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

  Cell()
    : element_id(ELEMENT_ID::AIR),  // default to AIR
      colour_data(std::make_unique<Colour>(std::array<byte, 2>{ 0, 0 }, std::array<byte, 2>{ 0, 0 }, std::array<byte, 2>{ 0, 0 }, COLOURMODE::SOLID)),
      direction(DIRECTION::NONE),
      state_id(STATE_ID::NO_STATE),
      flammable(false),
      temperature(0),
      fuel(0),
      life(0),
      branches(0),
      clone_element(ELEMENT_ID::NO_ELEMENT) {}

  Cell(ELEMENT_ID element_id, std::unique_ptr<Colour> colour_data, bool flammable, int temperature, byte fuel, byte life, byte branches, STATE_ID state_id = STATE_ID::NO_STATE, ELEMENT_ID clone_element_id = ELEMENT_ID::NO_ELEMENT)
    : element_id(element_id), colour_data(std::move(colour_data)), direction(DIRECTION::NONE), state_id(state_id), flammable(flammable), temperature(temperature), fuel(fuel), life(life), branches(branches), clone_element(clone_element_id) {
    colour_data->set_base_colour(colour_data->param_red, colour_data->param_green, colour_data->param_blue);
  }

  void update_colour_from_state_change() {
    // TODO: Implement color update based on new element
  }


  // void update_colour() {
  //   if COLOURMODE::NOISE
  // }

  // std::array<byte, 3> display_colour() {
  //   int r =
  //   return [r, g, b]
  // }
};
// dependency on Cell class
struct grid_return {
  byte x;
  byte y;
  Cell* cell;
};

constexpr byte width = 32;
constexpr byte height = 16;
//Cell grid[width][height];

constexpr byte INVALID_BYTE = 255;

inline bool is_valid_coordinate(const coordinate_return& coord) {  //Is the returned coordinate a proper value
  return coord.x != INVALID_BYTE || coord.y != INVALID_BYTE;
}


class Grid {
public:
  static constexpr byte WIDTH = 32;
  static constexpr byte HEIGHT = 16;
  std::array<std::array<Cell, HEIGHT>, WIDTH> grid;
  //    _ = ["Cell(air, temperature=23)" for _ in range(height)]
  //    self.grid = [_ for _ in range(width)]
  Grid() {}

  Cell& get(byte x, byte y) {
    return grid[x][y];
  }

  void set(byte x, byte y, ELEMENT_ID element_id) {
    grid[x][y] = cell;
  }

  inline bool in_bounds(byte x, byte y) {
    if ((x >= 0 && x < WIDTH) && (y >= 0 && y < HEIGHT)) {
      return true;
    }
    return false;
  }

  inline bool can_swap(byte x1, byte y1, byte x2, byte y2) {
    // TODO
    return true;
  }

  inline bool can_swap_after_random(byte x1, byte y1, byte x2, byte y2) {
    // """ Assumes the check has already been made to see if these can swap """
    // pixel1 = grid.get(x1, y1)
    // pixel2 = grid.get(x2, y2)
    // return random.random() < (pixel1.element.density / (pixel1.element.density + pixel2.element.density))
    return true;
  }

  inline void swap(byte x1, byte y1, byte x2, byte y2) {
    std::swap(grid[x1][y1], grid[x2][y2]);
  }

  std::array<coordinate_return, 3> check_unimpeded(byte x, byte y) {
    std::array<coordinate_return, 3> gap_list = { coordinate_return{ INVALID_BYTE, INVALID_BYTE },
                                                  coordinate_return{ INVALID_BYTE, INVALID_BYTE },
                                                  coordinate_return{ INVALID_BYTE, INVALID_BYTE } };

    Cell& cell = get(x, y);
    if (get(x, y).direction != DIRECTION::NONE) {

      int8_t direction = static_cast<int8_t>(cell.direction);

      if (in_bounds(x, y + direction) && can_swap(x, y, x, y + direction)) {
        gap_list[1] = { x, y + direction };
        return gap_list;  //need this to populate all 3 arrays
      }

      for (int8_t delta = -1; delta <= 1; delta = delta + 2) {
        if (in_bounds(x + delta, y + direction) and can_swap(x, y, x + delta, y + direction)) {
          coordinate_return free_coords = { x + delta, y + direction };
          gap_list[delta + 1] = free_coords;
        }
      }
    }
    return gap_list;  //middle value will guaranteed be invalid
  }

  void print() {
    for (byte i = 0; i < WIDTH; i++) {
      for (byte j = 0; j < HEIGHT; j++) {
        Cell& cell = get(i, j);
        if (cell.element_id == ELEMENT_ID::STONE) {
          Serial.print(" X ");
        } else {
          Serial.print("   ");
        }
      }
    }
    Serial.println("_______________________________________________");
  }
};

class Particle {
public:
  const ELEMENT_ID element_id;
  const Colour colour_params;
  const DIRECTION default_direction;

  Particle(ELEMENT_ID element_id, Colour colour_params, DIRECTION default_direction = DIRECTION::NONE)
    : element_id(element_id), colour_params(colour_params), default_direction(default_direction) {}

  virtual PARTICLETYPE getType() const {
    return PARTICLETYPE::PARTICLE;
  }

  virtual ~Particle() {}  // Virtual destructor for polymorphism
};

inline bool isUpdatable(Particle* particle) {
  PARTICLETYPE type = particle->getType();
  return (type == PARTICLETYPE::UPDATABLE || type == PARTICLETYPE::SOLID);
  // TODO implement all other materials
}


class Updatable : public Particle {
public:
  const std::array<int, 2> phase_change_temp;
  const std::array<ELEMENT_ID, 2> next_phase;
  const float thermal_conductivity;
  const DIRECTION default_direction;

  Updatable(ELEMENT_ID element_id, Colour colour_data, std::array<int, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, DIRECTION default_direction = DIRECTION::NONE)
    : Particle(element_id, colour_data), phase_change_temp(phase_change_temp), next_phase(next_phase), thermal_conductivity(thermal_conductivity), default_direction(default_direction) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::UPDATABLE;
  }

  virtual ~Updatable() {}  // Virtual destructor for polymorphism

  void act(Grid grid, byte x, byte y) {
    Cell& cell = grid.get(x, y);
    int current_temp = cell.temperature;

    Particle* particle = ELEMENT[static_cast<byte>(cell.element_id)];
    if (!isUpdatable(particle)) {  // SOLID inherits from UPDATABLE
      return;
      // safe to use
    }
    Updatable* updatable = static_cast<Updatable*>(particle);

    if (updatable->next_phase[0] != ELEMENT_ID::NO_ELEMENT && current_temp < updatable->phase_change_temp[0]) {
      cell.element_id = updatable->next_phase[0];
      cell.direction = updatable->default_direction;
      cell.update_colour_from_state_change();
      return;
    } else if (updatable->next_phase[1] != ELEMENT_ID::NO_ELEMENT && current_temp > updatable->phase_change_temp[1]) {
      cell.element_id = updatable->next_phase[1];
      cell.direction = updatable->default_direction;
      cell.update_colour_from_state_change();
      return;
    }

    if (updatable->next_phase[1] == ELEMENT_ID::NO_ELEMENT && cell.flammable && current_temp > updatable->phase_change_temp[1]) {
      cell.state_id = STATE_ID::BURNING;
    }
  }
};

class Solid : public Updatable {
public:
  float density;
  DIRECTION default_direction;

  Solid(ELEMENT_ID element_id, Colour colour_data, COLOURMODE colour_mode, std::array<int, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, float density, DIRECTION default_direction = DIRECTION::True)
    : Updatable(element_id, colour_data, phase_change_temp, next_phase, thermal_conductivity), density(density), default_direction(default_direction) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::SOLID;
  }

  void move(Grid grid, byte x, byte y) {
    std::array<coordinate_return, 3> gaps = grid.check_unimpeded(x, y);

    byte x2 = gaps[1].x;
    byte y2 = gaps[1].y;

    if (is_valid_coordinate({ x2, y2 }) && grid.can_swap_after_random(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }

    // Must try other free spots to the side
    bool left_to_right = RandomUtils::getRandomBool();
    coordinate_return chosen_gap = left_to_right ? gaps[0] : gaps[2];

    x2 = chosen_gap.x;
    y2 = chosen_gap.y;

    if (is_valid_coordinate(chosen_gap) && grid.can_swap(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }

    // Try the other side if first pick failed
    chosen_gap = left_to_right ? gaps[2] : gaps[0];
    x2 = chosen_gap.x;
    y2 = chosen_gap.y;

    if (is_valid_coordinate(chosen_gap) && grid.can_swap(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }
  }
};

Solid* stone = new Solid(
  ELEMENT_ID::STONE,
  Colour({ 0, 0 }, { 0, 0 }, { 0, 0 }, COLOURMODE::SOLID),
  COLOURMODE::SOLID,
  { 0, 100 },                               // temperature range
  { ELEMENT_ID::METAL, ELEMENT_ID::LAVA },  // phase changes
  0.1f,                                     // thermal conductivity
  10.0f,                                    // density
  DIRECTION::True);

Particle* ELEMENT[ELEMENT_COUNT] = {
  stone,
};

Element createElementInstance(ElementID id) {
    Particle* element = ELEMENT[id]; // Access element from ELEMENT array
    
    // Call the setup method specific to the element type
    element->setupProperties();
    
    // Now create and return the new element (assuming you just need an Element instance)
    Element newElement;
    newElement.ELEMENT_ID = id;
    newElement.temperature = element->temperature;
    newElement.color = element->color;
    newElement.phase = element->phaseChanges[0]; // Just using the first phase for simplicity
    
    return newElement;
}


Grid grid;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  grid.set(5, 5, ELEMENT_ID::STONE)
}

void loop() {
  grid.print();
  // put your main code here, to run repeatedly:
}
