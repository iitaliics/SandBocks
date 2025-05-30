#include <array>
#include <memory>
#include <random>
#include <cstdint>
#include <Adafruit_NeoPixel.h>
#include <algorithm>

constexpr uint8_t PIN = 1;

constexpr int16_t NUMPIXELS = 512;

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

class Cell;
class Grid;
class Particle;
class Updatable;
class Solid;

extern Particle* ELEMENT[];

/*
TODO list:
 - Particle states
 - Special Elements
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

  // Special TODO
  DESTRUCT,
  CLONE,
  GROW,
  // COLD,
  // HOT,
  // CONDUCT,
  // FLIP,
  // BUG,

  NO_ELEMENT,

  ELEMENT_ID_ENUM_COUNT  //Required for setting up the element array
};

const uint8_t WIDTH = 32;
const uint8_t HEIGHT = 16;
constexpr int ELEMENT_COUNT = static_cast<uint8_t>(ELEMENT_ID::ELEMENT_ID_ENUM_COUNT) - 1;

constexpr uint8_t INVALID_BYTE = 255;

constexpr uint8_t TOUCH_SAMPLE_COUNT = 5;
constexpr uint8_t TOUCH_STABILITY_THRESHOLD = 100;
constexpr uint8_t TOUCH_SAMPLE_INTERVAL_MS = 2;

enum class STATE_ID {
  NO_STATE,
  BURNING,
  DECAY
};

enum class DIRECTION {
  NONE = 0,
  UP = 1,
  DOWN = -1
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
  SPECIAL,
  // more types...
};

// Struct

struct coordinate_return {
  uint8_t x;
  uint8_t y;
};

struct ColourParameters {
  using param_t = std::array<uint8_t, 2>;
  param_t param_red;
  param_t param_green;
  param_t param_blue;
  COLOURMODE colour_mode = COLOURMODE::STATIC;

  ColourParameters(param_t r, param_t g, param_t b, COLOURMODE mode)
    : param_red(r), param_green(g), param_blue(b), colour_mode(mode) {}
};

//  If scope allows it, this will contain the data that the clone material will use
struct CloneData {
  ELEMENT_ID element_id;
  STATE_ID state_id;
  bool flammable;
  float temperature;
  uint8_t fuel;
  uint8_t life;
  uint8_t branches;
};

// Random
namespace RandomUtils {
// Random engine and distribution seed
static std::random_device rd;
static std::mt19937 gen(rd());  // Mersenne Twister engine for random numbers

// Generate a random byte  in the range [min, max]
uint8_t getRandomByte(uint8_t min, uint8_t max) {
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
  ColourParameters::param_t param_red;
  ColourParameters::param_t param_green;
  ColourParameters::param_t param_blue;

  uint8_t base_red;
  uint8_t base_green;
  uint8_t base_blue;

  COLOURMODE colour_mode;

  Colour(const ColourParameters& params)
    : param_red(params.param_red), param_green(params.param_green), param_blue(params.param_blue), colour_mode(params.colour_mode) {
    set_base_colour();
  }

  void set_base_colour() {
    this->base_red = (param_red[1] == 0) ? param_red[0] : RandomUtils::getRandomByte(param_red[0], param_red[1]);
    this->base_green = (param_green[1] == 0) ? param_green[0] : RandomUtils::getRandomByte(param_green[0], param_green[1]);
    this->base_blue = (param_blue[1] == 0) ? param_blue[0] : RandomUtils::getRandomByte(param_blue[0], param_blue[1]);
  }
};

class Cell {
public:
  ELEMENT_ID element_id;
  Colour colour_data;
  DIRECTION direction;
  STATE_ID state_id;
  bool flammable;
  float temperature;
  uint8_t fuel;
  uint8_t life;
  uint8_t branches;
  ELEMENT_ID clone_element;

  Cell()
    : element_id(ELEMENT_ID::AIR),  // default to AIR
      colour_data(Colour(ColourParameters({ 0, 0 }, { 0, 0 }, { 0, 0 }, COLOURMODE::SOLID))),
      direction(DIRECTION::UP),
      state_id(STATE_ID::NO_STATE),
      flammable(false),
      temperature(23.0),
      fuel(0),
      life(0),
      branches(0),
      clone_element(ELEMENT_ID::NO_ELEMENT) {}

  Cell(ELEMENT_ID element_id, Colour colour_data, DIRECTION direction = DIRECTION::NONE, bool flammable = false, float temperature = 23.0, uint8_t fuel = 0, uint8_t life = 0, uint8_t branches = 0, STATE_ID state_id = STATE_ID::NO_STATE, ELEMENT_ID clone_element_id = ELEMENT_ID::NO_ELEMENT)
    : element_id(element_id), colour_data(colour_data), direction(direction), state_id(state_id), flammable(flammable), temperature(temperature), fuel(fuel), life(life), branches(branches), clone_element(clone_element_id) {
    this->colour_data.set_base_colour();
  }

  std::array<uint8_t, 3> get_output_colour_from_behaviour();

  void update_colour_from_phase_change() {
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
  uint8_t x;
  uint8_t y;
  Cell* cell;

  grid_return()
    : x(INVALID_BYTE), y(INVALID_BYTE), cell(nullptr) {}
  grid_return(uint8_t x, uint8_t y, Cell* cell)
    : x(x), y(y), cell(cell) {}
};

constexpr uint8_t width = 32;
constexpr uint8_t height = 16;
//Cell grid[width][height];


inline bool is_valid_coordinate(const coordinate_return& coord) {  //Is the returned coordinate a proper value
  return coord.x != INVALID_BYTE && coord.y != INVALID_BYTE;
}

std::array<std::array<Cell, HEIGHT>, WIDTH> grid;

class Grid {
public:
  static constexpr uint8_t WIDTH = 32;
  static constexpr uint8_t HEIGHT = 16;
  //    _ = ["Cell(air, temperature=23)" for _ in range(height)]
  //    self.grid = [_ for _ in range(width)]
  Grid() {}

  Cell& get(uint8_t x, uint8_t y) {
    return grid[x][y];
  }

  void set(uint8_t x, uint8_t y, Cell pixel) {
    grid[x][y] = pixel;
  }

  bool is_empty(uint8_t x, uint8_t y) {
    return (get(x, y).element_id == ELEMENT_ID::AIR);
  }

  void draw(uint8_t x, uint8_t y, Cell pixel, float probability) {
    if (is_empty(x, y) && RandomUtils::getRandomBool(probability)) {
      set(x, y, pixel);
    }
  }

  inline bool in_bounds(uint8_t x, uint8_t y) {
    if ((x >= 0 && x < WIDTH) && (y >= 0 && y < HEIGHT)) {
      return true;
    }
    return false;
  }

  inline bool can_swap(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

  inline bool can_swap_after_random(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);

  inline void swap(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
    std::swap(grid[x1][y1], grid[x2][y2]);
  }

  std::array<grid_return, 8> check_surroundings(uint8_t x, uint8_t y) {
    std::array<int8_t, 3> delta = { -1, 0, 1 };
    std::array<grid_return, 8> neighbour_list = { { { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr } } };

    uint8_t count = 0;
    for (int8_t dx : delta) {
      for (int8_t dy : delta) {
        if (dx == 0 && dy == 0) {
          count++;
          continue;
        }
        if (in_bounds(x + dx, y + dy)) {
          neighbour_list[count] = grid_return(x + dx, y + dy, &get(x + dx, y + dy));
        }
        count++;
      }
    }
    return neighbour_list;
  }

  std::array<coordinate_return, 3> check_unimpeded(uint8_t x, uint8_t y) {
    std::array<coordinate_return, 3> gap_list = { coordinate_return{ INVALID_BYTE, INVALID_BYTE },
                                                  coordinate_return{ INVALID_BYTE, INVALID_BYTE },
                                                  coordinate_return{ INVALID_BYTE, INVALID_BYTE } };

    Cell& cell = get(x, y);

    if (cell.direction != DIRECTION::NONE) {

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

  std::array<uint8_t, 2> check_side_gap_distance(uint8_t x, uint8_t y) {
    // Measured from the x coordinate, the distance to the closest side gap
    std::array<uint8_t, 2> side_gaps = { 0, 0 };
    int8_t direction = static_cast<int8_t>(get(x, y).direction);
    if (direction != 0) {
      if (!in_bounds(x, y + direction)) {
        return side_gaps;
      }

      bool current_layer_gap = false;
      bool next_layer_gap = false;

      // Left Side
      if (x != 0) {
        for (uint8_t i = 0; i < x; i++) {
          next_layer_gap = can_swap(x, y, x - (i + 1), y + direction);
          current_layer_gap = can_swap(x, y, x - (i + 1), y);

          if (next_layer_gap && current_layer_gap) {
            side_gaps[0] = (i + 1);
            break;
          }
          if (!current_layer_gap) {
            break;
          }
          if (current_layer_gap) {
            side_gaps[0] = (i + 1);
          }
        }
      }

      // right Side
      if (x != WIDTH - 1) {
        for (uint8_t i = 1; i < WIDTH - x; i++) {
          next_layer_gap = can_swap(x, y, x + i, y + direction);
          current_layer_gap = can_swap(x, y, x + i, y);

          if (next_layer_gap && current_layer_gap) {
            side_gaps[1] = i;
            break;
          }
          if (!current_layer_gap) {
            break;
          }
          if (current_layer_gap) {
            side_gaps[1] = i;
          }
        }
      }
    }
    return side_gaps;
  }

  float get_new_temperature_difference(uint8_t x, uint8_t y);

  void get_pixel_temperatures() {
    uint16_t count = 0;
    for (uint8_t row = 0; row < HEIGHT; ++row) {
      bool forward = RandomUtils::getRandomBool();
      for (uint8_t col = 0; col < WIDTH; ++col) {
        uint8_t x = forward ? col : (WIDTH - 1 - col);
        float delta = get_new_temperature_difference(x, row);
        temperature_data[count++] = { float(x), float(row), delta };
      }
    }
  }

  void get_moving(bool up) {
    uint16_t count = 0;
    DIRECTION check = up ? DIRECTION::UP : DIRECTION::DOWN;

    for (uint8_t row = 0; row < HEIGHT; ++row) {
      bool forward = RandomUtils::getRandomBool();
      for (uint8_t col = 0; col < WIDTH; ++col) {
        uint8_t x = forward ? col : (WIDTH - 1 - col);
        if (!in_bounds(x, row)) {
          moving_data[count++] = { 0, 0, 0 };
          continue;
        }
        DIRECTION cell_dir = get(x, row).direction;
        moving_data[count++] = { x, row, static_cast<uint8_t>(cell_dir == check) };
      }
    }
  }

  void perform_pixel_behaviour();

  void print() {
    for (uint8_t i = 0; i < HEIGHT; i++) {
      char str[WIDTH * 2 + 1];
      for (uint8_t j = 0; j < WIDTH; j++) {
        Cell& cell = get(j, HEIGHT - 1 - i);
        if (cell.element_id == ELEMENT_ID::STONE) {
          str[j * 2] = 'O';
        } else if (cell.element_id == ELEMENT_ID::WATER) {
          str[j * 2] = '=';
        } else if (cell.element_id == ELEMENT_ID::STEAM) {
          str[j * 2] = '&';
        } else if (cell.element_id == ELEMENT_ID::LAVA) {
          str[j * 2] = '$';
        } else if (cell.element_id == ELEMENT_ID::AIR) {
          str[j * 2] = '.';
        } else {
          str[j * 2] = '?';
        }
        str[j * 2 + 1] = ' ';
      }
      str[64] = '\0';
      Serial.println(str);
    }
    Serial.println("_______________________________________________");
  }

  void print_t() {
    for (uint8_t i = 0; i < HEIGHT; i++) {
      // char str[WIDTH * 2 + 1];
      for (uint8_t j = 0; j < WIDTH; j++) {
        Cell& cell = get(j, HEIGHT - 1 - i);
        Serial.print(cell.temperature);
        Serial.print(' ');
        // str[j * 2 + 1] = ' ';
      }
      // str[64] = '\0';
      Serial.println(' ');
    }
    Serial.println("_______________________________________________");
  }

  void print_s() {
    for (uint8_t i = 0; i < HEIGHT; i++) {
      // char str[WIDTH * 2 + 1];
      for (uint8_t j = 0; j < WIDTH; j++) {
        Cell& cell = get(j, HEIGHT - 1 - i);

        Serial.print(static_cast<uint8_t>(cell.state_id));
        Serial.print(' ');
        // str[j * 2 + 1] = ' ';
      }
      // str[64] = '\0';
      Serial.println(' ');
    }
    Serial.println("_______________________________________________");
  }

  void print_c() {
    for (uint8_t i = 0; i < HEIGHT; i++) {
      // char str[WIDTH * 2 + 1];
      for (uint8_t j = 0; j < WIDTH; j++) {
        Cell& cell = get(j, HEIGHT - 1 - i);
        Serial.print(cell.colour_data.base_red);
        Serial.print(' ');
        // str[j * 2 + 1] = ' ';
      }
      // str[64] = '\0';
      Serial.println(' ');
    }
    Serial.println("_______________________________________________");
  }
private:
  // std::array<std::array<Cell, HEIGHT>, WIDTH> grid;

  // hoisted buffers — no longer locals
  std::array<std::array<float, 3>, WIDTH * HEIGHT> temperature_data;
  std::array<std::array<uint8_t, 3>, WIDTH * HEIGHT> moving_data;

  // fill temperature_data in place
  void compute_pixel_temperatures();
  // fill moving_data in place (up or down)
  void compute_moving(bool up);
};

class Particle {
public:
  const ELEMENT_ID element_id;
  const ColourParameters colour_data;
  const DIRECTION default_direction;
  const float density;

  Particle(ELEMENT_ID element_id, ColourParameters colour_data, DIRECTION default_direction = DIRECTION::NONE, float density = 0)
    : element_id(element_id), colour_data(colour_data), default_direction(default_direction), density(density) {}

  virtual PARTICLETYPE getType() const {
    return PARTICLETYPE::PARTICLE;
  }

  virtual void move(Grid& grid, uint8_t x, uint8_t y) {
    // Default: do nothing
  }

  virtual void act(Grid& grid, uint8_t x, uint8_t y) {
    // Default: do nothing
  }

  virtual ~Particle() {}  // Virtual destructor for polymorphism
};

inline bool isUpdatable(Particle* particle) {
  PARTICLETYPE type = particle->getType();
  return (type == PARTICLETYPE::UPDATABLE || type == PARTICLETYPE::SOLID || type == PARTICLETYPE::LIQUID || type == PARTICLETYPE::GAS);
  // TODO implement all other materials
}

inline Particle* getElementDataFromElementID(ELEMENT_ID element_id) {
  Particle* element = ELEMENT[static_cast<uint8_t>(element_id)];
  return element;
}

inline Particle* getElementDataFromCell(Cell* cell) {
  return getElementDataFromElementID(cell->element_id);
}


class Updatable : public Particle {
public:
  const std::array<float, 2> phase_change_temp;
  const std::array<ELEMENT_ID, 2> next_phase;
  const float thermal_conductivity;

  Updatable(ELEMENT_ID element_id, ColourParameters colour_data, std::array<float, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, DIRECTION default_direction = DIRECTION::NONE, float density = 0)
    : Particle(element_id, colour_data, default_direction, density), phase_change_temp(phase_change_temp), next_phase(next_phase), thermal_conductivity(thermal_conductivity) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::UPDATABLE;
  }

  virtual ~Updatable() {}  // Virtual destructor for polymorphism

  void act(Grid& grid, uint8_t x, uint8_t y) override {
    Cell& cell = grid.get(x, y);
    int current_temp = cell.temperature;

    Particle* particle = ELEMENT[static_cast<uint8_t>(cell.element_id)];
    if (!isUpdatable(particle)) {  // SOLID inherits from UPDATABLE
      return;
      // safe to use
    }
    Updatable* updatable = static_cast<Updatable*>(particle);

    if (updatable->next_phase[0] != ELEMENT_ID::NO_ELEMENT && current_temp < updatable->phase_change_temp[0]) {
      cell.element_id = updatable->next_phase[0];
      cell.direction = ELEMENT[static_cast<uint8_t>(updatable->next_phase[0])]->default_direction;
      cell.colour_data = ELEMENT[static_cast<uint8_t>(updatable->next_phase[0])]->colour_data;
      cell.update_colour_from_phase_change();
      cell.colour_data.set_base_colour();
      return;
    } else if (updatable->next_phase[1] != ELEMENT_ID::NO_ELEMENT && current_temp > updatable->phase_change_temp[1]) {
      cell.element_id = updatable->next_phase[1];
      cell.direction = ELEMENT[static_cast<uint8_t>(updatable->next_phase[1])]->default_direction;
      cell.colour_data = ELEMENT[static_cast<uint8_t>(updatable->next_phase[1])]->colour_data;

      cell.update_colour_from_phase_change();
      cell.colour_data.set_base_colour();

      return;
    }

    if (updatable->next_phase[1] == ELEMENT_ID::NO_ELEMENT && updatable->phase_change_temp[1] != 0 && current_temp > updatable->phase_change_temp[1]) {  // && cell.flammable
      cell.state_id = STATE_ID::BURNING;
    }

    // if (cell->state_id != STATE_ID::NO_STATE) {
    //   STATE[static_cast<uint8_t>(cell->state_id)].act();
    // }
  }
};

class Solid : public Updatable {
public:

  Solid(ELEMENT_ID element_id, ColourParameters colour_data, std::array<float, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, float density, DIRECTION default_direction = DIRECTION::DOWN)
    : Updatable(element_id, colour_data, phase_change_temp, next_phase, thermal_conductivity, default_direction, density) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::SOLID;
  }

  void move(Grid& grid, uint8_t x, uint8_t y) override {
    std::array<coordinate_return, 3> gaps = grid.check_unimpeded(x, y);

    uint8_t x2 = gaps[1].x;
    uint8_t y2 = gaps[1].y;

    if (is_valid_coordinate({ x2, y2 }) && grid.can_swap(x, y, x2, y2) && grid.can_swap_after_random(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }

    // Must try other free spots to the side
    bool left_to_right = RandomUtils::getRandomBool();
    coordinate_return chosen_gap = left_to_right ? gaps[0] : gaps[2];

    x2 = chosen_gap.x;
    y2 = chosen_gap.y;

    if (is_valid_coordinate(chosen_gap) && grid.can_swap(x, y, x2, y2) && grid.can_swap_after_random(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }

    // Try the other side if first pick failed
    chosen_gap = left_to_right ? gaps[2] : gaps[0];
    x2 = chosen_gap.x;
    y2 = chosen_gap.y;

    if (is_valid_coordinate(chosen_gap) && grid.can_swap(x, y, x2, y2) && grid.can_swap_after_random(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }
  }
};

class Liquid : public Updatable {
public:

  Liquid(ELEMENT_ID element_id, ColourParameters colour_data, std::array<float, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, float density, DIRECTION default_direction = DIRECTION::DOWN)
    : Updatable(element_id, colour_data, phase_change_temp, next_phase, thermal_conductivity, default_direction, density) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::LIQUID;
  }

  void move(Grid& grid, uint8_t x, uint8_t y) override {
    std::array<coordinate_return, 3> gaps = grid.check_unimpeded(x, y);

    uint8_t x2 = gaps[1].x;
    uint8_t y2 = gaps[1].y;

    if (is_valid_coordinate({ x2, y2 }) && grid.can_swap(x, y, x2, y2) && grid.can_swap_after_random(x, y, x2, y2)) {
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

    if (is_valid_coordinate(chosen_gap) && grid.can_swap(x, y, x2, y2) && grid.can_swap_after_random(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }

    // Wondering behaviour
    std::array<uint8_t, 2> gap_distances = grid.check_side_gap_distance(x, y);
    x2 = INVALID_BYTE;

    if (gap_distances[0] > 0 && gap_distances[1] > 0 && gap_distances[0] == gap_distances[1]) {
      x2 = x + (RandomUtils::getRandomBool() ? -1 : 1);
    } else if (gap_distances[0] > 0 && gap_distances[1] == 0) {
      x2 = x + (RandomUtils::getRandomBool() ? -1 : 0);
    } else if (gap_distances[0] > 0 && gap_distances[0] < gap_distances[1]) {
      x2 = x + (RandomUtils::getRandomBool() ? -1 : 0);
    } else if (gap_distances[1] > 0 && gap_distances[0] == 0) {
      x2 = x + (RandomUtils::getRandomBool() ? 0 : 1);
    } else if (gap_distances[1] > 0 && gap_distances[1] < gap_distances[0]) {
      x2 = x + (RandomUtils::getRandomBool() ? 0 : 1);
    }

    if (x2 != INVALID_BYTE && grid.can_swap(x, y, x2, y) && grid.can_swap_after_random(x, y, x2, y)) {
      grid.swap(x, y, x2, y);
    }
  }
};

class Gas : public Updatable {
public:

  Gas(ELEMENT_ID element_id, ColourParameters colour_data, std::array<float, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, float density, DIRECTION default_direction = DIRECTION::DOWN)
    : Updatable(element_id, colour_data, phase_change_temp, next_phase, thermal_conductivity, default_direction, density) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::GAS;
  }

  void move(Grid& grid, uint8_t x, uint8_t y) override {
    std::array<coordinate_return, 3> gaps = grid.check_unimpeded(x, y);

    uint8_t x2 = gaps[1].x;
    uint8_t y2 = gaps[1].y;

    if (is_valid_coordinate({ x2, y2 }) && grid.can_swap(x, y, x2, y2) && grid.can_swap_after_random(x, y, x2, y2)) {
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

    if (is_valid_coordinate(chosen_gap) && grid.can_swap(x, y, x2, y2) && grid.can_swap_after_random(x, y, x2, y2)) {
      grid.swap(x, y, x2, y2);
      return;
    }

    // Wondering behaviour
    std::array<uint8_t, 2> gap_distances = grid.check_side_gap_distance(x, y);
    x2 = INVALID_BYTE;

    if (gap_distances[0] > 0 && gap_distances[1] > 0 && gap_distances[0] == gap_distances[1]) {
      x2 = x + (RandomUtils::getRandomBool() ? -1 : 1);
    } else if (gap_distances[0] > 0 && gap_distances[1] == 0) {
      x2 = x + (RandomUtils::getRandomBool() ? -1 : 0);
    } else if (gap_distances[0] > 0 && gap_distances[0] < gap_distances[1]) {
      x2 = x + (RandomUtils::getRandomBool() ? -1 : 0);
    } else if (gap_distances[1] > 0 && gap_distances[0] == 0) {
      x2 = x + (RandomUtils::getRandomBool() ? 0 : 1);
    } else if (gap_distances[1] > 0 && gap_distances[1] < gap_distances[0]) {
      x2 = x + (RandomUtils::getRandomBool() ? 0 : 1);
    }

    if (x2 != INVALID_BYTE && grid.can_swap(x, y, x2, y) && grid.can_swap_after_random(x, y, x2, y)) {
      grid.swap(x, y, x2, y);
    }
  }
};

//Special classes
class Destruct : public Particle {
public:

  Destruct(ELEMENT_ID element_id = ELEMENT_ID::DESTRUCT, ColourParameters colour_data = ColourParameters({ 25, 55 }, { 25, 55 }, { 25, 55 }, COLOURMODE::NOISE), float density = 0.0)
    : Particle(element_id, colour_data, default_direction, density) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::SPECIAL;
  }

  virtual ~Destruct() {}  // Virtual destructor for polymorphism

  void act(Grid& grid, uint8_t x, uint8_t y) override {
    Cell& cell = grid.get(x, y);
    auto surrounding_data = grid.check_surroundings(x, y);

    std::array<uint8_t, 8> index_list = { INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE };
    uint8_t count = 0;
    uint8_t idx = 0;
    for (auto& data : surrounding_data) {
      if (data.x != INVALID_BYTE && data.cell->element_id != ELEMENT_ID::DESTRUCT) {
        index_list[count++] = idx;
      }
      idx++;
    }

    if (count > 0) {
      uint8_t random_index = RandomUtils::getRandomByte(0, count);

      uint8_t n_x = surrounding_data[index_list[random_index]].x;
      uint8_t n_y = surrounding_data[index_list[random_index]].y;

      // Particle* element = ELEMENT[0]; // Access element from ELEMENT array
      // Colour colour_copy(element->colour_data);
      Cell n_pixel;
      grid.set(n_x, n_y, n_pixel);
    }
  }
};

class Clone : public Particle {
public:

  Clone(ELEMENT_ID element_id = ELEMENT_ID::CLONE, ColourParameters colour_data = ColourParameters({ 150, 0 }, { 150, 0 }, { 0, 0 }, COLOURMODE::SOLID), float density = 0.0)
    : Particle(element_id, colour_data, default_direction, density) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::SPECIAL;
  }

  virtual ~Clone() {}  // Virtual destructor for polymorphism

  void act(Grid& grid, uint8_t x, uint8_t y) override;
};

class Grow : public Updatable {
public:
  Grow(ELEMENT_ID element_id = ELEMENT_ID::GROW, ColourParameters colour_data = ColourParameters({ 0, 0 }, { 150, 250 }, { 10, 0 }, COLOURMODE::STATIC),
       std::array<float, 2> phase_change_temp = { 0, 300.0 }, std::array<ELEMENT_ID, 2> next_phase = { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::NO_ELEMENT },
       float thermal_conductivity = 0.6, float density = 0.0, DIRECTION default_direction = DIRECTION::DOWN)
    : Updatable(element_id, colour_data, phase_change_temp, next_phase, thermal_conductivity, default_direction, density) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::SPECIAL;
  }

  virtual ~Grow() {}  // Virtual destructor for polymorphism

  void act(Grid& grid, uint8_t x, uint8_t y) override {
    Cell& cell = grid.get(x, y);
    auto surrounding_data = grid.check_surroundings(x, y);
    std::array<uint8_t, 8> index_list = { INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE,
                                          INVALID_BYTE };
    uint8_t count = 0;
    uint8_t idx = 0;

    uint8_t random_index;

    uint8_t n_x;
    uint8_t n_y;

    for (auto& data : surrounding_data) {
      if (data.x != INVALID_BYTE && (data.cell->element_id == ELEMENT_ID::AIR || data.cell->element_id == ELEMENT_ID::WATER || data.cell->element_id == ELEMENT_ID::STEAM)) {
        index_list[count++] = idx;
      }
      idx++;
    }

    if (count > 0) {
      if (count > 1) {
        if (RandomUtils::getRandomBool(0.05) && cell.branches > 0 && cell.life > 0) {
          random_index = RandomUtils::getRandomByte(0, count);
          uint8_t random_index_2 = RandomUtils::getRandomByte(0, count);

          if (random_index == random_index_2) {
            return;
          }

          // branch 1
          uint8_t n_x = surrounding_data[index_list[random_index]].x;
          uint8_t n_y = surrounding_data[index_list[random_index]].y;

          // Particle* element = ELEMENT[0]; // Access element from ELEMENT array
          // Colour colour_copy(element->colour_data);
          Cell growth_1(
            ELEMENT_ID::GROW,
            ColourParameters({ 0, 0 }, { 150, 250 }, { 10, 0 }, COLOURMODE::STATIC),
            DIRECTION::NONE,
            true,
            23,
            100,                // fuel,
            cell.life - 10,     // life,
            cell.branches - 1,  // branches,
            STATE_ID::NO_STATE);
          grid.set(n_x, n_y, growth_1);

          // branch 2
          n_x = surrounding_data[index_list[random_index_2]].x;
          n_y = surrounding_data[index_list[random_index_2]].y;

          // Particle* element = ELEMENT[0]; // Access element from ELEMENT array
          // Colour colour_copy(element->colour_data);
          Cell growth_2(
            ELEMENT_ID::GROW,
            ColourParameters({ 0, 0 }, { 150, 250 }, { 10, 0 }, COLOURMODE::STATIC),
            DIRECTION::NONE,
            true,
            23,
            100,                // fuel,
            cell.life - 10,     // life,
            cell.branches - 1,  // branches,
            STATE_ID::NO_STATE);
          grid.set(n_x, n_y, growth_2);

          cell.life = 0;
          cell.branches = 0;
          //
        }
      } else {
        if (RandomUtils::getRandomBool(0.1) && cell.life > 0) {

          random_index = RandomUtils::getRandomByte(0, count);

          n_x = surrounding_data[index_list[random_index]].x;
          n_y = surrounding_data[index_list[random_index]].y;

          // // Particle* element = ELEMENT[0]; // Access element from ELEMENT array
          // // Colour colour_copy(element->colour_data);
          Cell growth(
            ELEMENT_ID::GROW,
            ColourParameters({ 0, 0 }, { 150, 250 }, { 10, 0 }, COLOURMODE::STATIC),
            DIRECTION::NONE,
            true,
            23,
            100,             // fuel,
            cell.life - 10,  // life,
            cell.branches,   // branches,
            STATE_ID::NO_STATE);
          grid.set(n_x, n_y, growth);
          cell.life = 0;
          cell.branches = 0;
        }
      }
    }
  }
};

// Solids
Solid* sand = new Solid(
  ELEMENT_ID::SAND,
  ColourParameters({ 200, 250 }, { 125, 200 }, { 75, 100 }, COLOURMODE::BLACKBODY),
  { 0, 0 },                                            // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::NO_ELEMENT },  // phase changes
  0.4f,                                                // thermal conductivity
  1602.0f,                                             // density
  DIRECTION::DOWN);

Solid* ice = new Solid(
  ELEMENT_ID::ICE,
  ColourParameters({ 55, 85 }, { 55, 85 }, { 255, 0 }, COLOURMODE::SOLID),
  { 0, 0 },                                       // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::WATER },  // phase changes
  2.22f,                                          // thermal conductivity
  0.0f,                                           // density
  DIRECTION::NONE);

Solid* snow = new Solid(
  ELEMENT_ID::SNOW,
  ColourParameters({ 240, 254 }, { 240, 254 }, { 240, 254 }, COLOURMODE::SOLID),
  { 0, 0 },                                       // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::WATER },  // phase changes
  0.4f,                                           // thermal conductivity
  1602.0f,                                        // density
  DIRECTION::DOWN);

Solid* stone = new Solid(
  ELEMENT_ID::STONE,
  ColourParameters({ 0, 200 }, { 0, 200 }, { 0, 200 }, COLOURMODE::BLACKBODY),
  { 0, 1160 },                                   // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::LAVA },  // phase changes
  2.5f,                                          // thermal conductivity
  2500.0f,                                       // density
  DIRECTION::DOWN);

Solid* wood = new Solid(
  ELEMENT_ID::WOOD,
  ColourParameters({ 50, 100 }, { 25, 55 }, { 0, 0 }, COLOURMODE::SOLID),
  { 0, 250 },                                          // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::NO_ELEMENT },  // phase changes
  0.15f,                                               // thermal conductivity
  0.0f,                                                // density
  DIRECTION::NONE);

Solid* metal = new Solid(
  ELEMENT_ID::METAL,
  ColourParameters({ 57, 0 }, { 75, 0 }, { 80, 0 }, COLOURMODE::BLACKBODY),
  { 0, 250 },                                          // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::NO_ELEMENT },  // phase changes
  2.5f,                                                // thermal conductivity
  0.0f,                                                // density
  DIRECTION::NONE);

// TODO
Particle* block = new Particle(
  ELEMENT_ID::BLOCK,
  ColourParameters({ 100, 0 }, { 100, 0 }, { 100, 0 }, COLOURMODE::SOLID));

// Liquids
Liquid* water = new Liquid(
  ELEMENT_ID::WATER,
  ColourParameters({ 0, 0 }, { 0, 0 }, { 254, 0 }, COLOURMODE::SOLID),
  // ColourParameters({ 0, 200 }, { 0, 200 }, { 0, 0 }, COLOURMODE::BLACKBODY),

  { 0, 100 },                              // temperature range
  { ELEMENT_ID::ICE, ELEMENT_ID::STEAM },  // phase changes
  0.6f,                                    // thermal conductivity
  997.0f,                                  // density
  DIRECTION::DOWN);

Liquid* oil = new Liquid(
  ELEMENT_ID::OIL,
  ColourParameters({ 50, 0 }, { 25, 0 }, { 25, 0 }, COLOURMODE::SOLID),
  { 0, 300 },                                          // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::NO_ELEMENT },  // phase changes
  0.6f,                                                // thermal conductivity
  800.0f,                                              // density
  DIRECTION::DOWN);

Liquid* lava = new Liquid(
  ELEMENT_ID::LAVA,
  ColourParameters({ 254, 0 }, { 150, 250 }, { 150, 250 }, COLOURMODE::NOISE),
  { 1160, 0 },                                    // temperature range
  { ELEMENT_ID::STONE, ELEMENT_ID::NO_ELEMENT },  // phase changes
  2.0f,                                           // thermal conductivity
  2500.0f,                                        // density
  DIRECTION::DOWN);

Liquid* molten_metal = new Liquid(
  ELEMENT_ID::MOLTEN_METAL,
  ColourParameters({ 125, 0 }, { 75, 0 }, { 85, 0 }, COLOURMODE::BLACKBODY),
  { 1538, 0 },                                    // temperature range
  { ELEMENT_ID::METAL, ELEMENT_ID::NO_ELEMENT },  // phase changes
  2.5f,                                           // thermal conductivity
  7800.0f,                                        // density
  DIRECTION::DOWN);


// Gasses
Gas* air = new Gas(
  ELEMENT_ID::AIR,
  ColourParameters({ 0, 0 }, { 0, 0 }, { 0, 0 }, COLOURMODE::SOLID),
  { 0, 0 },                                            // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::NO_ELEMENT },  // phase changes
  0.026f,                                              // thermal conductivity
  1.23f,                                               // density
  DIRECTION::UP);

Gas* steam = new Gas(
  ELEMENT_ID::STEAM,
  ColourParameters({ 200, 0 }, { 200, 0 }, { 200, 0 }, COLOURMODE::TEMPERATURE_DOWN),
  { 100, 0 },                                     // temperature range
  { ELEMENT_ID::WATER, ELEMENT_ID::NO_ELEMENT },  // phase changes
  0.0184f,                                        // thermal conductivity
  0.6,                                            // density
  DIRECTION::UP);

Gas* smoke = new Gas(
  ELEMENT_ID::SMOKE,
  ColourParameters({ 254, 0 }, { 254, 0 }, { 254, 0 }, COLOURMODE::SOLID),
  { 0, 300 },                                    // temperature range
  { ELEMENT_ID::NO_ELEMENT, ELEMENT_ID::FIRE },  // phase changes
  0.6,                                           // thermal conductivity
  0.6,                                           // density
  DIRECTION::UP);

Gas* fire = new Gas(
  ELEMENT_ID::FIRE,
  ColourParameters({ 200, 254 }, { 0, 50 }, { 0, 50 }, COLOURMODE::STATIC),
  { 300, 0 },                                     // temperature range
  { ELEMENT_ID::SMOKE, ELEMENT_ID::NO_ELEMENT },  // phase changes
  0.6,                                            // thermal conductivity
  0.6,                                            // density
  DIRECTION::UP);

Destruct* destruct = new Destruct();

Clone* clone = new Clone();

Grow* grow = new Grow();


// List of Elements
Particle* ELEMENT[ELEMENT_COUNT] = {
  air,
  sand,
  water,
  ice,
  steam,
  snow,
  smoke,
  fire,
  oil,
  lava,
  stone,
  wood,
  molten_metal,
  metal,
  block,
  destruct,
  clone,
  grow,
};

//States data
class State {
public:
  const STATE_ID state_id;

  State(STATE_ID state_id = STATE_ID::NO_STATE)
    : state_id(state_id) {}

  virtual ~State() {}  // Virtual destructor for polymorphism

  virtual void act(Grid& grid, uint8_t x, uint8_t y) {
    //nuffing
  }
};

class Burning : public State {
public:

  Burning(STATE_ID state_id = STATE_ID::BURNING)
    : State(state_id) {}

  void act(Grid& grid, uint8_t x, uint8_t y) override {
    Cell& cell = grid.get(x, y);
    cell.temperature += 2;

    cell.fuel -= 1;
    if (cell.fuel == 0) {
      Cell cell;
      grid.set(x, y, cell);
    }
  }

  // void colour()
};

State* no_state = new State();

Burning* burning = new Burning();

State* STATE[3] = {
  no_state,
  burning,
  burning
};

// std::array<uint8_t, 3> Cell::get_output_colour_from_behaviour() {
//   COLOURMODE mode = colour_data.colour_mode;
//   // std::array<uint8_t, 3> colour_return = {0, 0, 0};

//   if (mode == COLOURMODE::SOLID || mode == COLOURMODE::STATIC) {
//     return { colour_data.base_red, colour_data.base_green, colour_data.base_blue };
//   } else if (mode == COLOURMODE::BLACKBODY) {
//     float temperature = temperature;
//     uint8_t blackbody_constant = 30;

//     uint8_t r = colour_data.base_red;
//     uint8_t g = colour_data.base_green;
//     uint8_t b = colour_data.base_blue;

//     uint8_t r_return = static_cast<uint8_t>((std::min(std::max(r + (2.0f * (temperature) / (blackbody_constant)), 0.0f), 254.0f)));
//     uint8_t g_return = static_cast<uint8_t>((std::min(std::max(g + (0.5f * (temperature) / (blackbody_constant)), 0.0f), 254.0f)));
//     uint8_t b_return = static_cast<uint8_t>((std::min(std::max(b + (1.0f * (temperature) / (blackbody_constant)), 0.0f), 254.0f)));

//     // std::array<uint8_t, 3> colour_return = {r_return, g_return, b_return};
//     return { r_return, g_return, b_return };
//     // } else if (mode == COLOURMODE::TEMPERATURE_DOWN) {
//     //   float temperature = temperature;
//     //   float ratio;
//     //   Updatable* element_data = ELEMENT[static_cast<uint8_t>(element_id)];
//   }

//   if (element_data->next_phase[0] != ELEMENT_ID::NO_ELEMENT and element_data->next_phase[1] != ELEMENT_ID::NO_ELEMENT) {
//     ratio = (temperature - element_data->phase_change_temp[0]) / (0.5 * element_data->phase_change_temp[1] - element_data->phase_change_temp[0]);
//   } else if (element_data->next_phase[0] != ELEMENT_ID::NO_ELEMENT) {
//     ratio = 1.5 - (element_data->phase_change_temp[0] / temperature);
//   } else {
//     ratio = std::max(0.5f, temperature / 200);
//   }


//   uint8_t r = colour_data.base_red;
//   uint8_t g = colour_data.base_green;
//   uint8_t b = colour_data.base_blue;

//   uint8_t r_return = static_cast<uint8_t>((std::min(std::max((r * ratio), 0.0f), 254.0f)));
//   uint8_t g_return = static_cast<uint8_t>((std::min(std::max((g * ratio), 0.0f), 254.0f)));
//   uint8_t b_return = static_cast<uint8_t>((std::min(std::max((b * ratio), 0.0f), 254.0f)));

//   return { r_return, g_return, b_return };
// }

bool Grid::can_swap(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  Cell& cell_1 = get(x1, y1);
  Cell& cell_2 = get(x2, y2);

  DIRECTION direction_1 = cell_1.direction;
  DIRECTION direction_2 = cell_2.direction;

  Particle* element_1 = ELEMENT[static_cast<uint8_t>(cell_1.element_id)];
  Particle* element_2 = ELEMENT[static_cast<uint8_t>(cell_2.element_id)];

  float element_1_density = element_1->density;
  float element_2_density = element_2->density;

  if (element_1_density == 0.0 || element_2_density == 0.0) {
    return false;
  }

  // Debugging
  // if (element_1->element_id == ELEMENT_ID::STEAM && y2 - y1 == 1 && x2 == x1) {
  //   Serial.print("STEAM CAN SWAP: ");
  //   Serial.print(" x1: ");
  //   Serial.print(x1);
  //   Serial.print(" y1: ");
  //   Serial.print(y1);
  //   Serial.print(" x2: ");
  //   Serial.print(x2);
  //   Serial.print(" y2: ");
  //   Serial.print(y2);
  //   Serial.print(" E1 Direction ");
  //   Serial.print(static_cast<int>(direction_1));
  //   Serial.print(" E2 Direction ");
  //   Serial.print(static_cast<int>(direction_2));
  //   Serial.print(" E1 Default Direction ");
  //   Serial.print(static_cast<int>(element_1->default_direction));
  //   Serial.print(" E1 Density ");
  //   Serial.print(element_1_density);
  //   Serial.print(" E2 Direction ");
  //   Serial.println(element_2_density);
  //   // Serial.print(static_cast<int>(direction_2));
  //   // Serial.println(static_cast<int>(direction_2));

  // }

  if (isUpdatable(element_2) && element_2_density > 0 && direction_1 != DIRECTION::NONE) {
    if (direction_1 != element_1->default_direction) {
      if (direction_1 == DIRECTION::DOWN && element_1_density < element_2_density) {
        return true;
      }
      if (direction_1 == DIRECTION::UP && element_1_density > element_2_density) {
        return true;
      }
    } else if (direction_1 != direction_2 && direction_2 != DIRECTION::NONE) {
      if (direction_1 == DIRECTION::UP) {
        return false;
      }
      return true;
    } else if (direction_1 == DIRECTION::DOWN && element_1_density > element_2_density) {
      return true;
    } else if (direction_1 == DIRECTION::UP && element_1_density < element_2_density) {
      return true;
    }
  }
  return false;
}

bool Grid::can_swap_after_random(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2) {
  // """ Assumes the check has already been made to see if these can swap """
  //TODO
  Cell& cell_1 = get(x1, y1);
  Cell& cell_2 = get(x2, y2);

  Particle* element_1 = ELEMENT[static_cast<uint8_t>(cell_1.element_id)];
  Particle* element_2 = ELEMENT[static_cast<uint8_t>(cell_2.element_id)];

  float element_1_density = element_1->density;
  float element_2_density = element_2->density;

  return RandomUtils::getRandomBool((element_1_density / (element_1_density + element_2_density)));
}

float Grid::get_new_temperature_difference(uint8_t x, uint8_t y) {
  Cell& current_pixel = get(x, y);
  Particle* current_element = getElementDataFromCell(&current_pixel);

  if (!isUpdatable(current_element)) {
    return 0;
  }

  auto neighbours = check_surroundings(x, y);
  float sum_influence = 0;
  uint8_t updatable_neighbour_count = 0;

  for (auto& data : neighbours) {
    Cell* neighbour = data.cell;
    if (neighbour) {
      Particle* p2 = getElementDataFromCell(neighbour);
      if (isUpdatable(p2)) {
        updatable_neighbour_count++;
        float t_diff = neighbour->temperature - current_pixel.temperature;
        float k1 = static_cast<Updatable*>(current_element)->thermal_conductivity;
        float k2 = static_cast<Updatable*>(p2)->thermal_conductivity;
        sum_influence += t_diff * ((k1 * k2) / (k1 + k2));
      }
    }
  }
  return (updatable_neighbour_count > 0)
           ? (sum_influence / updatable_neighbour_count)
           : 0;
}

void Grid::perform_pixel_behaviour() {
  get_pixel_temperatures();
  for (auto& temperature_entry : temperature_data) {
    uint8_t x = static_cast<uint8_t>(temperature_entry[0]);
    uint8_t y = static_cast<uint8_t>(temperature_entry[1]);
    if (!in_bounds(x, y)) continue;
    float temperature_delta = temperature_entry[2];
    Cell& cell = get(x, y);
    cell.temperature += temperature_delta;

    if (cell.state_id != STATE_ID::NO_STATE) {
      STATE[static_cast<uint8_t>(cell.state_id)]->act(*this, x, y);
      // TODO
    }

    Particle* current_element = getElementDataFromCell(&cell);

    current_element->act(*this, x, y);
    // TODO update colour?
  }


  // — up-moving pass —
  get_moving(true);
  for (auto& m : moving_data) {
    uint8_t x = m[0], y = m[1];
    // bail out if out-of-bounds or not flagged to move
    if (!in_bounds(x, y) || !m[2])
      continue;

    // 1) get the cell
    Cell& cell = get(x, y);
    // 2) pull out the Particle* for that cell
    Particle* element = getElementDataFromCell(&cell);
    // 3) call its move()
    element->move(*this, x, y);
  }

  // — down-moving pass —
  get_moving(false);
  for (auto& m : moving_data) {
    uint8_t x = m[0], y = m[1];
    if (!in_bounds(x, y) || !m[2])
      continue;

    Cell& cell = get(x, y);
    Particle* element = getElementDataFromCell(&cell);
    element->move(*this, x, y);
  }
}

Cell createCellInstance(ELEMENT_ID id,
                        float temperature = 23,
                        STATE_ID state_id = STATE_ID::NO_STATE,
                        bool flammable = false,
                        uint8_t fuel = 0,
                        uint8_t life = 0,
                        uint8_t branches = 0) {

  Particle* element = ELEMENT[static_cast<uint8_t>(id)];  // Access element from ELEMENT array
  Colour colour_copy(element->colour_data);

  // pixel attribute ordering
  // id,
  // colour_copy,
  // direction,
  // flammable,
  // temperature,
  // fuel,
  // life,
  // branches,
  // state_id,
  // clone_element_id,

  Cell pixel(
    id,
    colour_copy,
    element->default_direction,
    flammable,
    temperature,
    fuel,
    life,
    branches,
    state_id,
    ELEMENT_ID::NO_ELEMENT);

  return pixel;
}

Cell element_pallet(ELEMENT_ID element_id) {
  Cell ret;
  if (element_id == ELEMENT_ID::STEAM) {
    ret = createCellInstance(element_id, 150);
  } else if (element_id == ELEMENT_ID::SNOW) {
    ret = createCellInstance(element_id, -25);
  } else if (element_id == ELEMENT_ID::ICE) {
    ret = createCellInstance(element_id, -50);
  } else if (element_id == ELEMENT_ID::FIRE) {
    ret = createCellInstance(element_id, 800);
  } else if (element_id == ELEMENT_ID::MOLTEN_METAL) {
    ret = createCellInstance(element_id, 2000);
  } else if (element_id == ELEMENT_ID::GROW) {
    ret = createCellInstance(element_id, 23, STATE_ID::NO_STATE, true, 0, 100, 3);
  } else {
    // Default behaviour
    ret = createCellInstance(element_id);
  }
  return ret;
}

void Clone::act(Grid& grid, uint8_t x, uint8_t y) {
  Cell& cell = grid.get(x, y);
  auto surrounding_data = grid.check_surroundings(x, y);

  std::array<uint8_t, 8> index_list = { INVALID_BYTE,
                                        INVALID_BYTE,
                                        INVALID_BYTE,
                                        INVALID_BYTE,
                                        INVALID_BYTE,
                                        INVALID_BYTE,
                                        INVALID_BYTE,
                                        INVALID_BYTE };
  uint8_t count = 0;
  uint8_t idx = 0;


  if (cell.clone_element == ELEMENT_ID::NO_ELEMENT || cell.clone_element == ELEMENT_ID::CLONE) {
    // Get clone material
    for (auto& data : surrounding_data) {
      if (data.x != INVALID_BYTE && data.cell->element_id != ELEMENT_ID::AIR) {
        index_list[count++] = idx;
      }
      idx++;
    }

    if (count > 0) {
      uint8_t random_index = RandomUtils::getRandomByte(0, count);

      ELEMENT_ID clone_element_id = surrounding_data[index_list[random_index]].cell->element_id;
      cell.clone_element = clone_element_id;
    }
  } else {
    // Spawn Clone Material
    for (auto& data : surrounding_data) {
      if (data.x != INVALID_BYTE && data.cell->element_id == ELEMENT_ID::AIR) {
        index_list[count++] = idx;
      }
      idx++;
    }

    if (count > 0) {
      uint8_t random_index = RandomUtils::getRandomByte(0, count);

      uint8_t n_x = surrounding_data[index_list[random_index]].x;
      uint8_t n_y = surrounding_data[index_list[random_index]].y;

      // Particle* element = ELEMENT[0]; // Access element from ELEMENT array
      // Colour colour_copy(element->colour_data);
      Cell n_pixel = createCellInstance(cell.clone_element, 23, STATE_ID::NO_STATE, false, 0, 0, 0);
      grid.set(n_x, n_y, n_pixel);
    }
  }
}


class TouchSampler {
public:
  TouchSampler(uint8_t readPin, uint8_t outHighPin, uint8_t outLowPin, uint8_t falseReadPin, std::array<uint16_t, 2> range, uint8_t axisSize)
    : readPin(readPin), outHighPin(outHighPin), outLowPin(outLowPin), falseReadPin(falseReadPin), range(range), axisSize(axisSize) {
    index = 0;
    lastSampleTime = 0;
    sampleCount = 0;
    std::fill(samples.begin(), samples.end(), 0);
  }

  void preparePins() {
    pinMode(outHighPin, OUTPUT);
    pinMode(outLowPin, OUTPUT);
    pinMode(readPin, INPUT);
    pinMode(falseReadPin, INPUT);
    digitalWrite(outHighPin, HIGH);
    digitalWrite(outLowPin, LOW);
  }

  void readSignal() {
    preparePins();
    samples[index] = analogRead(readPin);
    index = (index + 1) % TOUCH_SAMPLE_COUNT;
    // if (sampleCount < TOUCH_SAMPLE_COUNT) sampleCount++;
  }

  bool isStableAndInRange() const {
    int sum = 0;
    for (int s : samples) sum += s;
    int avg = sum / TOUCH_SAMPLE_COUNT;

    int maxDev = 0;
    for (int s : samples) {
      int dev = abs(s - avg);
      if (dev > maxDev) maxDev = dev;
    }

    return (avg >= range[0] && avg <= range[1] && maxDev <= TOUCH_STABILITY_THRESHOLD);
  }

  uint16_t getAverage() const {
    int sum = 0;
    for (int s : samples) sum += s;
    return sum / TOUCH_SAMPLE_COUNT;
  }

  uint8_t getCoord() const {
    return (axisSize - static_cast<float>(axisSize) * (getAverage() - range[0]) / (range[1] - range[0]));
  }

private:
  uint8_t readPin, outHighPin, outLowPin, falseReadPin;
  std::array<uint16_t, 2> range;
  uint8_t axisSize;
  std::array<uint16_t, TOUCH_SAMPLE_COUNT> samples;
  int index;
  int sampleCount;
  unsigned long lastSampleTime;
};

TouchSampler xSampler(7, 6, 15, 16, { 200, 3900 }, 32);
TouchSampler ySampler(6, 7, 16, 15, { 780, 3250 }, 16);

Grid space;

void display() {
  int16_t num = 0;
  for (uint8_t y = 0; y < HEIGHT; y++) {
    for (uint8_t x = 0; x < WIDTH; x++) {
      if (y >= 8) {
        if (x % 2 != 0) {
          num = 8 * x + y - 8;
        } else {
          num = 8 * x + (7 - y) + 8;
        }
      } else {
        if (x % 2 == 0) {
          num = 256 + 8 * (31 - x) + (7 - (y % 8));
        } else {
          num = 256 + 8 * (31 - x) + (y % 8);
        }
      }

      Cell& cell = grid[x][y];
      uint8_t red = cell.colour_data.base_red;
      uint8_t green = cell.colour_data.base_green;
      uint8_t blue = cell.colour_data.base_blue;

      pixels.setPixelColor(num, pixels.Color(red, green, blue));
    }
  }
  pixels.show();  // Send the updated pixel colors to the hardware.
}

void unit_test() {
  Serial.println("frame ---------------");
  // grid.perform_pixel_behaviour();

  Serial.println("attempting in_bounds ---------------");
  Serial.println(space.in_bounds(0, 0));
  Serial.println(space.in_bounds(31, 15));
  Serial.println(space.in_bounds(32, 32));
  Serial.println("success ---------------");

  Serial.println("attempting get ---------------");
  space.get(0, 0);
  Cell& cell = space.get(5, 5);
  Serial.println("success ---------------");

  Serial.println("attempting get getElementDataFromCell ---------------");
  Particle* current_element = getElementDataFromCell(&cell);
  Serial.println("success ---------------");

  Serial.println("attempting get direction ---------------");
  Serial.println(static_cast<int8_t>(current_element->default_direction));
  Serial.println("success ---------------");

  Serial.println("attempting get temp data ---------------");
  space.get_pixel_temperatures();
  Serial.println("success ---------------");

  Serial.println("attempting particle act ---------------");
  current_element->act(space, 5, 5);
  Serial.println("success ---------------");

  Serial.println("attempting get_moving true ---------------");
  space.get_moving(true);
  Serial.println("success ---------------");

  Serial.println("attempting get_moving false ---------------");
  space.get_moving(false);
  Serial.println("success ---------------");


  Serial.println("attempting moving ---------------");
  current_element->move(space, 5, 5);
  Serial.println("success ---------------");

  Particle* e = ELEMENT[0];
  Serial.println(static_cast<int8_t>(e->default_direction));

  // grid.swap(5, 0, 5, 5);
  space.print();
}

coordinate_return get_touch_routine() {
  coordinate_return ret = { INVALID_BYTE, INVALID_BYTE };

  xSampler.readSignal();
  if (xSampler.isStableAndInRange()) {
    ret.x = xSampler.getCoord();
  }

  ySampler.readSignal();
  if (ySampler.isStableAndInRange()) {
    ret.y = ySampler.getCoord();
  }

  return ret;
}

coordinate_return get_touch_input() {
  std::array<uint16_t, 2> x_range = { 200, 3900 };
  std::array<uint16_t, 2> y_range = { 780, 3250 };
  coordinate_return coords = { INVALID_BYTE, INVALID_BYTE };



  coords = get_touch_routine();

  // Serial.print("X analog: ");
  // Serial.print(x_analog);

  // Serial.print(" Y analog: ");
  // Serial.print(y_analog);

  Serial.print(" X return: ");
  Serial.print(coords.x);

  Serial.print(" Y return: ");
  Serial.println(coords.y);

  // Exit early as it's not registering a proper touch
  if (coords.x == INVALID_BYTE || coords.y == INVALID_BYTE) return coords;

  uint8_t x = coords.x;
  uint8_t y = coords.y;

  ///
  int16_t num = 0;
  if (y >= 8) {
    if (x % 2 != 0) {
      num = 8 * x + y - 8;
    } else {
      num = 8 * x + (7 - y) + 8;
    }
  } else {
    if (x % 2 == 0) {
      num = 256 + 8 * (31 - x) + (7 - (y % 8));
    } else {
      num = 256 + 8 * (31 - x) + (y % 8);
    }
  }

  // pixels.clear();
  pixels.setPixelColor(num, pixels.Color(255, 255, 255));
  pixels.show();  // Send the updated pixel colors to the hardware.

  return coords;
}

void handle_draw(ELEMENT_ID selected_element_id) {
  coordinate_return touch_data = get_touch_input();
  if (touch_data.x != INVALID_BYTE && touch_data.y != INVALID_BYTE) {
    Cell cell = element_pallet(selected_element_id);
    space.draw(touch_data.x, touch_data.y, cell, 0.75);
  }
}

std::array<Cell, ELEMENT_COUNT> palette_elements;

class Palette {
public:
  uint8_t width;
  uint8_t row_space;
  uint16_t hold_counter = 250;
  bool visible = false; // THIS LINE
  ELEMENT_ID selected = ELEMENT_ID::SAND;
  

  Palette(uint8_t width, uint8_t row_space)
    : width(width), row_space(row_space) {
    this->setup();
  }

  void setup() {
    // for (auto& element : ELEMENT) {
    //   palette_elements[static_cast<uint8_t>(element.element_id)] = element_pallet(element->element_id);
    //   palette_elements[static_cast<uint8_t>(element.element_id)].colour_data.set_base_colour();
    // }

    for (int i = 0; i < ELEMENT_COUNT; i++) { //TODO replace with ELEMENT_COUNT
      ELEMENT_ID elem = static_cast<ELEMENT_ID>(i);
      palette_elements[i] = element_pallet(elem);
      Serial.println(palette_elements[i].colour_data.base_red);
      
      // palette_elements[i].colour_data.set_base_colour();
    }
    // Default: do nothing
  }

  void display_circle(uint8_t touch_x, uint8_t touch_y, uint8_t radius) {
    // uint8_t x, y; //inputs
    for (uint8_t x = 0; x < 32; x++) {
      for (uint8_t y = 0; y < 16; y++) {

        int16_t num = 0;
        if (y >= 8) {
          if (x % 2 != 0) {
            num = 8 * x + y - 8;
          } else {
            num = 8 * x + (7 - y) + 8;
          }
        } else {
          if (x % 2 == 0) {
            num = 256 + 8 * (31 - x) + (7 - (y % 8));
          } else {
            num = 256 + 8 * (31 - x) + (y % 8);
          }
        }

        if ((std::pow((x - touch_x), 2) + std::pow((y - touch_y), 2)) > std::pow(radius, 2) || ((std::pow((x - touch_x), 2) + std::pow((y - touch_y), 2)) < std::pow(radius - 1, 2))) {
          pixels.setPixelColor(num, pixels.Color(0, 0, 0));
          continue;
        }

        // Update colours for relevant colours TODO

        pixels.setPixelColor(num, pixels.Color(255, 255, 255));
      }
    }
    // pixels.show();
  };

  void select_routine() {
    
    // Find closest cell,
    coordinate_return coords = get_touch_routine();

    // Close circle in using timer
    if (coords.x != INVALID_BYTE && coords.y != INVALID_BYTE) { //if touch is registered
     
      uint8_t radius = 16;
      
      hold_counter -= 5;
      

      if (hold_counter <= 0) {

        // Need to go from x, y to the element index
        for (auto& element : ELEMENT) {
          if (coords.x == ((static_cast<uint8_t>(element->element_id) % width) * (32 / width) + (32 / width) / 2) &&
              coords.y == ((static_cast<uint8_t>(element->element_id) / width) * (row_space) + (16 / row_space) / 2)) {
            if (element->element_id != ELEMENT_ID::NO_ELEMENT) {
              selected = element->element_id;
              visible = false;
            }
          }
        }
        // selected = ELEMENT_ID::WATER;
        // visible = false;
        hold_counter = 250;
      }
      display_circle(coords.x, coords.y, 1 + radius * (hold_counter / 250.0));
      
    } else {
      hold_counter = 250;
    }
    // Close menu when finished and return to game.
  }

  void display() {
    uint8_t x, y;
    for (auto& element : ELEMENT) {
      // Serial.println(());
      // Serial.println((static_cast<uint8_t>(element->element_id)));
      
      // x = (static_cast<uint8_t>(element->element_id) * 32 / width) % width;
      // y = (static_cast<uint8_t>(element->element_id) * 32 / width) / width;

      x = ((static_cast<uint8_t>(element->element_id) % width) * (32 / width) + (32 / width) / 2) ;
      y = 16 - ((static_cast<uint8_t>(element->element_id) / width) * (row_space) + (16 / row_space) / 2) ;
      int16_t num = 0;
      if (y >= 8) {
        if (x % 2 != 0) {
          num = 8 * x + y - 8;
        } else {
          num = 8 * x + (7 - y) + 8;
        }
      } else {
        if (x % 2 == 0) {
          num = 256 + 8 * (31 - x) + (7 - (y % 8));
        } else {
          num = 256 + 8 * (31 - x) + (y % 8);
        }
      }

      // Update colours for relevant colours TODO

      uint8_t red = palette_elements[static_cast<uint8_t>(element->element_id)].colour_data.base_red;
      uint8_t green = palette_elements[static_cast<uint8_t>(element->element_id)].colour_data.base_green;
      uint8_t blue = palette_elements[static_cast<uint8_t>(element->element_id)].colour_data.base_blue;

      pixels.setPixelColor(num, pixels.Color(red, green, blue));
    }

  };

  void routine() {
    select_routine();
    display();
    pixels.show();
    pixels.clear();
  };
};

Palette* palette;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(460800);
  Cell test = createCellInstance(ELEMENT_ID::STONE, 10000.0);
  space.set(5, 5, test);  //This one is in the bottom corner

  Cell destrtuct = createCellInstance(ELEMENT_ID::DESTRUCT);
  space.set(31, 0, destrtuct);

  // Cell clone = createCellInstance(ELEMENT_ID::CLONE);
  // space.set(15, 10, clone);

  Cell block = createCellInstance(ELEMENT_ID::BLOCK);
  space.set(31, 10, block);

  Cell grow = createCellInstance(ELEMENT_ID::GROW, 23, STATE_ID::NO_STATE, true, 0, 100, 3);
  space.set(28, 6, grow);



  space.print();
  // grid.set(20, 14, test);  //This one is in the top right corner
  // grid.set(32, 16, test); //This one is missing from the grid.print
  // grid.set(0, 0, test); //This one is missing from the grid.print
  palette = new Palette(5, 3);
  pixels.begin();
}

void loop() {
  if ((millis() / 10000) % 30 == 4) {
    palette->visible = true;
  }

  if (palette->visible) {
    palette->routine();
  } else {
    space.perform_pixel_behaviour();
    handle_draw(palette->selected);
    display();
  }
}
