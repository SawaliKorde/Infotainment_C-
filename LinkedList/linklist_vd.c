

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Data {
    char name[50];
    int age;
    char email[50];
    struct Data *next;
} Data;


void saveToFile(Data *head);
void displayList(Data *head);
void freeList(Data *head);
void addData(Data **head, char *name, int age, char *email);

void addData(Data **head, char *name, int age, char *email) {
    Data *newNode = (Data *)malloc(sizeof(Data));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return;
    }

    strcpy(newNode->name, name);
    newNode->age = age;
    strcpy(newNode->email, email);
    newNode->next = NULL;

    if (*head == NULL) {
        *head = newNode;
    } else {
        Data *temp = *head;
        while (temp->next)
            temp = temp->next;
        temp->next = newNode;
    }
}


void saveToFile(Data *head) {
    FILE *fp = fopen("td.txt", "w");
    if (!fp) {
        printf("Error opening file!\n");
        return;
    }

    Data *temp = head;
    while (temp) {
        fprintf(fp, "%s %d %s\n", temp->name, temp->age, temp->email);
        temp = temp->next;
    }

    fclose(fp);
}

void displayList(Data *head) {
    if (head == NULL) {
        printf("List is empty.\n");
        return;
    }

    Data *temp = head;
    printf("\nDISPLAYING LIST\n");
    printf("-----------------------------\n");

    while (temp) {
        printf("Name: %s\n", temp->name);
        printf("Age: %d\n", temp->age);
        printf("Email: %s\n", temp->email);
        printf("-----------------------------\n");
        temp = temp->next;
    }
}

void freeList(Data *head) {
    Data *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main() {
    Data *head = NULL;
    int ch;
    char name[50];
    int age;
    char email[50];

    do {
        printf("==========\n");
        printf("1: ADD New DATA\n");
        printf("2: SHOW DATA\n");
        printf("3: EXIT\n");
        printf("==========\n");
        printf("ENTER Choice::>> ");
        scanf("%d", &ch);
        getchar(); // Consume newline to prevent fgets() issues

        switch (ch) {
            case 1:
                printf("ENTER NAME: ");
                fgets(name, 50, stdin);
                name[strcspn(name, "\n")] = 0; 

                printf("ENTER AGE: ");
                scanf("%d", &age);
                getchar(); // Consume newline

                printf("ENTER EMAIL: ");
                fgets(email, 50, stdin);
                email[strcspn(email, "\n")] = 0;

                addData(&head, name, age, email);
                saveToFile(head); // Save to file after adding
                break;

            case 2:
                displayList(head);
                break;

            case 3:
                printf("=========EXITING==========\n");
                break;

            default:
                printf("INVALID CHOICE\n");
                break;
        }
    } while (ch != 3);

    freeList(head);
    return 0;
}
