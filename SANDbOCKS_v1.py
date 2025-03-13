import random
import pygame
import math

"""
Todo list:
 - Pixel optimisation (at rest)
 - Profile code to see which parts take the most time
 - Temp change leads to burning for material with fuel
 X Type barriers
    * Don't let some types swap, act as block, eg only liquids through, not gas. Vise versa
 X Density and swap probabilities related
 X Temperature and density relation
    * Yes but no, doesn't work so good
 X Temperature and colour relation (red shift)
 X Pixel temperature increasing when insulated bug
    * I think to do with the 'burning' state
 X Pixel temperature 
 X Pixel state change at temperature values
 X Pixel temperature influence and thermal conductivity
    * Need a more rigorous way to calculate the temperature of each part
    * Maybe do some simple equations which work for 2 particles and then work up
    * ie if two particles touch with a difference in temperature, after some amount of time the two should be the average of both temperatures
    * when accounting for time in this instance, thermal conductivity will play a part.. Need some way of keeping the average temperature the same when one cools much quicker than the other..
    * the rate at which it does this is dependent on the thermal conductivity for each material
    * a high thermal conductive material will drop it's temperature faster, AND will change the other materials temp quickly too - trending towards the average temp
    * a low thermal conductive material will hold its temperature better, and won't influence other materials as much... ie more time to stabalise
    * could thermal density also play a role? 
 X clone material
 X GUI of sorts to pick and choose materials
 - GUI label of material
 X DESTRUCT AND CLONE TO ACCOUNT FOR ALL SURROUNDING PIXELS BEING ONE OF THE SAME
 X Make liquid settling not random, just have it go in the direction of the closest gap
"""

#1 to 1 size with LED grid
pixelWidth = 44 #pixel size
resolution = [32, 16] #pixel count
guiSize = 30 #in screen pixels

# pixelWidth = 8 #pixel size
# resolution = [150, 100] #pixel count
# guiSize = 30 #in screen pixels

class Grid:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.grid = [[getMaterial("air") for _ in range(height)] for _ in range(width)] 
        
    def clear(self, width, height): 
        self.grid = [[getMaterial("air") for _ in range(height)] for _ in range(width)] 
        
    def set(self, x, y, materialName): 
        self.grid[x][y] = getMaterial(materialName) 
        
    def get(self, x, y):
        return self.grid[x][y]

    def swap(self, x1, y1, x2, y2): 
        self.grid[x1][y1], self.grid[x2][y2] = self.grid[x2][y2], self.grid[x1][y1]

    def isEmpty(self, x, y):
        if self.grid[x][y].name == "air":
            return True
        if self.grid[x][y].name == "p_entrance":
            for portal in portals:
                if portal.entrance == (x, y):
                    if not portal.exit[1] + 1 == self.height:
                        return self.grid[portal.exit[0]][portal.exit[1]+1].name == "air"
        return False
    
    def checkSideGaps(self, x, y):
        #get distance to nearest gap
        initPixel = self.get(x, y)
        # print(initPixel.__dict__)
        ret = [0, 0] #left and right free
        if x == 0:
            ret[0] = 0
        elif x == self.width - 1:
            ret[1] = 0
            
        # right pass
        for right in range(self.width - x):
            pixel = self.get(x + right, y)
            if pixel.name == "air":
                ret[1] = right
                break
            elif not pixel.type or initPixel.type < pixel.type:
                ret[1] = right
            elif self.densityCheck(x, y, x + right, y):
                ret[1] = right
            

        for left in range(0, x + 1):
            pixel = self.get(x - left, y)
            if pixel.name == "air":
                ret[0] = left
                break
            elif not pixel.type or initPixel.type < pixel.type:
                ret[0] = left
            elif self.densityCheck(x, y, x - left, y):
                ret[0] = left
                
        
        return ret

    def checkContact(self, x, y):
        pixelList = []

        #check 3x3 grid around current pixel
        check_cell_delta = [-1, 0, 1]

        for deltaY in check_cell_delta: 
            for deltaX in check_cell_delta:
                if x + deltaX < 0 or x + deltaX >= self.width or y + deltaY < 0 or y + deltaY >= self.height or (deltaX == 0 and deltaY == 0):
                    continue
                elif not self.grid[x + deltaX][y + deltaY].name == "air":
                    pixel = self.grid[x + deltaX][y + deltaY]

                    # params
                    flammable = pixel.flammable
                    type = pixel.name
                    density = pixel.density
                    temperature = pixel.temperature if pixel.temperature else 23
                    conductivity = pixel.thermalConductivity
                    pixelList.append((x + deltaX, y + deltaY, type, flammable, density, temperature, conductivity))

        return pixelList
    
    def checkContactWithEverything(self, x, y): #include air
        pixelList = []

        #check 3x3 grid around current pixel
        check_cell_delta = [-1, 0, 1]

        for deltaY in check_cell_delta: 
            for deltaX in check_cell_delta:
                if x + deltaX < 0 or x + deltaX >= self.width or y + deltaY < 0 or y + deltaY >= self.height or (deltaX == 0 and deltaY == 0):
                    continue
                else:
                    pixel = self.grid[x + deltaX][y + deltaY]

                    # params
                    flammable = pixel.flammable
                    type = pixel.name
                    density = pixel.density
                    temperature = pixel.temperature if pixel.temperature else 23
                    conductivity = pixel.thermalConductivity
                    pixelList.append((x + deltaX, y + deltaY, type, flammable, density, temperature, conductivity))

        return pixelList

    def checkAirGap(self, x, y):
        pixelList = []

        #check 3x3 grid around current pixel
        check_cell_delta = [-1, 0, 1]

        for deltaY in check_cell_delta: 
            for deltaX in check_cell_delta:
                if x + deltaX < 0 or x + deltaX >= self.width or y + deltaY < 0 or y + deltaY >= self.height or (deltaX == 0 and deltaY == 0):
                    continue
                elif self.grid[x + deltaX][y + deltaY].name == "air":
                    # params
                    pixelList.append((x + deltaX, y + deltaY))

        return pixelList

    def densityCheck(self, x, y, newX, newY):
        return not self.grid[newX][newY].density == 0 and self.grid[x][y].density > self.grid[newX][newY].density
        # not (self.grid[x][y].direction * self.grid[x][y].density > self.grid[newX][newY].direction * self.grid[newX][newY].density) 
    
