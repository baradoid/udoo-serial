#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include <arpa/inet.h>
#include <netinet/in.h>
//#include <sys/types.h>
//#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

//#define SRV_IP "192.168.8.102"
#define SRV_IP "127.0.0.1"
#define SRV_PORT 8059

#pragma pack(push,1)
typedef struct{
    int16_t pos1;
    int16_t pos2;
    int8_t  distance;
    int8_t  headTemp;
    int8_t  batteryTemp;
    int32_t cashCount;
} CbDataUdp;
#pragma pack(pop)

void parseStr(char *s, CbDataUdp *cbDudp)
{
    char *pch, *pEnd;
    if(strlen(s) <20)
        return;
    pch = strtok(s, " ");
    for(int parInd = 0; pch != NULL; parInd++){
//        printf("pInd:%d >%s\n", parInd, pch);
        switch(parInd){
            case 0: cbDudp->pos1=(int)strtol(pch,&pEnd,16); break;
            case 1: cbDudp->pos2=(int)strtol(pch,&pEnd,16); break;
            case 2: break;
            case 3: cbDudp->distance=(int)strtol(pch,&pEnd,10);  break;
            case 4: break;
            case 5: break;
        }

        pch = strtok(NULL, " ");
    }

//    printf("e1:%d, e2:%d\n", cbDudp->pos1,  cbDudp->pos2);

}

int main()
{
    struct sockaddr_in si_other, si_localHost;
    int s;
//    char udpBuf[200];
    CbDataUdp cbDataUdp;

    if((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1){
        printf("error create socket");
        return -1;
    }
    printf("socket created %d\n", s);

    memset((char*)&si_other, 0, sizeof(si_other)); 
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(SRV_PORT);
    if(inet_aton(SRV_IP, &(si_other.sin_addr))==0){
        printf("inet_aton() failed\n");
        return -1;
    }

    memset((char*)&si_localHost, 0, sizeof(si_localHost));
    si_localHost.sin_family = AF_INET;
    si_localHost.sin_port = htons(SRV_PORT);
    if(inet_aton("127.0.0.1", &(si_localHost.sin_addr))==0){
        printf("inet_aton() failed\n");
        return -1;
    }


    memset(&cbDataUdp, 0, sizeof(CbDataUdp));
//    sprintf(udpBuf, "test_test\n");
    printf("dataBufLen: %d\n", (int)sizeof(CbDataUdp));


    struct termios spSettings;
    int fd;
    fd = open("/dev/ttyS4", O_RDWR | O_NOCTTY | O_NDELAY);
    printf("openId=%d\n", fd);
    if(fd == 1){
        printf("\n Error in openning ttyS4\n");
        return -1;
    }
    else
        printf("\n ttyS4 opened successfully\n");

    fcntl(fd, F_SETFL, 0);
    tcgetattr(fd, &spSettings);

    cfsetispeed(&spSettings, B115200);
    cfsetospeed(&spSettings, B115200);

    spSettings.c_cflag &= ~CRTSCTS;

    spSettings.c_cflag |= CREAD | CLOCAL;
    spSettings.c_cc[VMIN] = 0;
    spSettings.c_cc[VTIME] = 1;

    //spSettings.c_iflag &= ~(IXON | IXOFF | IXANY);
    
    tcsetattr(fd, TCSANOW,&spSettings); 
    char rBuff[128];
    int bytesRead = 0;
    int iter = 0;
    while(1){
        bytesRead = read(fd, &(rBuff[0]), 128);
        if(bytesRead > 0){
            printf("bytesRead: %d", bytesRead);
            rBuff[bytesRead] = 0;
            printf(">%s", rBuff);
            //if(bytesRead == 1){
            //    printf("bytesRead 1: %d", rBuff[0]);
            //}

            parseStr(rBuff, &cbDataUdp);

            
            if(sendto(s, &cbDataUdp, sizeof(CbDataUdp), 0, 
                (struct sockaddr*)&si_other, sizeof(si_other)) == -1){
                printf("udp dgrm err\n");
            }

        }

//        if(bytesRead == 0){
//            printf("%d", iter);
//        }
//        iter++;
    }


    close(fd);
}
