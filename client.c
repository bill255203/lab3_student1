#include "header.h"

int main(){
    /*---------------------------UDT SERVER----------------------------------*/
    srand(getpid());
        //Create socket.
    int socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        printf("Create socket fail!\n");
        return -1;
    }

    //Set up server's address.
    struct sockaddr_in serverAddr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr("127.0.0.1"),
        .sin_port = htons(45525)
    };
    int server_len = sizeof(serverAddr);

    //Connect to server's socket.
    if (connect(socket_fd, (struct sockaddr *)&serverAddr, server_len) == -1) {
        printf("Connect server failed!\n");
        close(socket_fd);
        exit(0);
    }
    /*---------------------------UDT SERVER-----------------------------------*/
    
    

    /*---------------------------3 way hand shake-----------------------------*/
    /*                                                                        */                                              
    // TODO: Make a three way handshake with RDT server by using TCP header char header[20] (lab2).
   // Create a TCP header for the SYN packet
    Segment syn_packet;
    memset(&syn_packet, 0, sizeof(Segment));

    // Set the values in the TCP header
    syn_packet.l4info.SourcePort = rand(); // Random source port
    syn_packet.l4info.DesPort = SERVER_PORT;
    syn_packet.l4info.SeqNum = 0; // Initial sequence number
    syn_packet.l4info.Flag = SYN; // SYN flag

    // Send the SYN packet
    send(socket_fd, &syn_packet, sizeof(Segment), 0);

    // Receive the SYN-ACK packet
    Segment synack_packet;
    recv(socket_fd, &synack_packet, sizeof(Segment), 0);

    // Check if it is a SYN-ACK packet
    if (synack_packet.l4info.Flag == (SYNACK | ACK)) {
        // Create an ACK packet to complete the handshake
        Segment ack_packet;
        memset(&ack_packet, 0, sizeof(Segment));

        // Set the values in the ACK packet
        ack_packet.l4info.SourcePort = synack_packet.l4info.DesPort;
        ack_packet.l4info.DesPort = synack_packet.l4info.SourcePort;
        ack_packet.l4info.SeqNum = synack_packet.l4info.AckNum;
        ack_packet.l4info.AckNum = synack_packet.l4info.SeqNum + 1;
        ack_packet.l4info.Flag = ACK; // ACK flag

        // Send the ACK packet
        send(socket_fd, &ack_packet, sizeof(Segment), 0);
    }
    else {
        printf("Received an unexpected packet during handshake. Handshake failed.\n");
        close(socket_fd);
        exit(0);
    }
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