def draw(grid, x, y, materialName, radius):
    if materialName == 'p_entrance':
                create_portal(x, y)
    elif materialName == 'air':
        erase(grid, x, y, radius)
    else:
        for column in range(grid.width):
            for row in range(grid.height):
                if math.sqrt((math.pow((x - column), 2)) + (math.pow((y - row), 2))) < radius:
                    if grid.grid[column][row].name == "air":
                        if random.random() <= getMaterial(materialName).fill:
                            grid.set(column, row, materialName)

def erase(grid, x, y, radius):
    if grid.grid[x][y].name == "p_entrance" or grid.grid[x][y].name == "p_exit":
        idx = 0
        for portal in portals:
            if portal.entrance == (x, y) or portal.exit == (x, y):
                grid.set(portal.entrance[0], portal.entrance[1], "air")
                grid.set(portal.exit[0], portal.exit[1], "air")
                portals.pop(idx)
                return
            idx += 1
        
    for column in range(grid.width):
        for row in range(grid.height):
            if math.sqrt((math.pow((x - column), 2)) + (math.pow((y - row), 2))) < radius:
                # check type
                if grid.grid[column][row] == (2 or 3):
                    pass
                else:
                    grid.set(column, row, "air")


def swapBuffer(grid, x, y, newX, newY):

    pixel = grid.get(x, y)
    newPixel = grid.get(newX, newY)

    if "barrier" in newPixel.name:
        blockType = newPixel.special['blockType']
        if not pixel.type == blockType:
            if newX - x > 0:
                deltaX = 1
            elif newX - x < 0:
                deltaX = -1
            else:
                deltaX = 0

            if newY - y > 0:
                deltaY = 1
            elif newY - y < 0:
                deltaY = -1
            else:
                deltaY = 0

            if newX + deltaX < 0 or newX + deltaX >= grid.width or newY + deltaY < 0 or newY + deltaY >= grid.height:
                pass
            else:
                # if not "barrier" in grid.get(newX + deltaX, newY + deltaY).name:
                swapBuffer(grid, x, y, newX + deltaX, newY + deltaY)

    # if newPixel.name == "destruct":
    #     grid.set(x, y, "air")
    if newPixel.name == "air":
        grid.swap(x, y, newX, newY)
    
    elif grid.densityCheck(x, y, newX, newY):
        if random.random() < (pixel.density / (pixel.density + newPixel.density)):
            grid.swap(x, y, newX, newY)

    elif newPixel.name == "p_entrance":
        for portal in portals:
            if portal.entrance == (newX, newY) and grid.isEmpty(portal.entrance[0], portal.entrance[1]):
                grid.swap(x, y, portal.exit[0], portal.exit[1]+1)

    # density check to see if swap passes, must add density material property
    # check direction by seeing if new y > y

def burning(grid, x, y):
    pixel = grid.grid[x][y]
    #temperature increase: 
    # grid.grid[x][y].temperature += 1

    #burning fuel decay
    pixel.fuel -= 1
    if pixel.fuel == 0:
        grid.set(x, y, "smoke")
        return

    #change to burning colours
    matProperties = getMaterial("burn")
    pixel.initColour = matProperties.initColour
    pixel.colourMode = matProperties.colourMode

    # spreading fire
    pixels = grid.checkContact(x, y)

    extinguishing_neighbours = 0
    for neighbour_pixel in pixels:
        if neighbour_pixel[3] is None:
            pass
        else:
            if neighbour_pixel[3] < 0:
                extinguishing_neighbours += 1
            if neighbour_pixel[3] > 0 and random.random() > 0.995:
                grid.grid[neighbour_pixel[0]][neighbour_pixel[1]].currentState = "burning"

    # check if suffocated
    # if extinguishing_neighbours > 5:
    #     extinguished(grid, x, y)

    #spawn new particles (embers, smoke)
    # TODO replace with temperature change of fire to smoke under a certain temperature
    # if x - 1 >= 0 or x + 2 < grid.width:
    #     if not y - 1 <= 0:
    #         # x = random.randint(x - 1, x + 1)
    #         if random.random() > 0.95 and grid.grid[x][y-1].name == "air":
    #             grid.set(x, y - 1, "smoke") if random.random() > 0.1 else grid.set(x, y - 1, "fire") 


