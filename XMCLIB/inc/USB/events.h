/**
 * @file events.h
 * @date 2015-06-20
 * NOTE:
 * This file is generated by DAVE. Any manual modification done to this file will be lost when the code is regenerated.
 *
 * @cond
 ***********************************************************************************************************************
 * USBD v4.0.6 - The USB core driver for XMC4000 family of controllers. It does the USB protocol handling.
 *
 * Copyright (c) 2015, Infineon Technologies AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,are permitted provided that the
 * following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this list of conditions and the  following
 *   disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
 *   following disclaimer in the documentation and/or other materials provided with the distribution.
 *
 *   Neither the name of the copyright holders nor the names of its contributors may be used to endorse or promote
 *   products derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE  FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY,OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT  OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * To improve the quality of the software, users are encouraged to share modifications, enhancements or bug fixes
 * with Infineon Technologies AG (dave@infineon.com).
 ***********************************************************************************************************************
 *
 * Change History
 * --------------
 *
 * 2015-02-16:
 *     - Initial version.
 * 2015-06-20:
 *     - Updated the file header.
 *
 * @endcond
 *
 */
/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

#ifndef USBEVENTS_H
#define USBEVENTS_H

/* Includes: */
#include <common.h> /* IFX */
#include <usb_mode.h>

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif


/** Event for USB device connection. This event fires when the
 * micro controller is in USB Device mode and the device is
 * connected to a USB host, beginning the enumeration process
 * measured by a rising level on the microcontroller's VBUS sense
 * pin.
 *
 *  This event is time-critical; exceeding OS-specific delays within
 *   this event handler (typically of around two seconds) will
 *   prevent the device from enumerating correctly.
 *
 *
 *  \see \ref Group_USBManagement for more information on the USB
 *  management task and reducing CPU usage.
 */
void EVENT_USB_Device_Connect (void);

/** Event for USB device disconnection. This event fires when the
 * microcontroller is in USB Device mode and the device is
 *  disconnected from a host, measured by a falling level on the
 *  microcontroller's VBUS sense pin.
 *
 *  \see \ref Group_USBManagement for more information on the USB
 *  management task and reducing CPU usage.
 */
void EVENT_USB_Device_Disconnect (void);

/** Event for control requests. This event fires when a the USB host
 *  issues a control request to the mandatory device control
 *  endpoint (of address 0). This may either be a standard
 *  request that the library may have a handler code for internally,
 *   or a class specific request issued to the device which must be
 *   handled appropriately. If a request is not processed in the
 *  user application via this event, it will be passed to the
 *  library for processing internally  if a suitable handler exists.
 *
 *  This event is time-critical; each packet within the request
 *  transaction must be acknowledged or sent within 50ms or the host
 *   will abort the transfer.
 *
 *  The library internally handles all standard control requests
 *  with the exceptions of SYNC FRAME,  SET DESCRIPTOR and
 *  SET INTERFACE. These and all other non-standard control requests
 *   will be left for the user to process via this event if desired.
 *    If not handled in the user application or by the library
 *    internally, unknown requests are automatically STALLed.
 *
 *  \note This event does not exist if the \c USB_HOST_ONLY token is
 *   supplied to the compiler (see  \ref Group_USBManagement
 *   documentation). \n\n
 *
 *  \note Requests should be handled in the same manner as described
 *   in the USB 2.0 Specification, or appropriate class
 *   specification. In all instances, the library has already read
 *   the request SETUP parameters into the \ref USB_ControlRequest
 *   structure which should then be used by the application to
 *   determine how to handle the issued request.
 */
void EVENT_USB_Device_ControlRequest (void);

/** Event for USB configuration number changed. This event fires
 *  when a the USB host changes the selected configuration number
 *  while in device mode. This event should be hooked in device
 *  applications to create the endpoints and configure the device
 *  for the selected configuration.
 *
 *  This event is time-critical; exceeding OS-specific delays within
 *   this event handler (typically of around
 *  one second) will prevent the device from enumerating correctly.
 *
 *  This event fires after the value of
 *  USB_Device_ConfigurationNumber has been changed.
 *
 *  \note This event does not exist if the \c USB_HOST_ONLY token is
 *   supplied to the compiler (see  \ref Group_USBManagement
 *   documentation).
 */
void EVENT_USB_Device_ConfigurationChanged (void);

/** Event for USB suspend. This event fires when a the USB host
 * suspends the device by halting its transmission of Start Of Frame
 *  pulses to the device. This is generally hooked in order to move
 *  the device over to a low power state until the host wakes up the
 *   device.
 *
 */
void EVENT_USB_Device_Suspend (void);

/** Event for USB wake up. This event fires when a the USB interface
 *  is suspended while in device mode, and the host wakes up the
 *  device by supplying Start Of Frame pulses. This is generally
 *  hooked to pull the user application out of a low power state and
 *   back into normal operating mode.
 *
 *  \see \ref EVENT_USB_Device_Suspend() event for accompanying
 *  Suspend event.
 */
void EVENT_USB_Device_WakeUp (void);

/** Event for USB interface reset. This event fires when the USB
 * interface is in device mode, and a the USB host requests that the
 *  device reset its interface. This event fires after the control
 *  endpoint has been automatically configured by the library.
 *
 *  This event is time-critical; exceeding OS-specific delays within
 *   this event handler (typically of around  two seconds) will
 *   prevent the device from enumerating correctly.
 *
 */
void EVENT_USB_Device_Reset (void);

/** Event for USB Start Of Frame detection, when enabled. This event
 *  fires at the start of each USB frame, once per millisecond, and
 *  is synchronized to the USB bus. This can be used as an accurate
 *  millisecond timer source when the USB bus is enumerated in
 *  device mode to a USB host.
 *
 *  This event is time-critical; it is run once per millisecond and
 *  thus long handlers will significantly degrade device
 *  performance. This event should only be enabled when needed to
 *  reduce device wake-ups.
 *
 */
void EVENT_USB_Device_StartOfFrame (void);

/* Event handler for the library USB Control Request reception
 * event. */
void EVENT_USB_Device_SetAddress (void);



/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

#endif

/** @} */

