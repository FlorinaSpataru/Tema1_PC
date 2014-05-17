#ifndef __CHECKSUM
#define __CHECKSUM

int checksum (char* message, int len, int count){
      int i, sum = 0;
      
      for (i = 0; i < len; i++){
         sum += message[i];
      }
      
      sum += count; 
      
      return sum;
}

#endif