def extinguished(grid, x, y):
    pixel = grid.grid[x][y]
    if pixel.name == "fire":
        return

    pixel.currentState = "extinguished"

    # replace properties
    if pixel.type == "3":
        pixel = getMaterial("smoke")
    else:
        matProperties = getMaterial(pixel.name)
        ratio = (pixel.fuel / matProperties.fuel)
        r = int(pixel.initColour[0] * ratio)
        g = int(pixel.initColour[1] * ratio)
        b = int(pixel.initColour[2] * ratio)
        pixel.initColour = (r, g, b)
        pixel.colour = (r, g, b)
        pixel.colourMode = matProperties.colourMode

    grid.grid[x][y] = pixel

# def temperatureChange(grid, x, y): #old
#     pixel = grid.grid[x][y]
#     neighbours = grid.checkContact(x, y)
#     temperature = pixel.temperature
#     if len(neighbours) > 0:
#         temp = 0
#         for neighbour in neighbours:
#             if neighbour[4]:
#                 temp += neighbour[5]
#         avg = temp / len(neighbours)

#         #all sorta at the same temp, start lowering it
#         if abs(avg - temperature) < 1:
#             if temperature < 23:
#                 temperature += abs(temperature - 23) / 23
#             elif temperature > 23:
#                 temperature -= abs(temperature - 23) / 23
        
#         avg_affect = (len(neighbours) / 8)
#         if avg > temperature:
#             temperature += (avg - temperature) * avg_affect
#         elif avg < temperature:
#             temperature -= (temperature - avg) * avg_affect
#         # else:
#         #     temperature -= 0.1
#     elif temperature < 23:
#         temperature += 0.1
#     elif temperature > 23:
#         temperature -= 0.1
        
#     return (x, y, temperature) if not temperature == pixel.temperature else None

def temperatureChange(grid, x, y):
    #rework to use the difference in temperature more than the temperature relative to 23
    # also need to include the thermal conductivity
    pixel = grid.grid[x][y]

    if pixel.name in ["fire", "cold", "hot", "hot_pulse", "cold_pulse"]:
        return (x, y, pixel.initTemperature)

    
    neighbours = grid.checkContactWithEverything(x, y)
    temperature = pixel.temperature
    difference = 0
    for neighbour in neighbours:
        if neighbour[5]:
            difference += (neighbour[5] - temperature) * neighbour[6]
    avg = difference / len(neighbours)
    
    temperature += avg

        #all sorta at the same temp, start lowering it
        # if avg >= 0 and avg < 1:
        #     temperature += (temperature - 23) / 23

        # if avg <= 0 and avg > -1:
        #     temperature -= (temperature - 23) / 23
        
        #old system
        # avg_affect = (len(neighbours) / 8) * 0 + 1
        # temperature += (avg) * avg_affect
        # print(pixel.name, "before", pixel.temperature, "after", temperature, "relative environment", avg)

    # elif temperature < 23:
    #     temperature += 0.1
    # elif temperature > 23:
    #     temperature -= 0.1
        
    return (x, y, temperature) if not temperature == pixel.temperature else None

def phaseChange(grid, x, y):
    pixel = grid.grid[x][y]
    if pixel.phaseChangeTemp[0] is not None and pixel.temperature < pixel.phaseChangeTemp[0] and random.random() > 0.75:
        grid.set(x, y, pixel.nextPhase[0])
        grid.grid[x][y].temperature = pixel.temperature
    elif pixel.phaseChangeTemp[1] is not None and pixel.temperature > pixel.phaseChangeTemp[1] and random.random() > 0.75:
        grid.set(x, y, pixel.nextPhase[1])
        grid.grid[x][y].temperature = pixel.temperature

    return grid.get(x, y)

def clone(grid, x, y):
    pixel = grid.grid[x][y]
    if pixel.special == None:
        pixels = grid.checkContact(x, y)
        noClone = [pixel for pixel in pixels if not pixel[2] == "clone"]
        if len(noClone):
            grid.grid[x][y].special = {"name": noClone[random.randint(0, len(noClone) - 1)][2]}
    
    else:
        pixels = grid.checkAirGap(x, y)
        if len(pixels):
            spawnPixel = pixels[(random.randint(0, len(pixels) - 1))]
            grid.set(spawnPixel[0], spawnPixel[1], grid.grid[x][y].special['name'])

def flip(grid, x, y):
    pixel = grid.grid[x][y]
    pixels = grid.checkContact(x, y)
    for pixel in pixels:
        if grid.grid[pixel[0]][pixel[1]].direction == 0 or grid.grid[pixel[0]][pixel[1]].density == 0:
            continue
        grid.grid[pixel[0]][pixel[1]].direction = -grid.grid[pixel[0]][pixel[1]].direction
        # grid.grid[pixel[0]][pixel[1]].density = 1 / grid.grid[pixel[0]][pixel[1]].density

