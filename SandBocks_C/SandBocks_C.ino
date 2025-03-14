int width = 32;
int height = 16;
Cell grid[width][height];

class Grid {
  public:
    int width;
    int height;
    _ = ["Cell(air, temperature=23)" for _ in range(height)]
    self.grid = [_ for _ in range(width)]
};

class Cell {
  public:
    

    // python code
    // def __init__(self, element, state=None, flammable=False, temperature=23.0, fuel=0, life=0, branches=0):
    //     self.element = element
    //     self.colour_params = element.colour
    //     self.colour_base = self.get_base_colour()
    //     self.colour_mode = element.colour_mode
    //     self.colour = self.colour_base
    //     self.direction = element.default_direction
    //     # True for down, false for up
    //     self.state = state
    //     self.flammable = flammable
    //     self.temperature = temperature
    //     self.fuel = fuel
    //     self.life = life
    //     self.branches = branches
    //     self.clone_element=None

    // def update_colour_from_state_change(self):
    //     self.colour_params = self.element.colour
    //     self.colour_mode = self.element.colour_mode
    //     self.colour_base = self.get_base_colour()
    //     self.colour = self.colour_base
    //     # self.colour = self.get_base_colour()

    // # def update_colour(self):
    // #     # if colourMode['STATIC'] in self.colour_mode:
    // #     #     return
    // #     if colourMode['NOISE'] in self.colour_mode and random.random() > 0.95:
    // #         self.colour = self.get_base_colour()

    // def get_base_colour(self):
    //     if isinstance(self.colour_params[0], list):
    //         r = random.randint(self.colour_params[0][0], self.colour_params[0][1])
    //     else:
    //         r = self.colour_params[0]
    //     if isinstance(self.colour_params[1], list):
    //         g = random.randint(self.colour_params[1][0], self.colour_params[1][1])
    //     else:
    //         g = self.colour_params[1]
    //     if isinstance(self.colour_params[2], list):
    //         b = random.randint(self.colour_params[2][0], self.colour_params[2][1])
    //     else:
    //         b = self.colour_params[2]
    //     return (r, g, b)
    
    // def update_colour(self):
        
    //     if colourMode['NOISE'] in self.colour_mode and random.random() > 0.95:
    //         self.colour = self.get_base_colour()
    //     if colourMode['TEMPERATURE_DOWN'] in self.colour_mode:
    //         self.colour = self.temperature_down_colour_mode()
    //     if colourMode['BLACKBODY'] in self.colour_mode:
    //         self.colour = self.blackbody_colour_mode()
    //     if self.state is not None:
    //         if hasattr(self.state, "colour"):
    //             self.colour = self.state.colour(cell = self)


    // def temperature_down_colour_mode(self):
    //     if self.element.next_phase[0] is not None and self.element.next_phase[1] is not None:
    //         ratio =  (self.temperature - self.element.phase_change_temp[0]) / (0.5 * self.element.phase_change_temp[1] - self.element.phase_change_temp[0])
    //     elif self.element.next_phase[0] is not None:
    //         ratio = 1.5 - (self.element.phase_change_temp[0] / self.temperature)
    //     else:
    //         ratio = max(0.5, self.temperature / 200)

    //     r = self.colour_base[0]
    //     g = self.colour_base[1]
    //     b = self.colour_base[2]


    //     r = int(min(max((r * ratio), 0), 254))
    //     g = int(min(max((g * ratio), 0), 254))
    //     b = int(min(max((b * ratio), 0), 254))

    //     return (r, g, b)


    // def blackbody_colour_mode(self):
    //     blackbody_constant = 30
    //     r = self.colour_base[0]
    //     g = self.colour_base[1]
    //     b = self.colour_base[2]

    //     # colours = self.get_base_colour()
    //     # r = colours[0]
    //     # g = colours[1]
    //     # b = colours[2]

    //     r = int(min(max(r + (2 *(self.temperature) / (blackbody_constant)), 0), 254))
    //     g = int(min(max(g + (0.5 * (self.temperature) / (blackbody_constant)), 0), 254))
    //     b = int(min(max(b + ((self.temperature) / (blackbody_constant)), 0), 254))

    //     return (r, g, b)
}

void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}
