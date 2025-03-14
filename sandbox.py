
import random
import pygame
import math

""" TODO:
    Rest of the elements
    Teleport
    Barriers
"""

# 1 to 1 size with LED grid
pixelWidth = 15
# pixel size
# resolution = [32, 16]
# pixel count
# guiSize = 30
# in screen pixels pixelWidth = 20
resolution = [40, 40]
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
            if not element1.direction == element1.element.default_direction:
                if element1.direction == True and element1.element.density < element2.element.density:
                    return True
                elif element1.direction == False and element1.element.density > element2.element.density:
                    return True

            elif not element1.direction == element2.direction:
                if element1.direction == False:
                    return False
                return True
            elif element1.direction == True and element1.element.density > element2.element.density:
                return True
            elif element1.direction == False and element1.element.density < element2.element.density:
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
                if current_layer_left_gap:
                    left = (i + 1)
        
        # if left == 0 and current_layer_left_gap:
        #     left = grid.width
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
                if not current_layer_right_gap:
                    break
                if current_layer_right_gap:
                    right = i
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
            if random.random() > 0.5:
                for column in range(grid.width):
                    changes.append((column, row, self.get_new_temperature_difference(column, row)))
            else:
                for column in range(grid.width):
                    changes.append((grid.width - column - 1, row, self.get_new_temperature_difference(grid.width - column - 1, row)))
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

    def perform_pixel_behaviour(self, temperature_data=None):
        for entry in temperature_data:
            # print(self.get(entry[0], entry[1]).__dict__, (entry[0], entry[1]))
            update_pixel = self.grid[entry[0]][entry[1]]
            update_pixel.temperature += entry[2]
            
            if not update_pixel.state == None:
                if hasattr(update_pixel.state, 'do'):
                    update_pixel.state.do(self, entry[0], entry[1])
            if hasattr(update_pixel.element, 'do'):
                # includes updating the colour, material state, performing special behaviours and updating internal attributes such as decay or fuel
                update_pixel.element.do(self, entry[0], entry[1])
                update_pixel.update_colour()

        up_update = self.get_upmoving()
        for column, row, current in up_update:
            if hasattr(current.element, 'move'):
                # includes moving the pixel in the direction specified in the class
                current.element.move(self, column, row)

        down_update = self.get_downmoving()
        for column, row, current in down_update:
            if hasattr(current.element, 'move'):
                # includes moving the pixel in the direction specified in the class
                current.element.move(self, column, row)

colourMode = {"SOLID": 0, "STATIC": 1, "NOISE": 2, "TEMPERATURE_DOWN": 3, "TEMPERATURE_UP": 4, "BLACKBODY": 5}

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
        self.colour = self.colour_base
        # self.colour = self.get_base_colour()

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
        if colourMode['TEMPERATURE_DOWN'] in self.colour_mode:
            self.colour = self.temperature_down_colour_mode()
        if colourMode['BLACKBODY'] in self.colour_mode:
            self.colour = self.blackbody_colour_mode()
        if self.state is not None:
            if hasattr(self.state, "colour"):
                self.colour = self.state.colour(cell = self)


    def temperature_down_colour_mode(self):
        if self.element.next_phase[0] is not None and self.element.next_phase[1] is not None:
            ratio =  (self.temperature - self.element.phase_change_temp[0]) / (0.5 * self.element.phase_change_temp[1] - self.element.phase_change_temp[0])
        elif self.element.next_phase[0] is not None:
            ratio = 1.5 - (self.element.phase_change_temp[0] / self.temperature)
        else:
            ratio = max(0.5, self.temperature / 200)

        r = self.colour_base[0]
        g = self.colour_base[1]
        b = self.colour_base[2]


        r = int(min(max((r * ratio), 0), 254))
        g = int(min(max((g * ratio), 0), 254))
        b = int(min(max((b * ratio), 0), 254))

        return (r, g, b)


    def blackbody_colour_mode(self):
        blackbody_constant = 30
        r = self.colour_base[0]
        g = self.colour_base[1]
        b = self.colour_base[2]

        # colours = self.get_base_colour()
        # r = colours[0]
        # g = colours[1]
        # b = colours[2]

        r = int(min(max(r + (2 *(self.temperature) / (blackbody_constant)), 0), 254))
        g = int(min(max(g + (0.5 * (self.temperature) / (blackbody_constant)), 0), 254))
        b = int(min(max(b + ((self.temperature) / (blackbody_constant)), 0), 254))

        return (r, g, b)

