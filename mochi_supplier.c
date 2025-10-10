#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============ STRUCTS ============
typedef struct {
    int bambooType;
    int quantity;
    char expiry_date[11];
    int supplierID;
} Shipment;

typedef struct {
    Shipment *shipments;
    int count;
    int capacity;
} ShipmentManager;

// ============ FUNCTION PROTOTYPES ============
void initManager(ShipmentManager *manager, int size);
void freeManager(ShipmentManager *manager);
void clearShipments(ShipmentManager *manager);

int readShipments(ShipmentManager *manager, const char *filename);
void printShipments(const ShipmentManager *manager);
int isValidDate(const char *date);

void addNewShipment(ShipmentManager *manager);
void saveShipments(const ShipmentManager *manager, const char *filename);

void removeShipment(ShipmentManager *manager);
void searchShipments(const ShipmentManager *manager);
void sortShipments(ShipmentManager *manager);
void generateReport(const ShipmentManager *manager, const char *filename);

// ============ HELPER FUNCTIONS ============
void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int isValidDate(const char *date) {
    if (strlen(date) != 10) return 0;
    if (date[4] != '-' || date[7] != '-') return 0;
    
    for (int i = 0; i < 10; i++) {
        if (i == 4 || i == 7) continue;
        if (date[i] < '0' || date[i] > '9') return 0;
    }
    return 1;
}

void initManager(ShipmentManager *manager, int size) {
    manager->shipments = (Shipment *)malloc(sizeof(Shipment) * size);
    manager->count = 0;
    manager->capacity = size;
}

void freeManager(ShipmentManager *manager) {
    free(manager->shipments);
}

void clearShipments(ShipmentManager *manager) {
    manager->count = 0;
}

void expandCapacity(ShipmentManager *manager) {
    manager->capacity *= 2;
    manager->shipments = (Shipment *)realloc(manager->shipments, 
                                             sizeof(Shipment) * manager->capacity);
}

// ============ READ SHIPMENTS ============
int readShipments(ShipmentManager *manager, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file '%s'\n", filename);
        return -1;
    }

    printf("\nReading shipments from '%s'...\n", filename);
    int loaded = 0;

    while (!feof(file)) {
        Shipment temp;
        if (fscanf(file, "%d %d %s %d", 
                   &temp.bambooType, &temp.quantity, 
                   temp.expiry_date, &temp.supplierID) == 4) {
            
            if (temp.bambooType >= 0 && temp.bambooType <= 9 && 
                temp.quantity > 0 && isValidDate(temp.expiry_date)) {
                
                if (manager->count == manager->capacity) {
                    expandCapacity(manager);
                }
                
                manager->shipments[manager->count] = temp;
                manager->count++;
                loaded++;
            }
        }
    }
    fclose(file);
    
    printf("Successfully loaded %d shipments.\n", loaded);
    return loaded;
}

// ============ PRINT SHIPMENTS ============
void printShipments(const ShipmentManager *manager) {
    if (manager->count == 0) {
        printf("\nNo shipments loaded.\n");
        return;
    }

    printf("\n%-8s %-10s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
    printf("---------------------------------------------\n");

    for (int i = 0; i < manager->count; i++) {
        printf("%-8d %-10d %-12s %-10d\n",
               manager->shipments[i].bambooType,
               manager->shipments[i].quantity,
               manager->shipments[i].expiry_date,
               manager->shipments[i].supplierID);
    }
}

// ============ ADD NEW SHIPMENT ============
void addNewShipment(ShipmentManager *manager) {
    Shipment newShip;
    
    printf("\n--- ADD NEW SHIPMENT ---\n");
    
    printf("Enter Bamboo Type (0-9): ");
    scanf("%d", &newShip.bambooType);
    if (newShip.bambooType < 0 || newShip.bambooType > 9) {
        printf("Invalid type.\n");
        clearInputBuffer();
        return;
    }
    
    printf("Enter Quantity: ");
    scanf("%d", &newShip.quantity);
    if (newShip.quantity <= 0) {
        printf("Invalid quantity.\n");
        clearInputBuffer();
        return;
    }
    
    printf("Enter Expiry Date (YYYY-MM-DD): ");
    scanf("%s", newShip.expiry_date);
    if (!isValidDate(newShip.expiry_date)) {
        printf("Invalid date format.\n");
        clearInputBuffer();
        return;
    }
    
    printf("Enter Supplier ID: ");
    scanf("%d", &newShip.supplierID);
    clearInputBuffer();
    
    if (manager->count == manager->capacity) {
        expandCapacity(manager);
    }
    
    manager->shipments[manager->count] = newShip;
    manager->count++;
    
    // Append to file
    FILE *file = fopen("shipments.txt", "a");
    if (file) {
        fprintf(file, "%d %d %s %d\n", 
                newShip.bambooType, newShip.quantity, 
                newShip.expiry_date, newShip.supplierID);
        fclose(file);
    }
    
    printf("Shipment added! Total: %d\n", manager->count);
}

// ============ SAVE SHIPMENTS ============
void saveShipments(const ShipmentManager *manager, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot save to file.\n");
        return;
    }
    
    for (int i = 0; i < manager->count; i++) {
        fprintf(file, "%d %d %s %d\n",
                manager->shipments[i].bambooType,
                manager->shipments[i].quantity,
                manager->shipments[i].expiry_date,
                manager->shipments[i].supplierID);
    }
    
    fclose(file);
    printf("Saved %d shipments to '%s'.\n", manager->count, filename);
}

