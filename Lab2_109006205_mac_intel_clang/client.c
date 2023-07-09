#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "header.h"

#define BUF_SIZE 1024
#define SIZE 30
#define PORT 45525

int* get_ip(char* s){
    char string[30];
    strcpy(string, s);
    // printf("%s\n",string);
    char * token = strtok(string, ".");
    int i=0;
    int *ip = malloc(4*sizeof(int));
    while( token != NULL ) {
        ip[i] = atoi(token);
        // printf("%d\n", ip[i]);
        i++;
        token = strtok(NULL, ".");
    }
    return ip;
}

void myheadercreater(Segment *s) {
    // Create the TCP header
    uint16_t source_port = htons(s->l4info.SourcePort);
    uint16_t dest_port = htons(s->l4info.DesPort);
    uint32_t seq_num = htonl(s->l4info.SeqNum);
    uint32_t ack_num = htonl(s->l4info.AckNum);
    uint16_t flags = htons(s->l4info.Flag);
    uint16_t window_size = htons(s->l4info.WindowSize);
    uint16_t data_offset = ((s->l4info.HeaderLen) << 4 )| flags;

    // Copy the header fields to the buffer
    char header_temp[20];
    memcpy(header_temp, &source_port, sizeof(source_port));
    memcpy(header_temp + 2, &dest_port, sizeof(dest_port));
    memcpy(header_temp + 4, &seq_num, sizeof(seq_num));
    memcpy(header_temp + 8, &ack_num, sizeof(ack_num));
    memcpy(header_temp + 12, &data_offset, sizeof(data_offset));
    memcpy(header_temp + 14, &window_size, sizeof(window_size));
    memset(header_temp + 16, 0, 2); // checksum field
    memset(header_temp + 18, 0, 2); // urgent pointer field

    printf("header_temp: ");
    for (int i = 0; i < sizeof(header_temp); i++) {
        printf("%02x ", header_temp[i]);
    }
    printf("\n");
    
    int* tcphdr = malloc(4*sizeof(int));
     for(int i = 0; i < 20; i++){
        uint32_t temp = header_temp[i];
        if(temp >( 0xFFFFFF00)) temp -= (0xFFFFFF00);
        tcphdr[i] = temp;
    }

    printf("tcpHeader: ");
    for (int i = 0; i < sizeof(header_temp); i++) {
        printf("%x ", tcphdr[i]);
    }
    printf("\n");

    uint32_t tcphdr01 = (tcphdr[0]*256)+tcphdr[1];
    uint32_t tcphdr23 = (tcphdr[2]*256)+tcphdr[3];
    uint32_t tcphdr45 = (tcphdr[4]*256)+tcphdr[5];
    uint32_t tcphdr67 = (tcphdr[6]*256)+tcphdr[7];
    uint32_t tcphdr89 = (tcphdr[8]*256)+tcphdr[9];
    uint32_t tcphdr1011 = (tcphdr[10]*256)+tcphdr[11];
    uint32_t tcphdr1213 = (tcphdr[12]*256)+tcphdr[13];
    uint32_t tcphdr1415 = (tcphdr[14]*256)+tcphdr[15];

    uint32_t tcphdr_temp = tcphdr01 + tcphdr23 + tcphdr45 + tcphdr67 + tcphdr89 + tcphdr1011 + tcphdr1213 + tcphdr1415;

    uint32_t sourceIp = inet_addr(s->l3info.SourceIpv4);
    uint32_t desIp = inet_addr(s->l3info.DesIpv4);
    uint16_t protocol = htons(s->l3info.protocol);
    uint16_t tcp_len = htons(sizeof(L4info));

    //Create pseudoheader
    memcpy(s->pseudoheader, &sourceIp, sizeof(uint32_t));
    memcpy(s->pseudoheader + 4, &desIp, sizeof(uint32_t));
    memcpy(s->pseudoheader + 8, &protocol, sizeof(protocol));
    memcpy(s->pseudoheader + 9, &tcp_len, sizeof(tcp_len));// update the length field in the pseudoheader

    // Calculate the TCP checksum
    uint16_t checksum = 0;
    int len = sizeof(header_temp);

    int *ip_source = get_ip(s->l3info.SourceIpv4);
    int ip_source12 = (ip_source[0] <<8) + ip_source[1];
    int ip_source34 = (ip_source[2]<<8) + ip_source[3];

    int *ip_des = get_ip(s->l3info.DesIpv4);
    int ip_des12 = (ip_des[0] <<8) + ip_des[1];
    int ip_des34 = (ip_des[2]<<8) + ip_des[3];

    int header_len = 20;

    int pesudoheader_temp = ip_source12 + ip_source34 + ip_des12 + ip_des34 + s->l3info.protocol + header_len;
    
    uint32_t checksum_temp = pesudoheader_temp + tcphdr_temp;

    int carry = checksum_temp;
    while (carry > 16) carry = carry >>16;
    int checksum_temp2 = ~((checksum_temp % (1<<16))+ carry);

    uint8_t checksum1 = (uint8_t)(checksum_temp2 >> 8);
    uint8_t checksum2 = (uint8_t)checksum_temp2;

    memcpy(header_temp + 16, &checksum1, sizeof(checksum1)); // update the checksum field in the TCP header
    memcpy(header_temp + 17, &checksum2, sizeof(checksum2)); 

    // Copy the TCP header to the buffer
    memcpy(s->header, header_temp, sizeof(header_temp));

}

int main(int argc , char *argv[])
{   
    //Create TCP socket.//

    //Set up server's address.//

    //Connect to server's socket.//
    
    //Receive message from server and print it out.//

    int socket_fd;
    struct sockaddr_in serv_addr;
    char buffer[BUF_SIZE];

    //Create TCP Socket
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        printf("[-]Create failed");
        return -1;
    }
    printf("[+]Client Socket created.\n");
    memset(&serv_addr, '\0', sizeof(serv_addr));

    //Set up server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    //Connect to server socket
    if(connect(socket_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("[-]Connection failed");
        return -1;
    }
    printf("[+]Connected to server.\n");
    
    // Receive the server's message:
    if(recv(socket_fd, buffer, BUF_SIZE, 0) < 0){
        printf("[-]Error receiving server's msg\n");
        return -1;
    }
    printf("[+]Data Recv:  %s\n", buffer);
    
    //Instantiate a segment
    Segment s;
    receivedata(socket_fd,&s);

    //Create header
    myheadercreater(&s);

    //Send header to server
    sendheader(socket_fd,s.header);
    
    close(socket_fd);
    printf("[+]Closing connection.\n");
}
