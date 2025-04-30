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

constexpr byte INVALID_BYTE = 255;

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

struct ColourParameters {
  std::array<byte, 2> param_red;
  std::array<byte, 2> param_green;
  std::array<byte, 2> param_blue;
  COLOURMODE colour_mode = COLOURMODE::STATIC;

  ColourParameters(std::array<byte, 2> r, std::array<byte, 2> g, std::array<byte, 2> b, COLOURMODE mode)
    : param_red(r), param_green(g), param_blue(b), colour_mode(mode) {}
};

//  If scope allows it, this will contain the data that the clone material will use
struct CloneData {
  ELEMENT_ID element_id;
  STATE_ID state_id;
  bool flammable;
  float temperature;
  byte fuel;
  byte life;
  byte branches;
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
  std::array<byte, 2> param_red;
  std::array<byte, 2> param_green;
  std::array<byte, 2> param_blue;

  byte base_red;
  byte base_green;
  byte base_blue;

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
  byte fuel;
  byte life;
  byte branches;
  ELEMENT_ID clone_element;

  Cell()
    : element_id(ELEMENT_ID::AIR),  // default to AIR
      colour_data(Colour(ColourParameters({ 0, 0 }, { 0, 0 }, { 0, 0 }, COLOURMODE::SOLID))),
      direction(DIRECTION::NONE),
      state_id(STATE_ID::NO_STATE),
      flammable(false),
      temperature(23.0),
      fuel(0),
      life(0),
      branches(0),
      clone_element(ELEMENT_ID::NO_ELEMENT) {}

  Cell(ELEMENT_ID element_id, Colour colour_data, DIRECTION direction = DIRECTION::NONE, bool flammable = false, float temperature = 23.0, byte fuel = 0, byte life = 0, byte branches = 0, STATE_ID state_id = STATE_ID::NO_STATE, ELEMENT_ID clone_element_id = ELEMENT_ID::NO_ELEMENT)
    : element_id(element_id), colour_data(colour_data), direction(direction), state_id(state_id), flammable(flammable), temperature(temperature), fuel(fuel), life(life), branches(branches), clone_element(clone_element_id) {
    this->colour_data.set_base_colour();
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

  grid_return()
    : x(INVALID_BYTE), y(INVALID_BYTE), cell(nullptr) {}
  grid_return(byte x, byte y, Cell* cell)
    : x(x), y(y), cell(cell) {}
};

constexpr byte width = 32;
constexpr byte height = 16;
//Cell grid[width][height];


inline bool is_valid_coordinate(const coordinate_return& coord) {  //Is the returned coordinate a proper value
  return coord.x != INVALID_BYTE && coord.y != INVALID_BYTE;
}


class Grid {
public:
  static constexpr byte WIDTH = 32;
  static constexpr byte HEIGHT = 16;
  //    _ = ["Cell(air, temperature=23)" for _ in range(height)]
  //    self.grid = [_ for _ in range(width)]
  Grid() {}

  Cell& get(byte x, byte y) {
    return grid[x][y];
  }

  void set(byte x, byte y, Cell pixel) {
    grid[x][y] = pixel;
  }

  bool is_empty(byte x, byte y) {
    return (get(x, y).element_id == ELEMENT_ID::AIR);
  }

