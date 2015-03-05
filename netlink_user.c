#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <string.h>
#include <time.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <netdb.h>
#include <ctype.h>


#define NETLINK_TEST 31 // defined by user
#define RECIVE_BUF 4096 // recive buffer size 
#define HDR_SIZE 12

struct sockaddr_nl dst_addr, src_addr;
struct nlmsghdr* nlh = NULL;
struct iovec iov;
int sock_fd = -1;
struct msghdr msg;

// construct message
//char str[] = "Message from user";
char str[512];
char *mac_buffer;
char mac_tmp[18] = {0};
char *mac_tmp_ptr = mac_tmp;
char ap_mac[18] = {0};


int init_netlink(void)
{
	// get ap mac
	get_ap_mac(ap_mac);

	//debug
	printf("init_netlink\n");

    // create netlink socket
    if (-1 == (sock_fd = (socket(PF_NETLINK, SOCK_DGRAM, NETLINK_TEST)))) {
        perror("Can't create netlink socket!");
        return -1;
    }

    // for sending to kernel
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;
    dst_addr.nl_groups = 0;

    // for reciving from kernel
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;
    bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr));

    strcpy(str, "lzy");
    int len = strlen(str);
    printf("str len: %d\n", len);
    //if(NULL == (nlh = malloc(NLMSG_SPACE(len)))) {
    if(NULL == (nlh = malloc(NLMSG_SPACE(RECIVE_BUF)))) {
        perror("allocate memory failed\n");
    }
    //memset(nlh, 0, NLMSG_SPACE(len));
    
    memset(nlh, 0, NLMSG_SPACE(RECIVE_BUF));
    
    // fill the netlink header
    //nlh->nlmsg_len = NLMSG_SPACE(len)+sizeof(struct nlmsghdr);
    nlh->nlmsg_len = NLMSG_SPACE(RECIVE_BUF)+sizeof(struct nlmsghdr);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_type = 0;
    nlh->nlmsg_flags = 0;
    // netlink payload
    strcpy(NLMSG_DATA(nlh), str);
    // ...
    memset(&iov, 0, sizeof(iov));
    iov.iov_base = (void*) nlh;
    iov.iov_len = nlh->nlmsg_len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void*) &dst_addr;
    msg.msg_namelen = sizeof(dst_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

	return 0;

}

void main_netlink(void) {
	// debug 
	printf("main_netlink\n");

    // send
    sendmsg(sock_fd, &msg, 0);
    free(nlh);
    
    // recive
    if(NULL == (nlh = malloc(NLMSG_SPACE(RECIVE_BUF)))) {
        perror("allocate memory failed\n");
    }
    memset(nlh, 0, RECIVE_BUF);
    recvmsg(sock_fd, &msg, 0);
    printf("Recive from kernel: %s\n", NLMSG_DATA(nlh));
	printf("size: %d\n", strlen(NLMSG_DATA(nlh)));
	// store mac data to buffer
	mac_buffer = (char *)malloc(strlen(NLMSG_DATA(nlh)));
	int mac_buffer_size = strlen(NLMSG_DATA(nlh));
	memcpy(mac_buffer, NLMSG_DATA(nlh), mac_buffer_size);	
    free(nlh);
    // close netlink socket
    close(sock_fd);	



	// read each mac, construct JSON
	char *mac_send_server = (char *)malloc(mac_buffer_size*5);
	char *mac_send_server_head = mac_send_server;

	// socket header

	
	// read each mac
	*(mac_buffer + strlen(mac_buffer)) = '\0';  // delete last  ";"
	mac_tmp_ptr = strtok(mac_buffer, ";");
	while(mac_tmp_ptr != NULL) { 
		if(strlen(mac_tmp_ptr) == 17) {
			
			sprintf(mac_send_server, "mmac:%s;gmac:%s;", ap_mac,mac_tmp_ptr);
			mac_send_server += strlen(mac_send_server);

			
		}
		
		// traverse
		mac_tmp_ptr = strtok(NULL, ";"); 
	}	

	printf("--------\n%s\n--------\n", mac_send_server_head);



/*
	printf("%s\n", mac_buffer);
	char *mac_tmp_ptr = mac_tmp;

	mac_send_server = (char *)malloc(mac_size*6);
	memset(mac_send_server, 0, mac_size*6);
	char *mac_send_server_head = mac_send_server;
	mac_msg_size = 0;
	strcat(mac_send_server + HDR_SIZE, "{\"row\":[");
	mac_msg_size = 21;
	mac_send_server += 21;

	//\"time\":%ld,\"gmac\":\"%s\",\"mmac\":\"%s\"", 

	
	// read each mac
	*(mac_buffer + strlen(mac_buffer)) = '\0';  // delete last ';
	mac_tmp_ptr= strtok(mac_buffer, ";");
	while(mac_tmp_ptr != NULL) { 
		//sprintf(mac_send_server + HDR_SIZE + mac_msg_size, "time:%ld,gmac:%s,mmac:%s", (long)time(NULL), mac_tmp_ptr, ap_mac);
		sprintf(mac_send_server, "time:%ld,gmac:%s,mmac:%s", (long)time(NULL), mac_tmp_ptr, ap_mac);
		mac_send_server += strlen(mac_send_server);

		printf("%s\n", mac_send_server);
		mac_tmp_ptr = strtok(NULL, ";"); 
	}

	strcat(mac_send_server, "]}");
	mac_msg_size += 2;

	printf("----------\n%s\n---------\n", mac_send_server_head);

	printf("%d\n", mac_msg_size);

 */

	free(mac_buffer); 
 
	
	printf("main netlink end\n");

    return;
}
