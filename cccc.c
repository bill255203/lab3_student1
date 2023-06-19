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
typedef struct {
  uint32_t source_ip;
  uint32_t dest_ip;
  uint16_t protocol;
  uint16_t tcp_header_length;
} PseudoHeader;

void reconstructPseudoHeader(const char *header, PseudoHeader *pseudoHeader) {
  // Extract fields from the header
  uint32_t source_ip =
      (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
  uint32_t dest_ip =
      (header[4] << 24) | (header[5] << 16) | (header[6] << 8) | header[7];
  uint16_t protocol = 124;
  uint16_t tcp_header_length = 0x1C;

  // Set fields of the pseudo header
  pseudoHeader->source_ip = source_ip;
  pseudoHeader->dest_ip = dest_ip;
  pseudoHeader->protocol = protocol;
  pseudoHeader->tcp_header_length = tcp_header_length;
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
  FILE *file = fopen("received image.jpg", "wb");
  if (file == NULL) {
    perror("Fail to open");
    exit(1);
  }

  // TASK 2 HERE---------------------------------------------------------
  Segment segment;
  while (1) {
    char packet[1024]; // Buffer to hold the received packet (header + payload)

    int bytes_received = recv(socket_fd, packet, sizeof(packet), 0);
    if (bytes_received == -1) {
      perror("recv failed");
      exit(1);
    }

    char header[20];
    char payload[1000];
    int payload_length = bytes_received - sizeof(header);

    // Process the received packet
    if (bytes_received >= 20) {
      // Extract the header from the packet
      memcpy(header, packet, sizeof(header));

      // Extract the payload from the packet
      memcpy(payload, packet + sizeof(header), payload_length);
    }

    // get the real checksum
    uint8_t byte16 = header[16] & 0xFF;
    uint8_t byte17 = header[17] & 0xFF;
    uint16_t real_checksum = (byte16 << 8) | byte17;
    printf("real checksum: %d\n", real_checksum);

    // create a buffer to store and calculate checksum
    char buffer[1024];
    PseudoHeader pseudoHeader;
    reconstructPseudoHeader(header, &pseudoHeader);
    uint8_t pseudoheader[12];
    pseudoheader[0] = pseudoHeader.source_ip >> 24;
    pseudoheader[1] = pseudoHeader.source_ip >> 16;
    pseudoheader[2] = pseudoHeader.source_ip >> 8;
    pseudoheader[3] = pseudoHeader.source_ip;
    pseudoheader[4] = pseudoHeader.dest_ip >> 24;
    pseudoheader[5] = pseudoHeader.dest_ip >> 16;
    pseudoheader[6] = pseudoHeader.dest_ip >> 8;
    pseudoheader[7] = pseudoHeader.dest_ip;
    pseudoheader[8] = 0;
    pseudoheader[9] = pseudoHeader.protocol;
    pseudoheader[10] = 0;
    pseudoheader[11] = pseudoHeader.tcp_header_length;

    memcpy(buffer, header, sizeof(header)); // Copy the 20-byte header
    memcpy(buffer + sizeof(header), pseudoheader,
           sizeof(pseudoheader)); // Copy the pseudoheader
    memcpy(buffer + sizeof(header) + sizeof(pseudoheader), payload,
           payload_length); // Copy the payload

    uint16_t checksum = mychecksum(
        buffer, sizeof(header) + sizeof(pseudoheader) + payload_length);
    printf("calculated checksum: %d\n", checksum);

    // get sequence number
    uint8_t byte4 = header[4];
    uint8_t byte5 = header[5];
    uint8_t byte6 = header[6];
    uint8_t byte7 = header[7];
    uint32_t seq_num = (byte4 << 24) | (byte5 << 16) | (byte6 << 8) | byte7;

    // Add 1000 to the sequence number
    seq_num += payload_length;
    printf("seq_num: %d\n", seq_num);
    fwrite(payload, sizeof(char), payload_length, file);

    char ack_header[20];
    myheadercreater(ack_header, 124, seq_num, ACK, source_port);

    // send only the header back
    send(socket_fd, ack_header, sizeof(ack_header), 0);
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
  /*       5. We offer mychecksum() for you to use to corrupt the packet.   */
  /*                                                                        */
  /*       When you debug this lab, you should comment out the code in task */
  /*       2, 3, 4.                                                        */
  /*                                                                        */
  /*       After you finish task 2, 3, 4, you should be able to run the code*/
  /*       without the "segmentation fault" error.                          */
  /*       It will generate a output.jpg same as server.                    */
  /*                                                                        */
  /*       Then you can start to write your code to complete task5 and task6*/
  /*-------------------------Something important----------------------------*/
  return 0;
}
