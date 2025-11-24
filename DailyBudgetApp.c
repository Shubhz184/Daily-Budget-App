#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// IMPROVEMENT: Moved struct definitions to the top for clarity
typedef struct TransactionNode
{
    int date;
    float amount;
    char category[50];
    struct TransactionNode *next;
} TransactionNode;

typedef struct DayRecordNode
{
    int date;
    float budget;
    float remaining;
    struct TransactionNode *transactions; // CHANGE: Added pointer to store transactions for this day
    struct DayRecordNode *next;
} DayRecordNode;

// HELPER: Free memory to prevent leaks when reloading or exiting
void freeMonthRecord(DayRecordNode *head) {
    DayRecordNode *current = head;
    while (current != NULL) {
        DayRecordNode *nextDay = current->next;
        
        // Free transactions inside the day
        TransactionNode *tCurrent = current->transactions;
        while (tCurrent != NULL) {
            TransactionNode *tNext = tCurrent->next;
            free(tCurrent);
            tCurrent = tNext;
        }

        free(current);
        current = nextDay;
    }
}

DayRecordNode *createMonthRecord(float monthlyBudget, int daysInMonth)
{
    DayRecordNode *head = NULL, *temp, *newNode;
    float dailyBudget = monthlyBudget / daysInMonth;

    for (int i = 1; i <= daysInMonth; i++)
    {
        newNode = (DayRecordNode *)malloc(sizeof(DayRecordNode));
        newNode->date = i;
        newNode->budget = dailyBudget;
        newNode->remaining = dailyBudget;
        newNode->transactions = NULL; // Initialize to NULL
        newNode->next = NULL;

        if (head == NULL)
        {
            head = newNode;
        }
        else
        {
            temp->next = newNode;
        }
        temp = newNode;
    }
    return head;
}

void addTransaction(DayRecordNode *monthlyRecord, int date, float amount, char *category, int daysInMonth)
{
    DayRecordNode *dayRecord = monthlyRecord;

    while (dayRecord != NULL && dayRecord->date != date)
    {
        dayRecord = dayRecord->next;
    }

    if (dayRecord == NULL)
    {
        printf("Invalid date!\n");
        return;
    }

    // 1. Create the transaction
    TransactionNode *newTransaction = (TransactionNode *)malloc(sizeof(TransactionNode));
    newTransaction->date = date;
    newTransaction->amount = amount;
    strcpy(newTransaction->category, category);
    newTransaction->next = NULL;

    // 2. LINK THE TRANSACTION (FIXED BUG)
    // Insert at beginning of the list for this day (O(1) operation)
    newTransaction->next = dayRecord->transactions;
    dayRecord->transactions = newTransaction;

    // 3. Update Budget Logic
    float difference = dayRecord->remaining - amount;
    if (difference >= 0)
    {
        dayRecord->remaining = difference;
    }
    else
    {
        dayRecord->remaining = 0;
        
        // FIX: Check to ensure we don't divide by zero if it's the last day
        int remainingDays = daysInMonth - date;
        
        if (remainingDays > 0) {
            float deficitPerDay = -difference / remainingDays;
            DayRecordNode *temp = dayRecord->next;
            while (temp != NULL)
            {
                temp->remaining -= deficitPerDay;
                temp = temp->next;
            }
            printf("Deficit distributed over remaining %d days.\n", remainingDays);
        } else {
            printf("Warning: Over budget on the last day! No days left to distribute deficit.\n");
        }
    }
}

void displayRemainingBudget(DayRecordNode *monthlyRecord, int date)
{
    DayRecordNode *dayRecord = monthlyRecord;

    while (dayRecord != NULL && dayRecord->date != date)
    {
        dayRecord = dayRecord->next;
    }

    if (dayRecord == NULL)
    {
        printf("Invalid date!\n");
    }
    else
    {
        printf("\n--- Status for Day %d ---\n", date);
        printf("Remaining budget: %.2f\n", dayRecord->remaining);
        
        // Added: Display transactions for verification
        TransactionNode *t = dayRecord->transactions;
        if(t != NULL) printf("Transactions:\n");
        while(t != NULL) {
            printf(" - %s: %.2f\n", t->category, t->amount);
            t = t->next;
        }
        printf("-------------------------\n");
    }
}