def grow(grid, x, y):
    pixel = grid.grid[x][y]
    
    if pixel.special['life'] <= 0:
        return
        
    contact = grid.checkContactWithEverything(x, y)

    airGaps = []
    pixels = []

    for cell in contact:
        if cell[2] == "air":
            airGaps.append(cell)
        elif cell[2] == "water":
            airGaps.append(cell)
            pixels.append(cell)
        else:
            pixels.append(cell)

    life_loss = -5
    for cell in pixels:
        #positive life
        if cell[2] == "water":
            life_loss += 5
        if cell[2] == "wood":
            life_loss += 2

        #negative life
        if cell[2] == "oil":
            life_loss += -20

    if len(airGaps) and random.random() > 0.9:
        spawnPixel = airGaps[random.randint(0, len(airGaps)-1)]
        grid.set(spawnPixel[0], spawnPixel[1], "grow")
        grid.grid[spawnPixel[0]][spawnPixel[1]].special = {"name": "grow",
                                                            "life": pixel.special['life'] + life_loss,
                                                            "branches": pixel.special['branches']}
        if pixel.special['branches'] > 0 and random.random() > 0.9:
            spawnPixel = airGaps[random.randint(0, len(airGaps)-1)]
            grid.grid[spawnPixel[0]][spawnPixel[1]].special = {"name": "grow",
                                                                "life": pixel.special['life'] - 10,
                                                                "branches": pixel.special['branches'] - 1}
        #only grow once
        pixel.special = {"name": "grow",
                        "life": 0,
                        "branches": 0}

def destruct(grid, x, y):
        pixels = grid.checkContact(x, y)
        noDestroy = [pixel for pixel in pixels if not pixel[2] == "destruct"]
        if len(noDestroy):
            destroy = noDestroy[random.randint(0, len(noDestroy) - 1)]
            erase(grid, destroy[0], destroy[1], 1)
            

def updateColour(grid, x, y):
    pixel = grid.grid[x][y]
    if pixel.colourMode == "noise":
        if random.random() > 0.9:
            r = random.randrange(pixel.initColour[0] - 25, pixel.initColour[0] + 25)
            g = random.randrange(pixel.initColour[1] - 25, pixel.initColour[1] + 25)
            b = random.randrange(pixel.initColour[2] - 25, pixel.initColour[2] + 25)
        else:
            r, g, b = pixel.colour

    elif pixel.colourMode == "rampDown":
        ratio = (pixel.lifetime / pixel.initialLifetime)
        r = int(pixel.initColour[0] * ratio)
        g = int(pixel.initColour[1] * ratio)
        b = int(pixel.initColour[2] * ratio)

    elif pixel.colourMode == "temperature":
        ratio = (pixel.temperature / pixel.initTemperature)
        r = int(pixel.initColour[0] * ratio)
        g = int(pixel.initColour[1] * ratio)
        b = int(pixel.initColour[2] * ratio)

    elif pixel.colourMode == "blackbody":
        blackbody_r = (pixel.temperature / 500) * 255
        blackbody_g = (pixel.temperature / 1000) * 255
        blackbody_b = (pixel.temperature / 750) * 255
        ratio = (pixel.temperature / pixel.initTemperature)
        r = int(pixel.initColour[0] + blackbody_r)
        g = int(pixel.initColour[1] + blackbody_g)
        b = int(pixel.initColour[2] + blackbody_b)

    r = r if r < 255 else 255
    r = r if r >= 0 else 0

    g = g if g < 255 else 255
    g = g if g >= 0 else 0

    b = b if b < 255 else 255
    b = b if b >= 0 else 0

    grid.grid[x][y].colour = (  (r, g, b) )