class Particle:
    def __init__(self, name, colour, colour_mode):
        self.name = name
        self.colour = colour
        self.colour_mode = colour_mode
        self.default_direction = None

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
            # cell.temperature = cell.element.phase_change_temp[0] - 1
            # print(1, cell.__dict__)

            cell.element = cell.element.next_phase[0]
            cell.direction = cell.element.default_direction
            cell.update_colour_from_state_change()
            # print(2222, cell.__dict__)

            return

        elif cell.element.next_phase[1] is not None and current_temp > cell.element.phase_change_temp[1]:
            # cell.temperature = cell.element.phase_change_temp[1] + 1
            cell.element = cell.element.next_phase[1]
            cell.direction = cell.element.default_direction
            cell.update_colour_from_state_change()
            
            return
        if cell.element.next_phase[1] is None and cell.flammable == True and current_temp > cell.element.phase_change_temp[1]:
            cell.state = burning

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
                return

        gap_distances = grid.check_side_gap_distance(x, y)
        x2 = -1
        if gap_distances[0] > 0 and gap_distances[1] > 0 and gap_distances[0] == gap_distances[1]:
            x2 = x + random.randint(-1, 1)
        elif (gap_distances[0] > 0 and gap_distances[1] == 0):
            x2 = x + random.randint(-1, 0)
        elif (gap_distances[0] > 0 and gap_distances[0] < gap_distances[1]):
            x2 = x + random.randint(-1, 0)
        elif gap_distances[1] > 0 and gap_distances[0] == 0:
            x2 = x + random.randint(0, 1)
        elif gap_distances[1] > 0 and (gap_distances[1] < gap_distances[0]):
            x2 = x + random.randint(0, 1)

        if not x2 == -1 and grid.can_swap(x, y, x2, y) and grid.can_swap_after_random(x, y, x2, y):
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
        x2 = -1
        if gap_distances[0] > 0 and gap_distances[1] > 0 and gap_distances[0] == gap_distances[1]:
            x2 = x + random.randint(-1, 1)
        elif (gap_distances[0] > 0 and gap_distances[1] == 0):
            x2 = x + random.randint(-1, 0)
        elif (gap_distances[0] > 0 and gap_distances[0] < gap_distances[1]):
            x2 = x + random.randint(-1, 0)
        elif gap_distances[1] > 0 and gap_distances[0] == 0:
            x2 = x + random.randint(0, 1)
        elif gap_distances[1] > 0 and (gap_distances[1] < gap_distances[0]):
            x2 = x + random.randint(0, 1)

        if not x2 == -1 and grid.can_swap(x, y, x2, y) and grid.can_swap_after_random(x, y, x2, y):
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
        super().do(grid, x, y)
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

class Bug(Updatable):
    def __init__(self, name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity):
        super().__init__(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity)
        self.default_direction = None

    def do(self, grid, x, y):
        super().do(grid, x, y)
        cell = grid.get(x, y)
        contact = grid.check_surroundings(x, y)
        relevantContact = [element for element in contact if (isinstance(element[2].element, Solid) or (isinstance(element[2].element, Updatable) and element[2].direction is None and not element[2].element.name == bug.name))]
        if cell.life > 0 and len(relevantContact):
            cell.life -= 1
            return

        elif cell.life <= 0 or not len(relevantContact):
            if random.random() > 0.6:
                cell.life = random.randint(20, 100)
                relevantFree = [element for element in contact if isinstance(element[2].element, Gas)]
                if len(relevantFree):
                    chosenOne = random.choice(relevantFree)
                    grid.swap(x, y, chosenOne[0], chosenOne[1])
            

