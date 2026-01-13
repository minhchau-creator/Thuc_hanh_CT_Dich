#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s <binary_file>\n", argv[0]);
    return 1;
  }

  FILE *file = fopen(argv[1], "rb");
  if (file == NULL) {
    printf("Error opening file: %s\n", argv[1]);
    return 1;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  fseek(file, 0, SEEK_SET);

  printf("File: %s\n", argv[1]);
  printf("Size: %ld bytes\n\n", size);

  // Read header
  int codeSize;
  fread(&codeSize, sizeof(int), 1, file);
  printf("Code Size: %d instructions\n\n", codeSize);

  // Read and display instructions
  for (int i = 0; i < codeSize; i++) {
    unsigned char op;
    int p, q;

    fread(&op, sizeof(unsigned char), 1, file);
    fread(&p, sizeof(int), 1, file);
    fread(&q, sizeof(int), 1, file);

    printf("%d:  ", i);

    // Decode opcode
    switch (op) {
    case 0:
      printf("LA");
      break;
    case 1:
      printf("LV");
      break;
    case 2:
      printf("LC");
      break;
    case 3:
      printf("LI");
      break;
    case 4:
      printf("INT");
      break;
    case 5:
      printf("DCT");
      break;
    case 6:
      printf("J");
      break;
    case 7:
      printf("FJ");
      break;
    case 8:
      printf("HL");
      break;
    case 9:
      printf("ST");
      break;
    case 10:
      printf("CALL");
      break;
    case 11:
      printf("EP");
      break;
    case 12:
      printf("EF");
      break;
    case 13:
      printf("RC");
      break;
    case 14:
      printf("RI");
      break;
    case 15:
      printf("WRC");
      break;
    case 16:
      printf("WRI");
      break;
    case 17:
      printf("WLN");
      break;
    case 18:
      printf("AD");
      break;
    case 19:
      printf("SB");
      break;
    case 20:
      printf("ML");
      break;
    case 21:
      printf("DV");
      break;
    case 22:
      printf("NEG");
      break;
    case 23:
      printf("CV");
      break;
    case 24:
      printf("EQ");
      break;
    case 25:
      printf("NE");
      break;
    case 26:
      printf("GT");
      break;
    case 27:
      printf("LT");
      break;
    case 28:
      printf("GE");
      break;
    case 29:
      printf("LE");
      break;
    default:
      printf("UNKNOWN(%d)", op);
    }

    // Show operands if needed
    if (op <= 10) { // Instructions with operands
      printf(" %d,%d", p, q);
    }

    printf("\n");
  }

  fclose(file);
  return 0;
}
