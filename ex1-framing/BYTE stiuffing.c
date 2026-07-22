#include <stdio.h>
#include <string.h>

#define MAX 3000

char binary_data[MAX];
char stuffed[MAX];
char framed[MAX];
char corrupted_framed[MAX];
char destuffed[MAX];

// Binary definitions based on Decimal 126 and Decimal 163
const char* FLAG_BIN = "01111110"; // Decimal 126 (Flag)
const char* ESC_BIN  = "10100011"; // Decimal 163 (Escape)

// Helper function to print binary streams with visual spaces
void print_formatted_binary(const char* label, const char* binary_str) {
    printf("%s", label);
    for (int i = 0; binary_str[i] != '\0'; i++) {
        printf("%c", binary_str[i]);
        if ((i + 1) % 8 == 0 && binary_str[i + 1] != '\0') {
            printf(" ");
        }
    }
    printf("\n");
}

int main()
{
    char input[100];
    int i, j, k;

    printf("Enter the alphanumeric string: ");
    scanf("%s", input);

    // 1. Convert Alphanumeric Input to 8-bit Binary Stream using custom ASCII maps
    int bit_index = 0;
    for(i = 0; input[i] != '\0'; i++)
    {
        unsigned char current_char = input[i];

        // Custom ASCII mapping requested by your teacher
        if (current_char == 'F') {
            current_char = 126; // Maps 'F' to Flag (01111110)
        } else if (current_char == 'E') {
            current_char = 163; // Maps 'E' to Escape (10100011)
        }

        // Convert the byte value to 8-bit binary characters
        for(j = 7; j >= 0; j--)
        {
            binary_data[bit_index++] = ((current_char >> j) & 1) ? '1' : '0';
        }
    }
    binary_data[bit_index] = '\0';
    int total_bits = bit_index;

    // 2. Binary Byte Stuffing (Processing 8 bits at a time)
    int stuffed_index = 0;
    for(i = 0; i < total_bits; i += 8)
    {
        char current_block[9];
        strncpy(current_block, &binary_data[i], 8);
        current_block[8] = '\0';

        // Check if the current 8-bit block matches our custom Flag or Escape binary string
        if(strcmp(current_block, FLAG_BIN) == 0 || strcmp(current_block, ESC_BIN) == 0)
        {
            // Insert the 8-bit binary pattern for our custom ESC (Decimal 163 -> 10100011)
            for(k = 0; k < 8; k++) stuffed[stuffed_index++] = ESC_BIN[k];
        }

        // Insert the actual data byte bits
        for(k = 0; k < 8; k++) stuffed[stuffed_index++] = current_block[k];
    }
    stuffed[stuffed_index] = '\0';

    // 3. Framing (Wrapping with the custom FLAG_BIN)
    int framed_index = 0;
    for(k = 0; k < 8; k++) framed[framed_index++] = FLAG_BIN[k];
    for(k = 0; k < stuffed_index; k++) framed[framed_index++] = stuffed[k];
    for(k = 0; k < 8; k++) framed[framed_index++] = FLAG_BIN[k];
    framed[framed_index] = '\0';

    // Print standard stuffed and framed outputs
    print_formatted_binary("\nOriginal Binary Data : ", binary_data);
    print_formatted_binary("Stuffed Binary Data  : ", stuffed);
    print_formatted_binary("Framed Binary Data   : ", framed);

    // 4. Error Injection Menu Selection (Updated)
    strcpy(corrupted_framed, framed);
    int choice;
    printf("\nChoose transmission scenario:\n");
    printf("1. Send the same data without error\n");
    printf("2. Inject Single-Bit Corruption\n");
    printf("3. Inject Multiple-Bit Corruption\n");
    printf("Enter choice (1-3): ");
    scanf("%d", &choice);

    if (choice == 2) {
        int error_pos;
        printf("\nTotal bits in framed data: %d", framed_index);
        printf("\Enter bit position to inject error (1 to %d): ", framed_index);
        scanf("%d", &error_pos);

        if(error_pos >= 1 && error_pos <= framed_index) {
            int idx = error_pos - 1; // 1-based to 0-based
            corrupted_framed[idx] = (corrupted_framed[idx] == '0') ? '1' : '0';
            printf("--- Single-bit error injected successfully at position %d ---\n", error_pos);
        } else {
            printf("--- Invalid bit position! No error injected. ---\n");
        }
    }
    else if (choice == 3) {
        int num_errors;
        printf("\nTotal bits in framed data: %d", framed_index);
        printf("\nHow many bits do you want to corrupt? ");
        scanf("%d", &num_errors);

        if(num_errors > 0 && num_errors <= framed_index) {
            for(i = 0; i < num_errors; i++) {
                int error_pos;
                printf("Enter bit position %d to flip (1 to %d): ", i + 1, framed_index);
                scanf("%d", &error_pos);

                if(error_pos >= 1 && error_pos <= framed_index) {
                    int idx = error_pos - 1;
                    corrupted_framed[idx] = (corrupted_framed[idx] == '0') ? '1' : '0';
                } else {
                    printf("Invalid position! Skipping this bit.\n");
                }
            }
            printf("--- Multiple-bit errors injected successfully! ---\n");
        } else {
            printf("--- Invalid number of errors! No changes made. ---\n");
        }
    }
    else {
        printf("--- Sending data cleanly without any changes ---\n");
    }

    print_formatted_binary("Receiver Frame Data  : ", corrupted_framed);

    // 5. De-stuffing (Operating on the receiver frame data)
    int destuffed_index = 0;
    // Strip the 8-bit starting and ending flags
    for(i = 8; i < framed_index - 8; i += 8)
    {
        char current_block[9];
        strncpy(current_block, &corrupted_framed[i], 8);
        current_block[8] = '\0';

        // If our custom ESC pattern (10100011) is found, skip it and read the next 8 bits
        if(strcmp(current_block, ESC_BIN) == 0)
        {
            i += 8;
            // Prevent array overflow if corruption alters frame boundaries
            if (i >= framed_index - 8) break;
            strncpy(current_block, &corrupted_framed[i], 8);
            current_block[8] = '\0';
        }

        for(k = 0; k < 8; k++) destuffed[destuffed_index++] = current_block[k];
    }
    destuffed[destuffed_index] = '\0';

    print_formatted_binary("Destuffed Binary Data: ", destuffed);

    // 6. Validation and Error Message Output
    if(strcmp(binary_data, destuffed) != 0) {
        printf("\nError: Data mismatch detected!\n");
        printf("Transmission discarded\n");
    } else {
        printf("\nSuccess: Data verified successfully!\n");
        printf("Transmission Successful.\n");
    }

    return 0;
}
