
import random
import pygame
import math

""" TODO:
    Catching things on fire
    Fire burning fuel
    Fire emitting smoke
    Rest of the elements
    Teleport
    Flip
    Barriers
    TempConduct clone
"""

# 1 to 1 size with LED grid
pixelWidth = 44
# pixel size
# resolution = [32, 16]
# pixel count
# guiSize = 30
# in screen pixels pixelWidth = 20
resolution = [15, 15]
# pixel count
# resolution = [5, 5]
guiSize = 30
# in screen pixels


class Grid:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        # pixel = Cell(air, direction=None, temperature=23)
        self.grid = [[Cell(air, temperature=23) for _ in range(height)] for _ in range(width)]

    def print(self):
        # transpose = [["#" if self.grid[j][i].element.name == fire.name else " " for j in range(len(self.grid))] for i in range(len(self.grid[0]))]
        # transpose = [[self.grid[j][i]['element'].name for j in range(len(self.grid))] for i in range(len(self.grid[0]))]
        transpose = [[self.grid[j][i].element.name for j in range(len(self.grid))] for i in range(len(self.grid[0]))]
        reverse = [transpose[len(transpose) - i - 1] for i in range(len(transpose))]
        for entry in reverse:
            print(entry)
        print("_______________________________________________")
        return reverse

    def get(self, x, y):
        return self.grid[x][y]

    def in_bounds(self, x, y):
        return 0 <= x < self.width and 0 <= y < self.height

    def draw(self, x, y, element):
        if self.get(x, y).element == air and random.random() > 0.1:
            self.set(x, y, element)

    def set(self, x, y, element):
        self.grid[x][y] = element

    def swap(self, x1, y1, x2, y2):
        element2 = self.get(x2, y2)
        if isinstance(element2.element, Updatable):
            self.grid[x1][y1], self.grid[x2][y2] = self.grid[x2][y2], self.grid[x1][y1]

    def is_empty(self, x, y):
        return self.grid[x][y].element.name == air.name

    def can_swap(self, x1, y1, x2, y2):
        element1 = self.get(x1, y1)
        element2 = self.get(x2, y2)
        if isinstance(element2.element, Updatable) and hasattr(element2.element, "density") and element2.direction is not None:
            if element1.direction == True and element1.element.density > element2.element.density:
                return True
            if element1.direction == False and element1.element.density < element2.element.density:
                return True
        return False

    def check_surroundings(self, x, y):
        delta = [-1, 0, 1]
        element_list = []
        for delX in delta:
            for delY in delta:
                if delX == 0 and delY == 0:
                    continue
                if self.in_bounds(x + delX, y + delY):
                    element_list.append((x + delX, y + delY, self.get(x + delX, y + delY)))
        return element_list

    def check_unimpeded(self, x, y):
        direction = (-1 if self.get(x, y).direction else 1)
        if self.in_bounds(x, y + direction) and self.can_swap(x, y, x, y + direction):
            return [(x, y + direction)]
        delta = [-1, 1]
        gap_list = []
        for delX in delta:
            if self.in_bounds(x + delX, y + direction):
                if self.can_swap(x, y, x + delX, y + direction):
                    gap_list.append((x + delX, y + direction))
        return gap_list

    def check_side_gap_distance(self, x, y):
        direction = (-1 if self.get(x, y).direction else 1)
        # 2 passes, one for level in direction of direction, one for current level to make sure it doesn't try to go through walls
        if y + direction >= grid.height or y + direction < 0:
            return [0, 0]
        # left side
        left = 0
        # 0 means it cannot move in that direction
        current_layer_left_gap = False
        if not x == 0:
            for i in range(0, x):
                next_layer_left_gap = self.can_swap(x, y, x - (i + 1), y + direction)
                current_layer_left_gap = self.can_swap(x, y, x - (i + 1), y)
                if next_layer_left_gap and current_layer_left_gap:
                    left = (i + 1)
                    break
                if not current_layer_left_gap:
                    break
        if left == 0 and current_layer_left_gap:
            left = grid.width
        # right side
        right = 0
        # 0 means it cannot move in that direction
        current_layer_right_gap = False
        if not x == self.width - 1:
            for i in range(1, self.width - x):
                next_layer_right_gap = self.can_swap(x, y, x + i, y + direction)
                current_layer_right_gap = self.can_swap(x, y, x + i, y)
                if next_layer_right_gap and current_layer_right_gap:
                    right = i
                    break
                if not current_layer_left_gap:
                    break
        if right == 0 and current_layer_right_gap:
            right = grid.width
        return [left, right]

    def can_swap_after_random(self, x1, y1, x2, y2):
        """ Assumes the check has already been made to see if these can swap """
        pixel1 = grid.get(x1, y1)
        pixel2 = grid.get(x2, y2)
        return random.random() < (pixel1.element.density / (pixel1.element.density + pixel2.element.density))

    def update_grid(self):
        temps = self.get_pixel_temperatures()
        self.perform_pixel_behaviour(temperature_data=temps)
        return self

    def get_pixel_temperatures(self):
        changes = []
        for row in range(grid.height):
            for column in range(grid.width):
                changes.append((column, row, self.get_new_temperature_difference(column, row)))
        return changes

    def get_new_temperature_difference(self, x, y):
        current = self.get(x, y)
        if not isinstance(current.element, Updatable):
            return 0
        neighbours = self.check_surroundings(x, y)
        # proportion = len(neighbours) / 8
        # 0 to 1 scaling depending on how many pixels are in contact
        sum_influence = 0
        updatable_neighbour_count = 0
        for neighbour in neighbours:
            particle = neighbour[2]
            if isinstance(particle.element, Updatable):
                updatable_neighbour_count += 1
                # sum_influence += (particle.temperature - current.temperature) * (particle.element.thermal_conductivity)
                sum_influence += (particle.temperature - current.temperature) * ((current.element.thermal_conductivity * particle.element.thermal_conductivity) / (current.element.thermal_conductivity + particle.element.thermal_conductivity))
        return sum_influence / updatable_neighbour_count if updatable_neighbour_count > 0 else 0 


    def get_upmoving(self):
        changes = []
        for row in range(grid.height):
            if random.random() < 0.5:
                for column in range(grid.width):
                    pixel = grid.get(column, grid.height - 1 - row)
                    if pixel.direction is not None and not pixel.direction:
                        changes.append((column, grid.height - 1 - row, pixel))
            else:
                for column in range(grid.width):
                    pixel = grid.get(grid.width - 1 - column, grid.height - 1 - row)
                    if pixel.direction is not None and not pixel.direction:
                        changes.append((grid.width - 1 - column, grid.height - 1 - row, pixel))
        return changes

    def get_downmoving(self):
        changes = []
        for row in range(grid.height):
            if random.random() < 0.5:
                for column in range(grid.width):
                    pixel = grid.get(column, row)
                    if pixel.direction:
                        changes.append((column, row, pixel))
            else:
                for column in range(grid.width):
                    pixel = grid.get(grid.width - 1 - column, row)
                    if pixel.direction:
                        changes.append((grid.width - 1 - column, row, pixel))
        return changes

    def get_static(self):
        changes = []
        for row in range(grid.height):
            if random.random() < 0.5:
                for column in range(grid.width):
                    pixel = grid.get(column, row)
                    if pixel.direction is None:
                        changes.append((column, row, pixel))
            else:
                for column in range(grid.width):
                    pixel = grid.get(grid.width - 1 - column, row)
                    if pixel.direction is None:
                        changes.append((grid.width - 1 - column, row, pixel))
        return changes

    def perform_pixel_behaviour(self, temperature_data=None):
        for entry in temperature_data:
            # print(self.get(entry[0], entry[1]).__dict__, (entry[0], entry[1]))
            self.grid[entry[0]][entry[1]].temperature += entry[2]
            # print(self.get(entry[0], entry[1]).__dict__, (entry[0], entry[1]), entry[2])
        static_update = self.get_static()
        for column, row, current in static_update:
            if not current.state == None:
                if hasattr(current.state, 'do'):
                    current.state.do(self, column, row)
            #
            if not temperature_data == None:
                # current.temperature = temperature_data[column][row]
                if hasattr(current.element, 'do'):
                    # includes updating the colour, material state, performing special behaviours and updating internal attributes such as decay or fuel
                    current.element.do(self, column, row)
                    current.update_colour()

        up_update = self.get_upmoving()
        for column, row, current in up_update:
            if not current.state == None:
                if hasattr(current.state, 'do'):
                    current.state.do(self, column, row)
            #
            if not temperature_data == None:
                # current.temperature = temperature_data[column][row]
                if hasattr(current.element, 'do'):
                    # includes updating the colour, material state, performing special behaviours and updating internal attributes such as decay or fuel
                    current.element.do(self, column, row)
                    current.update_colour()

                if hasattr(current.element, 'move'):
                    # includes moving the pixel in the direction specified in the class
                    current.element.move(self, column, row)

        down_update = self.get_downmoving()
        for column, row, current in down_update:
            if not current.state == None:
                if hasattr(current.state, 'do'):
                    current.state.do(self, column, row)
            #
            if not temperature_data == None:
                # current.temperature = temperature_data[column][row]
                if hasattr(current.element, 'do'):
                    # includes updating the colour, material state, performing special behaviours and updating internal attributes such as decay or fuel
                    current.element.do(self, column, row)
                    current.update_colour()

                if hasattr(current.element, 'move'):
                    # includes moving the pixel in the direction specified in the class
                    current.element.move(self, column, row)

