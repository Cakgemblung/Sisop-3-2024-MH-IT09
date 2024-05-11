# Sisop-3-2024-MH-IT19

| Nama          | NRP          |
| ------------- | ------------ |
| Kevin Anugerah Faza | 5027231027 |
| Muhammad Hildan Adiwena | 5027231077 |
| Nayyara Ashila | 5027231083 |

## Soal 1
## Soal 1
Berikut adalah code yang kami buat untuk mengerjakan soal 1

CODE `auth.c`
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 0x00001234
#define MAX_FILENAME_SIZE 100
#define MAX_FILEDATA_SIZE 10000

struct shared_data {
    int count;
    struct {
        char filename[MAX_FILENAME_SIZE];
        char filedata[MAX_FILEDATA_SIZE];
    } files[2]; // Increase according to what we need
};
```
Saya mengawali dengan mendeklarasikan library yang diperlukan oleh code ini. Diawali dengan mendefine `key` pada shared memory yakni `0x00001234`. Kemudian membuat structure dari shared data yang akan dimasukkan pada shared memory.
```
int main() {
    DIR *d;
    struct dirent *dir;
    d = opendir("./new-data");
    if (d) {
        int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0644|IPC_CREAT);
        if (shmid == -1) {
            perror("shmget");
            exit(1);
        }
        struct shared_data *data = shmat(shmid, NULL, 0);
        if (data == (void *) -1) {
            perror("shmat");
            exit(1);
        }
        data->count = 0;
        while ((dir = readdir(d)) != NULL && data->count < 2) {
            if (strstr(dir->d_name, "trashcan.csv") || strstr(dir->d_name, "parkinglot.csv")) {
                strncpy(data->files[data->count].filename, dir->d_name, MAX_FILENAME_SIZE);

                char filepath[1024];
                sprintf(filepath, "./new-data/%s", dir->d_name);
                FILE *file = fopen(filepath, "r");
                if (file != NULL) {
                    size_t new_len = fread(data->files[data->count].filedata, sizeof(char), MAX_FILEDATA_SIZE-1, file);
                    if (new_len == 0) {
                        fputs("Error reading file", stderr);
                    } else {
                        data->files[data->count].filedata[++new_len] = '\0'; // Just to be safe
                    }
                    fclose(file);
                }

                data->count++;
            } else {
                char filepath[1024];
                sprintf(filepath, "./new-data/%s", dir->d_name);
                remove(filepath);
            }
        }
        closedir(d);
        shmdt(data);
    }
    return 0;
}
```

Pada int main ini, saya membuat pointer agar dapat memudahkan untuk akses terhadap direktori yang dituju. Kemudian mendapatkan shared memory dengan `shmid` agar mengetahui shared memory mana yang akan diakses. Selanjutnya melakukan penyalinan dari data yang ingin disalin yakni file yang memiliki nama `trashcan.csv` dan `parkinglot.csv`.

CODE `rate.c`
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY 0x00001234
#define MAX_FILENAME_SIZE 100
#define MAX_FILEDATA_SIZE 10000

struct shared_data {
    int count;
    struct {
        char filename[MAX_FILENAME_SIZE];
        char filedata[MAX_FILEDATA_SIZE];
    } files[2]; // Increase this if you want to handle more files
};
```

