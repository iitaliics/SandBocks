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

enum ELEMENT {
  // AIR,
  
  // SAND,
  
  // WATER,
  // ICE,
  // STEAM,
  // SNOW,
  
  // SMOKE,
  // FIRE,

  // OIL,

  // LAVA,
  // STONE,

  // WOOD,

  // MOLTEN_METAL,
  // METAL,

  // BLOCK,

  // // Special
  // DESTRUCT,
  // GROW,
  // CLONE,
  // COLD,
  // HOT,
  // CONDUCT,
  // FLIP,
  // BUG
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
