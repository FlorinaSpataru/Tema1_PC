#ifndef __TASK2
#define __TASK2

#include "lib.h"

void sort2 (msg* vector, int dimF){
   int i, j;
   my_pkt p1, p2;
   msg aux;
   
   for (i = 0; i < dimF; i++){
      for (j = 0; j < dimF - 1; j++){
         p1 = *((my_pkt *) vector[j].payload);
         p2 = *((my_pkt *) vector[j + 1].payload);
         if ((p1.count > p2.count) && (p1.count != 0) && (p2.count != 0)){
            aux = vector[j + 1];
            vector[j + 1] = vector[j];
            vector[j] = aux;
         }
      }
   }
   
}

int send_t2(int fr, int filesize, int dimF, int timeout){
   
   msg t;
   my_pkt p;
   int readInfo, expectedAck, seqNo, i;
   char buffer[MSGSIZE];
   int res, sentSize = 0;
   msg messages[dimF];
   int acks[dimF];
   int buffDiff;
   
   // Send file contents 
   printf("[SERVER] File transfer begins.\n");
   
   memset(acks, 0, dimF);
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

   while (sentSize < filesize) {
      printf ("\n");
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      res = recv_message_timeout(&t, timeout);
      if (res < 0) {
         aux = 0;
         expectedAux = expectedAck;
         printf("[SENDER] Expected ack %d\n", expectedAck);
         while (aux != dimF){          
            if (acks[expectedAux%dimF] == 0){
               send_message(&messages[expectedAux%dimF]);
               p = *((my_pkt *) messages[expectedAux%dimF].payload);
               printf("[SENDER] Sending count %d\n", p.count);
            }
            expectedAux ++;
            aux ++;
            if (expectedAux == dimF)
               expectedAux = 0;
         }
      }
      else {
         p = *((my_pkt *) t.payload);
         acks[p.count%dimF] = 1;
         
         // Correct message
         if (p.count == expectedAck){
            aux = 0;
            expectedAux = expectedAck;
            
            while (aux != dimF){
               if (acks[expectedAux%dimF] == 0){
                  p = *((my_pkt *) messages[expectedAux%dimF].payload);
                  printf("[SENDER] Expected ack %d\n", expectedAck);
                  break;
               }
               expectedAux ++;
               aux ++;
               if (expectedAux == dimF)
                  expectedAux = 0;
            }
            
            if (p.count == expectedAck) 
               buffDiff = dimF;
            else 
               buffDiff = p.count - expectedAck;
            
            sentSize += buffDiff*(t.len - sizeof(int));
            
            for (i = 0; i < buffDiff; i++){
               
               readInfo = read(fr, buffer, PKTSIZE);
               expectedAck++;
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
                  acks[p.count%dimF] = 0;
                  printf("[SENDER] Sending count %d\n", p.count);
                  // Sending a new message
                  send_message(&t);                  
               }
            }
         }
      }
   }
      
   return 0;
}

int receive_t2 (int fw, int filesize, int dimF) {
   
   msg t;
   my_pkt p;
   int readSize, index, bufferCount, lastCorr, expected, i;
   msg buffer[dimF];
   
   expected = 0;
   readSize = 0;
   bufferCount = 0;
   
   // Wait for file contents 
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
      
      if (p.count > expected) { // Wrong message
         printf("[RECEIVER] Wrong order. Adding to buffer\n");
         buffer[bufferCount] = t;
         bufferCount++;
         printf("[RECEIVER] Sending ack for %d\n", p.count);
         send_message(&t);
         
      } 
      else if (p.count == expected){ // Correct message
      // Increase readSize with size written in file
      readSize += t.len - sizeof(int); 
      printf("[RECEIVER] Writing in file count %d\n", p.count);
      write(fw, p.payload, t.len - sizeof(int));
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      p.count = expected; // Send ack for correct message
      lastCorr = p.count;
      expected ++; // Expecting next message
      
      t.len = sizeof(p.payload) + sizeof(int);
      memcpy(t.payload, &p, t.len);
      printf("[RECEIVER] Correct. Sending ack for %d\n", p.count);
      send_message(&t);
      }
     
      // Checking buffer contents
      
      index = 0;
      memset(t.payload, 0, sizeof(t.payload));
      memset(p.payload, 0, sizeof(p.payload));
      
      while (1){
         memset(p.payload, 0, sizeof(p.payload));
         t = buffer[index];
         p = *((my_pkt*) t.payload);
         if (p.count == lastCorr + 1){
            index ++;
            lastCorr ++;
            readSize += t.len - sizeof(int); 
            printf("[RECEIVER] Writing in file count %d\n", p.count);
            write(fw, p.payload, t.len - sizeof(int));
            expected ++;
         }
         else break;
      }
      
      bufferCount -= index;
      sort2 (buffer, dimF);
      printf ("\n");
      
      while (index != 0){
         for (i = 1; i < dimF; i++){
            buffer[i - 1] = buffer[i];
         }
         index --;
      }
   }
   
close(fw);

return 0;

}

#endif