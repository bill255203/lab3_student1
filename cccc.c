#include "header.h"
void myheadercreater(char *header, uint32_t seq, uint32_t ack, uint8_t flag,
                     uint16_t source_port) {
  uint8_t byte0 = source_port >> 8;
  uint8_t byte1 = source_port;
  header[0] = byte0;
  header[1] = byte1;
  uint16_t dest_port = SERVER_PORT;
  uint8_t byte2 = dest_port >> 8;
  uint8_t byte3 = dest_port;
  header[2] = byte2;
  header[3] = byte3;

  uint8_t byte4 = seq >> 24;
  uint8_t byte5 = seq >> 16;
  uint8_t byte6 = seq >> 8;
  uint8_t byte7 = seq;
  header[4] = byte4;
  header[5] = byte5;
  header[6] = byte6;
  header[7] = byte7;

  uint8_t byte8 = ack >> 24;
  uint8_t byte9 = ack >> 16;
  uint8_t byte10 = ack >> 8;
  uint8_t byte11 = ack;
  header[8] = byte8;
  header[9] = byte9;
  header[10] = byte10;
  header[11] = byte11;
  header[12] = 0x50;
  header[13] = flag;

  uint16_t win = 65535;
  uint8_t byte14 = win >> 8;
  uint8_t byte15 = win;
  header[14] = byte14;
  header[15] = byte15;
}
int main() {
  srand(time(NULL));
  uint16_t source_port = rand() % 65536;
  /*---------------------------UDT SERVER----------------------------------*/
  srand(getpid());
  // Create socket.
  int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
  if (socket_fd < 0) {
    printf("Create socket fail!\n");
    return -1;
  }

  // Set up server's address.
  struct sockaddr_in serverAddr = {.sin_family = AF_INET,
                                   .sin_addr.s_addr = inet_addr("127.0.0.1"),
                                   .sin_port = htons(45525)};
  int server_len = sizeof(serverAddr);

  // Connect to server's socket.
  if (connect(socket_fd, (struct sockaddr *)&serverAddr, server_len) == -1) {
    printf("Connect server failed!\n");
    close(socket_fd);
    exit(0);
  }
  /*---------------------------UDT SERVER-----------------------------------*/

  /*---------------------------3 way hand shake-----------------------------*/
  /*                                                                        */
  // TODO: Make a three way handshake with RDT server by using TCP header char
  // header[20] (lab2). Create a TCP header for the SYN packet
  char header[20];
  myheadercreater(header, 123, 0, SYN, source_port);
  send(socket_fd, (void *)header, sizeof(header), 0);
  recv(socket_fd, (void *)header, sizeof(header), 0);
  uint64_t byte1, byte2, byte3, byte4;
  byte1 = header[4] & 0xFF;
  byte2 = header[5] & 0xFF;
  byte3 = header[6] & 0xFF;
  byte4 = header[7] & 0xFF;
  uint64_t rev_seq = (byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4;
  byte1 = header[8] & 0xFF;
  byte2 = header[9] & 0xFF;
  byte3 = header[10] & 0xFF;
  byte4 = header[11] & 0xFF;
  uint64_t rev_ack = (byte1 << 24) + (byte2 << 16) + (byte3 << 8) + byte4;

  myheadercreater(header, rev_ack, rev_seq + 1, ACK, source_port);
  send(socket_fd, (void *)header, sizeof(header), 0);
  FILE *file = fopen("received image. jpg", "wb");
  if (file == NULL) {
    perror("Fail to open");
    exit(1);
  }
  float corruption_probability = 0.3;

  Segment segment;
  uint32_t expected_seq = 0;
  while (1) {
    // Receive a segment
    recv(socket_fd, (void *)&segment, sizeof(segment), 0);

    // Calculate checksum and verify integrity
    uint16_t received_checksum = ntohs(segment.l4info.CheckSum);
    segment.l4info.CheckSum =
        0; // Set checksum field to 0 for checksum calculation
    uint16_t calculated_checksum =
        mychecksum((char *)&segment, sizeof(segment));
    printf("Calculated checksum: %d\n", calculated_checksum);
    // // Introduce corruption based on the corruption probability
    // float random_number =
    //     (float)rand() / RAND_MAX; // Generate a random number between 0 and 1
    // if (random_number < corruption_probability ||
    //     received_checksum != calculated_checksum) {
    //   // Discard the segment or request retransmission

    //   // Option 1: Discard the segment
    //   continue;

    //   // Option 2: Request retransmission
    //   // myheadercreater(segment.header, 0, expected_seq, ACK);
    //   // send(socket_fd, (void *)&segment, sizeof(segment), 0);
    //   // continue;
    // }

    // Check if this is the last segment
    /*if (segment.p_len < 1000) {
      // This is the last segment, exit the loop
      fwrite(segment.payload, sizeof(char), segment.p_len, file);
      break;
    }*/

    // // Check the sequence number
    // uint32_t received_seq = ntohl(segment.l4info.SeqNum);
    // if (received_seq != expected_seq) {
    //   // Request retransmission by sending an ACK with the expected sequence
    //   // number
    //   myheadercreater(segment.header, 0, expected_seq, ACK);
    //   send(socket_fd, (void *)&segment, sizeof(segment), 0);
    //   continue;
    // }

    recv(socket_fd, (void *)&segment, sizeof(segment), 0);
    // Calculate checksum and verify integrity
    fwrite(segment.payload, sizeof(char), segment.p_len, file);
    // Extract the sequence number from the header and add 1000
    // Extract the bytes from segment.header
    uint8_t byte4 = segment.header[4];
    uint8_t byte5 = segment.header[5];
    uint8_t byte6 = segment.header[6];
    uint8_t byte7 = segment.header[7];
    // Reconstruct the sequence number
    uint32_t seq_num = (byte4 << 24) | (byte5 << 16) | (byte6 << 8) | byte7;
    // Add 1000 to the sequence number
    seq_num += 1000;
    printf("seq_num: %d\n", seq_num);
    myheadercreater(segment.header, 124, seq_num, ACK, source_port);
    send(socket_fd, (void *)&segment, sizeof(segment), 0);
  }

  fclose(file);
  // Wait
  /*----------------------------receive data--------------------------------*/
  /*                                                                        */
  /* TODO: Receive data from the RDT server.                                */
  /*       Each packet will be 20bytes TCP header + 1000bytes paylaod       */
  /*       exclude the last one. (the payload may not be exactly 1000bytes) */
  /*                                                                        */
  /* TODO: Once you receive the packets, you should check whether it's      */
  /*       corrupt or not (checksum) , and send the corresponding ack       */
  /*       packet (also a char[20] ) back to the server.                    */
  /*                                                                        */
  /* TODO: fwrite the payload into a .jpg file, and check the picture.      */
  /*                                                                        */
  /*----------------------------receive data--------------------------------*/

  /*-------------------------Something important----------------------------*/
  /* NOTE: TO make lab3 simple                                              */
  /*                                                                        */
  /*       1. The SrcIP and DesIP are both 127.0.0.1,                       */
  /*          Tcp header length will be 20byts, windowsize = 65535 bytes    */
  /*       2. The Handshake packets won't be corrupt.                       */
  /*       3. The packet will only corrupt but not miss or be disordered.   */
  /*       4. Only the packets come from server may corrupt.(don't have to  */
  /*          worry that the ack sent by client will corrupt.)              */
  /*       5. We offer mychecksum() for you to verify the checksum, and     */
  /*          don't forget to verify pseudoheader part.                     */
  /*       6. Once server finish transmit the file, it will close the       */
  /*          client socket.                                                */
  /*       7. You can adjust server by                                      */
  /*          ./server {timeout duration} {corrupt probability}             */
  /*                                                                        */
  /*-------------------------Something important----------------------------*/
}
