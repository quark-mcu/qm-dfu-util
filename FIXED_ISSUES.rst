Fixed Issues
************

Issues fixed since version 1.3.0.

=========== ====================================================================
Issue       No support for COM ports >= COM10 on Windows
----------- --------------------------------------------------------------------
Implication On Windows, when the used serial port is equal to or greater than
            COM10, the error "Cannot open serial device" is returned.
----------- --------------------------------------------------------------------
Workaround  Users should change the COM port number for their device to
            something less than COM10. This can be done via the Windows Device
            Manager by editing the advanced settings of the serial port.
=========== ====================================================================