colourMode = {"SOLID": 0, "STATIC": 1, "NOISE": 2, "TEMPERATURE": 3, "RAMP_UP": 4, "RAMP_DOWN": 5, }

class Cell:
    def __init__(self, element, state=None, flammable=False, temperature=23.0, fuel=0, life=0, branches=0):
        self.element = element
        self.colour_params = element.colour
        self.colour_base = self.get_base_colour()
        self.colour_mode = element.colour_mode
        self.colour = self.colour_base
        self.direction = element.default_direction
        # True for down, false for up
        self.state = state
        self.flammable = flammable
        self.temperature = temperature
        self.fuel = fuel
        self.life = life
        self.branches = branches
        self.clone_element=None

    def update_colour_from_state_change(self):
        self.colour_params = self.element.colour
        self.colour_mode = self.element.colour_mode
        self.colour_base = self.get_base_colour()
        self.colour = self.get_base_colour()

    # def update_colour(self):
    #     # if colourMode['STATIC'] in self.colour_mode:
    #     #     return
    #     if colourMode['NOISE'] in self.colour_mode and random.random() > 0.95:
    #         self.colour = self.get_base_colour()

    def get_base_colour(self):
        if isinstance(self.colour_params[0], list):
            r = random.randint(self.colour_params[0][0], self.colour_params[0][1])
        else:
            r = self.colour_params[0]
        if isinstance(self.colour_params[1], list):
            g = random.randint(self.colour_params[1][0], self.colour_params[1][1])
        else:
            g = self.colour_params[1]
        if isinstance(self.colour_params[2], list):
            b = random.randint(self.colour_params[2][0], self.colour_params[2][1])
        else:
            b = self.colour_params[2]
        return (r, g, b)
    
    def update_colour(self):
        
        if colourMode['NOISE'] in self.colour_mode and random.random() > 0.95:
            self.colour = self.get_base_colour()
        if colourMode['TEMPERATURE'] in self.colour_mode:
            self.colour = self.temperature_colour_mode()
        if self.state is not None:
            if hasattr(self.state, "colour"):
                self.colour = self.state.colour(cell = self)

    def temperature_colour_mode(self):
        blackbody_constant = 30
        r = self.colour_base[0]
        g = self.colour_base[1]
        b = self.colour_base[2]

        # colours = self.get_base_colour()
        # r = colours[0]
        # g = colours[1]
        # b = colours[2]

        r = min(max(r + (2 *(self.temperature) / (blackbody_constant)), 0), 254)
        g = min(max(g + (0.5 * (self.temperature) / (blackbody_constant)), 0), 254)
        b = min(max(b + ((self.temperature) / (blackbody_constant)), 0), 254)

        return (r, g, b)

