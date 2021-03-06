# Clock_tnt
![Clock-tnt](clock-tnt.png)  
*When your box no longer displays the time, you recycle an old tnt receiver.*  
  
Clock based on a Digital Terrestrial Receiver (TNT) module,  
Brand: SilverCrest  
Model: "SL 65 T" sold by Lidl  

The front panel is composed of:
 - 4 digits 7 segments plus a point (LIM-5622G + 74HC164).
 - 7 push buttons
 - 1 IR receiver (not used)

## Sketch ##
### digits ###
```
  (+)
  5V  o---------------------+--+--+--+
                            |  |  |  |
  /DG4                      |  |  | |/
  #9  o-----------------------------|\ QD4
                            |  |  |  |
                            |  |  |  |
  /DG3                      |  | |/  |
  #8  o--------------------------|\  | QD3
                            |  |  |  |
                            |  |  |  |
  /DG2                      | |/  |  |
  #7  o-----------------------|\  |  | QD2
                            |  |  |  |
                            |  |  |  |
  /DG1                     |/  |  |  |
  #6  o--------------------|\  |  |  | QD1
                            |  |  |  |
                         +--+--+--+--+--+
                         | Digit1,2,3,4 |
                         +---++++++++---+
                             ||||||||
  DSA                      +-++++++++-+
  #4  o------------------->+ 74HC164  |
                           +-+--------+
  CP                         |
  #5  o----------------------+
```
### Buttons ###
```
  /HRM         /HRP         /MNM         /MNP
  #10 o-----+  #11 o-----+  #12 o-----+  #13 o-----+
            |            |            |            |
         [-\          [-\          [-\          [-\
            |            |            |            |
  GND o-----+------------+------------+------------+
```
This project has been developped on an Arduino Leonardo but it can run on many other boards.  
You needs to install the "DS3231" library.
