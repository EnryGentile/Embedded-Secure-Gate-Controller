---
pagetitle: Release Notes for Contiki STM32 Library Middleware
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for <mark>Contiki STM32 Library Middleware</mark>
Copyright &copy; [2021]{style="background-color: yellow;"} STMicroelectronics\
    
[![ST logo](_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>

# License

This software package is
licensed by ST under SLA0055, the "License"; You may not use this component except in compliance
with the License. You may obtain a copy of the License at:


[[SLA0055: SOFTWARE LICENSE AGREEMENT]{style="background-color: yellow;"}](http://www.st.com/SLA0055)


# Purpose

Contiki STM32 Library is a middleware for the the X-CUBE-SUBG2 expansion software.
This middleware provides the ST platform specific implementation for Contiki6LP Middleware.

Contiki_STM32_Library software features:

 - Binding to support STM F4, L4 and L1 in Contiki6LP middleware.
 - Binding to support SPIRIT1 and S2LP SubGHz radios in Contiki6LP middleware.

:::

::: {.col-sm-12 .col-lg-8}
# Update History
::: {.collapse}
<input type="checkbox" id="collapse-section4" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__1.3.0 / April 19th 2021__</label>
<div>			

## Main Changes

### Product update

 - Changed radio_arch_refresh_status() implementation 
 - Several fixes to radio_driver

## Contents
 - Contiki target files for ST platform, modified to fit STM32Cube environment.

## Development Toolchains and Compilers

 - IAR Embedded Workbench for ARM (EWARM) toolchain v8.50.9 + ST-LINK
 - Keil Microcontroller Development Kit (MDK-ARM) toolchain v5.32 + ST-LINK
 - STM32CubeIDE toolchain v1.6.1 + ST-LINK

## Supported Devices and EVAL boards

 - STM32F401RE and STM32L152RE devices
 - NUCLEO-F401RE Board RevC
 - NUCLEO-L152RE Board RevC
 - X-NUCLEO-IDS01A4 Board
 - X-NUCLEO-IDS01A5 Board
 - X-NUCLEO-S2868A1 Board
 - X-NUCLEO-S2915A1 Board

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__1.2.0 / December 16th 2019__</label>
<div>			

## Main Changes

### Product update

 - Added support for X-NUCLEO-S2915A1
 - New expansion board X-NUCLEO-S2868A2 is automatically supported because pin to pin compatibile with X-NUCLEO-S2868A1

## Contents
 - Contiki target files for ST platform, modified to fit STM32Cube environment.

## Development Toolchains and Compilers

 - IAR Embedded Workbench for ARM (EWARM) toolchain v8.20.2 + ST-LINK
 - Keil Microcontroller Development Kit (MDK-ARM) toolchain v5.24.2 + ST-LINK
 - System Workbench for STM32 (SW4STM32) toolchain v1.16.0 (IDE 2.5.0) + ST-LINK

## Supported Devices and EVAL boards

 - STM32F401RE and STM32L152RE devices
 - NUCLEO-F401RE Board RevC
 - NUCLEO-L152RE Board RevC
 - X-NUCLEO-IDS01A4 Board
 - X-NUCLEO-IDS01A5 Board
 - X-NUCLEO-S2868A1 Board
 - X-NUCLEO-S2915A1 Board

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__1.1.1 / June 1st 2019__</label>
<div>			

## Main Changes

### Product update

 - Fixed radio driver that caused a problem to Border Router for NUCLEO-F401RE platform
 - Changed TIM implementation for rtimer-arch.

## Contents
 - Contiki target files for ST platform, modified to fit STM32Cube environment.

## Development Toolchains and Compilers

 - IAR Embedded Workbench for ARM (EWARM) toolchain v8.20.2 + ST-LINK
 - Keil Microcontroller Development Kit (MDK-ARM) toolchain v5.24.2 + ST-LINK
 - System Workbench for STM32 (SW4STM32) toolchain v1.16.0 (IDE 2.5.0) + ST-LINK

## Supported Devices and EVAL boards

 - STM32F401RE and STM32L152RE devices
 - NUCLEO-F401RE Board RevC
 - NUCLEO-L152RE Board RevC
 - X-NUCLEO-IDS01A4 Board
 - X-NUCLEO-IDS01A5 Board
 - X-NUCLEO-S2868A1 Board
 - X-NUCLEO-S2915A1 Board

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__1.1.0 / December 10th 2018__</label>
<div>			

## Main Changes

### Product update

 - Added Low Power support for S2-LP radio (SNIFF Mode and, still experimental ContikiMAC)
 - Refactoring/renaming of some files

## Contents
 - Contiki target files for ST platform, modified to fit STM32Cube environment.

## Development Toolchains and Compilers

 - IAR Embedded Workbench for ARM (EWARM) toolchain v8.20.2 + ST-LINK
 - Keil Microcontroller Development Kit (MDK-ARM) toolchain v5.24.2 + ST-LINK
 - System Workbench for STM32 (SW4STM32) toolchain v1.16.0 (IDE 2.5.0) + ST-LINK

## Supported Devices and EVAL boards

 - STM32F401RE and STM32L152RE devices
 - NUCLEO-F401RE Board RevC
 - NUCLEO-L152RE Board RevC
 - X-NUCLEO-IDS01A4 Board
 - X-NUCLEO-IDS01A5 Board
 - X-NUCLEO-S2868A1 Board
 - X-NUCLEO-S2915A1 Board

</div>
:::

::: {.collapse}
<input type="checkbox" id="collapse-section4" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__1.0.0 / August 10th 2018__</label>
<div>			

## Main Changes

### Product update

 - First release

</div>
:::

:::
:::

<footer class="sticky">
::: {.columns}
::: {.column width="95%"}
:::
::: {.column width="5%"}

:::
:::
</footer>
