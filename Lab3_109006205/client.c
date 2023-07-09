#include "header.h"

#define BUF_SIZE 1024
#define SIZE 30
#define PORT 45525
#define CORRUPT_DETECT 0


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
    printf("[+]Connected to server\n");
    /*---------------------------UDT SERVER-----------------------------------*/
    

    /*---------------------------3 way hand shake-----------------------------*/
    /*                                                                        */                                              
    /* TODO: Make a three way handshake with RDT server by using TCP header   */
    /*       char header[20] (lab2).                                          */
    /*       Make sure the SrcPort(Random), DesPort(45525), Seg#, Ack#, FLAG  */
    /*       are correct.                                                     */
    /*                                                                        */                                              
    /*---------------------------3 way hand shake-----------------------------*/
    char o_buffer[20], i_buffer[1020];
    uint32_t currentSeg, currentAck;
    uint16_t sPort, dPort;
    Segment sendS, recvS;
    currentSeg = rand();
    currentAck = 0;
    sPort = (rand()%65535);
    dPort = SERVER_PORT;
    
    sendS.l4info.SourcePort = sPort;
    sendS.l4info.DesPort = dPort;
    sendS.l4info.HeaderLen = 20;
    sendS.l4info.Flag = 0;
    sendS.l4info.WindowSize = 65535;

    sendS.l4info.SeqNum = currentSeg;
    sendS.l4info.AckNum = currentAck;
    sendS.l4info.Flag = SYN;

    myheadercreater(&sendS);
    memcpy(o_buffer, &sendS.header, sizeof(o_buffer));

    // Send the packet over the socket
    send(socket_fd, o_buffer, sizeof(o_buffer), 0);

    recvpacket(socket_fd, i_buffer,sizeof(i_buffer),&recvS,0);
    currentAck = recvS.l4info.SeqNum + 1;
    currentSeg = recvS.l4info.AckNum;

    replyS(&sendS,currentSeg,currentAck,ACK);
    sendpacket(socket_fd, o_buffer, sizeof(o_buffer), &sendS,0);

            
    

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
    Segment last_recv_packet = recvS;

    FILE* file = fopen("received_image.jpg", "wb");
    if (file == NULL) {
        perror("Fail to open");
        exit(1);
    }

    int mark = 1;
   while(1){
        ssize_t byterecv = recvpacket(socket_fd, i_buffer,sizeof(i_buffer),&recvS,0);
        if(byterecv == -1){
            perror("Failed to receive\n");
            return -1;
        }
        recvS.p_len = byterecv - sizeof(recvS.header);
        printf("Payload length: %d\n", recvS.p_len);
        if(byterecv >=20){
            memcpy(recvS.header, i_buffer, sizeof(i_buffer));
            memcpy(recvS.payload, i_buffer + sizeof(recvS.header), recvS.p_len);
        }
        printf("CHECKSUM MYHEADERCREATER %d\n", recvS.l4info.CheckSum);
        
        //real checksum 1
        recvS.l4info.CheckSum = ((recvS.header[16] & 0xFF) << 8) | ((recvS.header[17] & 0xFFF));
        recvS.header[16] = recvS.header[17] = 0;

        if(recvS.l4info.SeqNum == sendS.l4info.AckNum){
            last_recv_packet = recvS;
            fwrite(recvS.payload, 1, recvS.p_len,file);
            currentAck = last_recv_packet.l4info.SeqNum + last_recv_packet.p_len;

            replyS(&sendS,currentSeg,currentAck,ACK);
            sendpacket(socket_fd, o_buffer, sizeof(o_buffer), &sendS,0);

            sleep(0);
        }
        else{
            printf("Rdt client: Dropped corrupt packet\n");
            sendpacket(socket_fd, o_buffer, sizeof(o_buffer), &sendS,0);
            sleep(0);
        }
    }

    //IP = "127.0.0.1"    
    //windowsize = 65535
    //header_len = 20
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

