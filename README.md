# Overview

This project is a part of Bachelor's thesis about implementing an Internet interface, with simple echo server examples.
It implements a simple echo server for STM32F429ZI using ESP8266 module, communicating with it and configuring it via AT commands.
The project structure was generated using STM32CubeIDE, and as such, all of the modifications to it are contained within the
Core/src/main.c file.

# How to use

The solution is targeted for Nucleo-F429ZI board. If a user wants to upload the project to a standalone STM32F429ZI MCU, a reconfiguration in the .ioc file might be necessary. The program communicates with ESP8266 using USART6 and with a computer connected via programming USB with USART3 interface. The module enable line is controled via PB3 GPIO pin.

After a power up or reset, the STM32 microcontroller resets the ESP8266 module and reconfigures it. At the end of the configuration process, the module is told to connect to a WiFi network with specific SSID and password contained within the code, and waits for it to receive an IP address via DHCP service. After this step is complete, the system can operate as a simple echo server.

To oversee the configuration process and the working process, user needs to use a serial port monitor on connected computer (for tests purposes Tera Term was used) with baudrate of 115200. This is necessary to obtain the module's IP address, in order to connect to it with TCP/IP Client. The module operates on port 7. The connection and echo functionality was tested with Hercules Setup Utility.  
