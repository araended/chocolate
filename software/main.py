# main.py
from py_controller import Controller

def main():
    controller = Controller()

    while True:
        buttons = controller.receive_button()

        if buttons:
            print("Pressed:", buttons)

if __name__ == "__main__":
    main()