class Burning(Static):
    def __init__(self, name, threshold):
        super().__init__(name)
        self.threshold = threshold

    def do(self, grid, x, y):
        cell = grid.get(x, y)
        cell.temperature += cell.temperature * ((300 / cell.temperature) - 1)
        contact = grid.check_surroundings(x, y)
        relevantFlammableContact = [element for element in contact if element[2].flammable]
        relevantFreeContact = [element for element in contact if element[2].element == air]
        for neighbour in relevantFlammableContact:
            focus_pixel = grid.grid[neighbour[0]][neighbour[1]]
            if focus_pixel.temperature > self.threshold and random.random() < 1 - (focus_pixel.temperature / self.threshold):
                focus_pixel.state = burning
        
        # smoke responsibility
        if len(relevantFreeContact):
            chosenOne = random.choice(relevantFreeContact)
            if random.random() > 0.95:
                # grid.grid[chosenOne[0]][chosenOne[1]] = Cell(smoke, decay, False, cell.temperature, max(cell.fuel / 2, 1))

                chosenOne[2].state = decay
                chosenOne[2].element = smoke
                chosenOne[2].direction = False
                chosenOne[2].colour_base = smoke.colour
                # chosenOne[2].update_colour_from_state_change()
                chosenOne[2].fuel = max(cell.fuel / 2, 1)

        if cell.temperature < self.threshold:
            # cell = Cell(smoke, decay, False, cell.temperature, cell.fuel)

            cell.state = decay
            cell.element = smoke
            cell.direction = False
            cell.colour_base = smoke.colour
            cell.update_colour_from_state_change()
            
            # cell.colour_base = (254, 254, 254)
            
        cell.fuel -= 1
        if cell.fuel <= 0:
            # cell = Cell(air, temperature=cell.temperature)

            cell.element = air
            cell.flammable = False
            cell.direction = False
            cell.state = None
            cell.update_colour_from_state_change()

    def colour(self, cell):
        if random.random() > 0.9:
            r = random.randint(150, 250)
            g = random.randint(100, 150)
            b = random.randint(50, 100)
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
            cell.flammable = False
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
            choice = random.choice(relevantContact) 
            element = (choice[2] if choice[2].element is not clone else None)
            pixel.clone_element = Cell(element.element, element.state, element.flammable, element.temperature, element.fuel, element.life, element.branches)
        
        else:
            contact = grid.check_surroundings(x, y)
            relevantContact = [element for element in contact if element[2].element == air]
            if not len(relevantContact):
                        return
            chosenOne = random.choice(relevantContact)
            grid.draw(chosenOne[0], chosenOne[1], Cell(pixel.clone_element.element, pixel.clone_element.state, pixel.clone_element.flammable, pixel.clone_element.temperature, pixel.clone_element.fuel, pixel.clone_element.life, pixel.clone_element.branches))

class Temperature(Updatable):
    def __init__(self, name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity, temperature):
        super().__init__(name, colour, colour_mode, phase_change_temp, next_phase, thermal_conductivity)
        self.clone_element = None
        self.default_direction = None
        self.temperature = temperature

    def do(self, grid, x, y):
        pixel = grid.get(x, y)
        pixel.temperature = self.temperature

class Flip(Particle):
    def __init__(self, name, colour, colour_mode):
        super().__init__(name, colour, colour_mode)
        self.default_direction = None
    
    def do(self, grid, x, y):
        contact = grid.check_surroundings(x, y)
        relevantContact = [element for element in contact if not element[2].element == air and element[2].direction is not None]
        if not len(relevantContact):
            return
        chosenOne = random.choice(relevantContact)
        chosenOne[2].direction = not chosenOne[2].direction

# Materials
air = Gas("Air", (0, 0, 0), (colourMode["BLACKBODY"],), [None, None], [None, None], 0.026, 1.23, False)

sand = Solid("Sand", ([200, 250], [125, 200], [75, 100]), (colourMode['STATIC'], colourMode['BLACKBODY']), [None, None], [None, None], 0.4, 1602, True)

water = Liquid("Water", (0, 0, 255), (colourMode["TEMPERATURE_DOWN"],), [0, 100], [None, None], 0.6, 997, True)
ice = Updatable("Ice", ([55, 85], [55, 85], 255), (colourMode["STATIC"],), [None, 0], [None, water], 2.22)
steam = Gas("Steam", (200, 200, 200), (colourMode["TEMPERATURE_DOWN"],), [100, None], [water, None], 0.0184, 0.6, False)
snow = Solid("Snow", ([240, 254], [240, 254], [240, 254]), (colourMode["STATIC"], ), [None, 0], [None, water], 0.1, 70, True)
water.next_phase = [ice, steam]
ice.next_phase = [None, water]
steam.next_phase = [water, None]

smoke = Gas("Smoke", (254, 254, 254), (colourMode["STATIC"],), [None, 300], [None, None], 0.6, 0.6, False)
fire = Gas("Fire", ([200, 254], [0, 50], [0, 50]), (colourMode["NOISE"],), [300, None], [smoke, None], 0.6, 0.6, False)
smoke.next_phase=[None, fire]

