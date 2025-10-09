#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// ============ STRUCTS ============ //
typedef struct {
    int bambooType;        // valid 0 - 9
    int quantity;          // > 0
    char expiry_date[11];  // "YYYY-MM-DD"
    int supplierID;
} MyShipments;

typedef struct {
    MyShipments *shipments;
    int countOfShipments;
    int maxCapacity;
} ShipmentManager;

// ============ PROTOTYPES ============ //
ShipmentManager *create_manager(int initialCapacity);
void free_manager(ShipmentManager *m);
void clear_shipments(ShipmentManager *m);

int readShipments(ShipmentManager *manager, const char *filename);
void print_shipments(const ShipmentManager *m);
static int valid_date_yyyy_mm_dd(const char *s);

int appendShipmentToFile(const MyShipments *s, const char *filename);
int saveShipments(const ShipmentManager *manager, const char *filename);

void addNewShipment(ShipmentManager *manager);
int generateReport(const ShipmentManager *manager, const char *filename);

void removeSpoiledShipments(ShipmentManager *manager);
void print_shipments_numbered(const ShipmentManager *m);

void searchShipments(const ShipmentManager *manager);

void sortShipments(ShipmentManager *manager);

// ============ IMPLEMENTATION ============ //
ShipmentManager *create_manager(int initialCapacity) {
    if (initialCapacity < 1) initialCapacity = 1;
    ShipmentManager *m = (ShipmentManager *)malloc(sizeof *m);
    if (!m) return NULL;
    m->shipments = (MyShipments *)malloc(sizeof(MyShipments) * initialCapacity);
    if (!m->shipments) { free(m); return NULL; }
    m->countOfShipments = 0;
    m->maxCapacity = initialCapacity;
    return m;
}

void free_manager(ShipmentManager *m) {
    if (!m) return;
    free(m->shipments);
    free(m);
}

void clear_shipments(ShipmentManager *m) {
    if (!m) return;
    m->countOfShipments = 0; // keep capacity/buffer; just reset count
}


// Validate date format "YYYY-MM-DD"
static int valid_date_yyyy_mm_dd(const char *s) {
    if (!s) {
        printf("Invalid date: (null) pointer.\n");
        return 0;
    }
    if (strlen(s) != 10) {
        printf("Invalid date '%s': expected length 10 in format YYYY-MM-DD.\n", s);
        return 0;
    }
    if (s[4] != '-' || s[7] != '-') {
        printf("Invalid date '%s': expected '-' at positions 5 and 8 (YYYY-MM-DD).\n", s);
        return 0;
    }

    // Verify digits in all non-hyphen positions
    for (int i = 0; i < 10; ++i) {
        if (i == 4 || i == 7) continue;
        if (s[i] < '0' || s[i] > '9') {
            printf("Invalid date '%s': non-digit character at position %d.\n", s, i + 1);
            return 0;
        }
    }

    // Parse month and day
    int month = (s[5]-'0')*10 + (s[6]-'0');
    int day   = (s[8]-'0')*10 + (s[9]-'0');

    if (month < 1 || month > 12) {
        printf("Invalid date '%s': month %d must be between 1 and 12.\n", s, month);
        return 0;
    }
    if (day < 1 || day > 31) {
        printf("Invalid date '%s': day %d must be between 1 and 31.\n", s, day);
        return 0;
    }

    return 1;
}

// ----------------------------------------------- LOAD & READ SHIPMENTS -----------------------------------------------

