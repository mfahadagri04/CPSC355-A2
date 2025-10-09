#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============ STRUCTURE DEFINITIONS ============ //
// TODO: Define your structures here


// ============ FUNCTION DEFINITIONS ============ //
// TODO: Implement your functions here


// ============ MAIN FUNCTION ============ //
int main() {
    printf("<============ Mochi Supplier Management System ============>\n");

    // TODO: Init storage

    // TODO: Display Menu and handle user input
    int user_input;

    while (1) {
        printf("\n <============ Menu ============>\n");
        printf("1. Read Shipments\n");
        printf("2. Add new Shipments\n");
        printf("3. Remove Old/Spoiled Shipments\n");
        printf("4. Search Shipments\n");
        printf("5. Sort Shipments\n");
        printf("6. Generate a Report\n");
        printf("7. Exit\n");
        
        printf("\n Enter your choice: ");
        scanf("%d", &user_input);
        if (user_input == 7) {
            printf("Exiting the program. Goodbye!\n");
            break;
        } else if (user_input < 1 || user_input > 7) {
            printf("Invalid choice. Please try again.\n");
            continue;
        } else {
            // Valid input, break the loop to process
            break;
        }
    }

    switch (user_input) {
        case 1: ///
        case 2: ///
        case 3: ///
        case 4: ///
        case 5: ///
        case 6: ///
        // Add more cases as needed
    }
    // TODO: Cleanup and free memory

    return 0;
}