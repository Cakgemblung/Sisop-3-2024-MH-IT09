# Sisop-3-2024-MH-IT19

| Nama          | NRP          |
| ------------- | ------------ |
| Kevin Anugerah Faza | 5027231027 |
| Muhammad Hildan Adiwena | 5027231077 |
| Nayyara Ashila | 5027231083 |


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


Berikut adalah code yang kami buat untuk mengerjakan soal 3

CODE `action.c`
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* gap(float distance) {
    if (distance < 0) return "Invalid distance";
    else if (distance < 3.5) return "Gogogo";
    else if (distance >= 3.5 && distance <= 10) return "Push";
    else return "Stay out of trouble";
}

const char* fuel(char* fuel_input) {
    float fuel_percentage;

    // Check if input percentage string
    if (strchr(fuel_input, '%') != NULL) {
        fuel_percentage = atof(fuel_input);
    } 
    // check if input string or integeger
    else if (strchr(fuel_input, '.') == NULL) {
        fuel_percentage = atoi(fuel_input);
    } 
    // Input is float
    else {
        fuel_percentage = atof(fuel_input);
    }

    if (fuel_percentage > 80) return "Push Push Push";
    else if (fuel_percentage >= 50 && fuel_percentage <= 80) return "You can go";
    else return "Conserve Fuel";
}

const char* tire(int tire_usage) {
    if (tire_usage < 0) return "Invalid Tire Usage";
    if (tire_usage > 80) return "Go Push Go Push";
    else if (tire_usage >= 50 && tire_usage <= 80) return "Good Tire Wear";
    else if (tire_usage >= 30 && tire_usage < 50) return "Conserve Your Tire";
    else return "Box Box Box";
}

const char* tire_change(char* tire_type) {
    if (strcmp(tire_type, "Soft") == 0) return "Mediums Ready";
    else if (strcmp(tire_type, "Medium") == 0) return "Box for Softs";
    else return "Invalid Tire Type";
}


```
Pada file action.c saya membuat `const char*` yang akan merujuk masing-masing fungsi yang akan dipanggil oleh `paddock.c`. Seluruh masing-masing fungsi telah disesukan agar sesuai dengan ketentuan yang diminta oleh soal untuk `actions.c`

CODE `paddock.c`
```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "actions.c"

#define PORT 8080
#define BUFFER_SIZE 1024
#define LOG_FILE "/home/azrael/sisop/modul3/soal3/server/race.log"

// Function to write log
void write_log(char *source, char *command, char *info, const char *response) {
    chdir("/home/azrael/sisop/modul3/soal3/server"); // Change working directory
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        return;
    }
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    fprintf(log_file, "[%s] [%02d/%02d/%04d %02d:%02d:%02d]: [%s] [%s]\n", source, tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, command, info);
    fprintf(log_file, "[Paddock] [%02d/%02d/%04d %02d:%02d:%02d]: [%s] [%s]\n", tm.tm_mday, tm.tm_mon + 1, tm.tm_year + 1900, tm.tm_hour, tm.tm_min, tm.tm_sec, command, response);
    fclose(log_file);
}
```

Pada awal file ini saya mendeklarasikan library dan melakukan `define` yang akan dipakai pada program ini. Kemudian melakukan `include` pada file `actions.c` agar program ini dapat memanggil fungsi yang dijalankan oleh `actions.c` untuk dijalankan pada program ini. Kemudian saya membuat fungsi untuk menuliskan log pada `race.log`.
```
void wallahidaemon() {
      pid_t pid, sid;

  pid = fork();

  if (pid < 0) {
    exit(EXIT_FAILURE);
  }

  if (pid > 0) {
    exit(EXIT_SUCCESS);
  }

  umask(0);

  sid = setsid();
  if (sid < 0) {
    exit(EXIT_FAILURE);
  }

  if ((chdir("/home/azrael/sisop/modul3/soal3/server")) < 0) {
    exit(EXIT_FAILURE);
  }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
    
    //open specific file that acts like a blackhole (everything that happen in it will be erased immediately)
    stdin = fopen ("/dev/null", "r");
    stdout = fopen ("/dev/null", "w+");
    stderr = fopen ("/dev/null", "w+");
}
```

Pada fungsi `wallahidaemon` ini saya membuat agar program ini dapat dijalankan secara daemon. Kemudian saya melakukan sedikit perubahan pada bagian akhir agar program ini dapat berjalan secara daemon yakni dengan membuatnya melakukan akses kepada file `dev/null` untuk melakukan akses seperti write atau read. PERLU DIKETAHUI bahwa file ini akan menghapus semua manipulasi data yang berada didalamnya (tidak menyimpan).

```
int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create daemon process
    wallahidaemon();

    // Create socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Set address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
    while (1) {
        // Accept incoming connection
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        // Read command from driver
        int bytes_read = read(new_socket, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            perror("Read failed");
            close(new_socket);
            continue;
        }
        buffer[bytes_read] = '\0'; // Add null terminator

        // Parse command and info
        char *command = strtok(buffer, "|");
        char *info = strtok(NULL, "\n");

        // Process command
        const char *result;
        if (strcmp(command, "Gap") == 0) {
            float distance = atof(info);
            result = gap(distance);
        } else if (strcmp(command, "Fuel") == 0) {
            result = fuel(info);
        } else if (strcmp(command, "Tire") == 0) {
            int tire_usage = atoi(info);
            result = tire(tire_usage);
        } else if (strcmp(command, "Tire Change") == 0) {
            result = tire_change(info);
        } else {
            printf("Invalid command\n");
            close(new_socket);
            continue;
        }

        // Send result to driver
        int bytes_sent = send(new_socket, result, strlen(result), 0);
        if (bytes_sent < 0) {
            perror("Send failed");
        }

        // Write log
        write_log("Driver", command, info, result);

        close(new_socket);
    }

    return 0;
}
```
Pada fungsi `main` ini saya memanggil fungsi `wallahidaemon` dulu agar program dapat berjalan secara daemon. Kemudian saya melakukan pembuatan socker dan melakukan konfigurasi agar socket tersebut dapat berkomunikasi. Selanjutnya saya membuat agar socker dapat membaca `command` dan `info` dari program `driver`. Kemudian dilakukan pemrosesan pada input tersebut untuk selanjutnya diteruskan lagi "jawabannya" kepada program driver. Program ditutup dengan menuliskan log pada file `race.log`


CODE `driver.c`
```
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8080
#define IP "127.0.0.1"
#define BUFFER_SIZE 1024