  void draw(byte x, byte y, Cell pixel, float probability) {
    if (is_empty(x, y) && RandomUtils::getRandomBool(probability)) {
      set(x, y, pixel);
    }
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

  std::array<grid_return, 8> check_surroundings(byte x, byte y) {
    std::array<int8_t, 3> delta = { -1, 0, 1 };
    std::array<grid_return, 8> neighbour_list = { { { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr },
                                                    { INVALID_BYTE, INVALID_BYTE, nullptr } } };

    byte count = 0;
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

  std::array<byte, 2> check_side_gap_distance(byte x, byte y) {
    // Measured from the x coordinate, the distance to the closest side gap
    std::array<byte, 2> side_gaps = { 0, 0 };
    int8_t direction = static_cast<int8_t>(get(x, y).direction);
    if (direction != 0) {
      if (!in_bounds(x, y + direction)) {
        return side_gaps;
      }

      bool current_layer_gap = false;
      bool next_layer_gap = false;

      // Left Side
      if (x != 0) {
        for (byte i = 0; i < x; i++) {
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
        for (byte i = 1; i < WIDTH - x; i++) {
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

  float get_new_temperature_difference(byte x, byte y);

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
    DIRECTION check = up ? DIRECTION::True : DIRECTION::False;

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
    for (byte i = 0; i < HEIGHT; i++) {
      char str[WIDTH * 2 + 1];
      for (byte j = 0; j < WIDTH; j++) {
        Cell& cell = get(j, HEIGHT - 1 - i);
        if (cell.element_id == ELEMENT_ID::STONE) {
          str[j * 2] = 'O';
        } else {
          str[j * 2] = '.';
        }
        str[j * 2 + 1] = ' ';
      }
      str[64] = '\0';
      Serial.println(str);
    }
    Serial.println("_______________________________________________");
  }
private:
  std::array<std::array<Cell, HEIGHT>, WIDTH> grid;

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

  Particle(ELEMENT_ID element_id, ColourParameters colour_data, DIRECTION default_direction = DIRECTION::NONE)
    : element_id(element_id), colour_data(colour_data), default_direction(default_direction) {}

  virtual PARTICLETYPE getType() const {
    return PARTICLETYPE::PARTICLE;
  }

  virtual void move(Grid& grid, byte x, byte y) {
    // Default: do nothing
  }

  virtual void act(Grid& grid, byte x, byte y) {
    // Default: do nothing
  }

  virtual ~Particle() {}  // Virtual destructor for polymorphism
};

inline bool isUpdatable(Particle* particle) {
  PARTICLETYPE type = particle->getType();
  return (type == PARTICLETYPE::UPDATABLE || type == PARTICLETYPE::SOLID);
  // TODO implement all other materials
}

inline Particle* getElementDataFromElementID(ELEMENT_ID element_id) {
  Particle* element = ELEMENT[0];
  // Particle* element = ELEMENT[static_cast<uint8_t>(element_id)];
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
  const DIRECTION default_direction;

  Updatable(ELEMENT_ID element_id, ColourParameters colour_data, std::array<float, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, DIRECTION default_direction = DIRECTION::NONE)
    : Particle(element_id, colour_data), phase_change_temp(phase_change_temp), next_phase(next_phase), thermal_conductivity(thermal_conductivity), default_direction(default_direction) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::UPDATABLE;
  }

  virtual ~Updatable() {}  // Virtual destructor for polymorphism

  void act(Grid& grid, byte x, byte y) override {
    Cell& cell = grid.get(x, y);
    int current_temp = cell.temperature;

    Particle* particle = ELEMENT[0]; //ONCE ALL MATERIALS ARE ADDED FIX THIS
    // Particle* particle = ELEMENT[static_cast<byte>(cell.element_id)];
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

  Solid(ELEMENT_ID element_id, ColourParameters colour_data, std::array<float, 2> phase_change_temp, std::array<ELEMENT_ID, 2> next_phase, float thermal_conductivity, float density, DIRECTION default_direction = DIRECTION::True)
    : Updatable(element_id, colour_data, phase_change_temp, next_phase, thermal_conductivity, default_direction), density(density), default_direction(default_direction) {}

  virtual PARTICLETYPE getType() const override {
    return PARTICLETYPE::SOLID;
  }

  void move(Grid& grid, byte x, byte y) override {
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
  ColourParameters({ 0, 0 }, { 0, 0 }, { 0, 0 }, COLOURMODE::SOLID),
  { 0, 100 },                               // temperature range
  { ELEMENT_ID::METAL, ELEMENT_ID::LAVA },  // phase changes
  0.1f,                                     // thermal conductivity
  10.0f,                                    // density
  DIRECTION::True);

// Particle* ELEMENT[ELEMENT_COUNT] = {
//   stone,
// };

Particle* ELEMENT[ELEMENT_COUNT] = {
  stone,
};

float Grid::get_new_temperature_difference(byte x, byte y) {
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
                        byte fuel = 0,
                        byte life = 0,
                        byte branches = 0) {
  // Particle* element = ELEMENT[static_cast<byte>(id)]; // Access element from ELEMENT array
  // TEMP remove 0 and replace with ELEMENT_ID::
  Particle* element = ELEMENT[0];  // Access element from ELEMENT array
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


Grid grid;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Cell test = createCellInstance(ELEMENT_ID::STONE);
  grid.set(5, 5, test);    //This one is in the bottom corner
  // grid.set(20, 14, test);  //This one is in the top right corner
  // grid.set(32, 16, test); //This one is missing from the grid.print
  // grid.set(0, 0, test); //This one is missing from the grid.print
}

void loop() {
  // Serial.println("TEST");
  Serial.println("frame ---------------");
  // grid.perform_pixel_behaviour();
  
  Serial.println("attempting in_bounds ---------------");
  Serial.println(grid.in_bounds(0, 0));
  Serial.println(grid.in_bounds(31, 15));
  Serial.println(grid.in_bounds(32, 32));
  Serial.println("success ---------------");

  Serial.println("attempting get ---------------");
  grid.get(0, 0);
  Cell& cell = grid.get(31, 15);
  Serial.println("success ---------------");

  Serial.println("attempting get getElementDataFromCell ---------------");
  Particle* current_element = getElementDataFromCell(&cell);
  Serial.println("success ---------------");

  Serial.println("attempting get temp data ---------------");
  grid.get_pixel_temperatures();
  Serial.println("success ---------------");

  Serial.println("attempting particle act ---------------");
  current_element->act(grid, 5, 5);
  Serial.println("success ---------------");

  Serial.println("attempting get_moving true ---------------");
  grid.get_moving(true);
  Serial.println("success ---------------");

  Serial.println("attempting get_moving false ---------------");
  grid.get_moving(false);
  Serial.println("success ---------------");

  
  Serial.println("attempting moving ---------------");
  current_element->move(grid, 5, 5);
  Serial.println("success ---------------");


  // grid.swap(5, 0, 5, 5);
  grid.print();
  // put your main code here, to run repeatedly:
}
