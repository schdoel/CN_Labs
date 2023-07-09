#pragma once
#include <sys/socket.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#define SERVER_PORT 45525
#define SYN 0x02
#define ACK 0X10 
#define SYNACK 0X12 
#define ACKPSH 0X18

typedef struct l4info{
    uint32_t SourcePort,DesPort,SeqNum,AckNum,HeaderLen,Flag,WindowSize,CheckSum;
}L4info;
typedef struct l3info{
    char SourceIpv4[16], DesIpv4[16];
    uint32_t protocol;
}L3info;
typedef struct Segment{
    char header[20];
    char pseudoheader[12];
    char payload[1000];
    int p_len;
    L3info l3info;
    L4info l4info;
}Segment;
uint16_t mychecksum(char* buffer, int packet_len);
void initS(Segment* seg, uint32_t srcPort, uint32_t desPort);
void replyS(Segment* seg, uint32_t seqNum, uint32_t ackNum, uint32_t flag);
void sendpacket(int socket_fd, char* buffer, int buffer_len, Segment* seg, int flags);
ssize_t recvpacket(int socket_fd, char* buffer, size_t buffer_len, Segment* recvS, int flags);
int* get_ip(char* s);
void myheadercreater(Segment *s);

uint16_t mychecksum(char* buffer, int buffer_len){
    if((buffer_len)%2==1) buffer_len++;
    uint32_t checksum = 0;
    uint16_t* p = (uint16_t*)buffer;
    for(int i =0;i<(buffer_len/2);i++){    
        checksum += (*(p+i)); 
        while(checksum>0xffff){
            checksum = (checksum&0xffff) + (checksum>>16);
            
        }
    }
    
    checksum = (~(checksum))&0xffff;
    return (uint16_t)ntohs(checksum);
}


void initS(Segment* seg, uint32_t srcPort, uint32_t desPort) {
    memset(seg, 0, sizeof(Segment));
    seg->l4info.SourcePort = srcPort;
    seg->l4info.DesPort = desPort;
    seg->l4info.HeaderLen = 20;
    seg->l4info.Flag = 0;
    seg->l4info.WindowSize = 65535;
}

void replyS(Segment* seg, uint32_t seqNum, uint32_t ackNum, uint32_t flag) {
    seg->l4info.SeqNum = seqNum;
    seg->l4info.AckNum = ackNum;
    seg->l4info.Flag = flag;
}

void sendpacket(int socket_fd, char* buffer, int buffer_len, Segment* sendS, int flags){
    
    myheadercreater(sendS);
    memcpy(buffer, &sendS->header, buffer_len);

    // Send the packet over the socket
    ssize_t bytes_sent = send(socket_fd, buffer, buffer_len, 0);
    printf("[+]Send packet\n");
    if (bytes_sent < 0) {
        perror("Error sending packet");
        exit(1);
    }
}

ssize_t recvpacket(int socket_fd, char* buffer, size_t buffer_len, Segment* recvS, int flags) {
    // Receive the packet data from the socket
    ssize_t bytes_received = recv(socket_fd, buffer, buffer_len, flags);
    printf("[+]Receive packet\n");
    memcpy(recvS->header, buffer, sizeof(recvS->header));

    
    recvS->l4info.SourcePort = htons(*((uint32_t *)(recvS->header + sizeof(char))));
    recvS->l4info.DesPort = htons(*((uint32_t *)(recvS->header + sizeof(char)* 2)));
    recvS->l4info.SeqNum = ntohl(*((uint32_t *)(recvS->header + sizeof(char) * 4)));
    recvS->l4info.AckNum = ntohl(*((uint32_t *)(recvS->header + sizeof(char) * 8)));
    
    myheadercreater(recvS);
    printf("[+]Receive packet\n");

    return bytes_received;
}


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

void myheadercreater(Segment* s) {
    // Create the TCP header
    uint16_t source_port = htons(s->l4info.SourcePort);
    uint16_t dest_port = htons(s->l4info.DesPort);
    uint32_t seq_num = htonl(s->l4info.SeqNum);
    uint32_t ack_num = htonl(s->l4info.AckNum);
    uint16_t flags = s->l4info.Flag;// htons(s->l4info.Flag);
    uint16_t window_size = htons(s->l4info.WindowSize);
    uint16_t data_offset = htons((s->l4info.HeaderLen << 12) | s->l4info.Flag);//((s->l4info.HeaderLen) << 4 )| flags;
    

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
    
    
    int* tcphdr = malloc(4*sizeof(int));
     for(int i = 0; i < 20; i++){
        uint32_t temp = header_temp[i];
        if(temp >( 0xFFFFFF00)) temp -= (0xFFFFFF00);
        tcphdr[i] = temp;
    }

    uint32_t tcphdr01 = (tcphdr[0]*256)+tcphdr[1];
    uint32_t tcphdr23 = (tcphdr[2]*256)+tcphdr[3];
    uint32_t tcphdr45 = (tcphdr[4]*256)+tcphdr[5];
    uint32_t tcphdr67 = (tcphdr[6]*256)+tcphdr[7];
    uint32_t tcphdr89 = (tcphdr[8]*256)+tcphdr[9];
    uint32_t tcphdr1011 = (tcphdr[10]*256)+tcphdr[11];
    uint32_t tcphdr1213 = (tcphdr[12]*256)+tcphdr[13];
    uint32_t tcphdr1415 = (tcphdr[14]*256)+tcphdr[15];
    uint32_t tcphdr_temp = tcphdr01 + tcphdr23 + tcphdr45 + tcphdr67 + tcphdr89 + tcphdr1011 + tcphdr1213 + tcphdr1415;

    uint32_t sourceIp = inet_addr("127.0.0.1");
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
    
    memcpy((void*)&s->l4info.CheckSum, header_temp + 16, sizeof(s->l4info.CheckSum));
    // Copy the TCP header to the buffer
    memcpy(s->header, header_temp, sizeof(header_temp));
}
