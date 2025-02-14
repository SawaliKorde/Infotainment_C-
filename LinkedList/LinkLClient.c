#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

typedef struct Person {
    char name[50];
    int age;
    char email[50];
    struct Person *next;
} Person;

char *buffer;
void readData(int sock,Person *head) {
	int bytes = recv(sock,buffer,BUFFER_SIZE-1,0);
	if(bytes > 0){
		buffer[bytes] = '\0';
		printf("%s\n",buffer);
	}else{
		perror("Server response error");
	}
    /*Person *temp = head;
    printf("\n---- List of People ----\n");
    while (temp != NULL) {
        printf("Name: %s, Age: %d, Email: %s\n", temp->name, temp->age, temp->email);
        temp = temp->next;
    }*/
}

Person* createNode(char name[50],int age,char email[50]) {
	Person *newNode = (Person*)malloc(sizeof(Person));
	if (!newNode) {
	printf("Memory allocation error!\n");
	exit(1);
	}
	strcpy(newNode->name,name);
	newNode->age = age;
	strcpy(newNode->email,email);
	newNode->next = NULL;
	return newNode;
}	

void addPersonToNode(Person **head, char name[50], int age, char email[50]) {
	Person *newNode = createNode(name,age,email);
	newNode->next = *head;
	*head = newNode;
}

void displayList(Person *head) {
	Person *temp = head;
	printf("Displaying all Details: ");
	while (temp) {
		printf("%s -> %d -> %s ->", temp->name,temp->age,temp->email);
		temp = temp->next;
	}
	printf("NULL\n");
}
    
    
void addPerson(int sockfd) {
    Person person;
    getchar();
    
    printf("Enter Name: ");
    fgets(person.name, sizeof(person.name), stdin);
    person.name[strcspn(person.name, "\n")] = 0;

    printf("Enter Age: ");
    scanf("%d", &person.age);
    getchar();

    printf("Enter Email: ");
    fgets(person.email, sizeof(person.email), stdin);
    person.email[strcspn(person.email, "\n")] = 0;

    send(sockfd, &person, sizeof(person), 0);
    printf("Person added successfully!\n");
}

void searchPerson(int sockfd) {
    char name[50];
    getchar();
    
    printf("Enter Name to Search: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    send(sockfd, name, sizeof(name), 0);

    Person result;
    recv(sockfd, &result, sizeof(result), 0);
    if (strcmp(result.name, "Not Found") == 0) {
        printf("Person not found!\n");
    } else {
        printf("Found: %s, Age: %d, Email: %s\n", result.name, result.age, result.email);
    }
}

void deletePerson(int sockfd) {
    char name[50];
    getchar();
    
    printf("Enter Name to Delete: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    send(sockfd, name, sizeof(name), 0);
    printf("Person deleted successfully!\n");
}

void menu() {
    int choice;
    Person *head = NULL;
    while (1) {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;
        connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));

        printf("\n1. Add Person\n2. Search Person\n3. Delete Person\n4. Show All\n5. Exit\nSelect option: ");
        scanf("%d", &choice);
        send(sockfd, &choice, sizeof(choice), 0);

        if (choice == 1)
            addPerson(sockfd);
        else if (choice == 2)
            searchPerson(sockfd);
        else if (choice == 3)
            deletePerson(sockfd);
        else if (choice == 4){
       	    readData(sockfd,head);
            printf("Showing all persons (check server output).\n");
        }
        else if (choice == 5) {
            printf("Exiting...\n");
            close(sockfd);
            break;
        } else
            printf("Invalid choice!\n");

        close(sockfd);
    }
}

int main() {
    menu();
    return 0;
}

