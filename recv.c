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
   int task_index;
   msg t;
   my_sum s;
   int fw, filesize, dimF;
   char filename[SUMSIZE];
   char aux[SUMSIZE];
   int check;
   
   task_index = atoi(argv[1]);
   printf("[RECEIVER] Receiver starts.\n");
   printf("[RECEIVER] Task index=%d\n", task_index);
   
   init(HOST, PORT2);
   
   s.count = -23;
   
   // Wait for filename
   while (s.count != -3){
      memset(t.payload, 0, sizeof(t.payload));
      if (recv_message(&t) < 0) {
         perror("[RECEIVER] Receive message");
         return -1;
      }
      
      s = *((my_sum*) t.payload);
      check = checksum(s.payload, (t.len - 2*sizeof(int)), s.count);
      if (check == s.checksum)
         printf("[RECEIVER] Received count %d\n", s.count);
      else
         s.count = -23;
   }
   // Extract filename
   memcpy(aux, s.payload, t.len - 2*sizeof(int));
   strcat(filename, "recv_");
   strcat(filename, aux);
   
   printf("[RECEIVER] Filename: %s\n", filename);
   
   // Send ACK for filename 
   memset(t.payload, 0, sizeof(t.payload));
   memset(s.payload, 0, sizeof(s.payload));
   
   s.count = -3;
   t.len = strlen(s.payload) + sizeof(int);
   memcpy(t.payload, &s, t.len);
   send_message(&t);
   
   // Wait for filesize
   while (s.count != -2){
      memset(t.payload, 0, sizeof(t.payload));
      if (recv_message(&t) < 0) {
         perror("Receive message");
         return -1;
      }
      
      s = *((my_sum*) t.payload);
      check = checksum(s.payload, (t.len - 2*sizeof(int)), s.count);
      if (check == s.checksum)
         printf("[RECEIVER] Received count %d\n", s.count);
      else
         s.count = -23;
   }
   
   memcpy(&filesize, s.payload, sizeof(int));
   printf("[RECEIVER] Filesize: %d\n", filesize);
   
   // Send ACK for filesize
   memset(t.payload, 0, sizeof(t.payload));
   memset(s.payload, 0, sizeof(s.payload));
   
   t.len = strlen(s.payload) + sizeof(int);
   memcpy(t.payload, &s, t.len);
   send_message(&t);
   
   // Wait for dimF
   while (s.count != -1){
      memset(t.payload, 0, sizeof(t.payload));
      if (recv_message(&t) < 0) {
         perror("Receive message");
         return -1;
      }
      
      s = *((my_sum*) t.payload);
      check = checksum(s.payload, (t.len - 2*sizeof(int)), s.count);
      if (check == s.checksum)
         printf("[RECEIVER] Received count %d\n", s.count);
      else
         s.count = -23;
   }
   
   memcpy(&dimF, s.payload, sizeof(int));
   printf("[RECEIVER] Window size: %d\n", dimF);
   
   // Send ACK for dimF
   memset(t.payload, 0, sizeof(t.payload));
   memset(s.payload, 0, sizeof(s.payload));
   
   t.len = strlen(s.payload) + sizeof(int);
   memcpy(t.payload, &s, t.len);
   send_message(&t);
   
   fw = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0644);
   
   if (task_index == 0)
      receive_t0(fw, filesize);
   if (task_index == 1)
      receive_t1(fw, filesize);
   if (task_index == 2)
      receive_t2(fw, filesize, dimF);
   if (task_index == 3)
      receive_t3(fw, filesize, dimF);
   
   close (fw);
   
   return 0;
}
