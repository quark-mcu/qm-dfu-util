qm-dfu-util - Intel® Quark™ Microcontroller Device Firmware Upgrade Utility
###########################################################################

*qm-dfu-util* is a fork of |dfu-util|_, a host side implementation of the `DFU
1.0`_ and `DFU 1.1`_ specification of the USB forum.

DFU is intended to download and upload firmware to devices connected over USB.
It ranges from small devices like micro-controller boards up to mobile phones.
*qm-dfu-util* allows you to download firmware to your Intel® Quark™
Microcontroller by using the serial interface (UART).

For more information about this functionality and how to interface with the
QMSI bootloader, please refer to https://github.com/quark-mcu/qmsi/blob/master/doc/dfu.rst .

.. |dfu-util| replace:: *dfu-util*
.. _dfu-util: http://dfu-util.gnumonks.org
.. _`DFU 1.0`: http://www.usb.org/developers/devclass_docs/usbdfu10.pdf
.. _`DFU 1.1`: http://www.usb.org/developers/devclass_docs/DFU_1.1.pdf
