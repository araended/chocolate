import pygame
import math

pygame.init()
pygame.joystick.init()

if pygame.joystick.get_count() == 0:
    raise Exception("No joystick detected")

j = pygame.joystick.Joystick(0)
j.init()

ORBIT_SPEED       = 0.40   # Overall orbit speed  (0.0 – 1.0)
ORBIT_RADIUS_TUNE = 1.0    # Radius scale; sway/yaw = ORBIT_RADIUS_TUNE
ORBIT_DIRECTION   = 1      # +1 = CCW,  −1 = CW
ORBIT_SURGE_TRIM  = 0.0    # Fine-trim if range drifts; + = move toward object

RADIUS_STEP = 0.1          # Increment for but_4 / but_5
SPEED_STEP  = 0.05         # Increment for but_6 / but_7
RADIUS_MIN  = 0.1
RADIUS_MAX  = 20.0
SPEED_MIN   = 0.05
SPEED_MAX   = 1.0

# L1 / L2 = Roll Gripper (motor A), R1 / R2 = Grip Control (motor B)

def deadzone(v, t=0.1):
    return 0 if abs(v) < t else v

def normalize(vals):
    m = max(abs(v) for v in vals)
    return [v / m if m > 1 else v for v in vals]


# Persistent state 
g = 0
h = 0

orbit_active      = False
orbit_yaw_accum   = 0.0    # accumulated yaw (radians) for 1-orbit stop

prev_hat_y  = 0
prev_hat_x  = 0
prev_but_0  = 0
prev_but_1  = 0
prev_but_2  = 0
prev_but_3  = 0
prev_but_4  = 0
prev_but_5  = 0
prev_but_6  = 0
prev_but_7  = 0


def get_data():
    global g, h, prev_hat_y, prev_hat_x
    global orbit_active, orbit_yaw_accum
    global ORBIT_SPEED, ORBIT_RADIUS_TUNE, ORBIT_SURGE_TRIM
    global prev_but_0, prev_but_1, prev_but_2, prev_but_3
    global prev_but_4, prev_but_5, prev_but_6, prev_but_7

    pygame.event.pump()

    # RAW INPUTS
    x   = deadzone(-j.get_axis(0))
    y   = deadzone(-j.get_axis(1))
    yaw = deadzone(j.get_axis(2))
    heave = 0   # heave lewat D-pad (stik kanan yaw)
    but_0 = j.get_button(0) #triangle + cam
    but_1 = j.get_button(1) #O mati orbit
    but_2 = j.get_button(2) #X nyala orbit
    but_3 = j.get_button(3) #petak -cam
    but_4 = j.get_button(4) #L1 roll +
    but_5 = j.get_button(5) #R1 grip +
    but_6 = j.get_button(6) #L2 roll -
    but_7 = j.get_button(7) #R2 grip -

    hat_x, hat_y = j.get_hat(0)

    # ORBIT START, Button 2, rising edge
    if but_2 == 1 and prev_but_2 != 1:
        orbit_active    = True
        orbit_yaw_accum = 0.0
        print(f"[ORBIT] Started  | speed={ORBIT_SPEED:.2f}  radius_tune={ORBIT_RADIUS_TUNE:.2f}")

    # ORBIT ABORT, Button 1, rising edge
    if but_1 == 1 and prev_but_1 != 1:
        if orbit_active:
            orbit_active = False
            print(f"[ORBIT] Aborted  | yaw_accum={math.degrees(orbit_yaw_accum):.1f}°")

    # THRUSTER MIX 
    if orbit_active:
        # Orbital commands, mathematically coupled (one radius tuning value)
        orb_sway  = ORBIT_DIRECTION * ORBIT_SPEED
        orb_yaw   = ORBIT_DIRECTION * (ORBIT_SPEED / ORBIT_RADIUS_TUNE)
        orb_surge = ORBIT_SURGE_TRIM

        #      Surge        Sway        Yaw
        FL =  orb_surge + orb_sway  + orb_yaw
        FR =  orb_surge - orb_sway  - orb_yaw
        BL = -orb_surge + orb_sway  - orb_yaw
        BR = -orb_surge - orb_sway  + orb_yaw

        # Accumulate yaw angle to detect one full orbit (2π rad)
        orbit_yaw_accum += abs(orb_yaw)
        if orbit_yaw_accum >= 2 * math.pi:
            orbit_active = False
            print(f"[ORBIT] Completed one full orbit  | speed={ORBIT_SPEED:.2f}  radius_tune={ORBIT_RADIUS_TUNE:.2f}")

    else:
        # Manual mode
        FL =  y - x + yaw
        BR = -y + x + yaw
        FR =  y + x - yaw
        BL = -y - x - yaw

        # FL =  y 
        # FR =  y 
        # BL = -y 
        # BR = -y
        # if BL >0 and BR >0 :
        #     FL=0
        #     FR=0
        # if x != 0:
        #     if x > 0 :
        #         FL =  y + x 
        #         BL = -y + x 
        #     else :
        #         FR =  y - x 
        #         BR = -y - x
        # if yaw != 0 :
        #     if yaw > 0:
        #         FL =  y + x + yaw
        #         BR = -y - x + yaw
        #     else:
        #         FR =  y - x - yaw
        #         BL = -y + x - yaw
        
    FL, FR, BL, BR = normalize([FL, FR, BL, BR])     

    # VERTICAL 
    VL = heave
    VR = heave
    VL, VR = normalize([VL, VR]) 

    # BUTTON COUNTERS  h (but_0 / but_3) 
    if but_0 == 1 and prev_but_0 != 1:
        h = min(h + 10, 180)

    if but_3 == 1 and prev_but_3 != 1:
        h = max(h - 10, 0)

    # HAT COUNTER  g (hat_x) 
    if hat_y == 1 and prev_hat_y != 1:
        g += 5
    elif hat_y == -1 and prev_hat_y != -1:
        g -= 5

    # D-pad kiri/kanan = Heave Control (langkah halus +-1)
    if hat_x == 1 and prev_hat_x != 1:
        g += 1
    elif hat_x == -1 and prev_hat_x != -1:
        g -= 1

    # STORE PREVIOUS BUTTON STATES 
    prev_but_0 = but_0
    prev_but_1 = but_1
    prev_but_2 = but_2
    prev_but_3 = but_3
    prev_but_4 = but_4
    prev_but_5 = but_5
    prev_but_6 = but_6
    prev_but_7 = but_7
    prev_hat_y = hat_y
    prev_hat_x = hat_x

    BL = int(BL * 400 + 1500)
    BR = int(BR * 400 + 1500)
    # if BL >= 1500:
    #     BL = ((BL-1500)*2)+1000
    # else :
    #     BL = 1000

    # if BR >= 1500:
    #     BR = ((BR-1500)*2)+1000
    # else :
    #     BR = 1000

    # PWM SCALING 
    a = int(FL * 400 + 1500)
    b = int(FR * 400 + 1500)
    c = int(BL)
    d = int(BR)
    e = int(VL * 400 + 1500)
    f = int(VR * 400 + 1500)
    if yaw !=0 :
        e = 1500
        f = 1500
    if g !=0 :
        e = 1500 + g
        f = 1500 + g

    # ARM (L298N): -1 / 0 / +1 
    roll = int(but_4) - int(but_6)   # L1 = +1, L2 = -1 -> motor A (wrist)
    grip = int(but_5) - int(but_7)   # R1 = +1, R2 = -1 -> motor B (gripper)

    return [a, b, c, d, e, f, g, h, roll, grip]