class Particle:
    def __init__(self, name, colour, colour_mode):
        self.name = name
        self.colour = colour
        self.colour_mode = colour_mode

class Static:
    def __init__(self, name):
        self.name = name
 
class Updatable(Particle):
    def __init__(self, name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity):
        super().__init__(name, colour, colour_mode)
        self.phase_change_temp = phase_change_temp
        self.next_phase = next_phase
        self.thermal_conductivity = thermal_conductivity
        self.default_direction = None

    def do(self, grid, x, y):
        cell = grid.get(x, y)
        current_temp = cell.temperature
        if cell.element.next_phase[0] is not None and current_temp < cell.element.phase_change_temp[0]:
            cell.element = cell.element.next_phase[0]
            cell.direction = cell.element.default_direction
            cell.update_colour_from_state_change()

        elif cell.element.next_phase[1] is not None and current_temp > cell.element.phase_change_temp[1]:
            cell.element = cell.element.next_phase[1]
            cell.direction = cell.element.default_direction
            cell.update_colour_from_state_change()

class Solid(Updatable):
    def __init__(self, name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity, density, default_direction):
        super().__init__(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity)
        self.density = density
        self.default_direction = default_direction

    def do(self, grid, x, y):
        super().do(grid, x, y)
        # self.move(grid, x, y)

    def move(self, grid, x, y):
        gaps = grid.check_unimpeded(x, y)
        if len(gaps):
            i, j = random.choice(gaps)
            if grid.can_swap_after_random(x, y, i, j):
                grid.swap(x, y, i, j)

