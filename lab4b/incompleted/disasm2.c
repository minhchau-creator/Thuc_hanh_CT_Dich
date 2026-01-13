#include <stdio.h>
#include <stdlib.h>

typedef int WORD;

struct Instruction {
  int op; // enum OpCode
  WORD p;
  WORD q;
};

const char *opNames[] = {"LA",  "LV",  "LC",   "LI", "INT", "DCT", "J",   "FJ",
                         "HL",  "ST",  "CALL", "EP", "EF",  "RC",  "RI",  "WRC",
                         "WRI", "WLN", "AD",   "SB", "ML",  "DV",  "NEG", "CV",
                         "EQ",  "NE",  "GT",   "LT", "GE",  "LE"};

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

  printf("=== Reference Binary: %s ===\n", argv[1]);
  printf("File Size: %ld bytes\n", size);
  printf("Instruction count: %ld\n\n", size / sizeof(struct Instruction));

  // Read and display instructions
  struct Instruction inst;
  int count = 0;

  while (fread(&inst, sizeof(struct Instruction), 1, file) == 1) {
    printf("%d:  ", count);

    if (inst.op >= 0 && inst.op < 30) {
      printf("%s", opNames[inst.op]);

      // Show operands based on instruction type
      if (inst.op == 0 || inst.op == 1 || inst.op == 10) { // LA, LV, CALL
        printf(" %d,%d", inst.p, inst.q);
      } else if (inst.op == 2 || inst.op == 4 || inst.op == 5 || inst.op == 6 ||
                 inst.op == 7) {
        // LC, INT, DCT, J, FJ
        printf(" %d", inst.q);
      }
    } else {
      printf("UNKNOWN(%d) p=%d q=%d", inst.op, inst.p, inst.q);
    }

    printf("\n");
    count++;
  }

  printf("\nTotal: %d instructions\n", count);

  fclose(file);
  return 0;
}
