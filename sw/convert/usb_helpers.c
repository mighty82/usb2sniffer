//-----------------------------------------------------------------
//                       USB Sniffer
//                           V0.1
//                     Ultra-Embedded.com
//                    Copyright 2015-2020
//
//               Email: admin@ultra-embedded.com
//
//                       License: LGPL
//-----------------------------------------------------------------
//
// Copyright (C) 2015 - 2020 Ultra-Embedded.com
//
// This source file may be used and distributed without         
// restriction provided that this copyright statement is not    
// removed from the file and that any derivative work contains  
// the original copyright notice and the associated disclaimer. 
//
// This source file is free software; you can redistribute it   
// and/or modify it under the terms of the GNU Lesser General   
// Public License as published by the Free Software Foundation; 
// either version 2.1 of the License, or (at your option) any   
// later version.
//
// This source is distributed in the hope that it will be       
// useful, but WITHOUT ANY WARRANTY; without even the implied   
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      
// PURPOSE.  See the GNU Lesser General Public License for more 
// details.
//
// You should have received a copy of the GNU Lesser General    
// Public License along with this source; if not, write to the 
// Free Software Foundation, Inc., 59 Temple Place, Suite 330, 
// Boston, MA  02111-1307  USA
//-----------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "log_format.h"
#include "usb_helpers.h"

//-----------------------------------------------------------------
// Tables:
//-----------------------------------------------------------------

/** CRC table for the CRC-16. The poly is 0x8005 (x^16 + x^15 + x^2 + 1) */
const uint16_t crc16_table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040
};

//-----------------------------------------------------------------
// usb_swap8
//-----------------------------------------------------------------
static uint8_t usb_swap8(uint8_t b) 
{
   b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
   b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
   b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
   return b;
}
//-----------------------------------------------------------------
// usb_swap5
//-----------------------------------------------------------------
static uint8_t usb_swap5(uint8_t value)
{
    return (
        ((value & 0x01) << 4) |
        ((value & 0x02) << 2) |
         (value & 0x04) |
        ((value & 0x08) >> 2) |
        ((value & 0x10) >> 4)
    );
}
//-----------------------------------------------------------------
// usb_crc5
//-----------------------------------------------------------------
static uint8_t usb_crc5(uint16_t data)
{
    uint32_t  a;
    uint32_t  crc;
    uint32_t  crc_n;
    int bits = 11;

    a   = (usb_swap8(data) << 8) | (usb_swap8((data >> 8) & 0x7));
    crc = (0x1f << 11);
    
    while (bits--) {
        crc_n   = (crc << 1) & 0xffff;
        crc_n  ^= ((a ^ crc) & (1<<15)) ? (0x05 << 11) : 0;
        crc     = crc_n;
        a       = (a << 1) & 0xffff;
    }
    
    crc >>= 11;
    crc ^= 0x1f;

    return (uint8_t)usb_swap5(crc);
}
//-----------------------------------------------------------------
// usb_get_crc16: Calculate expected CRC for block of data
//-----------------------------------------------------------------
uint16_t usb_get_crc16(uint8_t *buffer, int len)
{
    uint16_t crc = 0xFFFF;

    while (len--)
        crc = (crc >> 8) ^ crc16_table[(crc ^ (*buffer++)) & 0xff];

    return crc ^ 0xFFFF;
}

