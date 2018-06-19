# Handwheel
internal Arduino Controller for Estlcam handwheel
to be used with Estlcam v11 and higher[1].
# Purpose
estlcam is a great tool to controll your CNC router.
One feature is a handwheel which controlls the basic funktionality without the need of Keyboard or mouse.
But for me it was not enough to controll Estlcam with the handwheel, i desired to controll up to four mor relais outputs.
due to the lack of wires in the cable i needed a controller placed inside the handwheel to
  - Toggle Relais states (only Relais 1-3)
    - press of button AUX1-3 changes State of Relais 1-3 
    - LEDs arround Buttons show Relais states
    - default all Relais off
 - control LEDs of Estlcam controlled Buttons (except X,Y,Z)
   - select (cycle) modes using AUX4 Button
     - All LEDs ON (default)
     - only OK
     - PGM Start/Stop and Spindle Start/Stop        
     - All OFF
# Wireing
See source cod to see how everything must be wired together
# Estlcam Homepage   
[1] https://www.estlcam.de