void read_input(char *input, int max_length) {
    fgets(input, max_length, stdin);
    input[strcspn(input, "\n")] = '\0'; // Deleting newline from the input
}
```

Pada code ini saya melakukan declare library dan `define` yang nantinya akan digunakan pada program ini. Kemudian membuat fungsi `read_input` yang akan melakukan pembacaan pada input nanti.

```
int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};
    char command[BUFFER_SIZE] = {0};
    char info_input[BUFFER_SIZE] = {0};

    //Create new socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //Set the server address and port number
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    //Convert the IP address from text format to binary format and store it in the server address structure
    if (inet_pton(AF_INET, IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    //Make a connection to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read Command and Info
        printf("Command (Gap/Fuel/Tire/Tire Change): ");
        read_input(command, BUFFER_SIZE);
        printf("Info: ");
        read_input(info_input, BUFFER_SIZE);
        char info[BUFFER_SIZE] = {0};
        strncpy(info, info_input, BUFFER_SIZE - 1);

        // Sending command and info
        char combined_command_info[BUFFER_SIZE] = {0};
        sprintf(combined_command_info, "%s|%s", command, info); // Using "|" as a delimiter
        send(sock, combined_command_info, strlen(combined_command_info), 0);

        // Receive respons from the server
        int bytes_read = read(sock, buffer, BUFFER_SIZE);
        if (bytes_read < 0) {
            perror("Read failed");
            exit(1);
        }
        buffer[bytes_read] = '\0';

        // Showing the output
        printf("[Driver] : [%s] [%s]\n", command, info);
        printf("[Paddock]: [%s]\n", buffer);

        // Cleaning Buffer
        memset(buffer, 0, sizeof(buffer));

        // Stop after finished
        break;
    }

    return 0;
}
```

Pada fungsi `main` ini saya membuat agar program driver ini dapat berkomunikasi dengan socket yang dibuat pada `padddock.c` sebelumnya. Kemudian saya menggunaakn fungsi `while(1)` agar program dapat berjalan secara CLI seperti salah satu yang diperbolehkan oleh soal. Kemudian saya membuat agar programnya dapat membaca `command` dan `info` yang diinputkan oleh user, kemudian mengirimnya ke socket, kemudian menerima jawaban dari `paddock.c` melalui socket tersebut untuk mengeluarkan output yang diminta oleh soal. Kemudian akan dilakukan cleaning buffer agar tidak terjadi hal yang tidak diinginkan dan selanjutnya program akan berhenti.

Berikut ini adalah hasil output dari code kami.

program paddock dapat berjalan secara daemon
![image](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/144968322/f1d00438-5e9b-4e37-8adb-fc2e963744a3)

jalannya program driver
![image](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/144968322/3f24d276-882c-4d87-ad7d-9fc4060c279c)

isi dari race.log
![image](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/144968322/e04b62f3-f24b-4c0c-8976-5d6c9a317e82)

Berikut code yang saya gunakan untuk mengerjakan soal no 4

berikut adalah code untuk server.
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <curl/curl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define LOG_FILE "change.log"
#define FILENAME "myanimelist.csv"
#define DOWNLOAD_URL "https://drive.google.com/uc?export=download&id=10p_kzuOgaFY3WT6FVPJIXFbkej2s9f50"


// Function to download file using wget
int download_file(const char *url, const char *filename) {
    char command[BUFFER_SIZE];
    snprintf(command, sizeof(command), "wget -O %s %s", filename, url);
    return system(command);
}

// Function to read the content of a file
void read_file_content(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file) {
        char buffer[BUFFER_SIZE];
        while (fgets(buffer, BUFFER_SIZE, file)) {
            printf("%s", buffer);
        }
        fclose(file);
    } else {
        printf("Failed to open file for reading\n");
    }
}

// Function to log changes
void log_change(const char *type, const char *message) {
    FILE *log_file = fopen(LOG_FILE, "a");
    if (log_file != NULL) {
        time_t current_time;
        time(&current_time);
        struct tm *local_time = localtime(&current_time);
        fprintf(log_file, "[%02d/%02d/%02d] [%s] %s\n", local_time->tm_mday, local_time->tm_mon + 1, local_time->tm_year % 100, type, message);
        fclose(log_file);
    } else {
        printf("Failed to open log file\n");
        fflush(stdout); // Force writing output to terminal
    }
}

// Function to add new anime entry
void add_anime_entry(const char *entry) {
    FILE *file = fopen(FILENAME, "a");
    if (file) {
        fprintf(file, "%s\n", entry);
        fclose(file);
        // Log addition
        log_change("ADD", entry);
    } else {
        printf("Failed to open file for adding entry\n");
    }
}

// Function to edit anime entry
void edit_anime_entry(const char *old_entry, const char *new_entry) {
    FILE *file = fopen(FILENAME, "r+");
    if (file) {
        char line[BUFFER_SIZE];
        long int pos;
        while (fgets(line, BUFFER_SIZE, file)) {
            if (strstr(line, old_entry) == line) {
                pos = ftell(file) - strlen(line);
                fseek(file, pos, SEEK_SET);
                fprintf(file, "%s\n", new_entry);
                fclose(file);
                // Log edit
                log_change("EDIT", new_entry);
                return;
            }
        }
        printf("Anime entry not found for editing\n");
        fclose(file);
    } else {
        printf("Failed to open file for editing entry\n");
    }
}

// Function to delete anime entry
void delete_anime_entry(const char *title) {
    FILE *file = fopen(FILENAME, "r+");
    if (file) {
        FILE *temp_file = fopen("temp.csv", "w");
        char line[BUFFER_SIZE];
        int found = 0;
        while (fgets(line, BUFFER_SIZE, file)) {
            if (strstr(line, title) != line) {
                fprintf(temp_file, "%s", line);
            } else {
                found = 1;
            }
        }
        fclose(file);
        fclose(temp_file);
        remove(FILENAME);
        rename("temp.csv", FILENAME);
        // Log deletion
        if (found) {
            log_change("DEL", title);
        } else {
            printf("Anime entry not found for deletion\n");
        }
    } else {
        printf("Failed to open file for deleting entry\n");
    }
}

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE] = {0};
    int valread;

    // Read client message
    valread = read(client_socket, buffer, BUFFER_SIZE);
    printf("Client message: %s\n", buffer);
    fflush(stdout); // Force writing output to terminal

    // Process client message
    if (strcmp(buffer, "exit\n") == 0) {
        printf("Closing connection with client\n");
        close(client_socket);
        return;
    } else if (strcmp(buffer, "download\n") == 0) {
        // Download file using wget
        int success = download_file(DOWNLOAD_URL, FILENAME);
        if (success == 0) {
            printf("File downloaded successfully\n");
            // Respond to client
            char *response = "File downloaded successfully";
            send(client_socket, response, strlen(response), 0);
        } else {
            fprintf(stderr, "Failed to download file\n");
            // Respond to client
            char *response = "Failed to download file";
            send(client_socket, response, strlen(response), 0);
        }
    } else if (strcmp(buffer, "read\n") == 0) {
        // Read the content of the downloaded file
        printf("File content:\n");
        read_file_content(FILENAME);
        // Respond to client
        char *response = "File content displayed on server terminal";
        send(client_socket, response, strlen(response), 0);
    } else {
        // Respond to client
        char *response = "Message received";
        send(client_socket, response, strlen(response), 0);
    }

    // Close client socket
    close(client_socket);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create server socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind server socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Binding failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);
    fflush(stdout); // Force writing output to terminal

    // Accept client connections and handle messages
    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *) &address, (socklen_t *) &addrlen)) < 0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        handle_client(client_socket);
    }

    return 0;
}
```

berikut file untuk client.
```
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> // For inet_pton

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connection failed");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server\n");

    // Send message to server
    char message[BUFFER_SIZE];
    printf("Enter message: ");
    fgets(message, BUFFER_SIZE, stdin);
    send(sock, message, strlen(message), 0);
    printf("Message sent\n");

    // Receive response from server
    char buffer[BUFFER_SIZE] = {0};
    read(sock, buffer, BUFFER_SIZE);
    printf("Server response: %s\n", buffer);

    close(sock);
    return 0;
}
```

berikut adalah hasil mendownload file

![Screenshot from 2024-05-11 22-19-30](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/151426649/5e28f2c4-90c2-40ba-8e17-a9bd8ff72fc7)

berikut hasil saat command exit

![Screenshot from 2024-05-11 22-20-22](https://github.com/Cakgemblung/Sisop-3-2024-MH-IT09/assets/151426649/d3fad0a7-f3d1-4dfd-afec-644f7fc131cd)

maaf untuk menampilkan isi dari file dan lain sebagainya tersebut saya masih mengalami masalah.