class Liquid(Updatable):
    def __init__(self, name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity, density, default_direction):
        super().__init__(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity)
        self.density = density
        self.default_direction = default_direction

    def do(self, grid, x, y):
        super().do(grid, x, y)
        # self.move(grid, x, y)

    def move(self, grid, x, y):
        gaps = grid.check_unimpeded(x, y)
        if len(gaps):
            i, j = random.choice(gaps)
            if grid.can_swap_after_random(x, y, i, j):
                grid.swap(x, y, i, j)
        gap_distances = grid.check_side_gap_distance(x, y)
        x2 = 0
        if gap_distances[0] > 0 and gap_distances[1] > 0 and gap_distances[0] == gap_distances[1]:
            x2 = x + random.randint(-1, 1)
        elif gap_distances[0] > 0 and (gap_distances[0] < gap_distances[1] or gap_distances[1] == 0):
            x2 = x + random.randint(-1, 0)
        elif gap_distances[1] > 0 and (gap_distances[1] < gap_distances[0] or gap_distances[0] == 0):
            x2 = x + random.randint(0, 1)
        if not x2 == 0 and grid.can_swap(x, y, x2, y) and grid.can_swap_after_random(x, y, x2, y):
            grid.swap(x, y, x2, y)

class Gas(Updatable):
    def __init__(self, name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity, density, default_direction):
        super().__init__(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity)
        self.density = density
        self.default_direction = default_direction

    def do(self, grid, x, y):
        super().do(grid, x, y)
        # self.move(grid, x, y)

    def move(self, grid, x, y):
        gaps = grid.check_unimpeded(x, y)
        if len(gaps):
            i, j = random.choice(gaps)
            if grid.can_swap_after_random(x, y, i, j):
                grid.swap(x, y, i, j)
        gap_distances = grid.check_side_gap_distance(x, y)
        x2 = 0
        if gap_distances[0] > 0 and gap_distances[1] > 0 and gap_distances[0] == gap_distances[1]:
            x2 = x + random.randint(-1, 1)
        elif gap_distances[0] > 0 and (gap_distances[0] < gap_distances[1] or gap_distances[1] == 0):
            x2 = x + random.randint(-1, 0)
        elif gap_distances[1] > 0 and (gap_distances[1] < gap_distances[0] or gap_distances[0] == 0):
            x2 = x + random.randint(0, 1)
        if not x2 == 0 and grid.can_swap(x, y, x2, y) and grid.can_swap_after_random(x, y, x2, y):
            grid.swap(x, y, x2, y)