# game logic and rendering
def updatePixel(grid, x, y):
    pixel = grid.grid[x][y]
    # clone behaviour
    if pixel.name == "clone":
        clone(grid, x, y)
        return

    if pixel.name == "flip":
        flip(grid, x, y)
        return

    if pixel.name == "grow":
        grow(grid, x, y)
        # return
    
    # replaced with simpler implementation
    # Reverted back for better function
    if pixel.name == "destruct":
        destruct(grid, x, y)
        # return

    #decay
    if pixel.lifetime and pixel.lifetime > 0:
        pixel.lifetime -= 1
        if pixel.lifetime == 0:
            grid.set(x, y, "air")

    #burning behaviour
    if (pixel.currentState == "burning" and pixel.fuel):
        burning(grid, x, y)

    if grid.grid[x][y].name == "air":
        return

    pixel = phaseChange(grid, x, y)

    #colour updates
    if not pixel.colourMode == "static":
        updateColour(grid, x, y)

    #check if pixel needs to move:
    # This function causes a LOT of lag...
    # if not len(grid.checkAirGap(x, y)):
    #     return

    #downmoving
    if pixel.direction == -1:
        if y < grid.height-1:
            if not grid.isEmpty(x, y+1):
                    
                deltaX = 0
                deltaY = 0
                
                # if grid.densityCheck(x, y, x, y+1):
                swapBuffer(grid, x, y, x, y+1)
                    # return

                # Particle is below current one, check left and right side of below particle for free space
                free = grid.checkSideGaps(x, y+1)

                if pixel.type == 1:

                    if free[0] == 1 and free[1] == 1:
                        if random.random() < 0.5:
                            deltaY = 1
                            deltaX = 1
                        else:
                            deltaY = 1
                            deltaX = -1  
                    elif free[0] == 1:
                        deltaY = 1
                        deltaX = -1
                    elif free[1] == 1:
                        deltaY = 1
                        deltaX = 1
                    swapBuffer(grid, x, y, x+deltaX, y+deltaY)
                    # return

                elif pixel.type == 2 or pixel.type == 3:
                    #right next to a spot
                    if free[0] == 1 and free[1] == 1:
                        if random.random() < 0.5:
                            deltaY = 1
                            deltaX = 1
                        else:
                            deltaY = 1  
                            deltaX = -1
                    elif free[0] == 1:
                        deltaY = 1
                        deltaX = -1
                    elif free[1] == 1:
                        deltaY = 1
                        deltaX = 1
                    else:
                        # must wander for a spot in grid below
                        if free[0] > 1 and free[1] > 1:
                            if random.random() < free[0] / (free[0] + free[1]):
                                deltaX = 1
                            else:
                                deltaX = -1
                        elif free[0] > 1:
                            deltaX = -1
                        elif free[1] > 1:
                            deltaX = 1
                        else: 
                            # on flat surface, should randomly move left and right
                            free = grid.checkSideGaps(x, y)
                            if free[0] == 0 and free[1] == 0:
                                return
                            elif free[0] > 1 and free[1] > 1:
                                if random.random() <= free[0] / (free[0] + free[1]):
                                    deltaX = -1
                                else:
                                    deltaX = +1
                            elif free[0] == free[1]:
                                if random.random() >= 0.5:
                                    deltaX = -1
                                else:
                                    deltaX = +1
                            elif free[0] >= 1:
                                deltaX = -1
                            elif free[1] >= 1:
                                deltaX = 1

                            if random.random() < 0.05 and not deltaX == 0: 
                                swapBuffer(grid, x, y, x+deltaX, y) 
                            return

                swapBuffer(grid, x, y, x+deltaX, y+deltaY)
                    # return
            
            else: # nothing below
                swapBuffer(grid, x, y, x, y+1)
                return

    # upmoving
    if pixel.direction == 1:
        if y > 0:
            if not grid.isEmpty(x, y-1):

                deltaX = 0
                deltaY = 0
                
                # if grid.densityCheck(x, y, x, y-1):
                swapBuffer(grid, x, y, x, y-1)
                

                # Particle is above current one, check left and right side of above particle for free space
                free = grid.checkSideGaps(x, y-1)

                if pixel.type == 1:
                    if free[0] == 1 and free[1] == 1:
                        if random.random() < 0.5:
                            deltaY = -1
                            deltaX = 1
                        else:
                            deltaY = -1
                            deltaX = -1  
                    elif free[0] == 1:
                        deltaY = -1
                        deltaX = -1
                    elif free[1] == 1:
                        deltaY = -1
                        deltaX = 1
                    swapBuffer(grid, x, y, x+deltaX, y+deltaY)

                if pixel.type == 2 or pixel.type == 3:
                    #right next to a spot
                    if free[0] == 1 and free[1] == 1:
                        if random.random() < 0.5:
                            deltaY = -1
                            deltaX = 1
                        else:
                            deltaY = -1  
                            deltaX = -1
                    elif free[0] == 1:
                        deltaY = -1
                        deltaX = -1
                    elif free[1] == 1:
                        deltaY = -1
                        deltaX = 1
                    else:
                        # must wander for a spot
                        if free[0] > 1 and free[1] > 1:
                            if random.random() < (free[0] / free[1]) / 2:
                                deltaX = 1
                            else:
                                deltaX = -1
                        elif free[0] > 1:
                            deltaX = -1
                        elif free[1] > 1:
                            deltaX = 1
                        else: 
                            # on flat surface, should randomly move left and right
                            free = grid.checkSideGaps(x, y)
                            if free[0] == 0 and free[1] == 0:
                                return
                            elif free[0] > 1 and free[1] > 1:
                                if random.random() <= free[0] / (free[0] + free[1]):
                                    deltaX = -1
                                else:
                                    deltaX = +1
                            elif free[0] == free[1]:
                                if random.random() >= 0.5:
                                    deltaX = -1
                                else:
                                    deltaX = +1
                            elif free[0] >= 1:
                                deltaX = -1
                            elif free[1] >= 1:
                                deltaX = 1

                            if random.random() < 0.05 and not deltaX == 0: 
                                swapBuffer(grid, x, y, x+deltaX, y) 
                            return

                    swapBuffer(grid, x, y, x+deltaX, y+deltaY)
                    # return
            else: # nothing below
                swapBuffer(grid, x, y, x, y-1)
                return

def updateGrid(grid):

    #property changes
    stagedTemperatureChanged = []
    for column in range(grid.width):
        for row in range(grid.height):
            if not grid.grid[column][row].name == "air":
                change = temperatureChange(grid, column, row)
                if change is not None:
                    stagedTemperatureChanged.append(change)
    
    for change in stagedTemperatureChanged:
        grid.grid[change[0]][change[1]].temperature = change[2]

    
    # upmoving
    for column in range(grid.width):
        for row in range(grid.height):
            if not grid.grid[column][row].name == "air":
                if random.random() < 0.5:    
                    updatePixel(grid, column, row)
                else:
                    updatePixel(grid, grid.width - 1 - column, row)

    # downmoving
    for column in range(grid.width):
        for row in range(grid.height):
            if not grid.grid[column][row].name == "air":
                if random.random() < 0.5:    
                    updatePixel(grid, column, grid.height - 1 - row) #reverse order otherwise gravity doesn't work
                else:
                    updatePixel(grid, grid.width - 1 - column, grid.height - 1 - row) #reverse order otherwise gravity doesn't work

