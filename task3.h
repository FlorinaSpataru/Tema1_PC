#ifndef __TASK3
#define __TASK3

#include "lib.h"
#include "checksum.h"

void sort3 (msg* vector, int dimF){
   int i, j;
   my_sum s1, s2;
   msg aux;
   
   for (i = 0; i < dimF; i++){
      for (j = 0; j < dimF - 1; j++){
         s1 = *((my_sum *) vector[j].payload);
         s2 = *((my_sum *) vector[j + 1].payload);
         if ((s1.count > s2.count) && (s1.count != 0) && (s2.count != 0)){
            aux = vector[j + 1];
            vector[j + 1] = vector[j];
            vector[j] = aux;
         }
      }
   }
   
}

int send_t3(int fr, int filesize, int dimF, int timeout){
   
   msg t;
   my_sum s;
   int readInfo, expectedAck, seqNo, i;
   char buffer[SUMSIZE];
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
      
      readInfo = read(fr, buffer, SUMSIZE);
      if (readInfo <= 0) break; 
      
      memset(t.payload, 0, sizeof(t.payload));
      memset(s.payload, 0, sizeof(s.payload));
      
      memcpy(s.payload, buffer, readInfo);
      
      s.count = seqNo;
      s.checksum = checksum (s.payload, readInfo, s.count);
      seqNo ++;
      printf("[SENDER] Sending count %d\n", s.count);
      
      t.len = 2*sizeof(int) + readInfo;
      memcpy(t.payload, &s, t.len);
      send_message(&t);
      messages[i] = t; // Adding in message buffer
      
   }

   int aux;
   int expectedAux;

   while (sentSize < filesize) {
      printf ("\n");
      memset(t.payload, 0, sizeof(t.payload));
      memset(s.payload, 0, sizeof(s.payload));
      
      res = recv_message_timeout(&t, timeout);
      if (res < 0) {
         aux = 0;
         expectedAux = expectedAck;
         printf("[SENDER] Expected ack %d\n", expectedAck);
         while (aux != dimF){          
            if (acks[expectedAux%dimF] == 0){
               send_message(&messages[expectedAux%dimF]);
               s = *((my_sum *) messages[expectedAux%dimF].payload);
               printf("[SENDER] Sending count %d\n", s.count);
            }
            expectedAux ++;
            aux ++;
            if (expectedAux == dimF)
               expectedAux = 0;
         }
      }
      else {
         s = *((my_sum *) t.payload);
         acks[s.count%dimF] = 1;
         
         // Correct message
         if (s.count == expectedAck){
            aux = 0;
            expectedAux = expectedAck;
            
            while (aux != dimF){
               if (acks[expectedAux%dimF] == 0){
                  s = *((my_sum *) messages[expectedAux%dimF].payload);
                  printf("[SENDER] Expected ack %d\n", expectedAck);
                  break;
               }
               expectedAux ++;
               aux ++;
               if (expectedAux == dimF)
                  expectedAux = 0;
            }
            
            if (s.count == expectedAck) 
               buffDiff = dimF;
            else 
               buffDiff = s.count - expectedAck;
            
            sentSize += buffDiff*(t.len - 2*sizeof(int));
            
            for (i = 0; i < buffDiff; i++){
               
               readInfo = read(fr, buffer, SUMSIZE);
               expectedAck++;
               if (readInfo > 0){
                  memset(t.payload, 0, sizeof(t.payload));
                  memset(s.payload, 0, sizeof(s.payload));
                  
                  memcpy(s.payload, buffer, readInfo);
                  // Adding sequence number to message
                  s.count = seqNo;
                  s.checksum = checksum (s.payload, readInfo, s.count);
                  seqNo ++; // and increment it after
                  
                  t.len = 2*sizeof(int) + readInfo;
                  memcpy(t.payload, &s, t.len);
                  
                  // Update message buffer
                  messages[s.count%dimF] = t;
                  acks[s.count%dimF] = 0;
                  printf("[SENDER] Sending count %d\n", s.count);
                  // Sending a new message
                  send_message(&t);                  
               }
            }
         }
      }
   }
      
   return 0;
}

int receive_t3 (int fw, int filesize, int dimF) {
   
   msg t;
   my_sum s;
   int readSize, index, bufferCount, lastCorr, expected, i;
   msg buffer[dimF];
   int check;
   
   expected = 0;
   readSize = 0;
   bufferCount = 0;
   
   // Wait for file contents 
   while (readSize < filesize) {
      // Clear message
      memset(t.payload, 0, sizeof(t.payload));
      memset(s.payload, 0, sizeof(s.payload));
      
      // Receive new message
      if (recv_message(&t) < 0) {
         perror("[RECEIVER] Receive message");
         return -1;
      }
      
      // Checking contents
      s = *((my_sum*) t.payload);
      check = checksum(s.payload, (t.len - 2*sizeof(int)), s.count);
      if (check == s.checksum){
         printf("[RECEIVER] Received count %d - expected %d\n", s.count, expected);
         
         if (s.count > expected) { // Wrong message
            printf("[RECEIVER] Wrong order. Adding to buffer\n");
            buffer[bufferCount] = t;
            bufferCount++;
            printf("[RECEIVER] Sending ack for %d\n", s.count);
            send_message(&t);
            
         } 
         else if (s.count == expected){ // Correct message
         // Increase readSize with size written in file
         readSize += t.len - 2*sizeof(int); 
         printf("[RECEIVER] Writing in file count %d\n", s.count);
         write(fw, s.payload, t.len - 2*sizeof(int));
         
         memset(t.payload, 0, sizeof(t.payload));
         memset(s.payload, 0, sizeof(s.payload));
         
         s.count = expected; // Send ack for correct message
         lastCorr = s.count;
         expected ++; // Expecting next message
         
         t.len = sizeof(s.payload) + 2*sizeof(int);
         memcpy(t.payload, &s, t.len);
         printf("[RECEIVER] Correct. Sending ack for %d\n", s.count);
         send_message(&t);
         }
      
         // Checking buffer contents
         
         index = 0;
         memset(t.payload, 0, sizeof(t.payload));
         memset(s.payload, 0, sizeof(s.payload));
         
         while (1){
            memset(s.payload, 0, sizeof(s.payload));
            t = buffer[index];
            s = *((my_sum*) t.payload);
            if (s.count == lastCorr + 1){
               index ++;
               lastCorr ++;
               readSize += t.len - 2*sizeof(int); 
               printf("[RECEIVER] Writing in file count %d\n", s.count);
               write(fw, s.payload, t.len - 2*sizeof(int));
               expected ++;
            }
            else break;
         }
         
         bufferCount -= index;
         sort3 (buffer, dimF);
         printf ("\n");
         
         while (index != 0){
            for (i = 1; i < dimF; i++){
               buffer[i - 1] = buffer[i];
            }
            index --;
         }
      }
   }
   
close(fw);

return 0;

}

#endif