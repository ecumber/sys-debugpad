# sys-hidplus
A Nintendo Switch sysmodule that emulates a debug controller

# Why does this exist?
Some Switch developer units (SDEV) have a special controller port on the back. This port is actually meant for a Wii Classic Controller Pro, presumably because the port had existed for a while and was simpler to implement.
Certain games such as the Mario Kart 8 Deluxe prototype have support for this controller and lock certain features behind it. While a game-specific cheat could be used to reroute debug controller to a normal one, the problem is it's game-specific and won't work for anything else. This sysmodule emulates a debug controller directly and works on any game that supports it.

# Disclaimer
Bugs occur and I'm in no way responsible of any damage caused by this sysmodule or its misuse. If you get any issue, please report it on the Issues tab.


# Instructions
First, copy the contents of the nswitch folder in the build zip file to the Switch microSD card. Make sure to use Atmosphere because this sysmodule isn't tested with other CFWs.

After that, make sure you have Python3 installed and open cmd on the location of the extracted pc folder from the build zip file and execute `py -3 -m pip install inputs` (you'll have to do this step only once). Once inputs is installed, you can then connect to your switch by using `python3 input_pc.py {SWITCH IP}`, replacing {SWITCH IP} with the IP address of your Switch (You can check it by going to Settings > Internet on your system).

You can modify the behaviour of the controller emulation on the python file by changing the conType values. If you set it to 0, you'll be able to disconnect the controller (useful if the Switch disconnects the controller for some reason). If you set it to 1, you'll be able to emulate a Pro Controller. If you set it to 2 or 3, you'll be able to use the experimental sideways joycon emulation, it has some issues but in some games such as Clubhouse Games, it'll be playable.


# Stuff to do
* Anarchy mode (3 players using 1 single emulated controller)
* Keyboard Compatibility
* Make the compatibility for sideways joycons emulation better
* Add 4th controller emulation
* Add 5-8th controller emulation
* Add gyro compatibility
* Add USB compatibility
* Add rumble support


# Known issues
* Causes a crash if the controller button is held for too long



# Special Thanks
* PaskaPinishkes - original sysmodule
* jakibaki - base and inspiration for this sysmodule (hid-mitm), a lot of the code comes from it
* cathery - helping me with my dumb questions and issues
* ReSwitched Discord Server - help in the support channels
* **Everyone using this sysmodule - thanks a lot!**
