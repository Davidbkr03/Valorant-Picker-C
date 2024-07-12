from asyncio.windows_events import NULL
from time import sleep
import pyautogui
import cv2
import numpy as np
import os
from ctypes import WinDLL
from os import listdir
from os.path import isfile, join
import threading
import msvcrt as msvrct
from pynput import keyboard
import pygetwindow as pgw

def scale_image(image, screen_width):
    # Get the original image dimensions
    height, width = image.shape[:2]
    
    #calculate multiplier based on users screen compared to 2560x1440
    multiplier = screen_width / 2560
    
    #scale the image using the multiplier
    scaled_image = cv2.resize(image, (int(width * multiplier), int(height * multiplier)))
    
    return scaled_image

def show_files(onlyfiles):    
    mypath = "."
    print("Please choose the image you want to search for:")
    for i in range(len(onlyfiles)):
        print(f"{i+1}. {onlyfiles[i].replace('.png', '')}")
        
    #get user input
    while True:
        try:
            picked = int(input("Enter the number of the agent you want to select: "))
            if picked > 0 and picked <= len(onlyfiles):
                break
            else:
                print("Please enter a valid number")
        except:
            user_input = None 
    return picked - 1

def is_app_in_focus(app_name):
    active_window = pgw.getActiveWindow()
    return app_name in active_window.title

def keyboard_listener():
    with keyboard.Listener(on_press=listen_for_cancel) as listener:
        listener.join()

def listen_for_cancel(key):
    global stop_flag
    try:
        if key.char == 'c' and is_app_in_focus("picker"):
            stop_flag = True
            key.char = None
    except:
        pass

def main_task():
    global stop_flag
    #loop
    while True:
        
        #clear screen
        os.system('cls')
        
        # Try to flush the buffer
        while msvrct.kbhit():
            msvrct.getch()#reset stop flag

        stop_flag = False

        user32, kernel32, shcore = WinDLL("user32", use_last_error=True), WinDLL("kernel32", use_last_error=True), WinDLL("shcore", use_last_error=True)

        screen_width, screen_height = pyautogui.size()
        print(f"Screen resolution: {screen_width}x{screen_height}")


        #show the user all the png files in the directory as a nice numbered list to choose from
        onlyfiles = [f for f in listdir(".") if isfile(join(".", f)) and f.endswith('.png')]
        picked = show_files(onlyfiles)



        testing = onlyfiles[picked]
        print(f"Image selected: {testing}")
        print("Press 'c' at any point to choose a different agent")

        # filenames
        image_filename = testing.replace('.png', '')
        second_image = 'LOCKIN'
        initial = 'INITIAL'

        search = False

        # Main loop
        while not stop_flag:
            #check for initial image image on screen
            try:
                scaled = scale_image(cv2.imread(initial + '.png'), screen_width)
                initial_center = pyautogui.locateCenterOnScreen(scaled, confidence=0.8)
                print(f"Initial image found at coordinates: {initial_center}")
                search = True
            except:
                pass
    
            while search == True  and not stop_flag: #search
               print(f"Searching for images")
               testing = False
               # Locates the center of the image
               while search == True  and not stop_flag:
                          try:
                            scaled = scale_image(cv2.imread(image_filename + '.png'), screen_width)
                            image_center = pyautogui.locateCenterOnScreen(scaled, confidence=0.8)
                            scaled = scale_image(cv2.imread(second_image + '.png'), screen_width)
                            second_center = pyautogui.locateCenterOnScreen(scaled, confidence=0.8)
                            search = False
                          except:
                            pass


               try:
                    if image_center:
                        print(f"Image found at coordinates: {image_center}")
                        # Move the mouse to the coordinates of the image
                        pyautogui.moveTo(image_center)
                        #pyautogui.click(clicks=2)# Click the image
                        user32.mouse_event(2, 0, 0, 0, 0)
                        sleep(0.005)
                        user32.mouse_event(4, 0, 0, 0, 0)

                        print(f"Second image found at coordinates: {second_center}")
                        # Move the mouse to the coordinates of the image
                        pyautogui.moveTo(second_center)
                        #pyautogui.click(clicks=2)# Click the image
                        user32.mouse_event(2, 0, 0, 0, 0)
                        sleep(0.005)
                        user32.mouse_event(4, 0, 0, 0, 0)
                    else:
                        print("Image not found on the screen.")
               except:
                    pass
               
def main():
    # Event object used to signal the stop condition to the main task
    stop_event = threading.Event()
    
    stop_flag = False
    
    # Start the keyboard listener thread
    keyboard_thread = threading.Thread(target=keyboard_listener)
    keyboard_thread.start()

    main_task()
    
    keyboard_thread.join()
    print("Program stopped")

    

if __name__ == "__main__":
    main()