def display(grid):
    for y in range(grid.height):
        # pygame.draw.line(screen, (40, 40, 40), [0, y*pixelWidth], [grid.width*pixelWidth, y*pixelWidth], 5)
        for x in range(grid.width):
            # if y == 0:
            #     pygame.draw.line(screen, (40, 40, 40), [x*pixelWidth, 0], [x*pixelWidth, grid.height*pixelWidth], 5)
            material = grid.get(x, y)
            pygame.draw.rect(screen, material.colour, [x*pixelWidth, y*pixelWidth, pixelWidth, pixelWidth])

def gui(action, materialSelect, mouseX, mouseY):
    # draw GUI
    guiY = resolution[1]*pixelWidth
    idx = 0
    borderGap = 5

    useable_materials = [material for material in materials if material not in materials_mask]

    for material in useable_materials:
        if materialSelect == material or (action == 1 and mouseX > idx*guiSize and mouseX < idx*guiSize + guiSize and mouseY > guiY and mouseY < guiY + guiSize):
            materialSelect = material
            pygame.draw.rect(screen, (255, 255, 255), [idx*guiSize, guiY, guiSize, guiSize])
            
            # text = font.render(material[materialSelect], True, (255, 255, 255), (0, 255, 0))
            # textRect = text.get_rect()
            # textRect.center = (50, 50)
            

        pygame.draw.rect(screen, getMaterial(material).palletColour, [idx*guiSize + borderGap, guiY + borderGap, guiSize - borderGap*2, guiSize - borderGap*2])
        idx += 1

    return materialSelect

def debug(grid, x, y):
    print(grid.grid[x][y].__dict__)
    print(grid.checkSideGaps(x,y))
    print(portals)
        

class Material:
    def __init__(self, name, colour, direction, type, flammable, temperature = 23, phaseChangeTemp = [None, None], nextPhase = [None, None], thermalConductivity = 0, density = 0, fill = 1.0, colourMode = "static", currentState = None, fuel = None, lifetime = None, special = None):
        self.name = name

        # work random colours
        colourBuff = []
        palletBuff = []
        for value in colour:
            if isinstance(value, list):
                colourBuff.append(random.randint(value[0], value[1]))
                palletBuff.append(value[0])
            else:
                colourBuff.append(value)
                palletBuff.append(value)

        # material palette colour
        self.palletColour = palletBuff        
        
        self.initColour = colourBuff
        # (r g b)

        self.colour = self.initColour
        # (r g b)

        self.direction = direction
        # 1: up, -1: dowm, 0: static

        self.type = type
        # 0: static, 1: solid, 2: liquid, 3: gas
        
        self.flammable = flammable
        # -1 for extinguishing, 0 for non flammable, 1 for flammable

        self.initTemperature = temperature
        # current temperature - room temp is at 23 by default

        self.temperature = self.initTemperature
        # current temperature - room temp is at 23 by default

        self.phaseChangeTemp = phaseChangeTemp
        # temp at which below value 1 it will solidify, or above value 2 will evaporate. If None for either value it won't change to that state

        self.nextPhase = nextPhase
        #The material to replace current one during each phase change

        self.thermalConductivity = thermalConductivity
        # how well heat is transferred, 0 to 1

        self.density = density
        #used so materials can swap past each other. 0 for static and solid type

        self.fill = fill
        # used for draw function to determine if there is noise in the draw stroke. Float from 0-1 (0 to 100%)

        self.colourMode = colourMode
        # Whether the colour updates over time, Default static, rampDown, rampUp, noise. Used in the updateColour def
        
        self.currentState = currentState
        # default "normal".  for describing different states of the material such as burning

        self.fuel = fuel
        # default None. Amount of time the material will burn for before disappearing

        self.initialLifetime = lifetime

        self.lifetime = self.initialLifetime
        # default None - persistant. If specified it's the number of frames it lasts for before becoming air

        self.special = special
        #for extra conditions specific to one material eg clone

# Remove some entries from the paint pallet
materials_mask = [
    "p_exit", "burn"
]

