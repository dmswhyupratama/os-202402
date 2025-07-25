# 📝 Laporan Tugas Akhir
**Mata Kuliah:** Sistem Operasi  
**Semester:** Genap / Tahun Ajaran 2024–2025  
**Nama:** Dimas Wahyu Pratama  
**NIM:** 240202858  
**Modul yang Dikerjakan:**  
Modul 4: Subsistem Kernel Alternatif (chmod dan /dev/random)

## 🎯 Tujuan
Modul ini bertujuan untuk:

- Implementasi syscall `chmod(path, mode)` untuk mengatur mode file (read-only atau read-write) secara sederhana.
- Menambahkan driver pseudo-device `/dev/random` untuk menghasilkan byte acak.

## 🗂️ File yang Diubah
| File         | Tujuan                                    |
|--------------|-------------------------------------------|
| fs.h         | Tambah field `mode` di inode              |
| fs.c, file.c | Cek mode akses sebelum tulis              |
| sysfile.c    | Implementasi syscall `chmod()`            |
| syscall.c    | Registrasi syscall                        |
| syscall.h    | Nomor syscall baru                        |
| user.h       | Deklarasi syscall                         |
| usys.S       | Entry syscall                             |
| Makefile     | Daftar program user uji                   |
| random.c     | Device handler `/dev/random`              |
| devsw[] di file.c | Registrasi driver device             |

## ⚙️ Bagian A – System Call chmod()
### 🔹 1. Tambahkan Field `mode` ke struct inode
File: `file.h`, cari `struct inode`:

Tambahkan:
```c
short mode;    // 0 = read-write (default), 1 = read-only
```
Jika tidak ingin ganggu disk layout, simpan hanya di memori; cukup set nilainya di runtime (bersifat volatile).

### 🔹 2. Tambahkan syscall chmod()
a. `syscall.h` – Tambahkan nomor syscall baru:
```c
#define SYS_chmod 27
```

b. `user.h` – Tambahkan deklarasi:
```c
int chmod(char *path, int mode);
```

c. `usys.S` – Tambahkan syscall:
```asm
SYSCALL(chmod)
```

d. `syscall.c` – Daftarkan syscall:
```c
extern int sys_chmod(void);
...
[SYS_chmod] sys_chmod,
```

e. `sysfile.c` – Tambahkan fungsi sys_chmod:
```c
#include "fs.h"

int sys_chmod(void) {
  char *path;
  int mode;
  struct inode *ip;

  if(argstr(0, &path) < 0 || argint(1, &mode) < 0)
    return -1;

  begin_op();
  if((ip = namei(path)) == 0){
    end_op();
    return -1;
  }

  ilock(ip);
  ip->mode = mode;       // set field mode inode
  iupdate(ip);           // simpan ke disk (opsional)
  iunlock(ip);
  end_op();

  return 0;
}
```

### 🔹 3. Cegah `write()` jika mode read-only
File: `file.c`, fungsi `filewrite()`

Tambahkan sebelum `writei()`:
```c
if(f->ip && f->ip->mode == 1){  // mode read-only
  return -1;
}
```

### 🔹 4. Program Uji `chmodtest.c`
```c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main() {
  int fd = open("myfile.txt", O_CREATE | O_RDWR);
  write(fd, "hello", 5);
  close(fd);

  chmod("myfile.txt", 1);  // set read-only

  fd = open("myfile.txt", O_RDWR);
  if(write(fd, "world", 5) < 0)
    printf(1, "Write blocked as expected\n");
  else
    printf(1, "Write allowed unexpectedly\n");

  close(fd);
  exit();
}
```

## ⚙️ Bagian B – Device Pseudo `/dev/random`
### 🔹 1. Buat File Baru `random.c`
// random.c
```c
#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"

static uint seed = 123456; // Seed awal

int randomread(struct inode *ip, char *dst, int n) {
  for(int i = 0; i < n; i++){
    seed = seed * 1664525 + 1013904223; // Rumus LCG
    dst[i] = seed & 0xFF; // Ambil byte paling rendah
  }
  return n;
}
```

### 🔹 2. Registrasi Device Driver
a. File: `file.c`
Tambahkan di awal:
```c
extern int randomread(struct inode*, char*, int);
```
Lalu di `devsw[]` (fungsi `fileinit()`):
```c
devsw[3].read = randomread;
devsw[3].write = 0;
```

### 🔹 3. Tambahkan Device Node `/dev/random`
a. File `init.c`
```c
// Dalam fungsi main() di init.c
...
  dup(0);  // stdout
  dup(0);  // stderr

  mkdir("dev");
  mknod("/dev/random", 1, 3); // type=1 (device), major=3
...
```

