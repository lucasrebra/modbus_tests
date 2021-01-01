# Modbus_ingen10
Automatización de tests ESP32

## Table of Contents
1. [Características modbus TCP/IP](#caracteristicas-modbus-tcp/ip)
2. [Technologies](#technologies)
3. [Installation](#installation)
4. [Partitions](#partitions)


## General Info
***
Write down the general informations of your project. It is worth to always put a project status in the Readme file. This is where you can add it. 
### Screenshot

## Technologies
***
A list of technologies used within the project:
* [Technologie name](https://example.com): Version 12.3 
* [Technologie name](https://example.com): Version 2.34
* [Library name](https://example.com): Version 1234

## Installation
***
A little intro about the installation. 
```
 git clone https://example.com
 cd ../path/to/the/file
 npm install
 npm start
```
Side information: To use the application in a special environment use ```lorem ipsum``` to start

## Partitions
```
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x1F0000,
app1,     app,  ota_1,   0x200000,0x200000,
```

Se deberá modificar el archivo default.csv localizado en el siguiente entramado de carpetas con la estructura
descrita al inicio de este apartado. Esto se debe a que, al tener que utilizar el wifi y el bluetooth a la vez
requerimos particionar el disco eliminando aquellos apartados del mismo que no sean estrictamente necesarios.

```
~/.platformio/packages/framework-arduinoespressif32/tools/partitions

```