Pada `rate.c` dimulai dengan komposisi yang sama dengan `auth.c` namun akan berbeda pada pendekatan di fungsi `main`. Berikut adalah fungsi `main` nya.
```
int main() {
    // Attach to shared memory
    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0644);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    struct shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    for (int i = 0; i < data->count; i++) {
        // Make a copy of the file data
        char filedata_copy[MAX_FILEDATA_SIZE];
        strcpy(filedata_copy, data->files[i].filedata);

        // Process the CSV data from the copy
        char *line = strtok(filedata_copy, "\n");
        float max_rating = 0.0;
        char best_place[1024] = {0};
        char best_filename[MAX_FILENAME_SIZE] = {0};
        strcpy(best_filename, data->files[i].filename); // Copy the file name from shared memory

        // Read the CSV data line by line
        while (line != NULL) {
            char place[1024];
            float rating;
            // Assuming the CSV format is: place_name,rating
            if (sscanf(line, "%[^,],%f", place, &rating) == 2) {
                if (rating > max_rating) {
                    max_rating = rating;
                    strcpy(best_place, place);
                }
            }
            line = strtok(NULL, "\n");
        }

        // Output the best place with the highest rating
        printf("Output:\n");
        printf("Type: %s\n", strstr(best_filename, "trashcan") ? "Trash Can" : "Parking Lot");
        printf("Filename: %s\n", best_filename);
        printf("------------------------\n");
        printf("Name: %s\n", best_place);
        printf("Rating: %.2f\n", max_rating);
    }

    // Detach from shared memory
    shmdt(data);

    return 0;
}
```

Secara garis besar, tujuan dari masing-masing code telah diberikan sebagai comment pada code tersebut. Program akan membuat salinan dari data yang telah tertulis pada shared memory terlebih dahulu dan ini dilakukan agar file tidak "hilang" saat dilakukan pengurutan highest rating seperti yang diminta pada soal. Kemudian untuk melakukan pengurutan, program akan membaca file salinan yang telah dibuat pada shared memory untuk dilakukan pengurutan sesuai yang diminta.

CODE `db.c`
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>

#define SHM_KEY 0x00001234
#define MAX_FILENAME_SIZE 100
#define MAX_FILEDATA_SIZE 10000