// ============ REMOVE SHIPMENT ============
void removeShipment(ShipmentManager *manager) {
    if (manager->count == 0) {
        printf("\nNo shipments to remove.\n");
        return;
    }
    
    // Print with numbers
    printf("\n%-5s %-8s %-10s %-12s %-10s\n", "#", "Type", "Quantity", "Expiry", "Supplier");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < manager->count; i++) {
        printf("[%-3d] %-8d %-10d %-12s %-10d\n",
               i + 1,
               manager->shipments[i].bambooType,
               manager->shipments[i].quantity,
               manager->shipments[i].expiry_date,
               manager->shipments[i].supplierID);
    }
    
    printf("\nEnter shipment number to delete (1-%d), or 0 to cancel: ", 
           manager->count);
    int choice;
    scanf("%d", &choice);
    clearInputBuffer();
    
    if (choice == 0) {
        printf("Cancelled.\n");
        return;
    }
    
    if (choice < 1 || choice > manager->count) {
        printf("Invalid selection.\n");
        return;
    }
    
    int index = choice - 1;
    
    // Shift all elements after deleted one
    for (int i = index; i < manager->count - 1; i++) {
        manager->shipments[i] = manager->shipments[i + 1];
    }
    
    manager->count--;
    printf("Shipment deleted! Remaining: %d\n", manager->count);
}

