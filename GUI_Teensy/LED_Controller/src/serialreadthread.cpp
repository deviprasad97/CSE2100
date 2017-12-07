#include <glib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "global.h"

#define READ_THREAD_SLEEP_DURATION_US 100000
#define BETWEEN_CHARACTERS_TIMEOUT_US 1000
#define MAX_VOLTAGE 3.3
#define MAX_ADC_VALUE 1024.0
#define POT_COMMAND 'P'

// define packet parameters
#define PACKET_START_BYTE  0xAA
#define PACKET_OVERHEAD_BYTES  3
#define PACKET_MIN_BYTES  PACKET_OVERHEAD_BYTES + 1
#define PACKET_MAX_BYTES  255

bool validatePacket(unsigned int packetSize, unsigned char *packet)
{
    // check the packet size
    if(packetSize < PACKET_MIN_BYTES || packetSize > PACKET_MAX_BYTES)
    {
        return false;
        return false;
    }

    // check the start byte
    if(packet[0] != PACKET_START_BYTE)
    {
        return false;
    }

    // check the length byte
    if(packet[1] != packetSize)
    {
        return false;
    }

    // compute the checksum
    char checksum = 0x00;
    for(unsigned int i = 0; i < packetSize - 1; i++)
    {
        checksum = checksum ^ packet[i];
    }

    // check to see if the computed checksum and packet checksum are equal
    if(packet[packetSize - 1] != checksum)
    {
        return false;
    }

    // all validation checks passed, the packet is valid
    return true;
}

gpointer Serial_Read_Thread()
{
  ssize_t r_res;
  char ob[50];
  unsigned int count=0;
  static unsigned char buffer[PACKET_MAX_BYTES];
  unsigned int packetSize = PACKET_MIN_BYTES;
  double voltage_disp;

  while(!kill_all_threads)
    {
      if(ser_dev!=-1)
	     {
	        r_res = read(ser_dev,ob,1);
	         //	  cout<<(int)ob[0]<<endl;
	          if(r_res==0)
	           {
	              usleep(BETWEEN_CHARACTERS_TIMEOUT_US);
	           }
	           else if(r_res<0)
	           {
	              cerr<<"Read error:"<<(int)errno<<" ("<<strerror(errno)<<")"<<endl;
	           }
	           else
	            {
        	      //this means we have received a byte;
        	      //the byte is in ob[0]

        	      //TODO: receive an entire packet and validate it.


        	      //TODO: Once validated, check if the third byte is a 'P'
        	      //decode the payload into a single int between 0 and MAX_ADC_VALUE
        	      //convert it into a voltage 0->0V  MAX_ADC_VALUE->MAX_VOLTAGE
        	      //load that converted value into voltage_disp
        	      //the next three lines need to be there for the value to be
        	      //displayed (once you loaded voltage_value with the received
        	      //voltage value

                  char b = ob[0];

                  // handle the byte according to the current count
                  if(count == 0 && b == PACKET_START_BYTE)
                  {
                      // this byte signals the beginning of a new packet
                      buffer[count] = b;
                      count++;
                      continue;
                  }
                  else if(count == 0)
                  {
                      // the first byte is not valid, ignore it and continue
                      continue;
                  }
                  else if(count == 1)
                  {
                      // this byte contains the overall packet length
                      buffer[count] = b;

                      // reset the count if the packet length is not in range
                      if(packetSize < PACKET_MIN_BYTES || packetSize > PACKET_MAX_BYTES)
                      {
                          count = 0;
                      }
                      else
                      {
                          packetSize = b;
                          count++;
                      }
                      continue;
                  }
                  else if(count < packetSize)
                  {
                      // store the byte
                      buffer[count] = b;
                      count++;
                  }

      	        if(count >= packetSize)
                  {
                      // validate the packet
                      if(validatePacket(packetSize, buffer))
                      {
              					g_mutex_lock(mutex_to_protect_voltage_display);
              					int vd = buffer[3];
              					vd *= 256;
              					vd += buffer[4];
              					double voltage_disp = vd / 310.3;

              					sprintf(c_voltage_value, "%1.3f", voltage_disp);

              					g_mutex_unlock(mutex_to_protect_voltage_display);
      				        }

                      // reset the count
                      count = 0;
                  }

      	     /* g_mutex_lock(mutex_to_protect_voltage_display);
      	      sprintf(c_voltage_value,"%1.3f",voltage_disp);

      	      g_mutex_unlock(mutex_to_protect_voltage_display);
      	*/
      	    }
      	}
      else
	       usleep(READ_THREAD_SLEEP_DURATION_US);
    }

  return NULL;
}