//-----------------------------------------------------------------
// usb_get_pid
//-----------------------------------------------------------------
uint8_t usb_get_pid(uint32_t value)
{
    uint8_t pid = ((value >> LOG_TOKEN_PID_L) & LOG_TOKEN_PID_MASK);
    pid |= (~(pid << 4)) & 0xF0;
    return pid;
}
//-----------------------------------------------------------------
// usb_get_cycle_delta
//-----------------------------------------------------------------
uint16_t usb_get_cycle_delta(uint32_t value)
{
    return ((value >> LOG_CTRL_CYCLE_L) & LOG_CTRL_CYCLE_MASK) << 8;
}
//-----------------------------------------------------------------
// usb_get_rst_state
//-----------------------------------------------------------------
int usb_get_rst_state(uint32_t value)
{
    return ((value >> LOG_RST_STATE_L) & LOG_RST_STATE_MASK) << 8;
}
//-----------------------------------------------------------------
// usb_get_token_device
//-----------------------------------------------------------------
uint8_t usb_get_token_device(uint32_t value)
{
    uint16_t data = ((value >> LOG_TOKEN_DATA_L) & LOG_TOKEN_DATA_MASK);
    return data & 0x7F;
}
//-----------------------------------------------------------------
// usb_get_token_endpoint
//-----------------------------------------------------------------
uint8_t usb_get_token_endpoint(uint32_t value)
{
    uint16_t data = ((value >> LOG_TOKEN_DATA_L) & LOG_TOKEN_DATA_MASK);
    return (data >> 7) & 0xF;
}
//-----------------------------------------------------------------
// usb_get_sof_crc5
//-----------------------------------------------------------------
uint8_t usb_get_sof_crc5(uint32_t value)
{
    return usb_crc5(usb_get_sof_frame(value));
}
//-----------------------------------------------------------------
// usb_get_token_crc5
//-----------------------------------------------------------------
uint8_t usb_get_token_crc5(uint32_t value)
{
    uint16_t data = ((value >> LOG_TOKEN_DATA_L) & LOG_TOKEN_DATA_MASK);
    return usb_crc5(data);
}
//-----------------------------------------------------------------
// usb_get_data_length
//-----------------------------------------------------------------
uint16_t usb_get_data_length(uint32_t value)
{
    return ((value >> LOG_DATA_LEN_L) & LOG_DATA_LEN_MASK);
}
//-----------------------------------------------------------------
// usb_get_sof_frame
//-----------------------------------------------------------------
uint16_t usb_get_sof_frame(uint32_t value)
{
    return ((value >> LOG_SOF_FRAME_L) & LOG_SOF_FRAME_MASK);
}
//-----------------------------------------------------------------
// usb_get_split_hub_addr
//-----------------------------------------------------------------
uint8_t usb_get_split_hub_addr(uint32_t value)
{
    uint16_t data = ((value >> LOG_SPLIT_DATA_L) & LOG_SPLIT_DATA_MASK);
    return (data >> 0) & 0x7F;
}
//-----------------------------------------------------------------
// usb_get_split_complete
//-----------------------------------------------------------------
uint8_t usb_get_split_complete(uint32_t value)
{
    uint16_t data = ((value >> LOG_SPLIT_DATA_L) & LOG_SPLIT_DATA_MASK);
    return (data >> 7) & 0x1;
}
//-----------------------------------------------------------------
// usb_get_pid_str
//-----------------------------------------------------------------
char* usb_get_pid_str(uint8_t pid)
{
    static char unknown[16];
    switch (pid)
    {
        // Token
        case PID_OUT:
            return "OUT";
        case PID_IN:
            return "IN";
        case PID_SOF:
            return "SOF";
        case PID_SETUP:
            return "SETUP";
        case PID_PING:
            return "PING";
        // Data
        case PID_DATA0:
            return "DATA0";
        case PID_DATA1:
            return "DATA1";
        case PID_DATA2:
            return "DATA2";
        case PID_MDATA:
            return "MDATA";
        // Handshake
        case PID_ACK:
            return "ACK";
        case PID_NAK:
            return "NAK";
        case PID_STALL:
            return "STALL";
        case PID_NYET:
            return "NYET";
        // Special
        case PID_PRE:
            return "PRE/ERR";
        case PID_SPLIT:
            return "SPLIT";
        default: 
            break;        
    }
    
    sprintf(unknown, "UNKNOWN %02x", pid);
    return unknown;
}
