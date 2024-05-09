# Sisop-3-2024-MH-IT19

| Nama          | NRP          |
| ------------- | ------------ |
| Kevin Anugerah Faza | 5027231027 |
| Muhammad Hildan Adiwena | 5027231077 |
| Nayyara Ashila | 5027231083 |

## Soal 2

Berikut code yang digunakan
```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <math.h>
#include <time.h>
#include <ctype.h> // Untuk fungsi tolower

void numberToWords(int number, char *words) {
    char *units[] = {"", "satu", "dua", "tiga", "empat", "lima", "enam", "tujuh", "delapan", "sembilan"};
    char *teens[] = {"", "sebelas", "dua belas", "tiga belas", "empat belas", "lima belas", "enam belas", "tujuh belas", "delapan belas", "sembilan belas"};
    char *tens[] = {"", "sepuluh", "dua puluh", "tiga puluh", "empat puluh", "lima puluh", "enam puluh", "tujuh puluh", "delapan puluh", "sembilan puluh"};

    if (number < 0) {
        strcpy(words, "ERROR");
    } else if (number < 10) {
        sprintf(words, "%s", units[number]);
    } else if (number < 20) {
        sprintf(words, "%s", teens[number - 10]);
    } else if (number == 10) {
        sprintf(words, "%s", tens[1]); 
    } else {
        int unit = number % 10;
        int ten = number / 10;
        sprintf(words, "%s %s", tens[ten], units[unit]);
    }
}


int stringToNumber(char *str) {
    if (strcmp(str, "nol") == 0) {
        return 0;
    } else if (strcmp(str, "satu") == 0) {
        return 1;
    } else if (strcmp(str, "dua") == 0) {
        return 2;
    } else if (strcmp(str, "tiga") == 0) {
        return 3;
    } else if (strcmp(str, "empat") == 0) {
        return 4;
    } else if (strcmp(str, "lima") == 0) {
        return 5;
    } else if (strcmp(str, "enam") == 0) {
        return 6;
    } else if (strcmp(str, "tujuh") == 0) {
        return 7;
    } else if (strcmp(str, "delapan") == 0) {
        return 8;
    } else if (strcmp(str, "sembilan") == 0) {
        return 9;
    } else {
        return -1; 
    }
}


void writeToLog(char *operation, char *num1_str, char *num2_str, char *result_str) {
    time_t rawtime;
    struct tm *info;
    char timestamp[80];

    time(&rawtime);
    info = localtime(&rawtime);

    strftime(timestamp, sizeof(timestamp), "[%d/%m/%y %H:%M:%S]", info);

    FILE *file = fopen("histori.log", "a");
    if (file != NULL) {
    	if (strcmp(operation, "-kali") == 0) {
        fprintf(file, "%s [KALI] %s kali %s sama dengan %s\n", timestamp, num1_str, num2_str, result_str);
    	} 
    	else if (strcmp(operation, "-tambah") == 0) {
        fprintf(file, "%s [TAMBAH] %s tambah %s sama dengan %s\n", timestamp, num1_str, num2_str, result_str);
    	} 
    	else if (strcmp(operation, "-kurang") == 0) {
        	if(strcmp(result_str, "ERROR") == 0){
        		fprintf(file, "%s [KURANG] ERROR pada pengurangan\n", timestamp);
        	}
        	else{
        		fprintf(file, "%s [KURANG] %s kurang %s sama dengan %s\n", timestamp, num1_str, num2_str, result_str);
        	}
    	} 
    	else if (strcmp(operation, "-bagi") == 0) {
        	fprintf(file, "%s [BAGI] %s bagi %s sama dengan %s\n", timestamp, num1_str, num2_str, result_str);
    	}
        fclose(file);
    } else {
        printf("Error: Cannot open file.\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <operation>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num1, num2;
    char operator[10];

   
    strcpy(operator, argv[1]);

    
    char strNum1[10], strNum2[10];
    printf("Masukkan dua angka: ");
    scanf("%s %s", strNum1, strNum2);

   
    num1 = stringToNumber(strNum1);
    num2 = stringToNumber(strNum2);

    
    if (num1 == -1 || num2 == -1) {
        printf("Invalid input\n");
        return EXIT_FAILURE;
    }

    int pipes[2];
    if (pipe(pipes) == -1) {
        perror("pipe");
        return EXIT_FAILURE;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) { 
        close(pipes[0]); 
        dup2(pipes[1], STDOUT_FILENO); 
        close(pipes[1]); 

        
        int result;
        if (strcmp(operator, "-kali") == 0) {
            result = num1 * num2;
        } else if (strcmp(operator, "-tambah") == 0) {
            result = num1 + num2;
        } else if (strcmp(operator, "-kurang") == 0) {
            result = num1 - num2;
        } else if (strcmp(operator, "-bagi") == 0) {
            if (num2 == 0) {
                
                printf("ERROR");
                return EXIT_FAILURE;
            }
            result = num1 / num2;
        } else {
            printf("Invalid operator\n");
            return EXIT_FAILURE;
        }

        
        char resultInWords[100];
        if (result < 0) {
            strcpy(resultInWords, "ERROR");
        } else if (result == 10) {
            strcpy(resultInWords, "sepuluh");
        } else {
            numberToWords(result, resultInWords);
        }

        printf("%s", resultInWords);

        exit(EXIT_SUCCESS);
    } else { 
        close(pipes[1]); 
        wait(NULL);

        char resultInWords[100];
        read(pipes[0], resultInWords, sizeof(resultInWords));
        close(pipes[0]); 
        
        if (strcmp(resultInWords, "ERROR") == 0) {
            printf("ERROR\n");
        } else {
            printf("\"Hasil %s %s dan %s adalah %s.\"\n", operator + 1, strNum1, strNum2, resultInWords);
        }

       
        writeToLog(operator, strNum1, strNum2, resultInWords);

        return EXIT_SUCCESS;
    }
}
```
### 2) a. Sesuai request dari adiknya Max ingin nama programnya dudududu.c. Sebelum program parent process dan child process, ada input dari user berupa 2 string. Contoh input: tiga tujuh.

Pada soal ini saya menggunakan fungsi ``void numberToWords(int number, char *words) {`` yang akan mengubah angka kemudian sistem akan membacanya sebagai string. 
 Berikut hasilnya   

https://web.whatsapp.com/f76e6322-e541-477a-a23e-dfdcfe131c40

pada inputan diatas sistem sudah bisa membaca 2 string yang dimasukkan pengguna yaitu "tujuh tiga".

### 2) b. Pada parent process, program akan mengubah input menjadi angka dan melakukan perkalian dari angka yang telah diubah. Contoh: tiga tujuh menjadi 21. 

Pada soal ini saya menggunakan fungsi 
```
int stringToNumber(char *str) {
    if (strcmp(str, "nol") == 0) {
        return 0;
    } else if (strcmp(str, "satu") == 0) {
        return 1;
    } else if (strcmp(str, "dua") == 0) {
        return 2;
    } else if (strcmp(str, "tiga") == 0) {
        return 3;
    } else if (strcmp(str, "empat") == 0) {
        return 4;
    } else if (strcmp(str, "lima") == 0) {
        return 5;
    } else if (strcmp(str, "enam") == 0) {
        return 6;
    } else if (strcmp(str, "tujuh") == 0) {
        return 7;
    } else if (strcmp(str, "delapan") == 0) {
        return 8;
    } else if (strcmp(str, "sembilan") == 0) {
        return 9;
    } else {
        return -1; 
    }
}
```

dimana fungsinya untuk mengubah dua string dari inputan pengguna menjadi angka.



