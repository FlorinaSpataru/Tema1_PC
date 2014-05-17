#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>

#include "lib.h"
#include "task0.h"
#include "task1.h"
#include "task2.h"
#include "task3.h"
#include "checksum.h"

int main(int argc, char *argv[])
{
   char *filename;
   int task_index, speed, delay, dimF, timeout;
   msg t;
   my_sum s;
   int fr, filesize;
   struct stat f_status;
   
   task_index = atoi(argv[1]);
   filename = argv[2];
   speed = atoi(argv[3]);
   delay = atoi(argv[4]);
   
   dimF = (speed * delay * 1000)/(8 * MSGSIZE);
   if (2*delay > 1000 ) timeout = 2*delay;
   else timeout = 1000;
   
   printf("[SENDER] Sender starts.\n");
   printf("[SENDER] Filename=%s, task_index=%d, speed=%d, delay=%d\n", filename, task_index, speed, delay);
   
   init(HOST, PORT1);
   
   fr = open(filename, O_RDONLY);
   fstat(fr, &f_status);
   
   filesize = (int) f_status.st_size;
   printf("[SENDER] File size: %d\n", filesize);
   
   
   // Gonna send filename - s.count = -3 
   while (1){
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(s.payload, 0, sizeof(s.payload));
      
      memcpy(s.payload, filename, strlen(filename));
      s.count = -3;
      s.checksum = checksum(s.payload, strlen(filename), s.count);
      t.len = 2*sizeof(int) + strlen(filename);
      memcpy(t.payload, &s, t.len); 
      send_message(&t);
      printf("[SENDER] Filename sent.\n");
      
      // Wait for filename ACK 
      memset(t.payload, 0, sizeof(t.payload));
      
      recv_message_timeout(&t, timeout);
      
      s = *((my_sum*) t.payload);
      if (s.count == -3) break;
   }
   
   // Gonna send filesize - s.count = -2 
   while (1){
      memset(t.payload, 0, sizeof(t.payload));
      memset(s.payload, 0, sizeof(s.payload));
      
      memcpy(s.payload, &filesize, sizeof(int));
      s.count = -2;
      s.checksum = checksum(s.payload, sizeof(int), s.count);
      t.len = sizeof(int) * 3;
      memcpy(t.payload, &s, t.len);
      send_message(&t);
      printf("[SENDER] Filesize sent.\n");
      
      // Wait for filesize ACK 
      memset(t.payload, 0, sizeof(t.payload));
      recv_message_timeout(&t, timeout);
      
      s = *((my_sum*) t.payload);
      if (s.count == -2) break;
   }
   
   // Gonna send dimF - s.count = -1
   while (1){
      memset(t.payload, 0, sizeof(t.payload));
      memset(s.payload, 0, sizeof(s.payload));
      
      memcpy(s.payload, &dimF, sizeof(int));
      s.count = -1;
      s.checksum = checksum(s.payload, sizeof(int), s.count);
      t.len = sizeof(int) * 3;
      memcpy(t.payload, &s, t.len);
      send_message(&t);
      printf("[SENDER] Window size sent.\n");
      
      // Wait for dimF ACK 
      memset(t.payload, 0, sizeof(t.payload));
      recv_message_timeout(&t, timeout);
      
      s = *((my_sum*) t.payload);
      if (s.count == -1) break;
   }
   
   if (task_index == 0)
      send_t0(fr, filesize, dimF);
   if (task_index == 1)
      send_t1(fr, filesize, dimF, timeout);
   if (task_index == 2)
      send_t2(fr, filesize, dimF, timeout);
   if (task_index == 3)
      send_t3(fr, filesize, dimF, timeout);
   
   
   close(fr);
   
   return 0;
}
