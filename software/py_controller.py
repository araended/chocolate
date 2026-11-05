# controller.py
import pygame

class Controller:
    def __init__(self):
        pygame.init()
        pygame.joystick.init()

        if pygame.joystick.get_count() == 0:
            raise Exception("No controller connected")

        self.joystick = pygame.joystick.Joystick(0)
        self.joystick.init()

    def receive_button(self):
        pygame.event.pump()

        pressed_buttons = []

        for i in range(self.joystick.get_numbuttons()):
            if self.joystick.get_button(i):
                pressed_buttons.append(i)

        return pressed_buttons