struct shared_data {
    int count;
    struct {
        char filename[MAX_FILENAME_SIZE];
        char filedata[MAX_FILEDATA_SIZE];
    } files[2]; // Increase this if you want to handle more files
};
```

Dimulai dengan konsep yang sama yakni inisialisasi struct `shared data`, include library, dan juga define pada key shared memory yang digunakan.

```
int main() {
    // Attach to shared memory
    int shmid = shmget(SHM_KEY, sizeof(struct shared_data), 0644);
    if (shmid == -1) {
        perror("shmget");
        exit(1);
    }
    struct shared_data *data = shmat(shmid, NULL, 0);
    if (data == (void *) -1) {
        perror("shmat");
        exit(1);
    }

    // Open the log file
    FILE *log_file = fopen("db.log", "a");
    if (log_file == NULL) {
        perror("fopen");
        exit(1);
    }

    for (int i = 0; i < data->count; i++) {
        // Create the new file path
        char new_filepath[1024];
        sprintf(new_filepath, "./database/%s", data->files[i].filename);

        // Write the file data to the new file
        FILE *new_file = fopen(new_filepath, "w");
        if (new_file == NULL) {
            perror(new_filepath); // Print the error message
            exit(1);
        } else {
            fputs(data->files[i].filedata, new_file);
            fclose(new_file);
        }

        // Get the current time
        time_t t = time(NULL);
        struct tm tm = *localtime(&t);

        // Write the log entry
        fprintf(log_file, "[%02d/%02d/%04d %02d:%02d:%02d] [%s] [%s]\n",
                tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec,
                strstr(data->files[i].filename, "trashcan") ? "Trash Can" : "Parking Lot",
                data->files[i].filename);

        // Remove the original file
        char old_filepath[1024];
        sprintf(old_filepath, "/home/azrael/sisop/modul3/soal1/new-data/%s", data->files[i].filename);
        if (remove(old_filepath) != 0) {
            perror(old_filepath); // Print the error message
            exit(1);
        }
    }

    // Close the log file
    fclose(log_file);

    // Detach from shared memory
    shmdt(data);

    return 0;
}
```

Pada fungsi `main` ini saya melakukan "pemindahan" data dari shared memory ke file direktori yang diminta oleh soal. Kemudian membuka file log yakni `db.log` untuk kemudian menuliskan log pemindahan yang dilakukan oleh program ini pada file log yang diminta.

Berikut ini adalah hasil output dari code kami.

tree sebelum dijalankan
![image](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/144968322/58afde45-92ef-4a80-8a7b-256fc8fa3ffc)

output `rate.c`
![image](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/144968322/a04bc105-8dbd-4c9d-8572-a1aa82042c02)

isi file db.log
![image](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/144968322/3cddfad8-4a06-404a-98a5-d179c416b247)

tree setelah dijalankan
![image](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/144968322/a1c92184-2803-41a2-b81a-0096c1c56c04)


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

Pada soal ini saya menggunakan fungsi ``void numberToWords(int number, char *words) {`` yang akan mengubah angka dimana sistem akan membacanya sebagai string. 
 Berikut hasilnya   

![WhatsApp Image 2024-05-10 at 1 35 16 AM](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/80327619/3b81dcb6-c396-4f00-bae0-851d67913070)


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

dimana fungsinya untuk mengubah dua string dari inputan pengguna kemudian sistem akan membaca string tersebut sebagai angka.

### 2) c. Pada child process, program akan mengubah hasil angka yang telah diperoleh dari parent process menjadi kalimat. Contoh: `21` menjadi “dua puluh satu”.
Disini saya menggunakan `fungsi void numberToWords(int number, char *words) {`
   Fungsi `numberToWords()` ini melakukan konversi dari bilangan bulat menjadi kata-kata dalam Bahasa Indonesia. Contohnya, bilangan 21 akan diubah menjadi "dua puluh satu". Fungsi ini digunakan di dalam child process untuk mengonversi hasil perhitungan matematika (misalnya, hasil perkalian, penjumlahan, pengurangan, atau pembagian) menjadi kalimat yang dapat ditampilkan kepada pengguna atau ditulis ke dalam file log.

Di dalam child process `(if (pid == 0))` hasil perhitungan disimpan dalam bentuk bilangan bulat, dan kemudian diubah menjadi kata-kata menggunakan fungsi `numberToWords()` Hasil yang sudah diubah menjadi kata-kata ini kemudian ditampilkan ke layar atau dituliskan ke dalam file log.

### 2) d. Max ingin membuat program kalkulator dapat melakukan penjumlahan, pengurangan, dan pembagian, maka pada program buatlah argumen untuk menjalankan program : 
- perkalian	: ./kalkulator -kali
- penjumlahan	: ./kalkulator -tambah
- pengurangan	: ./kalkulator -kurang
- pembagian	: ./kalkulator -bagi
Kemudian kalkulator yang dibuatnya cuma menampilkan hasil positif jika bernilai negatif maka program akan print “ERROR”.

Berikut adalah hasil apabila ia menghasilkan operasi negatif maka akan menampilkan pesan 'ERROR'

![WhatsApp Image 2024-05-10 at 1 53 45 AM](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/80327619/d979e78a-7181-49b0-80b8-6e6c26b8e771)

Disini kami menggunakan fungsi ` if (number < 0) { strcpy(words, "ERROR");` apabila hasil operasi tersebut menghasilkan bilangan negatif, maka akan memeberi pesan `"ERROR"` pada sistemnya.

### 2) e. Setelah diberi semangat, Max pun melanjutkan programnya dia ingin (pada child process) kalimat akan di print dengan contoh format : 
- perkalian	: “hasil perkalian tiga dan tujuh adalah dua puluh satu.”
- penjumlahan	: “hasil penjumlahan tiga dan tujuh adalah sepuluh.”
- pengurangan	: “hasil pengurangan tujuh dan tiga adalah empat.”
- pembagian	: “hasil pembagian tujuh dan tiga adalah dua.”

Ini adalah code yang digunakan
```
int result;
        if (strcmp(operator, "-kali") == 0) {
            result = num1 * num2;
        } else if (strcmp(operator, "-tambah") == 0) {
            result = num1 + num2;
        } else if (strcmp(operator, "-kurang") == 0) {
            result = num1 - num2;
        } else if (strcmp(operator, "-bagi") == 0) {
            if (num2 == 0) {
```
Potongan Fungsi yang diatas adalah bagian dari proses perhitungan dalam program. Di sinilah operasi matematika sesuai dengan permintaan yang diberikan oleh pengguna dieksekusi.
- `int result;=` Variabel result dideklarasikan untuk menyimpan hasil perhitungan.
- `if (strcmp(operator, "-kali") == 0)` Pengecekan apakah operator yang diberikan adalah "-kali" (perkalian). Jika ya, maka operasi perkalian (num1 * num2) dilakukan dan hasilnya disimpan dalam variabel result.
- `else if (strcmp(operator, "-tambah") == 0)` berfungsi Jika operator adalah "-tambah" (penjumlahan), maka operasi penjumlahan (num1 + num2) dilakukan dan hasilnya disimpan dalam result.

- `else if (strcmp(operator, "-kurang") == 0)` berfungsi Jika operator adalah "-kurang" (pengurangan), maka operasi pengurangan (num1 - num2) dilakukan dan hasilnya disimpan dalam result.

- `else if (strcmp(operator, "-bagi") == 0)` berfungsi Jika operator adalah "-bagi" (pembagian).

  Berikut adalah output yang dihasilkan melalui inputan string pengguna
  
![WhatsApp Image 2024-05-10 at 1 53 45 AM](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/80327619/566c20f9-7905-4593-a7fe-a1b1bcd4580e)


### 2) f. Max ingin hasil dari setiap perhitungan dicatat dalam sebuah log yang diberi nama histori.log. Pada parent process, lakukan pembuatan file log berdasarkan data yang dikirim dari child process. 
- Format: [date] [type] [message]
- Type: KALI, TAMBAH, KURANG, BAGI
- Ex:
* [10/03/24 00:29:47] [KALI] tujuh kali enam sama dengan empat puluh dua.
* [10/03/24 00:30:00] [TAMBAH] sembilan tambah sepuluh sama dengan sembilan belas.
* [10/03/24 00:30:12] [KURANG] ERROR pada pengurangan.

Sebelumnya saya mengalami sedikit kesalahan pada tampilan histori.log yang belum sesuai dengan format dari soalnya seperti berikut ini

![WhatsApp Image 2024-05-10 at 2 05 10 AM](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/80327619/9aecb2b8-b29d-4e3d-8c57-a56c7b100224)

Sehingga setelah diperbaiki kami berhasil menampilkan histori.log sesuai dengan format permintan soal


Berikut adalah hasil yang sudah sesuai dengan format soal 

![WhatsApp Image 2024-05-10 at 2 01 10 AM](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/80327619/f59e3e97-81e6-430a-9af4-aa3d3de331a3)

Kami menggunakan fungsi `void writeToLog(char *operation, char *num1_str, char *num2_str, char *result_str) {` yang berfungsi untuk menampilkan histori.log sesuai dengan format yang diminta.
Kemudian kita menggunakan fungsi ` strftime(timestamp, sizeof(timestamp), "[%d/%m/%y %H:%M:%S]", info);` untuk menampilkan waktu  dan juga tanggal saat sistem melakukan operasi seperti contohnya `[10/03/24 00:30:12]`

Fungsi ini
```
FILE *file = fopen("histori.log", "a");
    if (file != NULL) {
    	if (strcmp(operation, "-kali") == 0) {
        fprintf(file, "%s [KALI] %s kali %s sama dengan %s\n", timestamp, num1_str, num2_str, result_str);
    	}
```
Digunakan untuk menampilkan format operasi yang diminta pada soal yaitu `[TAMBAH] sembilan tambah sepuluh sama dengan sembilan belas.` dimana terdapat nama operasi dengan seluruh hurufnya uppercase seperti `[TAMBAH]`

Fungsi ini ` printf("\"Hasil %s %s dan %s adalah %s.\"\n", operator + 1, strNum1, strNum2, resultInWords);` digunakan agar histori tersebut bisa menampilkan kalimat sesuai dengan format misalnya `tujuh kali enam sama dengan empat puluh dua.`


## Soal 3