// Read Shipments from a file (append into in-memory array)
int readShipments(ShipmentManager *manager, const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open file '%s'\n", filename);
        printf("Make sure the file exists in the current directory.\n");
        return -1;
    }

    printf("\nReading shipments from '%s'...\n", filename);

    char line[256];
    int line_number = 0;
    int loaded_count = 0;

    while (fgets(line, sizeof(line), file)) {
        line_number++;

        // trim leading whitespace
        char *p = line;
        while (*p==' ' || *p=='\t' || *p=='\r' || *p=='\n') p++;

        // skip blank or commented lines
        if (*p=='\0' || *p=='#' || (p[0]=='/' && p[1]=='/')) continue;

        int type, qty, supplier;
        char date[16];
        if (sscanf(p, "%d %d %15s %d", &type, &qty, date, &supplier) != 4) {
            printf("Warning: Line %d - Could not parse 4 fields. Skipping.\n", line_number);
            continue;
        }
        if (type < 0 || type > 9 || qty <= 0 || !valid_date_yyyy_mm_dd(date)) {
            printf("Warning: Line %d - Invalid data. Skipping.\n", line_number);
            continue;
        }

        // ensure capacity
        if (manager->countOfShipments == manager->maxCapacity) {
            int newCap = (manager->maxCapacity < 1024)
                       ? manager->maxCapacity * 2
                       : manager->maxCapacity + manager->maxCapacity / 2;
            MyShipments *nb = (MyShipments *)realloc(manager->shipments, sizeof(MyShipments)*newCap);
            if (!nb) { fclose(file); return -1; }
            manager->shipments = nb;
            manager->maxCapacity = newCap;
        }

        // append
        MyShipments *dst = &manager->shipments[manager->countOfShipments++];
        dst->bambooType = type;
        dst->quantity   = qty;
        strncpy(dst->expiry_date, date, sizeof(dst->expiry_date));
        dst->expiry_date[10] = '\0';
        dst->supplierID = supplier;
        loaded_count++;
    }
    fclose(file);

    if (loaded_count == 0) {
        printf("No valid shipments found in file.\n");
    } else {
        printf("Successfully loaded %d shipment(s) from file. Total in memory: %d\n",
               loaded_count, manager->countOfShipments);
    }
    return loaded_count;
}