# special materials
class Destruct(Particle):
    def __init__(self, name, colour, colour_mode):
        super().__init__(name, colour, colour_mode)
        self.default_direction = None

    def do(self, grid, x, y):
        contact = grid.check_surroundings(x, y)
        relevantContact = [element for element in contact if not element[2].element == air]
        if not len(relevantContact):
            return
        chosenOne = random.choice(relevantContact)
        grid.set(chosenOne[0], chosenOne[1], Cell(air))

class Grow(Updatable):
    def __init__(self, name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity):
        super().__init__(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity)
        self.default_direction = None

    def do(self, grid, x, y):
        cell = grid.get(x, y)
        if cell.life > 0:
            contact = grid.check_surroundings(x, y)
            relevantContact = [element for element in contact if (element[2].element.name == air.name) or element[2].element.name == water.name]
            if not len(relevantContact):
                return
            chosenOnes = random.choices(relevantContact, k=2)
            if random.random() > 0.95 and len(chosenOnes) > 1 and cell.branches > 0 and cell.life > 0:
                cell.branches -= 1
                grid.draw(chosenOnes[1][0], chosenOnes[1][1], Cell(grow, life=cell.life - 10, branches=cell.branches, flammable=True, fuel = 100))
                grid.draw(chosenOnes[0][0], chosenOnes[0][1], Cell(grow, life=cell.life - 10, branches=cell.branches, flammable=True, fuel = 100))
                cell.life = 0

            elif random.random() > 0.9 and len(chosenOnes) > 0 and cell.life > 0:
                grid.draw(chosenOnes[0][0], chosenOnes[0][1], Cell(grow, life=cell.life - 10, branches=cell.branches, flammable=True, fuel = 100))
                cell.life = 0

class Burning(Static):
    def __init__(self, name, threshold):
        super().__init__(name)
        self.threshold = threshold

    def do(self, grid, x, y):
        cell = grid.get(x, y)
        contact = grid.check_surroundings(x, y)
        relevantFlammableContact = [element for element in contact if element[2].flammable]
        relevantFreeContact = [element for element in contact if element[2].element == air]
        for neighbour in relevantFlammableContact:
            if random.random() > 0.9:
                grid.grid[neighbour[0]][neighbour[1]].state = burning
        
        # smoke responsibility
        if len(relevantFreeContact):
            chosenOne = random.choice(relevantFreeContact)
            if random.random() > 0.95:
                chosenOne[2].state = decay
                chosenOne[2].element = smoke
                chosenOne[2].colour_base = smoke.colour
                chosenOne[2].update_colour_from_state_change()
                chosenOne[2].fuel = max(cell.fuel / 2, 1)

        if cell.temperature < self.threshold:
            cell.state = decay
            cell.element = smoke
            cell.colour_base = smoke.colour
            cell.update_colour_from_state_change()
            
            # cell.colour_base = (254, 254, 254)
            
        cell.fuel -= 1
        if cell.fuel <= 0:
            cell.element = air
            cell.state = None
            cell.update_colour_from_state_change()

    def colour(self, cell):
        if random.random() > 0.9:
            r = random.randint(150, 200)
            g = random.randint(100, 200)
            b = random.randint(50, 150)
            return (r, g, b)
        return cell.colour

