// Copyright (c) 2020, XMOS Ltd, All rights reserved
#ifndef __sqi_access_h__
#define __sqi_access_h__

extern unsigned int DEVICE_SECTOR_WORD_SIZE;

// copy of tools 14.4 libsqiaccess

void DeviceAccess_Connect(void);
void DeviceAccess_Disconnect(unsigned int perm_disconnect);
unsigned int DeviceAccess_ReadWord(unsigned int word_address);

#ifdef __XC__
void DeviceAccess_Read(unsigned int word_address, 
                       unsigned int destination[num_words],
                       unsigned int num_words);
#else
void DeviceAccess_Read(unsigned int word_address, 
                       unsigned int * destination,
                       unsigned int num_words);
#endif

unsigned int DeviceAccess_Streamed_CRC(unsigned int word_address,
                                       unsigned int num_words,
                                       unsigned int expected_crc);

#endif
