#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mcrypt.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

int encrypt(void* buffer,int buffer_len, char* IV, char* key,int key_len )
{
  MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 ){return 1;}

  mcrypt_generic_init(td, key, key_len, IV);
  mcrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

int decrypt(void* buffer,int buffer_len,char* IV,char* key,int key_len)
{
  MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 ){return 1;}
  
  mcrypt_generic_init(td, key, key_len, IV);
  mdecrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

void display(char* ciphertext, int len){
  int v;
  for (v=0; v<len; v++){
    printf("%c", ciphertext[v]);
  }
  printf("\n");
}

int main()
{
  MCRYPT td, td2;
  char * Text = "My name is viraj";
  char* IV = "43212367835abcd";
  char *key = "0123456789abcdef";
  int keysize = 16; /* 128 bits */
  char* buffer;
  int buffer_len = 128;

  buffer = calloc(1, buffer_len);
  strncpy(buffer,Text, buffer_len);
  printf("Text:   %s\n", Text);
  encrypt(buffer, buffer_len, IV, key, keysize); 
  printf("cipher:  "); display(buffer , buffer_len);
  decrypt(buffer, buffer_len, IV, key, keysize);
  printf("decrypted text: %s\n", buffer);
  
  return 0;
}
