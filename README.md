It's free firmware for the Allwinner H3 SoC's co-processor (ARISC)
---
* This firmware uses to make a real-time ``GPIO`` pulses generation and counting.
* This firmware can be used for the any ``CNC`` applications - ``STEP/DIR`` and ``PWM`` generation, 
  ``ABZ`` encoders counting.

How to build?
---
* You'll need any ``Linux OS`` and a ``custom toolchain``.
* Download the toolchain binaries from here - https://github.com/openrisc/newlib/releases
* Unpack toolchain binary files into the ``/opt/toolchains/or1k-elf`` folder
* Clone this repo to any folder:
  ``$ git clone https://github.com/orange-cnc/arisc_firmware.git``
* Build the firmware by the ``make all`` command

How to use?
---
* You'll need any ``Orange Pi`` board with ``Alwinner H3 SoC`` and any ``Linux OS`` built by ``armbian``.
  SD images can be found here - https://github.com/orange-cnc/armbian_build/releases, 
  and here - https://www.armbian.com/download/.
* Copy ``arisc-fw.code`` binary file and all files from repo's folder ``/loader`` 
  into the ``/boot`` folder of your ``Armbian OS``.
* Restart your ``Orange Pi`` board.
* Clone arisc linux API repo to any folder of your ``Armbian OS``: 
  ``$ git clone https://github.com/orange-cnc/arisc_api.git``
* Build arisc linux API by the ``make all`` command
* Run arisc linux API:
  ``$ ./arisc``