materials = {

    "air": {"colour": (0, 0, 0), 
            "direction": -1, 
            "type": 0, 
            "flammable": 0, 
            "temperature": 23.0,
            "thermalConductivity": 0.01,
            "phaseChangeTemp": [None, None], 
            "density": 0.1,
            "lifetime": None},

    "sand":{"colour": ([200, 255], [150, 200], 60), 
            "colourMode": "blackbody",
            # "colourMode": "static", 
            "direction": -1, 
            "type": 1, 
            'fill': 0.75,
            "flammable": 0,  
            "temperature": 23.0,
            "thermalConductivity": 0.2,
            "density": 1.2,
            "lifetime": None},

    "water":   {"colour": (0, 0, 255), 
                "colourMode": "temperature",
                "direction": -1, 
                "type": 2, 
                "flammable": -1,  
                "temperature": 23.0,
                "phaseChangeTemp": [0, 100],
                "nextPhase": ["ice", "steam"],
                "thermalConductivity": 0.5,
                "density": 1,
                "lifetime": None},

    "ice":   {"colour": ([120, 150], [120, 150], 255), 
                "direction": 0, 
                "type": 1, 
                "flammable": -1,  
                "temperature": -30.0,
                "phaseChangeTemp": [None, 0],
                "nextPhase": [None, "water"],
                "thermalConductivity": 0.5,
                "density": 1.1,
                "lifetime": None},
    
    "snow":   {"colour": ([240, 255], [240, 255], 255), 
                "direction": -1, 
                "type": 1, 
                "fill": 0.25,
                "flammable": -1,  
                "temperature": -30.0,
                "phaseChangeTemp": [None, 0],
                "nextPhase": [None, "water"],
                "thermalConductivity": 0.3,
                "density": 0.9,
                "lifetime": None},

    "steam":   {"colour": (200, 200, 255), 
                "colourMode": "temperature",
                "direction": 1, 
                "type": 3, 
                "flammable": -1,  
                "fill": 0.25,
                "temperature": 250.0,
                "phaseChangeTemp": [100, None],
                "nextPhase": ["water", None],
                "thermalConductivity": 0.1,
                "density": 0.5,
                "lifetime": None},
    
    "block":   {"colour": (75, 75, 75), 
                "direction": 0, 
                "type": 0, 
                "flammable": 0, 
                "lifetime": None},
    
    "wood":   {"colour": (100, 75, 25), 
                "direction": 0, 
                "type": 1, 
                "flammable": 1,  
                "temperature": 23,
                "thermalConductivity": 0.3,
                "fuel": 450,
                "lifetime": None},

    "grow":   {"colour": (0, [50, 255], 0), 
                "colourMode": "static", 
                "direction": 0, 
                "type": 1, 
                "flammable": 1,
                "fuel": 100,
                "special": {"name": "grow",
                            "life": 100,
                            "branches": 3}},

    "metal":   {"colour": (50, 50, 75), 
                "colourMode": "blackbody",
                "direction": 0, 
                "type": 1, 
                "flammable": 0,  
                "temperature": 23.0,
                "phaseChangeTemp": [None, 800],
                "nextPhase": [None, "molten_metal"],
                "thermalConductivity": 0.5,
                "density": 2.5,
                "fuel": None,
                "lifetime": None},

    "molten_metal":   {"colour": (50, 50, 75), 
                        "colourMode": "blackbody",
                        "direction": -1, 
                        "type": 2, 
                        "flammable": 0,  
                        "temperature": 1000.0,
                        "phaseChangeTemp": [800, None],
                        "nextPhase": ["metal", None],
                        "thermalConductivity": 0.5,
                        "density": 2.5,
                        "fuel": None,
                        "lifetime": None},

    "rock":   {"colour": ([150, 160], [150, 160], [150, 160]), 
                "colourMode": "blackbody",
                "direction": -1, 
                "type": 1, 
                "flammable": 0,  
                "temperature": 23.0,
                "phaseChangeTemp": [None, 200],
                "nextPhase": [None, "lava"],
                "thermalConductivity": 0.5,
                "density": 2,
                "fuel": None,
                "lifetime": None},

    "oil":   {"colour": (80, 50, 50), 
            "direction": -1, 
            "type": 2, 
            "flammable": 1,  
            "temperature": 23.0,
            "thermalConductivity": 0.4,
            "density": 0.9,
            "fuel": 450,
            "lifetime": None},

    "smoke":   {"colour": (50, 50, 50), 
                "colourMode": "rampDown", 
                "direction": 1, 
                "type": 3, 
                "flammable": -1,  
                "temperature": 23.0,
                "thermalConductivity": 0.01,
                "density": 0.05,
                "lifetime": 100},

    "fire":   {"colour": (245, 145, 100), 
                "colourMode": "noise", 
                "direction": 1, 
                "type": 3, 
                "flammable": 0,  
                "temperature": 600.0,
                "phaseChangeTemp": [150, None],
                "nextPhase": ["smoke", None],
                "thermalConductivity": 0.01,
                "density": 0.8,
                "currentState": "burning",
                "fuel": 30},

    "lava":   {"colour": (255, 200, 150), 
                "colourMode": "noise", 
                "direction": -1, 
                "type": 2, 
                "flammable": 0,  
                "temperature": 1200.0,
                "phaseChangeTemp": [200, None],
                "nextPhase": ["rock", None],
                "thermalConductivity": 0.5,
                "density": 1.4,
                "currentState": "burning",
                "fuel": -1},
    
    "burn":   {"colour": (250, 125, 75), 
                "colourMode": "noise", 
                "direction": None, 
                "type": None, #inherit from object being burnt ?
                "flammable": None, 
                "lifetime": None},

    "cold":   {"colour": (25, 200, 200), 
                "direction": 0, 
                "type": 0, 
                "flammable": -1,  
                "temperature": -1500.0,
                "thermalConductivity": 0.1,
                # "density": 100,
                "lifetime": None},

    "hot":   {"colour": (255, 50, 100), 
                "direction": 0, 
                "type": 0, 
                "flammable": 1,  
                "temperature": 1500.0,
                "thermalConductivity": 0.1,
                # "density": 100,
                "lifetime": None},

    "conduct":   {"colour": (150, 200, 150), 
                    "colourMode": "blackbody",
                    "direction": 0, 
                    "type": 0, 
                    "flammable": 1,  
                    "temperature": 23,
                    "thermalConductivity": 0.5,
                    # "density": 100,
                    "lifetime": None},

    "flip":   {"colour": (200, 255, 0), 
                "colourMode": "static", 
                "direction": 0, 
                "type": 0, 
                "flammable": 0,
                "special": None},

    "clone":   {"colour": (135, 110, 50), 
                "colourMode": "static", 
                "direction": 0, 
                "type": 0, 
                "flammable": 0,
                "special": None},

    "destruct":   {"colour": (50, 50, 50), 
                    "colourMode": "noise", 
                    "direction": 0, 
                    "type": 0, 
                    "flammable": 0,},

    "gas_barrier":   {"colour": (200, 200, 0), 
                    "colourMode": "static", 
                    "direction": 0, 
                    "type": 0, 
                    "flammable": 0,
                    "special": {'blockType': 3}},

    "liquid_barrier":   {"colour": (100, 125, 200), 
                        "colourMode": "static", 
                        "direction": 0, 
                        "type": 0, 
                        "flammable": 0,
                        "special": {'blockType': 2}},

    "solid_barrier":   {"colour": (150, 75, 100), 
                        "colourMode": "static", 
                        "direction": 0, 
                        "type": 0, 
                        "flammable": 0,
                        "special": {'blockType': 1}},

    "p_entrance":   {"colour": (100, 0, 255), 
                    "colourMode": "noise", 
                    "direction": 0, 
                    "type": 0, 
                    "flammable": 0, 
                    "lifetime": None},
    
   "p_exit":   {"colour": (255, 10, 200), 
                    "colourMode": "noise", 
                    "direction": 0, 
                    "type": 0, 
                    "flammable": 0, 
                    "lifetime": None},
}

