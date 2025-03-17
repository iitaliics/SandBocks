#include <array>

int width = 32;
int height = 16;
//Cell grid[width][height];

class Grid {
public:
  int width;
  int height;
  //    _ = ["Cell(air, temperature=23)" for _ in range(height)]
  //    self.grid = [_ for _ in range(width)]
};

struct coordinate_return {
  byte x;
  byte y;
}

struct grid_return {
  byte x;
  byte y;
  Cell cell;
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
  byte param_red[2];
  byte param_green[2];
  byte param_blue[2];

  byte base_red;
  byte base_green;
  byte base_blue;

  byte display_red;
  byte display_green;
  byte display_blue;

  COLOURMODE colour_mode;

  void set_base_colour(int param_r[2], int param_g[2], int param_b[2]) {
    if (param_r[1] == 0) {
      base_red = param_r[0];
    } else {
      base_red = random(param_r[0], param_r[1]);
    }
    if (param_g[1] == 0) {
      base_green = param_g[0];
    } else {
      base_green = random(param_g[0], param_g[1]);
    }
    if (param_b[1] == 0) {
      base_blue = param_b[0];
    } else {
      base_blue = random(param_b[0], param_b[1]);
    }
  };
};

class Particle {
public:
  String name;
  Colour colour;
  COLOURMODE colour_mode;
  DIRECTION direction;

  Particle(String name, Colour colour, COLOURMODE colour_mode = COLOURMODE::SOLID, DIRECTION direction = DIRECTION::NONE)
    : name(name), colour(colour), colour_mode(colour_mode), direction(direction) {} //AI told me to do this :(
};

class Updatable : public Particle {
public:
  int phase_change_temp[2];
  Particle next_phase[2];
  float thermal_conductivity;
  // does name, colour and colour_mode need to be here if it's used 3 lines below?

  Updatable(String name, Colour colour, COLOURMODE colour_mode, int phase_change_temp[2], Particle next_phase[2], float thermal_conductivity) {
    : Particle(name, colour, colour_mode)  {
      this->phase_change_temp[0] = phase_change_temp[0];
      this->phase_change_temp[1] = phase_change_temp[1];
      this->next_phase[0] = next_phase[0];
      this->next_phase[1] = next_phase[1];
    }
  }

  do(Grid grid, byte x, byte y) {
    // Cell cell = grid.get(x, y);
    // int current_temp = cell.temperature;
    if (cell.element.next_phase[0] != ELEMENT_ID::NONE && current_temp < cell.element.phase_change_temp[0]) {
      cell.element = ELEMENT[ELEMENT_ID[cell.element.next_phase[0]]];
      cell.direction = cell.element.default_direction;
      cell.update_colour_from_state_change()
      return
    } else if (cell.element.next_phase[1] != ELEMENT_ID::NONE && current_temp > cell.element.phase_change_temp[1]) {
      cell.element = ELEMENT[ELEMENT_ID[cell.element.next_phase[1]]];
      cell.direction = cell.element.default_direction;
      cell.update_colour_from_state_change()
      return
    }

    if (cell.element.next_phase[1] == ELEMENT_ID::NONE && cell.flammable && current_temp > cell.element.phase_change_temp[1]) {
      cell.state = STATE::BURNING;
    }
            
  }
};

class Solid : public Updatable {
  public:
  float density;
  DIRECTION default_direction;

  Solid(String name, Colour colour, COLOURMODE colour_mode, int phase_change_temp[2], Particle next_phase[2], float thermal_conductivity, float density, DIRECTION default_direction) {
    : Updatable(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity) {
      this->density = density;
      this->default_direction = default_direction;
    }
  };

  move(Grid grid, byte x, byte y) {
    coordinate_return gaps[2] = grid.check_unimpeded(x, y);
    // TODO check unimpeded when the direct below space is free will only return an array of length 1. But now we have to explicitly set it to 3

    // Get length of valid entries
    byte length = 0;
    for (auto i: gaps) {
      if (gaps[i][0] == NULL) {
        break;
      }
      length ++;
    }

    if (length == 1) {
      if (grid.can_swap_after_random(x, y, gaps[0][0], gaps[0][1])) {
        grid.swap(x, y, gaps[0][0], gaps[0][1])
      }
    }
    else if (length == 2) {
      byte index = random(length);
      if (grid.can_swap_after_random(x, y, gaps[index][0], gaps[index][1])) {
        grid.swap(x, y, gaps[index][0], gaps[index][1])
      }
    }
  }
}

class Liquid : public Updatable {
  public:
  float density;
  DIRECTION default_direction;

  Liquid(String name, Colour colour, COLOURMODE colour_mode, int phase_change_temp[2], Particle next_phase[2], float thermal_conductivity, float density, DIRECTION default_direction) {
    : Updatable(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity) {
      this->density = density;
      this->default_direction = default_direction;
    }
  };