class Decay(Static):
    def __init__(self, name):
        super().__init__(name)

    def do(self, grid, x, y):
        cell = grid.get(x, y)
        cell.fuel -= 1

        if cell.fuel <= 0:
            cell.element = air
            cell.state = None
            cell.update_colour_from_state_change()
        
    def colour(self, cell):
        r = cell.colour_base[0]
        g = cell.colour_base[1]
        b = cell.colour_base[2]

        return (int(r * cell.fuel / 200), int(g * cell.fuel / 200), int(b * cell.fuel / 200))
        

class Clone(Particle):
    def __init__(self, name, colour, colour_mode):
        super().__init__(name, colour, colour_mode)
        self.clone_element = None
        self.default_direction = None

    def do(self, grid, x, y):
        pixel = grid.get(x, y)
        if pixel.clone_element is None:
            
            contact = grid.check_surroundings(x, y)
            relevantContact = [element for element in contact if not element[2].element == air ]
            if not len(relevantContact):
                        return
            pixel.clone_element = random.choice(relevantContact)
        
        else:
            contact = grid.check_surroundings(x, y)
            relevantContact = [element for element in contact if element[2].element == air]
            if not len(relevantContact):
                        return
            chosenOne = random.choice(relevantContact)
            grid.draw(chosenOne[0], chosenOne[1], Cell(pixel.clone_element[2].element, pixel.clone_element[2].state, pixel.clone_element[2].flammable, pixel.clone_element[2].temperature, pixel.clone_element[2].fuel, pixel.clone_element[2].life, pixel.clone_element[2].branches))
            

# Materials
air = Gas("Air", (0, 0, 0), (colourMode["STATIC"],), [None, None], [None, None], 0.026, 1.23, False)
sand = Solid("Sand", ([200, 250], [125, 200], [75, 100]), (colourMode['STATIC'], colourMode['TEMPERATURE']), [None, None], [None, None], 0.4, 1602, True)
water = Liquid("Water", (0, 0, 255), (colourMode["TEMPERATURE"],), [0, 100], [None, None], 0.6, 997, True)
ice = Updatable("Ice", (100, 100, 255), (colourMode["STATIC"],), [None, 0], [None, water], 2.22)
steam = Gas("Steam", (50, 50, 50), (colourMode["TEMPERATURE"],), [100, None], [water, None], 0.0184, 0.6, False)
water.next_phase = [ice, steam]
smoke = Gas("Smoke", (254, 254, 254), (colourMode["STATIC"],), [None, 149], [None, None], 0.6, 0.6, False)
fire = Gas("Fire", ([200, 254], [0, 50], [0, 50]), (colourMode["NOISE"],), [149, None], [smoke, None], 0.6, 0.6, False)
smoke.next_phase=[None, fire]
oil = Liquid("Oil", (50, 25, 25), (colourMode["STATIC"],), [100, None], [None, None], 0.6, 800, True)
lava = Liquid("Lava", (254, [150, 250], [150, 250]), (colourMode['NOISE'],), [None, None], [None, None], 0.1, 900, True)
destruct = Destruct("Destruct", (0, 250, 10), (colourMode["NOISE"],),)
grow = Grow("Grow", (0, [150, 250], 10), (colourMode["STATIC"],), [None, None], [None, None], 0.6)
burning = Burning("Burning", threshold=149)
decay = Decay("Decay")
clone = Clone("Clone", (150, 150, 0), (colourMode['SOLID'], ))
block = Particle("Block", (100, 100, 100), (colourMode['STATIC'],))

def debug(grid):
    tally = 0
    total = 0
    for row in grid.grid:
    
        for column in row:
            
            if column.element == smoke or column.element == fire:
                # if column.state == burning:
                    # tally += 1
                print(column.state.name, column.fuel, column.temperature)
            # total += column.state
    print(tally)
    # print(total / tally)
    

