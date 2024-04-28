#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_OPTIONS 4
#define MAX_QUESTIONS 100
#define PASSWORD_LENGTH 20

// Question structure
typedef struct Question {
    char question[100];
    char options[MAX_OPTIONS][50];
    int correctOption;
    struct Question* next;
} Question;

// Hash table structure
typedef struct HashTable {
    Question* table[MAX_QUESTIONS];
} HashTable;

// Function prototypes
void addQuestion(HashTable* hashTable, Question** head, int* numQuestions);
void deleteQuestion(HashTable* hashTable, Question** head, int* numQuestions, int index);
void displayQuestions(Question* head);
void takeQuiz(Question* head);
void gradeQuiz(Question* head, int userAnswers[]);
void setPassword(char password[]);
bool verifyPassword(char password[], char inputPassword[]);
int hash(char* question);

int hash(char* question) {
    int sum = 0;
    for (int i = 0; question[i]; i++) {
        sum += question[i];
    }
    return sum % MAX_QUESTIONS;
}

int main() {
    Question* head = NULL;
    HashTable hashTable = {0};
    int numQuestions = 0;
    int choice;
    char password[PASSWORD_LENGTH] = {0};
    bool passwordSet = false;

    while (true) {
        printf("\nMain Menu:\n");
        printf("1. Are you a teacher?\n");
        printf("2. Are you a student?\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        getchar();  // clear newline left in the buffer

        if (choice == 1) {  // Teacher
            if (!passwordSet) {
                printf("Setting up password...\n");
                setPassword(password);
                passwordSet = true;
                printf("Password set successfully!\n");
            }
            char inputPassword[PASSWORD_LENGTH];
            printf("Enter password to access teacher menu: ");
            fgets(inputPassword, PASSWORD_LENGTH, stdin);
            inputPassword[strcspn(inputPassword, "\n")] = 0;  // Remove newline char

            if (verifyPassword(password, inputPassword)) {
                do {
                    printf("\nTeacher Menu:\n");
                    printf("1. Add a question\n");
                    printf("2. Delete a question\n");
                    printf("3. Display all questions\n");
                    printf("4. Take the quiz\n");
                    printf("5. Back to main menu\n");
                    printf("Enter your choice: ");
                    scanf("%d", &choice);
                    getchar();  // clear newline

                    switch (choice) {
                        case 1:
                            addQuestion(&hashTable, &head, &numQuestions);
                            break;
                        case 2: {
                            int index;
                            printf("Enter the index of the question to delete: ");
                            scanf("%d", &index);
                            getchar();  // clear newline
                            deleteQuestion(&hashTable, &head, &numQuestions, index);
                            break;
                        }
                        case 3:
                            displayQuestions(head);
                            break;
                        case 4:
                            takeQuiz(head);
                            break;
                        case 5:
                            printf("Returning to main menu...\n");
                            break;
                        default:
                            printf("Invalid choice. Please try again.\n");
                    }
                } while (choice != 5);
            } else {
                printf("Incorrect password. Access denied.\n");
            }
        } else if (choice == 2) {  // Student
            takeQuiz(head);
        } else if (choice == 3) {
            printf("Exiting...\n");
            break;
        } else {
            printf("Invalid choice. Please try again.\n");
        }
    }

    // Free memory allocated for questions
    while (head != NULL) {
        Question* temp = head;
        head = head->next;
        free(temp);
    }

    return 0;
}

void addQuestion(HashTable* hashTable, Question** head, int* numQuestions) {
    if (*numQuestions >= MAX_QUESTIONS) {
        printf("Cannot add more questions. Maximum limit reached.\n");
        return;
    }

    Question* newQuestion = (Question*)malloc(sizeof(Question));
    if (!newQuestion) {
        printf("Memory allocation failed.\n");
        return;
    }

    printf("Enter the question: ");
    fgets(newQuestion->question, sizeof(newQuestion->question), stdin);
    newQuestion->question[strcspn(newQuestion->question, "\n")] = 0;  // Remove newline

    printf("Enter the options:\n");
    for (int i = 0; i < MAX_OPTIONS; i++) {
        printf("Option %d: ", i + 1);
        fgets(newQuestion->options[i], sizeof(newQuestion->options[i]), stdin);
        newQuestion->options[i][strcspn(newQuestion->options[i], "\n")] = 0;  // Remove newline
    }

    printf("Enter the index of the correct option (1-4): ");
    scanf("%d", &newQuestion->correctOption);
    getchar();  // clear newline
    newQuestion->correctOption -= 1;  // Adjust for 0-based index

    // Add to the end of the linked list to maintain order
    if (*head == NULL) {
        *head = newQuestion;
        newQuestion->next = NULL;
    } else {
        Question* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = newQuestion;
        newQuestion->next = NULL;
    }

    int index = hash(newQuestion->question);
    // Add to hash table at the beginning of the list at index
    newQuestion->next = hashTable->table[index];
    hashTable->table[index] = newQuestion;

    (*numQuestions)++;
    printf("Question added successfully!\n");
}


void deleteQuestion(HashTable* hashTable, Question** head, int* numQuestions, int index) {
    if (index < 1 || index > *numQuestions) {
        printf("Invalid index.\n");
        return;
    }

    Question* current = *head;
    Question* previous = NULL;

    // Find the question at 'index'
    for (int i = 1; current != NULL && i < index; i++) {
        previous = current;
        current = current->next;
    }

    if (current == NULL) {
        printf("Question not found.\n");
        return;
    }

    if (previous == NULL) {
        *head = current->next; // Remove the head question
    } else {
        previous->next = current->next; // Unlink the question
    }

    int hashIndex = hash(current->question);
    Question* hashPrev = NULL;
    Question* hashCurr = hashTable->table[hashIndex];

    while (hashCurr != NULL && hashCurr != current) {
        hashPrev = hashCurr;
        hashCurr = hashCurr->next;
    }

    if (hashPrev == NULL) {
        hashTable->table[hashIndex] = hashCurr->next; // Remove from hash table head
    } else {
        hashPrev->next = hashCurr->next; // Unlink from hash table
    }

    free(current);
    (*numQuestions)--;
    printf("Question deleted successfully!\n");
}

void displayQuestions(Question* head) {
    if (!head) {
        printf("No questions available.\n");
        return;
    }

    Question* current = head;
    int qNum = 1;
    while (current) {
        printf("%d. %s\n", qNum++, current->question);
        for (int i = 0; i < MAX_OPTIONS; i++) {
            printf("   %c) %s\n", 'A' + i, current->options[i]);
        }
        printf("Correct option: %c\n", 'A' + current->correctOption);
        current = current->next;
    }
}

void takeQuiz(Question* head) {
    if (!head) {
        printf("No questions to display.\n");
        return;
    }

    Question* current = head;
    int userAnswers[MAX_QUESTIONS], qNum = 0, score = 0;
    while (current) {
        printf("\nQuestion %d: %s\n", qNum + 1, current->question);
        for (int i = 0; i < MAX_OPTIONS; i++) {
            printf("   %c) %s\n", 'A' + i, current->options[i]);
        }
        char answer;
        printf("Your answer (Enter option A/B/C/D in Capital Letter): ");
        scanf(" %c", &answer);
        userAnswers[qNum] = answer - 'A';
        if (userAnswers[qNum] == current->correctOption) {
            score++;
        }
        current = current->next;
        qNum++;
    }

    printf("\nYour score: %d/%d\n", score, qNum);
}

void gradeQuiz(Question* head, int userAnswers[]) {
    Question* current = head;
    int score = 0;
    int index = 0;
    while (current != NULL) {
        if (userAnswers[index] == current->correctOption) {
            score++;
        }
        current = current->next;
        index++;
    }
    printf("\nYour score: %d/%d\n", score, index);
}

void setPassword(char password[]) {
    printf("Enter a new password (no spaces): ");
    fgets(password, PASSWORD_LENGTH, stdin);
    password[strcspn(password, "\n")] = 0; // Remove newline char
}

bool verifyPassword(char password[], char inputPassword[]) {
    return strcmp(password, inputPassword) == 0;
}
