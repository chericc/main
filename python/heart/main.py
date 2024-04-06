import logging
import pygame
import math
import time

RED = (255, 0, 0)
BLACK = (0, 0, 0)


class Point:
    x = 0.0
    y = 0.0

    def __init__(self, x, y):
        self.x = x
        self.y = y





def gen_heart_points(width, height, big):
    points = []
    rad = 0.0
    while rad < 2 * math.pi:
        x = 16 * math.pow(math.sin(rad), 3)
        y = 13 * math.cos(rad) - 5 * math.cos(2 * rad) - 2 * math.cos(3 * rad) - math.cos(4 * rad)
        points.append(Point(x, -y))
        rad += 0.01

    mid_x = width / 2
    mid_y = height / 2

    for point in points:
        point.x *= big
        point.y *= big
        point.x += mid_x
        point.y += mid_y

    return points


def calc_big():
    time_now = time.time()
    time_now_sec = math.floor(time_now)
    time_milli_sec = time_now - time_now_sec
    #          -
    #      -     -
    #   -          -
    # -----
    # (1, 0.51) (0.7, 1)
    # y = 1 - (x-0.7)^2
    # y = 0.49 / 0.3 * (x - 0.7) + 1
    x = time_milli_sec 
    y = 0.0
    if x <= 0.7:
        y = 1 - math.pow(x - 0.7, 2)
    else:
        y = 1 - (0.49 / 0.3) * (x - 0.7)
    logging.debug("x={0},y={1}".format(x,y))
    return y * 3 + 2


def run(surface, clock):

    size = surface.get_size()

    points = gen_heart_points(size[0], size[1], calc_big())

    if len(points) > 0:
        last_point = points[0]
        for point in points:
            if point != last_point:
                pygame.draw.line(surface=surface, color=RED, start_pos=(last_point.x, last_point.y),
                                 end_pos=(point.x, point.y), width=4)
                last_point = point


def game(surface):
    running = True
    clock = pygame.time.Clock()
    while running:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False
        surface.fill(BLACK)
        run(surface, clock)
        pygame.display.flip()
        clock.tick(60)


def main():
    logging.basicConfig(level=logging.DEBUG)
    logging.debug("main start")

    width = 800
    height = 600
    size = (width, height)

    screen = pygame.display.set_mode(size=size, flags=pygame.RESIZABLE)
    pygame.display.set_caption("main")

    game(screen)


if __name__ == '__main__':
    main()