def getMaterial(name):
    return Material(name, **materials[name])

class Portal:
    def __init__(self):
        self.entrance = None
        self.exit = None

portals = []
current_portal = None
next_is_entrance = True
next_click = True

def create_portal(x, y):
    global current_portal, next_is_entrance, next_click
    if next_click:
        if next_is_entrance:
            current_portal = Portal()
            current_portal.entrance = (x, y)
            area.set(x, y, "p_entrance")  # Set entrance type
            next_is_entrance = False
        else:
            current_portal.exit = (x, y)
            area.set(x, y, "p_exit")  # Set exit type
            portals.append(current_portal)
            next_is_entrance = True
        next_click = False

def delete_portal(x, y):
    for portal in portals:
        if portal[0] == (x, y) or portal[1] == (x, y):
            del(portal)
    

#main loop
if __name__ == '__main__':

    # pygame 
    pygame.init()

    screen = pygame.display.set_mode(((resolution[0] * pixelWidth), (resolution[1] * pixelWidth + guiSize)))
    area = Grid(resolution[0] , resolution[1])

    # Loop until the user clicks the close button.
    done = False
    clock = pygame.time.Clock()

    # Create font data
    font = pygame.font.Font('freesansbold.ttf', 32)

    # init variables
    var_debug = False
    pause = False
    drawmode = -1
    materialSelect = "sand"
    radius = 1.0

    while not done:
        # This limits the while loop to a max of 60 times per second.
        # Leave this out and we will use all CPU we can.
        clock.tick(60)
        screen.fill("black")

        for event in pygame.event.get():  # User did something
            if event.type == pygame.QUIT:  # If user clicked close
                done = True  # Flag that we are done so we exit this loop

            if event.type == pygame.KEYDOWN:
                if event.key == pygame.K_SPACE:
                    pause = not pause
                if event.key == pygame.K_d:
                    var_debug = True

            if event.type == pygame.MOUSEBUTTONDOWN:
                if pygame.mouse.get_pressed()[0]: # Left click
                    drawmode = 1
                elif pygame.mouse.get_pressed()[1]: # middle click
                    drawmode = 2
                elif pygame.mouse.get_pressed()[2]: # Right click
                    drawmode = 0

            if event.type == pygame.MOUSEBUTTONUP:
                drawmode = -1
                next_click = True
        
            if event.type == pygame.MOUSEWHEEL:
                radius += event.y / 3
                if radius < 1:
                    radius = 1

        mouseX, mouseY = pygame.mouse.get_pos() # Get click position

        #gui render and material select
        materialSelect = gui(drawmode, materialSelect, mouseX, mouseY)
        
        mouseX, mouseY = int(mouseX/pixelWidth), int(mouseY/pixelWidth)
        if mouseX < 0 or mouseX >= resolution[0] or mouseY < 0 or mouseY >= resolution[1]:
            drawmode = -1

        if var_debug:
            var_debug = False
            debug(area, mouseX, mouseY)

        if drawmode >= 0:
            if drawmode == 0:
                erase(area, mouseX, mouseY, radius)
            elif drawmode == 2:
                create_portal(mouseX, mouseY)
                drawmode = -1
            else:
                draw(area, mouseX, mouseY, materialSelect, radius)
                

        if not pause: updateGrid(area) 
        display(area)
        pygame.display.update()

        
    