void saveDataToFile(DayRecordNode *monthlyRecord, char *filename)
{
    FILE *fp = fopen(filename, "w");
    if (fp == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    DayRecordNode *dayRecord = monthlyRecord;

    // Note: Currently only saving daily summaries, not individual transaction history
    // to keep the CSV format simple as per original design.
    while (dayRecord != NULL)
    {
        fprintf(fp, "%d,%.2f,%.2f\n", dayRecord->date, dayRecord->budget, dayRecord->remaining);
        dayRecord = dayRecord->next;
    }

    fclose(fp);
    printf("Data saved successfully!\n");
}

DayRecordNode *loadDataFromFile(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error opening file or file not found!\n");
        return NULL;
    }

    DayRecordNode *head = NULL, *temp, *newNode;
    int date;
    float budget, remaining;

    while (fscanf(fp, "%d,%f,%f", &date, &budget, &remaining) == 3)
    {
        newNode = (DayRecordNode *)malloc(sizeof(DayRecordNode));
        newNode->date = date;
        newNode->budget = budget;
        newNode->remaining = remaining;
        newNode->transactions = NULL; // Initialize transactions as empty
        newNode->next = NULL;

        if (head == NULL)
        {
            head = newNode;
        }
        else
        {
            temp->next = newNode;
        }
        temp = newNode;
    }

    fclose(fp);
    return head;
}

int main()
{
    float monthlyBudget;
    int daysInMonth = 30; // Default
    int choice, date;
    float amount;
    char category[50];
    char filename[50] = "budget_data.csv";
    
    // Initial setup
    DayRecordNode *monthlyRecord = NULL;

    printf("--- Budget Manager ---\n");

    do
    {
        printf("\nMenu:\n");
        printf("1. Initialize New Month\n"); // Changed logic to separate init
        printf("2. Add Transaction\n");
        printf("3. View Remaining Budget\n");
        printf("4. Save Data\n");
        printf("5. Load Data\n");
        printf("6. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice)
        {
        case 1:
            if (monthlyRecord != NULL) freeMonthRecord(monthlyRecord);
            printf("Enter monthly budget: ");
            scanf("%f", &monthlyBudget);
            printf("Enter number of days in the month: ");
            scanf("%d", &daysInMonth);
            monthlyRecord = createMonthRecord(monthlyBudget, daysInMonth);
            printf("Month initialized.\n");
            break;
        case 2:
            if (monthlyRecord == NULL) {
                 printf("Please Initialize or Load a month first.\n"); 
                 break; 
            }
            printf("Enter date (1-%d): ", daysInMonth);
            scanf("%d", &date);
            printf("Enter amount: ");
            scanf("%f", &amount);
            
            // FIX: Buffer flush and safe string input
            while(getchar() != '\n'); 
            printf("Enter category: ");
            fgets(category, 50, stdin);
            category[strcspn(category, "\n")] = 0; // Remove newline char

            addTransaction(monthlyRecord, date, amount, category, daysInMonth);
            break;
        case 3:
            if (monthlyRecord == NULL) {
                 printf("Please Initialize or Load a month first.\n"); 
                 break; 
            }
            printf("Enter date (1-%d): ", daysInMonth);
            scanf("%d", &date);
            displayRemainingBudget(monthlyRecord, date);
            break;
        case 4:
            if (monthlyRecord != NULL) saveDataToFile(monthlyRecord, filename);
            else printf("No data to save.\n");
            break;
        case 5:
            if (monthlyRecord != NULL) freeMonthRecord(monthlyRecord); // clean up old data
            monthlyRecord = loadDataFromFile(filename);
            if (monthlyRecord == NULL)
            {
                printf("No saved data found.\n");
            }
            else 
            {
                printf("Data loaded.\n");
                // Update daysInMonth based on loaded data (count nodes)
                DayRecordNode *temp = monthlyRecord;
                daysInMonth = 0;
                while(temp != NULL) { daysInMonth++; temp = temp->next; }
            }
            break;
        case 6:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid choice!\n");
        }
    } while (choice != 6);

    if (monthlyRecord != NULL) freeMonthRecord(monthlyRecord);

    return 0;
}
