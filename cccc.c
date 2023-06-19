uint32_t init1 = seq_num + 10000;
uint32_t init2 = init1 + 10000;
uint32_t init3 = init2 + 10000;
uint32_t init4 = init3 + 3939;
if (checksum == real_checksum) {
  // Add 1000 to the sequence number
  seq_num += payload_length;

  printf("seq_num: %d\n", seq_num);
  fwrite(payload, sizeof(char), payload_length, file);
  if (seq_num == init1 || seq_num == init2 || seq_num == init3 ||
      seq_num == init4) {
    char ack_header[20];
    myheadercreater(ack_header, 124, seq_num, ACK, source_port);
    send(socket_fd, ack_header, sizeof(ack_header), 0);
    printf("ACKdajdjfhigihhigfihgfhgfd\n\n\n");
  }
} else {
  char ack_header[20];
  myheadercreater(ack_header, 124, seq_num, ACK, source_port);
  send(socket_fd, ack_header, sizeof(ack_header), 0);
  printf("seq_num: %d\n", seq_num);
  printf("wrong info\n\n");
}