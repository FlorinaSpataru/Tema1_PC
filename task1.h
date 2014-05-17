#ifndef __TASK1
#define __TASK1

#include "lib.h"

int send_t1(int fr, int filesize, int dimF, int timeout){
   
   msg t;
   my_pkt p;
   int readInfo, sentSize, expectedAck, seqNo, i;
   char buffer[MSGSIZE];
   int res;
   msg messages[dimF];
   
   /* Send file contents */
   printf("[SERVER] File transfer begins.\n");
   
   expectedAck = 0;
   seqNo = 0;
   
   for (i = 0; i < dimF; i++){ // Sending first window
      
      readInfo = read(fr, buffer, PKTSIZE);
      if (readInfo <= 0) break; 
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      memcpy(p.payload, buffer, readInfo);
      
      p.count = seqNo;
      seqNo ++;
      printf("[SENDER] Sending count %d\n", p.count);
      
      t.len = sizeof(int) + readInfo;
      memcpy(t.payload, &p, t.len);
      send_message(&t);
      messages[i] = t; // Adding in message buffer
      
   }

   int aux;
   int expectedAux;
   sentSize = 0;

   while (sentSize < filesize) {
   
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      res = recv_message_timeout(&t, timeout);
      p = *((my_pkt *) t.payload);
      
      if (res == -1) p.count = -1;
      
      printf("[SENDER] Received ack %d\n", p.count);
      
      if (p.count == expectedAck){
         expectedAck++;
         
         // Increase sentSize 
         sentSize += t.len - sizeof(int); 
         
         // Reading new message from file
         readInfo = read(fr, buffer, PKTSIZE);
         if (readInfo > 0){
            memset(t.payload, 0, sizeof(t.payload));
            memset(p.payload, 0, sizeof(p.payload));
            
            memcpy(p.payload, buffer, readInfo);
            // Adding sequence number to message
            p.count = seqNo;
            seqNo ++; // and increment it after
            
            t.len = sizeof(int) + readInfo;
            memcpy(t.payload, &p, t.len);
            
            // Update message buffer
            messages[p.count%dimF] = t;
            printf("[SENDER] Sending count %d\n", p.count);
            send_message(&t);
            
         }
      }
      else {
         aux = 0;
         expectedAux = expectedAck;
         while (aux != dimF){
            send_message(&messages[expectedAux%dimF]);
            p = *((my_pkt *) messages[expectedAux%dimF].payload);
            printf("[SENDER] Sending count %d\n", p.count);
            expectedAux ++;
            aux ++;
            if (expectedAux == dimF)
               expectedAux = 0;
         }
      }
   }

   return 0;
}

int receive_t1 (int fw, int filesize) {
   
   msg t;
   my_pkt p;
   int readSize, expected = 0;
   
   readSize = 0;
   
   /* Wait for file contents */
   while (readSize < filesize) {
      // Clear message
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      // Receive new message
      if (recv_message(&t) < 0) {
         perror("[RECEIVER] Receive message");
         return -1;
      }
      
      // Checking contents
      p = *((my_pkt*) t.payload);
      printf("[RECEIVER] Received count %d - expected %d\n", p.count, expected);
      
      if (p.count != expected) { // Wrong message
         printf("[RECEIVER] Discarding.\n");
      } 
      else { // Correct message
      // Increase readSize with size written in file
      readSize += t.len - sizeof(int); 
      write(fw, p.payload, t.len - sizeof(int));
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      p.count = expected; // Send ack for correct message
      expected ++; // Expecting next message
      
      t.len = sizeof(p.payload) + sizeof(int);
      memcpy(t.payload, &p, t.len);
      printf("[RECEIVER] Correct. Sending ack for %d\n", p.count);
      send_message(&t);
      }
   }

   close(fw);

   return 0;

}

#endif