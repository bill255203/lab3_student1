#include <stdint.h>
#include <stdio.h>


typedef struct {
  uint16_t source_port;
  uint16_t dest_port;
  uint32_t seq;
  uint32_t ack;
  uint8_t reserved;
  uint8_t flag;
  uint16_t win;
} PseudoHeader;

void reconstructPseudoHeader(const char *header, PseudoHeader *pseudoHeader) {
  // Extract fields from the header
  uint16_t source_port = (header[0] << 8) | header[1];
  uint16_t dest_port = (header[2] << 8) | header[3];
  uint32_t seq =
      (header[4] << 24) | (header[5] << 16) | (header[6] << 8) | header[7];
  uint32_t ack =
      (header[8] << 24) | (header[9] << 16) | (header[10] << 8) | header[11];
  uint8_t flag = header[13];
  uint16_t win = (header[14] << 8) | header[15];

  // Set fields of the pseudo header
  pseudoHeader->source_port = source_port;
  pseudoHeader->dest_port = dest_port;
  pseudoHeader->seq = seq;
  pseudoHeader->ack = ack;
  pseudoHeader->reserved = 0; // Assuming reserved field is set to 0
  pseudoHeader->flag = flag;
  pseudoHeader->win = win;
}

int main() {
  // Example usage
  char header[16] = {/* Header values */};

  PseudoHeader pseudoHeader;
  reconstructPseudoHeader(header, &pseudoHeader);

  // Access reconstructed fields
  printf("Source Port: %u\n", pseudoHeader.source_port);
  printf("Destination Port: %u\n", pseudoHeader.dest_port);
  printf("Sequence Number: %u\n", pseudoHeader.seq);
  printf("Acknowledgment Number: %u\n", pseudoHeader.ack);
  printf("Reserved: %u\n", pseudoHeader.reserved);
  printf("Flag: %u\n", pseudoHeader.flag);
  printf("Window Size: %u\n", pseudoHeader.win);

  return 0;
}