  move(Grid grid, byte x, byte y) {
    coordinate_return gaps[3] = grid.check_unimpeded(x, y);
    // TODO check unimpeded when the direct below space is free will only return an array of length 1. But now we have to explicitly set it to 3
  }
}

class Gas : public Updatable {
  public:
  float density;
  DIRECTION default_direction;

  Gas(String name, Colour colour, COLOURMODE colour_mode, int phase_change_temp[2], Particle next_phase[2], float thermal_conductivity, float density, DIRECTION default_direction) {
    : Updatable(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity) {
      this->density = density;
      this->default_direction = default_direction;
    }
  };

  move(Grid grid, byte x, byte y) {
    coordinate_return gaps[3] = grid.check_unimpeded(x, y);
    // TODO check unimpeded when the direct below space is free will only return an array of length 1. But now we have to explicitly set it to 3
  }
}

// Special Material Classes:


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

  NONE
}

Particle* ELEMENT[] = {
  Gas(ELEMENT_ID::AIR, {{0, 0}, {0, 0}, {0, 0}}, {COLOURMODE::SOLID}, {NULL, NULL}, {ELEMENT_ID::NONE, ELEMENT_ID::NONE})
// Gas("Air", (0, 0, 0), (colourMode["BLACKBODY"],), [None, None], [None, None], 0.026, 1.23, False),
// Solid(, ([200, 250], [125, 200], [75, 100]), (colourMode['STATIC'], colourMode['BLACKBODY']), [None, None], [None, None], 0.4, 1602, True)

// water = Liquid("Water", (0, 0, 255), (colourMode["TEMPERATURE_DOWN"],), [0, 100], [None, None], 0.6, 997, True)
// ice = Updatable("Ice", ([55, 85], [55, 85], 255), (colourMode["STATIC"],), [None, 0], [None, water], 2.22)
// steam = Gas("Steam", (200, 200, 200), (colourMode["TEMPERATURE_DOWN"],), [100, None], [water, None], 0.0184, 0.6, False)
// snow = Solid("Snow", ([240, 254], [240, 254], [240, 254]), (colourMode["STATIC"], ), [None, 0], [None, water], 0.05, 70, True)
// water.next_phase = [ice, steam]
// ice.next_phase = [None, water]
// steam.next_phase = [water, None]

// smoke = Gas("Smoke", (254, 254, 254), (colourMode["STATIC"],), [None, 300], [None, None], 0.6, 0.6, False)
// fire = Gas("Fire", ([200, 254], [0, 50], [0, 50]), (colourMode["NOISE"],), [300, None], [smoke, None], 0.6, 0.6, False)
// smoke.next_phase=[None, fire]

// oil = Liquid("Oil", (50, 25, 25), (colourMode["SOLID"],), [None, 300], [None, None], 0.6, 800, True)

// lava = Liquid("Lava", (254, [150, 250], [150, 250]), (colourMode['NOISE'],), [1160, None], [None, None], 2, 2400, True)
// stone = Solid("Stone", (200, 200, [190, 210]), (colourMode['STATIC'], colourMode['BLACKBODY']), [None, 1160], [None, lava], 2.5, 2500, True)
// lava.next_phase = [stone, None]
// stone.next_phase = [None, lava]

// wood = Updatable("Wood", ([50, 100], [25, 55], 0), (colourMode['STATIC'],), [None, 250], [None, None], 0.15)

// molten_metal = Liquid("Molten Metal", (125, 75, 85), (colourMode['BLACKBODY'],), [1538, None], [None, None], 2.5, 7800, True)
// metal = Updatable("Metal", (75, 75, 85), (colourMode['BLACKBODY'],), [None, 1538], [None, molten_metal], 2.5)
// molten_metal.next_phase = [metal, None]
// metal.next_phase = [None, molten_metal]

// block = Particle("Block", (100, 100, 100), (colourMode["SOLID"], ))

// # Special
// destruct = Destruct("Destruct", ([25, 55], [25, 55], [25, 55]), (colourMode["NOISE"],),)
// grow = Grow("Grow", (0, [150, 250], 10), (colourMode["STATIC"],), [None, 300], [None, None], 0.6)
// clone = Clone("Clone", (150, 150, 0), (colourMode['SOLID'], ))
// cold = Temperature("Cold", ([50, 75], [50, 75], [100, 254]), (colourMode['NOISE'],), [None, None], [None, None], 0.05, -10000)
// hot = Temperature("Hot", ([100, 254], [50, 75], [50, 75]), (colourMode['NOISE'],), [None, None], [None, None], 0.05, 10000)
// conduct = Updatable("Conduct", (100, 200, 100), (colourMode["BLACKBODY"], colourMode["TEMPERATURE_DOWN"],), [None, None], [None, None], 5)
// flip = Flip("Flip", (25, 240, 0), (colourMode['SOLID'], ))
};

enum STATE_ID {
  NONE,
  BURNING,
  DECAY
}

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
