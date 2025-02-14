#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/ip.h>  // IP header structure
#include <netinet/tcp.h> // TCP header structure
#include <netinet/udp.h> //// UDP header structure
#include <netinet/ip_icmp.h> // ICMP header structure
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <netinet/if_ether.h>


int total = 0;
int tcp = 0;
int udp = 0;
int icmp = 0;
int others = 0;


typedef struct Protocol {
   char *protocalname;
   int pcount;
   int alarm_limit; //packets per second
   char *alarmstatus; // =>In Alarm, Normal
   int resetpcount; //=>Allow the pcount to be set to 0;
   struct Protocol *next;
}Protocol;

struct Protocol *head = NULL;

void addProtocol(const char *name, int alarm_limit) {
    struct Protocol *newNode = (struct Protocol *)malloc(sizeof(struct Protocol));
    if (!newNode) {
        perror("Memory allocation failed!\n");
        return;
    }

    newNode->protocalname = malloc(strlen(name) + 1); //new change to make sure memory allocated
    if (!newNode->protocalname) {
        perror("Memory allocation failed!\n");
        free(newNode);
        return;
    }

    strcpy(newNode->protocalname, name);
    newNode->pcount = 0;
    newNode->alarm_limit = alarm_limit;
    newNode->next = head;
    head = newNode;
}

void init_protocols()
{
	addProtocol("TCP", 10);
	addProtocol("UDP", 20);
	addProtocol("ICMP", 30);
	addProtocol("IP", 80);
}


void packet_handler(unsigned char *user, const struct pcap_pkthdr *pkthdr, const unsigned char *packet)
{
    // 1) Increase total packet count
    ++total;

    // 2) Identify the protocol from the IP header
    struct ip *iphdr = (struct ip*)(packet + 14);

    // 3) Find correct protocol in the linked list and increment pcount
    struct Protocol *current = head;
    while (current != NULL) {
        if ((strcmp(current->protocalname, "TCP") == 0 && iphdr->ip_p
== IPPROTO_TCP) ||
            (strcmp(current->protocalname, "UDP") == 0 && iphdr->ip_p
== IPPROTO_UDP) ||
            (strcmp(current->protocalname, "ICMP") == 0 && iphdr->ip_p
== IPPROTO_ICMP) ||
            (strcmp(current->protocalname, "IP") == 0 &&
                 iphdr->ip_p != IPPROTO_TCP &&
                 iphdr->ip_p != IPPROTO_UDP &&
                 iphdr->ip_p != IPPROTO_ICMP))
        {
            current->pcount++;
            // Check if pcount hits alarm limit
            if (current->pcount >= current->alarm_limit) {
                 
                printf("\n\033[1;31m Resetting %s PACKET \033[0m\n", current->protocalname);
                usleep(1);
                current->pcount = 0;
            }
            break;  // Found our protocol, stop searching
        }
        current = current->next;
    }

    // 4) Gather the current pcount values for each protocol
    int tcp_count = 0, udp_count = 0, icmp_count = 0, other_count = 0;

    current = head;
    while (current != NULL) {
        if (strcmp(current->protocalname, "TCP") == 0)
            tcp_count = current->pcount;
        else if (strcmp(current->protocalname, "UDP") == 0)
            udp_count = current->pcount;
        else if (strcmp(current->protocalname, "ICMP") == 0)
            icmp_count = current->pcount;
        else if (strcmp(current->protocalname, "IP") == 0)
            other_count = current->pcount;
        current = current->next;
    }

    // 5) Print bars based on each protocol's count
    printf("\nTCP : ");
    for (int i = 0; i < tcp_count; i++) {
        printf("=");
    }
    printf("\n");

    printf("UDP : ");
    for (int i = 0; i < udp_count; i++) {
        printf("-");
    }
    printf("\n");

    printf("ICMP: ");
    for (int i = 0; i < icmp_count; i++) {
        printf("-");
    }
    printf("\n");

    printf("Other: ");
    for (int i = 0; i < other_count; i++) {
        printf("=");
    }
    printf("\n");

    // 6) Print numeric counts (all from the linked list + global total)
    printf("\nTCP : %d   UDP : %d   ICMP : %d   Others : %d   Total : %d\n",
           tcp_count, udp_count, icmp_count, other_count, total);
}


int main(int argc, char *argv[]) {
    pcap_if_t *alldevs, *dev;
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle;
    
    init_protocols();	
    // Find all devices
    if (pcap_findalldevs(&alldevs, errbuf) == -1) {
        fprintf(stderr, "Error finding devices: %s\n", errbuf);
        return (2);
    }

    // Use the first device
    dev = alldevs;
    if (dev == NULL) {
        fprintf(stderr, "No devices found.\n");
        return (2);
    }

    // Open the device for capturing
    handle = pcap_open_live(dev->name, BUFSIZ, 1, 1000, errbuf);
    if (handle == NULL) {
        fprintf(stderr, "Couldn't open device %s: %s\n", dev->name, errbuf);
        return (2);
    }

    // Start capturing packets
    pcap_loop(handle, 0, packet_handler, NULL);

    // Close the session
    pcap_close(handle);
    pcap_freealldevs(alldevs); // Free the list of devices
    return (0);
}
