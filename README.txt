________________________________________________________________________________
________________________________________________________________________________
  _____                        __      
 / ___/__ ___ _  ___ ______ __/ /  ___ 
/ (_ / _ `/  ' \/ -_) __/ // / _ \/ -_)
\___/\_,_/_/_/_/\__/\__/\_,_/_.__/\__/ 
                                       
  _____          __           ____          ___     __          __         
 / ___/__  ___  / /________  / / /__ ____  / _ |___/ /__ ____  / /____ ____
/ /__/ _ \/ _ \/ __/ __/ _ \/ / / -_) __/ / __ / _  / _ `/ _ \/ __/ -_) __/
\___/\___/_//_/\__/_/  \___/_/_/\__/_/   /_/ |_\_,_/\_,_/ .__/\__/\__/_/   
                                                       /_/                 
  __  _________    ___      _             
 / / / / __/ _ )  / _ \____(_)  _____ ____
/ /_/ /\ \/ _  | / // / __/ / |/ / -_) __/
\____/___/____/ /____/_/ /_/|___/\__/_/   
________________________________________________________________________________
________________________________________________________________________________

________________________________________________________________________________
ABOUT:
________________________________________________________________________________

This is a Windows KMDF driver, working as a filter driver installed below the
Microsoft supplied HID class driver. This driver should work for all Windows
versions newer than 7 that support the KMDF framework. Earlier versions of
Windows need a minidriver to forward HID requests and needs to be written in
WDM.

________________________________________________________________________________
INSTALLATION:
________________________________________________________________________________

See INSTALL.txt for installation instructions. There currently is no installer,
so the driver must be installed manually. Also note that the driver, by default,
is not signed, or is test signed, so it will not run in a normal run mode of
Windows. Windows must be started in Test mode, and if the driver is unsigned,
signature checking must also be disabled.

________________________________________________________________________________
FEATURES:
________________________________________________________________________________

1. Support for all 4 controllers simultaneously.
2. Simple linear deadzone handling.
3. Supports recalibrating controllers by pressing the x-y-start button
	combination for three seconds and then letting go.
4. Device interface available for use with DeviceIoControl commands, including
	one for rumbling. 

The driver registers itself as four USB HID controllers, which appear regardless
of the actual state of the controllers. Now, plugging in a controller should
cause the driver to begin to feed its data to the corresponding HID controller.

Be aware, just as with the Gamecube the controller, the driver calibrates the
controller when it is plugged in. If any of the analog buttons or axis are
touched during this process, the controller may be mis-calibrated. One can
disconnect and reconnect the controller to re-calibrate, or press the x-y-start
button combination for three seconds and then let go to recalibrate the
controller.

The driver, by default, applies a little bit of deadzone handling, but does
little else in terms of processing raw input from the controllers. From
experimentation, it seems that the actual numbers coming in from the analog
devices in the controller vary from one controller to another by a decent
amount. As such, for Windows it is recommended to calibrate each controller in
the Game Controllers window.

For the programming inclined, the driver exposes an interface that can be
accessed view CreateFile and accepts DeviceIoControl codes to trigger a
recalibration, to set parameters with regards to deadzone handling, and to
trigger rumbling. See the ConsoleClient project in the source code tree for
an example of how to use the IOCTL commands. Currently, any altered settings do
not persist after a disconnect of the adapter.

The ConsoleClient application included in the source code allows for triggering
recalibration for any controller, modifying deadzone parameters (which algorithm
to use and what value to use as a threshold for all analog inputs), and for fun
toggling rumble. Note that for rumble to work the gray USB connector (which
supplies power for rumble) must be connected.

________________________________________________________________________________
LICENSING:
________________________________________________________________________________

This software is dual-licensed under the LGPLv3 and GPLv2 or later. See the
COPYING* files for copies of the licenses this driver is released under.

I am not closed to the idea of licensing this software under a BSD license if
someone were to present me a convincing enough argument, so ask me if there are
any questions or concern. I can be contacted with questions about licensing at
gabemarcano at yahoo dot com.