// Print all shipments in a table
void print_shipments(const ShipmentManager *m) {
    if (m->countOfShipments == 0) {
        printf("\nNo shipments loaded.\n");
        return;
    }

    printf("\n%-8s %-8s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
    printf("----------------------------------------\n");

    for (int i = 0; i < m->countOfShipments; ++i) {
        const MyShipments *s = &m->shipments[i];
        printf("%-8d %-8d %-12s %-10d\n",
               s->bambooType, s->quantity, s->expiry_date, s->supplierID);
    }
}

// ----------------------------------------------- ADD & SAVE SHIPMENTS -----------------------------------------------

// Append a single shipment to the end of the file (no overwrite)
int appendShipmentToFile(const MyShipments *s, const char *filename) {
    FILE *fp = fopen(filename, "a");  // append mode
    if (!fp) {
        printf("Error: could not open '%s' for appending.\n", filename);
        return -1;
    }
    if (fprintf(fp, "%d %d %s %d\n",
                s->bambooType, s->quantity, s->expiry_date, s->supplierID) < 0) {
        printf("Error: write failed.\n");
        fclose(fp);
        return -1;
    }
    if (fclose(fp) != 0) {
        printf("Warning: error closing '%s' after appending.\n", filename);
        return -1;
    }
    return 0;
}

// "Save All" (overwrite)
int saveShipments(const ShipmentManager *manager, const char *filename) {
    FILE *fp = fopen(filename, "w");  // overwrite with current list
    if (!fp) { printf("Error: could not open '%s' for writing.\n", filename); return -1; }
    for (int i = 0; i < manager->countOfShipments; ++i) {
        const MyShipments *s = &manager->shipments[i];
        fprintf(fp, "%d %d %s %d\n", s->bambooType, s->quantity, s->expiry_date, s->supplierID);
    }
    fclose(fp);
    return manager->countOfShipments;
}

// Add a new shipment via user input
void addNewShipment(ShipmentManager *manager) {
    int type, qty, supplier;
    char date[20];

    printf("\n<============ ADD NEW SHIPMENT ============>\n");

    // user input
    printf("Enter Bamboo Type (0 - 9): ");
    if (scanf("%d", &type) != 1 || type < 0 || type > 9) {
        printf("Invalid bamboo type.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }

    printf("Enter Quantity (>0): ");
    if (scanf("%d", &qty) != 1 || qty <= 0) {
        printf("Invalid quantity.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }

    printf("Enter Expiry Date (YYYY-MM-DD): ");
    if (scanf("%19s", date) != 1 || !valid_date_yyyy_mm_dd(date)) {
        printf("Invalid date format.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }

    printf("Enter Supplier ID: ");
    if (scanf("%d", &supplier) != 1) {
        printf("Invalid supplier ID.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }

    // clear leftover input
    { int ch; while ((ch = getchar()) != '\n' && ch != EOF) {} }

    // ensure capacity can expand
    if (manager->maxCapacity < 1) manager->maxCapacity = 1; // safety
    if (manager->countOfShipments == manager->maxCapacity) {
        int newCap = (manager->maxCapacity < 1024)
                   ? manager->maxCapacity * 2
                   : manager->maxCapacity + manager->maxCapacity / 2;
        MyShipments *newBuf = (MyShipments *)realloc(manager->shipments, sizeof(MyShipments) * newCap);
        if (!newBuf) {
            printf("Error: memory allocation failed.\n");
            return;
        }
        manager->shipments  = newBuf;
        manager->maxCapacity = newCap;
    }

    // append in memory
    MyShipments *s = &manager->shipments[manager->countOfShipments++];
    s->bambooType = type;
    s->quantity   = qty;
    strncpy(s->expiry_date, date, sizeof(s->expiry_date));
    s->expiry_date[sizeof(s->expiry_date) - 1] = '\0';
    s->supplierID = supplier;

    printf("\nNew shipment added! Total shipments: %d\n", manager->countOfShipments);

    // auto-append to file (no overwrite)
    if (appendShipmentToFile(s, "shipments.txt") == 0) {
        printf("Appended to 'shipments.txt'.\n");
    } else {
        printf("Could not append to 'shipments.txt'.\n");
    }
}

// Print the shipments with numbering for user selection
void print_shipments_numbered(const ShipmentManager *m) {
    if (!m || m->countOfShipments == 0) {
        printf("\nNo shipments loaded.\n");
        return;
    }

    printf("\n%-5s %-8s %-8s %-12s %-10s\n", "#", "Type", "Quantity", "Expiry", "Supplier");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < m->countOfShipments; ++i) {
        const MyShipments *s = &m->shipments[i];
        printf("[%d]   %-8d %-8d %-12s %-10d\n",
               i + 1, s->bambooType, s->quantity, s->expiry_date, s->supplierID);
    }
}

// helper: optionally shrink backing array to free memory
static void maybe_shrink_buffer(ShipmentManager *m) {
    if (!m) return;
    // shrink when usage < 50% and capacity is reasonably large
    if (m->maxCapacity > 8 && m->countOfShipments < m->maxCapacity / 2) {
        int newCap = m->maxCapacity / 2;
        if (newCap < m->countOfShipments) newCap = m->countOfShipments;
        MyShipments *nb = (MyShipments *)realloc(m->shipments, sizeof(MyShipments) * newCap);
        if (nb) {
            m->shipments = nb;
            m->maxCapacity = newCap;
        }
    }
}

// ----------------------------------------------- REMOVING SHIPMENTS -----------------------------------------------

// Remove spoiled shipments by user selection
void removeSpoiledShipments(ShipmentManager *manager) {
    if (!manager || manager->countOfShipments == 0) {
        printf("\nNo shipments to remove. Read (1) or Add (2) first.\n");
        return;
    }

    while (1) {
        print_shipments_numbered(manager);

        printf("\nEnter the number to delete (1-%d), or 0 to cancel: ",
               manager->countOfShipments);

        int choice;
        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        // clear trailing newline
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}

        if (choice == 0) {
            printf("Removal cancelled.\n");
            return;
        }
        if (choice < 1 || choice > manager->countOfShipments) {
            printf("Invalid selection. Choose between 1 and %d (or 0 to cancel).\n",
                   manager->countOfShipments);
            continue;
        }

        int idx = choice - 1;
        MyShipments removed = manager->shipments[idx];

        // shift left to fill the gap
        for (int i = idx; i < manager->countOfShipments - 1; ++i) {
            manager->shipments[i] = manager->shipments[i + 1];
        }
        manager->countOfShipments--;

        // optional: zero the now-unused last slot (not required, but neat)
        if (manager->countOfShipments >= 0) {
            MyShipments *tail = &manager->shipments[manager->countOfShipments];
            memset(tail, 0, sizeof(*tail));
        }

        // maybe shrink to free memory
        maybe_shrink_buffer(manager);

        printf("Deleted shipment: \n");
        printf("Type = %d\n",removed.bambooType);
        printf("Quantity = %d\n",removed.quantity);
        printf("Date = %s\n",removed.expiry_date);
        printf("Supplier ID = %d\n",removed.supplierID);

        if (manager->countOfShipments == 0) {
            printf("All shipments removed.\n");
            break;
        }

        // ask if they want to delete more in the same session
        printf("\nDelete another? (y/n): ");
        int ans = getchar();
        while (ans == '\n') ans = getchar();
        int c2; while ((c2 = getchar()) != '\n' && c2 != EOF) {} // clear rest of line
        if (ans != 'y' && ans != 'Y') break;
    }

    printf("\nUpdated Shipments List:\n");
    print_shipments_numbered(manager);
    printf("\nNote: Deletions are in-memory. Use option 3 (Save Shipments to File - overwrite) to persist.\n");
}

// ----------------------------------------------- SEARCH -----------------------------------------------

// Date comparison helper
static int compare_dates(const char *date1, const char *date2) {
    // Compare dates in YYYY-MM-DD format lexicographically
    return strcmp(date1, date2);
}

// Search by type helper
static void search_by_type(const ShipmentManager *manager) {
    int type;
    printf("\nEnter Bamboo Type to search (0-9): ");
    if (scanf("%d", &type) != 1 || type < 0 || type > 9) {
        printf("Invalid bamboo type.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
    
    printf("\n<============ SEARCH RESULTS: Bamboo Type %d ============>\n", type);
    int found = 0;
    
    printf("\n%-8s %-8s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < manager->countOfShipments; ++i) {
        const MyShipments *s = &manager->shipments[i];
        if (s->bambooType == type) {
            printf("%-8d %-8d %-12s %-10d\n",
                   s->bambooType, s->quantity, s->expiry_date, s->supplierID);
            found++;
        }
    }
    
    if (found == 0) {
        printf("No shipments found for bamboo type %d.\n", type);
    } else {
        printf("\nTotal shipments found: %d\n", found);
    }
}

// Search by supplier helper
static void search_by_supplier(const ShipmentManager *manager) {
    int supplier;
    printf("\nEnter Supplier ID to search: ");
    if (scanf("%d", &supplier) != 1) {
        printf("Invalid supplier ID.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
    
    printf("\n<============ SEARCH RESULTS: Supplier %d ============>\n", supplier);
    int found = 0;
    
    printf("\n%-8s %-8s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < manager->countOfShipments; ++i) {
        const MyShipments *s = &manager->shipments[i];
        if (s->supplierID == supplier) {
            printf("%-8d %-8d %-12s %-10d\n",
                   s->bambooType, s->quantity, s->expiry_date, s->supplierID);
            found++;
        }
    }
    
    if (found == 0) {
        printf("No shipments found for supplier %d.\n", supplier);
    } else {
        printf("\nTotal shipments found: %d\n", found);
    }
}

// Search by date range helper
static void search_by_date_range(const ShipmentManager *manager) {
    char startDate[20], endDate[20];
    
    printf("\nEnter Start Date (YYYY-MM-DD): ");
    if (scanf("%19s", startDate) != 1 || !valid_date_yyyy_mm_dd(startDate)) {
        printf("Invalid start date format.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }
    
    printf("Enter End Date (YYYY-MM-DD): ");
    if (scanf("%19s", endDate) != 1 || !valid_date_yyyy_mm_dd(endDate)) {
        printf("Invalid end date format.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
    
    if (compare_dates(startDate, endDate) > 0) {
        printf("Error: Start date must be before or equal to end date.\n");
        return;
    }
    
    printf("\n<============ SEARCH RESULTS: Date Range %s to %s ============>\n", 
           startDate, endDate);
    int found = 0;
    
    printf("\n%-8s %-8s %-12s %-10s\n", "Type", "Quantity", "Expiry", "Supplier");
    printf("----------------------------------------\n");
    
    for (int i = 0; i < manager->countOfShipments; ++i) {
        const MyShipments *s = &manager->shipments[i];
        if (compare_dates(s->expiry_date, startDate) >= 0 && 
            compare_dates(s->expiry_date, endDate) <= 0) {
            printf("%-8d %-8d %-12s %-10d\n",
                   s->bambooType, s->quantity, s->expiry_date, s->supplierID);
            found++;
        }
    }
    
    if (found == 0) {
        printf("No shipments found in the date range.\n");
    } else {
        printf("\nTotal shipments found: %d\n", found);
    }
}

// Main search function
void searchShipments(const ShipmentManager *manager) {
    if (!manager || manager->countOfShipments == 0) {
        printf("\nNo shipments to search. Read (1) or Add (2) first.\n");
        return;
    }
    
    printf("\n<============ SEARCH SHIPMENTS ============>\n");
    printf("[1] Search by Bamboo Type\n");
    printf("[2] Search by Supplier ID\n");
    printf("[3] Search by Date Range\n");
    printf("\nEnter your choice (1-3): ");
    
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 3) {
        printf("Invalid choice.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
    
    switch (choice) {
        case 1:
            search_by_type(manager);
            break;
        case 2:
            search_by_supplier(manager);
            break;
        case 3:
            search_by_date_range(manager);
            break;
    }
}

// ----------------------------------------------- SORTING -----------------------------------------------

// Comparison functions for qsort
static int cmp_quantity_desc(const void *a, const void *b) {
    const MyShipments *sa = (const MyShipments *)a;
    const MyShipments *sb = (const MyShipments *)b;
    // Sort by quantity descending (biggest to smallest)
    return sb->quantity - sa->quantity;
}

static int cmp_bamboo_type_asc(const void *a, const void *b) {
    const MyShipments *sa = (const MyShipments *)a;
    const MyShipments *sb = (const MyShipments *)b;
    // Sort by bamboo type ascending (0-9)
    return sa->bambooType - sb->bambooType;
}

static int cmp_date_asc(const void *a, const void *b) {
    const MyShipments *sa = (const MyShipments *)a;
    const MyShipments *sb = (const MyShipments *)b;
    // Sort by date ascending (earliest to latest)
    // strcmp works for YYYY-MM-DD format
    return strcmp(sa->expiry_date, sb->expiry_date);
}

// Main sort function
void sortShipments(ShipmentManager *manager) {
    if (!manager || manager->countOfShipments == 0) {
        printf("\nNo shipments to sort. Read (1) or Add (2) first.\n");
        return;
    }
    
    if (manager->countOfShipments == 1) {
        printf("\nOnly one shipment exists. Nothing to sort.\n");
        print_shipments(manager);
        return;
    }
    
    printf("\n<============ SORT SHIPMENTS ============>\n");
    printf("[1] Sort by Quantity (Biggest to Smallest)\n");
    printf("[2] Sort by Bamboo Type (0-9)\n");
    printf("[3] Sort by Date (Earliest to Latest)\n");
    printf("\nEnter your choice (1-3): ");
    
    int choice;
    if (scanf("%d", &choice) != 1 || choice < 1 || choice > 3) {
        printf("Invalid choice.\n");
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
        return;
    }
    int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
    
    printf("\nSorting shipments");
    
    switch (choice) {
        case 1:
            printf(" by Quantity (Biggest to Smallest)...\n");
            qsort(manager->shipments, manager->countOfShipments, 
                  sizeof(MyShipments), cmp_quantity_desc);
            break;
        case 2:
            printf(" by Bamboo Type (0-9)...\n");
            qsort(manager->shipments, manager->countOfShipments, 
                  sizeof(MyShipments), cmp_bamboo_type_asc);
            break;
        case 3:
            printf(" by Date (Earliest to Latest)...\n");
            qsort(manager->shipments, manager->countOfShipments, 
                  sizeof(MyShipments), cmp_date_asc);
            break;
    }
    
    printf("Sorting complete!\n");
    printf("\n<============ SORTED SHIPMENTS ============>\n");
    print_shipments(manager);
    
    printf("\nNote: Sorting is in-memory. Use option 3 (Save Shipments to File) to persist the sorted order.\n");
}

// ============ MAIN ============ //
int main(void) {
    const char *filename = "shipments.txt";

    printf("<============ Mochi Supplier Management System ============>\n");

    ShipmentManager *manager = create_manager(10);
    if (!manager) {
        fprintf(stderr, "Fatal: Out of memory creating manager.\n");
        return 1;
    }

    while (1) {
        int user_input;
        printf("\n<============ Menu ============>\n");
        printf("[1] Read Shipments\n");
        printf("[2] Add New Shipment\n");
        printf("[3] Save Shipments to File\n");
        printf("[4] Remove Old/Spoiled Shipments\n");
        printf("[5] Search Shipments\n");
        printf("[6] Sort Shipments (TODO)\n");
        printf("[7] Generate a Report\n");
        printf("[8] Exit\n");
        printf("\nEnter your choice: ");

        if (scanf("%d", &user_input) != 1) {
            printf("\nInvalid input! Please enter a number.\n");
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        if (user_input < 1 || user_input > 8) {
            printf("\nInvalid choice. Please try again.\n");
            int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}
            continue;
        }
        // eat newline after number so next input works
        int ch; while ((ch = getchar()) != '\n' && ch != EOF) {}

        switch (user_input) {
            case 1: {
                printf("\nYou selected: Read Shipments\n");
                printf("\n<============ READ SHIPMENTS ============>\n");
                clear_shipments(manager);  // replace in-memory with file contents
                int result = readShipments(manager, filename);
                if (result >= 0) print_shipments(manager);
                else printf("\nRead failed.\n");
                break;
            }

            case 2: {
                addNewShipment(manager);   // appends one row to shipments.txt automatically
                printf("\nUpdated Shipments List:\n");
                print_shipments(manager);
                break;
            }

            case 3: { // Save All (overwrite)
                if (manager->countOfShipments == 0) {
                    printf("Nothing to save.\n");
                    break;
                }
                int wrote = saveShipments(manager, filename);
                if (wrote >= 0)
                    printf("Saved %d shipment(s) to '%s'.\n", wrote, filename);
                else
                    printf("Save failed.\n");
                break;
            }

            case 4:
                removeSpoiledShipments(manager);
                break;

            case 5:
                searchShipments(manager);
                break;

            case 6:
                printf("Sort Shipments: TODO\n");
                break;

            case 7: { // Generate Report
                if (manager->countOfShipments == 0) {
                    printf("No shipments in memory. Read (1) or Add (2) first.\n");
                    break;
                }
                const char *out = "report.txt";
                if (generateReport(manager, out) == 0)
                    printf("✓ Report generated: '%s'\n", out);
                else
                    printf("⚠ Failed to generate report.\n");
                break;
            }

            case 8:
                printf("Exiting the program. Goodbye!\n");
                free_manager(manager);
                return 0;
        }
    }
}
