# GPS clock, satellite view
This project is mainly test for my Nextion library [see GitHub](https://github.com/KJaxaM/NextLibCpp). GPS chip produce very exact PPS (±10ns according to datasheet, ±50ns experimentally), so one can build extreme exact clock.

## Hardware
I use STM32  *Nucleo-F446RE* board, GPS breakout board *FGPMMOPA6H* and 3.2" *Nextion* display.

## Software
Programming language is C++ (compiler GNU v.20) in CubeIDE development tool. I try to use C++ std library both in Nextion library, gps messages parsing and so little \"*low level code*\" as possible.

___

#### Project site: [https://www.jaxasoft.se/gps_clock/gps_clock.html](https://www.jaxasoft.se/gps_clock/gps_clock.html)
#### My email:  <kris@jaxasoft.se>