oil = Liquid("Oil", (50, 25, 25), (colourMode["SOLID"],), [None, 300], [None, None], 0.6, 800, True)

lava = Liquid("Lava", (254, [150, 250], [150, 250]), (colourMode['NOISE'],), [1160, None], [None, None], 2, 2400, True)
stone = Solid("Stone", (200, 200, [190, 210]), (colourMode['STATIC'], colourMode['BLACKBODY']), [None, 1160], [None, lava], 2.5, 2500, True)
lava.next_phase = [stone, None]
stone.next_phase = [None, lava]

wood = Updatable("Wood", ([50, 100], [25, 55], 0), (colourMode['STATIC'],), [None, 250], [None, None], 0.15)

molten_metal = Liquid("Molten Metal", (75, 75, 85), (colourMode['BLACKBODY'],), [1538, None], [None, None], 6, 7800, True)
metal = Updatable("Metal", (75, 75, 85), (colourMode['BLACKBODY'],), [None, 1538], [None, molten_metal], 3)
molten_metal.next_phase = [metal, None]
metal.next_phase = [None, molten_metal]

block = Particle("Block", (100, 100, 100), (colourMode["SOLID"], ))

# Special
destruct = Destruct("Destruct", ([25, 55], [25, 55], [25, 55]), (colourMode["NOISE"],),)
grow = Grow("Grow", (0, [150, 250], 10), (colourMode["STATIC"],), [None, 300], [None, None], 0.6)
clone = Clone("Clone", (150, 150, 0), (colourMode['SOLID'], ))
cold = Temperature("Cold", ([50, 75], [50, 75], [100, 254]), (colourMode['NOISE'],), [None, None], [None, None], 0.05, -10000)
hot = Temperature("Hot", ([100, 254], [50, 75], [50, 75]), (colourMode['NOISE'],), [None, None], [None, None], 0.05, 10000)
conduct = Updatable("Conduct", (100, 200, 100), (colourMode["BLACKBODY"], colourMode["TEMPERATURE_DOWN"],), [None, None], [None, None], 5)
flip = Flip("Flip", (25, 240, 0), (colourMode['SOLID'], ))

# States
burning = Burning("Burning", threshold=149)
decay = Decay("Decay")

bug = Bug("Bug", (150, 200, 0), (colourMode['SOLID'], ), [None, 50], [None, None], 0.6)


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

grid.draw(17, 25, Cell(grow,  life = 100, branches = 1, flammable=True, fuel=100))
grid.draw(17, 24, Cell(clone))

grid.draw(35, 30, Cell(water, temperature=99))

grid.draw(12, 8, Cell(flip))

grid.draw(34, 30, Cell(clone))

grid.draw(39, 39, Cell(destruct))



for _ in range(40):
    # grid.draw(_, 21, Cell(ice, None, False, -2003))
    
    grid.draw(_, 4, Cell(conduct))


grid.draw(5, 35, Cell(cold))
grid.draw(2, 7, Cell(hot))

count = 0

while not done:
    # print("--------------------------")
    
    # debug(grid)

    # grid.draw(15, 15, Cell(fire, burning, False, 1000, 50, 0, 0))
    # grid.draw(37, 20, Cell(stone, temperature=1900))
                # pass
    # grid.draw(14, 14, Cell(destruct)) 

    # grid.draw(17, 15, Cell(clone))
    # grid.draw(18, 15, Cell(clone))
    # grid.draw(19, 15, Cell(clone))

    # grid.draw(7, 7, Cell(burning))

    
    # grid.draw(10, 25, Cell(water, temperature=102))
    # grid.draw(5, 22, Cell(water, temperature=23.5))
    # grid.draw(6, 22, Cell(water, temperature=23.5))
    # grid.draw(7, 22, Cell(water, temperature=23.5))


    # cell = grid.get(5, 5)
    # cell.temperature += 15

    # cell = grid.get(35, 35)
    # cell.temperature -= 15


    # grid.draw(5, 20, Cell(oil, temperature=-24, flammable=True, fuel=200))
    # grid.draw(6, 20, Cell(water))
        
    # grid.grid[20][39].temperature = -10
    
    # grid.draw(5, 5, Cell(ice, temperature=200))
    # grid.draw(6, 5, Cell(ice, temperature=90))
    # grid.draw(7, 5, Cell(ice, temperature=100))
    # grid.draw(1, 14, Cell(sand, temperature=40))
    
    # grid.draw(35, 7, Cell(lava, temperature=1900))
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