### 🔹 4. Program Uji `randomtest.c`
```c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main() {
  char buf[8];
  int fd = open("/dev/random", 0);
  if(fd < 0){
    printf(1, "cannot open /dev/random\n");
    exit();
  }

  read(fd, buf, 8);
  for(int i = 0; i < 8; i++)
    printf(1, "%d ", (unsigned char)buf[i]);

  printf(1, "\n");
  close(fd);
  exit();
}
```

## 📦 Integrasi ke Makefile
Di bagian `UPROGS`:
```make
_chmodtest\
_randomtest\
```
Tambahkan `random.o` ke `_kernel`:
```make
_kernel = ... kalloc.o random.o
```

## 🧪 Build dan Uji
```bash
make clean
make qemu-nox
```
Lalu di shell xv6:

## 🚧 Status Implementasi dan Kendala
**chmod():** Implementasi syscall chmod() berhasil sepenuhnya dan memberikan output yang sesuai harapan, output:
```
$ chmodtest
Write blocked as expected
```
Namun, untuk device pseudo /dev/random, masih terdapat kendala dalam menghasilkan output acak yang sesuai modul.

### Output randomtest yang Diharapkan:
```
241 6 82 99 12 201 44 73
```

### Output randomtest yang Dihasilkan Saat Ini:

Ketika randomtest dijalankan, output yang seringkali konsisten adalah:
```
randomtest
10 0 0 0 0 0 0 0
```
Terkadang, output yang tidak relevan dapat muncul, seperti:
```
$ randomtest
chmodtest
99 104 109 111 100 116 101 115
$ exec: fail
exec t failed
```
Penjelasan : ini terjadi karena saat saya menjalankan perintah "randomtest" dan output tidak menampilkan apapun, disaat itu saya menimpanya dengan perintah "chmodtest". dan menghasilkan output seperti diatas

### Analisis Kendala
Meskipun semua langkah integrasi (pendaftaran driver, pembuatan device node, modifikasi Makefile) telah dilakukan dan diverifikasi, output randomtest yang konsisten 10 0 0 0 0 0 0 0 mengindikasikan masalah inti pada generator angka acak itu sendiri dalam random.c. Dugaan terkuat adalah:
- **Kegagalan LCG:** Implementasi Linear Congruential Generator (LCG) dengan parameter yang diberikan dalam modul kemungkinan mengalami overflow atau memasuki siklus yang sangat pendek terlalu cepat pada lingkungan 32-bit XV6, sehingga menghasilkan nilai yang tidak bervariasi atau mengulang pola yang terbatas (misalnya, angka 10 adalah ASCII untuk newline, dan 0 sisanya).
- **Driver Tidak Terpanggil:** Debugging menggunakan cprintf di dalam fungsi randomread tidak menunjukkan output apapun di konsol QEMU, yang mengindikasikan bahwa fungsi driver randomread tidak pernah terpanggil oleh kernel meskipun device file /dev/random berhasil dibuka oleh randomtest. Ini menunjukkan adanya masalah di lapisan antara syscall read() dan pemanggilan fungsi driver yang terdaftar di tabel devsw.
- **Artefak Output:** `exec: fail` bisa disebabkan masalah I/O atau error kernel tersembunyi.

### Upaya Debugging
- Verifikasi Konfigurasi: Memastikan semua file yang relevan (Makefile, init.c, file.c, random.c, randomtest.c) telah dimodifikasi dan disimpan dengan benar sesuai instruksi modul, termasuk penempatan mknod dan pendaftaran devsw.
- Debugging Kernel (cprintf): Penambahan pernyataan cprintf di random.c (fungsi randomread) dan file.c (fungsi fileinit dan fileread) untuk melacak alur eksekusi di kernel dan mengonfirmasi pemanggilan fungsi driver.
- Pengujian cat: Menggunakan perintah cat /dev/random di shell XV6 untuk mengisolasi masalah dari randomtest.c. Namun, perintah ini juga gagal dengan exec: fail, yang mengindikasikan masalah di lapisan driver atau syscall yang lebih dalam, bukan pada cat itu sendiri (karena cat README berfungsi normal).
- Percobaan Generator Sederhana: Mengganti generator LCG di random.c dengan counter sederhana (debug_counter) untuk mengeliminasi kompleksitas LCG dan memastikan alur data driver berfungsi, namun driver tetap tidak terpanggil.
---
Kendala ini akan menjadi fokus perbaikan di tahap selanjutnya untuk mencapai implementasi RNG yang sepenuhnya fungsional dan stabil.

## 📘 Penutup
✅ Mengintegrasikan perizinan file dasar ke sistem file xv6 (chmod)  
✅ Menambahkan device baru berbasis driver (/dev/random)  
✅ Memahami mekanisme file inode dan subsistem perangkat (device I/O)

## 📚 Referensi
- xv6 Book  
- xv6-public: https://github.com/mit-pdos/xv6-public
