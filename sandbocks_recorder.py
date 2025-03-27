class recorder:
    def __init__(self, width, height, duration):
        self.width = width
        self.height = height
        self.duration = duration
        self.count = 0
        self.stream = [[[(0, 0, 0) for _ in range(self.height)] for _ in range(self.width)] for _ in range(duration)]
        
    def populate_frame(self, grid):
        if not hasattr(grid, "width") or not hasattr(grid, "height"):
            return
        if not grid.width == self.width or not grid.height == self.height:
            return
        
        if self.count < self.duration:
            for y in range(grid.height):
                for x in range(grid.width):
                    self.stream[self.count][x][y] = grid.get(x, grid.height - 1 - y).colour

            self.count += 1
        else:
            self.output()
        
    def output(self):
        serial_stream = []
        
        for frame in self.stream:
            frame_data = []
            for column in self.stream[0]:
                for row in self.stream[0][0]:
                    frame_data.append(self.stream[frame][column][row])

            serial_stream.append(frame, frame_data)
        
        print(serial_stream)