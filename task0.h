#ifndef __TASK0
#define __TASK0

#include "lib.h"

int send_t0(int fr, int filesize, int dimF){
   
   msg t;
   my_pkt p;
   int count, i;
   char buffer[MSGSIZE];
   
   /* Send file contents */
   printf("[SERVER] File transfer begins.\n");
   
   for (i = 0; i < dimF; i++){
      count = read(fr, buffer, MSGSIZE - sizeof(int));
      if (count <= 0) break; 
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      memcpy(p.payload, buffer, count);
      
      p.count  = i;
      printf("[SENDER] Sending count %d\n", p.count);
      
      t.len = sizeof(int) + count;
      memcpy(t.payload, &p, t.len);
      send_message(&t);
   }
   
   if (recv_message(&t) < 0) {
      perror("[SENDER] Receive error");
      return -1;
   }
   
   for (i = 0; i < (filesize - dimF); i++){
      
      count = read(fr, buffer, MSGSIZE - sizeof(int));
      if (count <= 0) break; 
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      memcpy(p.payload, buffer, count);
      
      p.count  = i + dimF;
      printf("[SENDER] Sending count %d\n", p.count);
      
      t.len = sizeof(int) + count;
      memcpy(t.payload, &p, t.len);
      send_message(&t);
      
      if (recv_message(&t) < 0) {
         perror("[SENDER] Receive error");
         return -1;
      }
      
      p = *((my_pkt *) t.payload); 
   }
   
   return 0;
}

int receive_t0 (int fw, int filesize) {
   
   msg t;
   my_pkt p;
   int read_so_far = 0;
   
   /* Wait for file contents */
   while (read_so_far < filesize) {
      memset(t.payload, 0, sizeof(t.payload));
      
      if (recv_message(&t) < 0) {
         perror("[RECEIVER] Receive message");
         return -1;
      }
      
      p = *((my_pkt*) t.payload);
      printf("[RECEIVER] Received count %d\n", p.count);
      
      read_so_far += t.len - sizeof(int);
      write(fw, p.payload, t.len - sizeof(int));;
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      t.len = strlen(p.payload) + sizeof(int);
      memcpy(t.payload, &p, t.len);
      send_message(&t);
   }
   
   close(fw);
   
   return 0;
   
}

#endif