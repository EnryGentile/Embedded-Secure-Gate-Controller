---
pagetitle: Release Notes for S2-LP SUBG1 Component Driver
lang: en
header-includes: <link rel="icon" type="image/x-icon" href="_htmresc/favicon.png" />
---

::: {.row}
::: {.col-sm-12 .col-lg-4}

<center>
# Release Notes for <mark>S2-LP SUBG1 Driver</mark>
Copyright &copy; [2021]{style="background-color: yellow;"} STMicroelectronics\
    
[![ST logo](_htmresc/st_logo_2020.png)](https://www.st.com){.logo}
</center>


# License

This software component is
licensed by ST under BSD 3-Clause, the "License"; You may not use this component except in compliance
with the License. You may obtain a copy of the License at:

[[BSD 3-Clause]{style="background-color: yellow;"}](https://opensource.org/licenses/BSD-3-Clause)

# Purpose

The S2-LP SUBG1 Component Driver is a driver for the STM32Cube. This software implements a Component Driver running on STM32 for STM's Radio S2-LP Low data rate and low power sub-1GHz transceiver devices.



Here is the list of references to user documents:

- [DS11896 Ultra-low power, high performance, sub-1GHz transceiver](https://www.st.com/resource/en/datasheet/s2-lp.pdf)

:::

::: {.col-sm-12 .col-lg-8}
# Update History
::: {.collapse}
<input type="checkbox" id="collapse-section1" checked aria-hidden="true">
<label for="collapse-section1" aria-hidden="true">__1.2.0 / April 14th 2021__</label>
<div>			

## Main Changes

### Maintenance release

This version is derived from S2-LP_DK_1.3.0 with some bug-fixes


: Fixed bugs list

  Headline
  ----------------------------------------------------------
  Fixed Radio Refresh Status implementation
  Fixed Packet Basic Address Init
  Fixed SQI API
  Fixed S2LPGetPreambleLength() API
  Fixed a warning in S2LP_Utils.c
  Fixed a warning in S2LP_Gpio.c


## Contents

<small>The components flagged by "[]{.icon-st-update}" have changed since the
previous release. "[]{.icon-st-add}" are new.</small>

  Name                                                        Version                                           License                                                                                                       Release note
  ----------------------------------------------------------- ------------------------------------------------- ------------------------------------------------------------------------------------------------------------- ------------------------------------------------------------------------------------------------------------------------------------------------
  **S2-LP SUBG1 driver**                                      V1.2.0 []{.icon-st-update}                        [[BSD 3-Clause]](https://opensource.org/licenses/BSD-3-Clause)                                                [release note](.\Release_Notes.html)

  : Components

\

## Known Limitations


  Headline
  ----------------------------------------------------------
  No known limitations
  
\

## Development Toolchains and Compilers

- IAR System Workbench v8.50.9
- ARM Keil v5.32
- STM32CubeIDE v1.6.1


## Supported Devices and Boards

- X-NUCLEO-S2868A1 rev. XS2868V1
- X-NUCLEO-S2868A2 rev. XS2868V2
- X-NUCLEO-S2915A1 rev. XS2915V1


## Backward Compatibility

N/A


</div>
:::

:::
:::

<footer class="sticky">
::: {.columns}
::: {.column width="95%"}
For complete documentation on **S2-LP** ,
visit: [[S2-LP](https://www.st.com/content/st_com/en/products/wireless-transceivers-mcus-and-modules/sub-1ghz-rf/s2-lp.html)]
:::
::: {.column width="5%"}

:::
:::
</footer>