// ============ SEARCH SHIPMENTS ============
void searchShipments(const ShipmentManager *manager) {
    if (manager->count == 0) {
        printf("\nNo shipments to search.\n");
        return;
    }
    
    printf("\n--- SEARCH SHIPMENTS ---\n");
    printf("[1] Search by Bamboo Type\n");
    printf("[2] Search by Supplier ID\n");
    printf("[3] Search by Date Range\n");
    printf("Enter choice: ");
    
    int choice;
    scanf("%d", &choice);
    clearInputBuffer();
    
    if (choice == 1) {
        int type;
        printf("Enter Bamboo Type (0-9): ");
        scanf("%d", &type);
        clearInputBuffer();
        
        printf("\nResults for Type %d:\n", type);
        printf("%-8s %-10s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
        printf("---------------------------------------------\n");
        
        int found = 0;
        for (int i = 0; i < manager->count; i++) {
            if (manager->shipments[i].bambooType == type) {
                printf("%-8d %-10d %-12s %-10d\n",
                       manager->shipments[i].bambooType,
                       manager->shipments[i].quantity,
                       manager->shipments[i].expiry_date,
                       manager->shipments[i].supplierID);
                found++;
            }
        }
        printf("Found %d shipments.\n", found);
    }
    else if (choice == 2) {
        int supplier;
        printf("Enter Supplier ID: ");
        scanf("%d", &supplier);
        clearInputBuffer();
        
        printf("\nResults for Supplier %d:\n", supplier);
        printf("%-8s %-10s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
        printf("---------------------------------------------\n");
        
        int found = 0;
        for (int i = 0; i < manager->count; i++) {
            if (manager->shipments[i].supplierID == supplier) {
                printf("%-8d %-10d %-12s %-10d\n",
                       manager->shipments[i].bambooType,
                       manager->shipments[i].quantity,
                       manager->shipments[i].expiry_date,
                       manager->shipments[i].supplierID);
                found++;
            }
        }
        printf("Found %d shipments.\n", found);
    }
    else if (choice == 3) {
        char start[11], end[11];
        printf("Enter Start Date (YYYY-MM-DD): ");
        scanf("%s", start);
        printf("Enter End Date (YYYY-MM-DD): ");
        scanf("%s", end);
        clearInputBuffer();
        
        printf("\nResults for date range %s to %s:\n", start, end);
        printf("%-8s %-10s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
        printf("---------------------------------------------\n");
        
        int found = 0;
        for (int i = 0; i < manager->count; i++) {
            if (strcmp(manager->shipments[i].expiry_date, start) >= 0 &&
                strcmp(manager->shipments[i].expiry_date, end) <= 0) {
                printf("%-8d %-10d %-12s %-10d\n",
                       manager->shipments[i].bambooType,
                       manager->shipments[i].quantity,
                       manager->shipments[i].expiry_date,
                       manager->shipments[i].supplierID);
                found++;
            }
        }
        printf("Found %d shipments.\n", found);
    }
}

// ============ SORT SHIPMENTS ============
int compareQuantity(const void *a, const void *b) {
    Shipment *s1 = (Shipment *)a;
    Shipment *s2 = (Shipment *)b;
    return s2->quantity - s1->quantity; // Descending
}

int compareType(const void *a, const void *b) {
    Shipment *s1 = (Shipment *)a;
    Shipment *s2 = (Shipment *)b;
    return s1->bambooType - s2->bambooType; // Ascending
}

int compareDate(const void *a, const void *b) {
    Shipment *s1 = (Shipment *)a;
    Shipment *s2 = (Shipment *)b;
    return strcmp(s1->expiry_date, s2->expiry_date); // Ascending
}

void sortShipments(ShipmentManager *manager) {
    if (manager->count == 0) {
        printf("\nNo shipments to sort.\n");
        return;
    }
    
    printf("\n--- SORT SHIPMENTS ---\n");
    printf("[1] Sort by Quantity (Largest First)\n");
    printf("[2] Sort by Bamboo Type (0-9)\n");
    printf("[3] Sort by Date (Earliest First)\n");
    printf("Enter choice: ");
    
    int choice;
    scanf("%d", &choice);
    clearInputBuffer();
    
    if (choice == 1) {
        qsort(manager->shipments, manager->count, sizeof(Shipment), compareQuantity);
        printf("Sorted by Quantity!\n");
    }
    else if (choice == 2) {
        qsort(manager->shipments, manager->count, sizeof(Shipment), compareType);
        printf("Sorted by Type!\n");
    }
    else if (choice == 3) {
        qsort(manager->shipments, manager->count, sizeof(Shipment), compareDate);
        printf("Sorted by Date!\n");
    }
    
    printShipments(manager);
}

// ============ GENERATE REPORT ============
void generateReport(const ShipmentManager *manager, const char *filename) {
    if (manager->count == 0) {
        printf("\nNo shipments to report.\n");
        return;
    }
    
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot create report.\n");
        return;
    }
    
    // Calculate totals by type
    int typeTotals[10] = {0};
    int totalAll = 0;
    
    for (int i = 0; i < manager->count; i++) {
        typeTotals[manager->shipments[i].bambooType] += manager->shipments[i].quantity;
        totalAll += manager->shipments[i].quantity;
    }
    
    // Write report
    fprintf(file, "========================================\n");
    fprintf(file, "   MOCHI'S BAMBOO INVENTORY REPORT\n");
    fprintf(file, "========================================\n\n");
    
    fprintf(file, "Total bamboo stock:\n");
    for (int i = 0; i < 10; i++) {
        fprintf(file, "Type %d: %d\n", i, typeTotals[i]);
    }
    
    // Find top 3 types
    fprintf(file, "\nTop 3 bamboo types: ");
    int printed = 0;
    for (int i = 0; i < 10 && printed < 3; i++) {
        int maxType = -1, maxQty = -1;
        for (int j = 0; j < 10; j++) {
            if (typeTotals[j] > maxQty) {
                maxQty = typeTotals[j];
                maxType = j;
            }
        }
        if (maxQty > 0) {
            if (printed > 0) fprintf(file, ", ");
            fprintf(file, "%d", maxType);
            typeTotals[maxType] = -1; // Mark as used
            printed++;
        }
    }
    fprintf(file, "\n");
    
    // Calculate supplier percentages
    fprintf(file, "\nSupplier statistics:\n");
    int supplierCounts[1000] = {0}; // Simple array for suppliers
    
    for (int i = 0; i < manager->count; i++) {
        supplierCounts[manager->shipments[i].supplierID] += manager->shipments[i].quantity;
    }
    
    for (int i = 0; i < 1000; i++) {
        if (supplierCounts[i] > 0) {
            double percent = (supplierCounts[i] * 100.0) / totalAll;
            fprintf(file, "Supplier %d: %.1f%%\n", i, percent);
        }
    }
    
    fprintf(file, "\n========================================\n");
    fclose(file);
    
    printf("Report generated: '%s'\n", filename);
}

// ============ MAIN ============
int main() {
    ShipmentManager manager;
    initManager(&manager, 10);
    
    printf("=== MOCHI'S BAMBOO SUPPLY MANAGER ===\n");
    
    while (1) {
        printf("\n--- MENU ---\n");
        printf("[1] Read Shipments from File\n");
        printf("[2] Add New Shipment\n");
        printf("[3] Save Shipments to File\n");
        printf("[4] Remove Shipment\n");
        printf("[5] Search Shipments\n");
        printf("[6] Sort Shipments\n");
        printf("[7] Generate Report\n");
        printf("[8] Exit\n");
        printf("Enter choice: ");
        
        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input!\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();
        
        switch (choice) {
            case 1:
                clearShipments(&manager);
                readShipments(&manager, "shipments.txt");
                printShipments(&manager);
                break;
            case 2:
                addNewShipment(&manager);
                break;
            case 3:
                saveShipments(&manager, "shipments.txt");
                break;
            case 4:
                removeShipment(&manager);
                break;
            case 5:
                searchShipments(&manager);
                break;
            case 6:
                sortShipments(&manager);
                break;
            case 7:
                generateReport(&manager, "report.txt");
                break;
            case 8:
                printf("Goodbye!\n");
                freeManager(&manager);
                return 0;
            default:
                printf("Invalid choice!\n");
        }
    }
}