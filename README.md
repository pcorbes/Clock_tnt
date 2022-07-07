# Clock_tnt
Clock based on a Digital Terrestrial Receiver (TNT) module,
Brand: SilverCrest
Model: "SL 65 T" sold by Lidl

The front panel is composed of:
 - 4 digits 7 segments plus a point (LIM-5622G + 74HC164).
 - 7 push buttons
 - 1 IR receiver (not used)

 ## Sketch ##
 ```
  ### digits ###
  (+)
     °---------------------------------+
                                       |
  /DG1, /DG2, /DG3, /DG4             |/
     °-------------------------------|\
                                       |
                               +-----++++-----+
                               | Digit1,2,3,4 |
                               +---++++++++---+
                                   ||||||||
  DSA                            +-++++++++-+
     °-------------------------->+ 74HC164  |
                                 +-+--------+
  CP                               |
     °-----------------------------+
```
  ### Buttons ###
```
  /HRM, /HRP, /MNM, /MNP
     °----------------------------------+
                                        |
                                     #-\
  GND                                   |
     °----------------------------------+
```
This project has been developped on an Arduino Leonardo but it can run on many other boards.
You needs to install the "DS3231" library.