def display(grid):
    for y in range(grid.height):
        # pygame.draw.line(screen, (40, 40, 40), [0, y*pixelWidth], [grid.width*pixelWidth, y*pixelWidth], 5)
        for x in range(grid.width):
            #
            # if y == 0:
                # pygame.draw.line(screen, (40, 40, 40), [x*pixelWidth, 0], [x*pixelWidth, grid.height*pixelWidth], 5)
            material = grid.get(x, grid.height - 1 - y)
            # print(material.__dict__)

            try:
                pygame.draw.rect(screen, material.colour, [x * pixelWidth, y * pixelWidth, pixelWidth, pixelWidth])
            except:
                print(grid.get(x, grid.height - 1 - y).__dict__)

pygame.init()
screen = pygame.display.set_mode(((resolution[0] * pixelWidth), (resolution[1] * pixelWidth + guiSize)))
# Loop until the user clicks the close button.
done = False
clock = pygame.time.Clock()
grid = Grid(resolution[0], resolution[1])
    # grid.set(10, 4, water)
    #
# for x in range(15):
    
        # grid.set(x, 3, Cell(ice, temperature=-10))
        # grid.set(x, 11, Cell(water, temperature=20))
        # grid.set(x, 4, Cell(ice, temperature=0))
        # grid.set(x, 3, Cell(ice, temperature=-14))
        # grid.set(x, 2, Cell(ice, temperature=-14))
        # grid.set(x, 1, Cell(ice, temperature=-14))
        # grid.set(x, 0, Cell(ice, temperature=-14))
        # grid.set(x, 4, water)
        # if x > 2 else None
        # grid.set(x, 3, water)
        # if x > 4 else None
        # grid.set(x, 2, water)
        # if x > 4 else None
        # grid.set(x, 1, water)
        # if x > 2 else None
        # grid.set(5+x, 5, Cell(block))
        # grid.set(x, 1, water)
        # grid.set(2, 3, steam)
        # grid.set(3, 3, steam)
        # grid.set(2, 0, Cell(destruct))
        # grid.set(2, 0, Cell(fire, state=burning, fuel=20, temperature=600))
        # grid.set(2, 2, Cell(grow, temperature=23, fuel=100, life=100, branches=3))
grid.print()
# grid.set(3, 14, Cell(water, direction=True, temperature=90))
# grid.set(0, 0, Cell(ice, temperature=23))
# grid.set(1, 0, Cell(ice, temperature=-14))
# grid.draw(6, 9, Cell(water, temperature=23))


while not done:
    # print("--------------------------")
    
    # debug(grid)

    grid.draw(5, 1, Cell(fire, burning, False, 300, 100, 0, 0))
                # pass
    # grid.draw(7, 11, Cell(destruct))

    grid.draw(7, 6, Cell(grow,  life = 100, branches = 1, flammable=True, fuel=100))

    # grid.draw(7, 7, Cell(burning))

    grid.draw(7, 5, Cell(clone))


    grid.set(0, 14, Cell(ice, temperature=-24))
        
    # grid.draw(5, 5, Cell(ice, temperature=200))
    # grid.draw(6, 5, Cell(ice, temperature=90))
    # grid.draw(7, 5, Cell(ice, temperature=100))
    # grid.draw(1, 14, Cell(sand, temperature=40))
    
    # grid.draw(13, 12, Cell(lava, temperature=900))
    # grid.print()
    # grid.draw(2, 14, Cell(water, temperature=23))
    # grid.set(1, 3, Cell(ice, direction=None, temperature=-1))
    # grid.set(5, 1, Cell(fire, state=burning, fuel=20, temperature=600))
    # grid.set(5, 10, Cell(water))
    # grid.set(6, 10, Cell(water))
    # grid.set(7, 10, Cell(water))
    # grid.set(8, 10, Cell(water))
    # grid.set(1, 1, Cell(water, direction=True, temperature=10))
    # grid.set(1, 11, Cell(sand, direction=True, temperature=90))
    for event in pygame.event.get():
        # User did something
        if event.type == pygame.QUIT:
            # If user clicked close done = True
            # Flag that we are done so we exit this loop
            done = True
    clock.tick(60)
    screen.fill("black")
    grid.update_grid()
    # grid.print()
    display(grid)
    pygame.display.update()
    # print(grid.update_grid().__dict__)
