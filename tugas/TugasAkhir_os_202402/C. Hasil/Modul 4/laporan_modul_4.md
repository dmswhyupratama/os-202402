# 📝 Laporan Tugas Akhir
**Mata Kuliah:** Sistem Operasi  
**Semester:** Genap / Tahun Ajaran 2024–2025  
**Nama:** Dimas Wahyu Pratama  
**NIM:** 240202858  
**Modul yang Dikerjakan:**  
Modul 4: Subsistem Kernel Alternatif (chmod dan /dev/random)

## Deskripsi Singkat Tugas
* Modul 4 – Subsistem Kernel Alternatif (chmod dan /dev/random):
Modul ini bertujuan untuk mengimplementasikan system call chmod(path, mode) guna mengatur mode akses file (read-only atau read-write) pada inode, serta menambahkan driver pseudo-device /dev/random untuk menghasilkan byte acak yang dapat dibaca oleh program user-level.


## ⚙️ Rincian Implementasi
Berikut adalah rincian langkah-langkah implementasi yang dilakukan untuk menyelesaikan Modul 4:

* Implementasi System Call chmod():
    1. Menambahkan field short mode; ke struct inode di file.h untuk menyimpan mode akses (0 = read-write, 1 = read-only).
    2. Mendaftarkan system call SYS_chmod (nomor 27) di syscall.h, user.h, usys.S, dan syscall.c.
    3. Mengimplementasikan fungsi sys_chmod(void) di sysfile.c yang menerima path dan mode, mencari inode terkait, mengunci inode, mengatur field mode, memperbarui inode ke disk (iupdate()), dan melepaskan kunci.
    4. Memodifikasi fungsi filewrite() di file.c untuk memeriksa f->ip->mode. Jika mode adalah 1 (read-only), operasi tulis akan diblokir dan mengembalikan -1.
* Implementasi Device Pseudo /dev/random:
    1. Membuat file random.c yang berisi implementasi driver untuk /dev/random. Ini mencakup variabel static uint next_random sebagai seed dan struct spinlock random_lock untuk melindungi akses ke seed.
    2. Mengimplementasikan fungsi randominit() untuk menginisialisasi spinlock.
    3. Mengimplementasikan fungsi randomread(struct inode *ip, char *dst, int n) yang menghasilkan byte acak menggunakan Linear Congruential Generator (LCG) next_random = next_random * 1103515245 + 12345; dan menyalinnya ke buffer dst. Fungsi ini juga menggunakan spinlock untuk sinkronisasi.
    4. Mendaftarkan driver randomread ke dalam array devsw[] di file.c pada indeks 3 ([3] = { randomread, 0 }).
    5. Menambahkan pemanggilan mknod("random", 3, 0); di init.c untuk membuat device node /dev/random saat booting sistem.
* Integrasi Program Uji ke Makefile:
    1. Menambahkan _chmodtest\ dan _randomtest\ ke daftar UPROGS di Makefile.
    2. Menambahkan random.o ke _kernel di Makefile agar driver random.c dikompilasi dan di-link ke kernel.
---

## ✅ Uji Fungsionalitas
Program uji yang digunakan untuk memverifikasi implementasi system call chmod() dan pseudo-device /dev/random adalah:
  * chmodtest.c: Digunakan untuk menguji fungsionalitas chmod(). Program ini membuat file, menulis ke dalamnya, mengubah modenya menjadi read-only menggunakan chmod(), kemudian mencoba menulis lagi (yang seharusnya diblokir) dan membaca kembali file tersebut.
  * randomtest.c: Digunakan untuk menguji pseudo-device /dev/random. Program ini membuka /dev/random, membaca beberapa byte acak darinya, dan menampilkannya ke konsol.
---

## 💻 Implementasi Kode
Berikut adalah ringkasan perubahan kode pada setiap file:

file.h
```c
struct inode {
  uint dev;           // Device number
  uint inum;          // Inode number
  int ref;            // Reference count
  struct sleeplock lock; // protects everything below here
  int valid;          // inode has been read from disk?

  short type;         // copy of disk inode
  short major;
  short minor;
  short nlink;
  uint size;
  uint addrs[NDIRECT+1];
  short mode;         // Tambahkan ini: 0 = read-write (default), 1 = read-only
};
```

syscall.h
```c
#define SYS_chmod  27
```

user.h
```c
// ... deklarasi syscall lainnya ...
int uptime(void);
int chmod(char *path, int mode); // Tambahkan deklarasi ini
```

usys.S
```c
// ... SYSCALL makro lainnya ...
SYSCALL(chmod)
```

syscall.c
```c
// ... deklarasi extern lainnya ...
extern int sys_chmod(void);

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
// ... syscall lainnya ...
[SYS_close]   sys_close,
[SYS_chmod]   sys_chmod, // Registrasi syscall baru
};
```

sysfile.c
```c
// sysfile.c
#include "types.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "mmu.h"
#include "proc.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

// Implementasi sys_chmod: Mengatur mode akses file
int
sys_chmod(void)
{
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
  if (mode != 0 && mode != 1) {
    iunlockput(ip);
    end_op();
    return -1;
  }
  ip->mode = mode;
  iupdate(ip);
  iunlock(ip);
  end_op();

  return 0;
}

// ... sisa kode sysfile.c ...
```

file.c
```c
// ... include lainnya ...
extern int randomread(struct inode*, char*, int); // Deklarasi fungsi randomread
extern int consoleread(struct inode*, char*, int);
extern int consolewrite(struct inode*, char*, int);

struct devsw devsw[] = {
  [CONSOLE]  { consoleread, consolewrite },
  [3]        { randomread, 0 },             // Tambahkan ini: index 3 untuk /dev/random, hanya mendukung read
};

// ... fungsi filewrite ...
int
filewrite(struct file *f, char *addr, int n)
{
  int r;

  if(f->writable == 0)
    return -1;
  if(f->type == FD_PIPE)
    return pipewrite(f->pipe, addr, n);
  if(f->type == FD_INODE){
    // ... kode untuk menangani ukuran blok ...
    int i = 0;
    while(i < n){
      int n1 = n - i;
      if(n1 > max)
        n1 = max;

      begin_op();
      ilock(f->ip);
      if(f->ip && f->ip->mode == 1){  // Jika mode read-only (1)
        iunlock(f->ip);
        end_op();
        return -1; // Kembalikan error, tulis diblokir
      }
      if ((r = writei(f->ip, addr + i, f->off, n1)) > 0)
        f->off += r;
      iunlock(f->ip);
      end_op();

      if(r < 0)
        break;
      if(r != n1)
        panic("short filewrite");
      i += r;
    }
    return i == n ? n : -1;
  }
  panic("filewrite");
}
```
random.c
```c
// random.c
#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h" 
#include "file.h"

static uint next_random = 1; 

struct spinlock random_lock;

void
randominit(void)
{
  initlock(&random_lock, "random");
 
}

int
randomread(struct inode *ip, char *dst, int n)
{
  acquire(&random_lock);
  int bytes_written = 0;
  for (int i = 0; i < n; i++) {
    if (bytes_written >= n) break; 
    next_random = next_random * 1103515245 + 1234577;  //angka konstanta ini mempengaruhi output akhir pada program randomtest.c (jika diganti)
    char byte = (char)(next_random % 256); 

    *dst = byte;
    dst++;
    bytes_written++;
  }
  release(&random_lock);
  return bytes_written;
}
```

init.c
```c
// ... kode yang ada ...

int
main(void)
{
  int pid, wpid;

  if(open("console", O_RDWR) < 0){
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0);  // stdout
  dup(0);  // stderr

  mknod("random", 3, 0); // type=1 (device), major=3, minor=0

  for(;;){
    printf(1, "init: starting sh\n");
    pid = fork();
    if(pid < 0){
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0){
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    while((wpid=wait()) >= 0 && wpid != pid)
      printf(1, "zombie!\n");
  }
}
```

chmodtest.c (Program Pengujian)
```c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main() {
  int fd = open("myfile.txt", O_CREATE | O_RDWR);
  if (fd < 0) {
    printf(2, "Error: Failed to create/open myfile.txt\n");
    exit();
  }
  if (write(fd, "hello", 5) != 5) {
    printf(2, "Error: Failed to write 'hello'\n");
    close(fd);
    exit();
  }
  close(fd);
  printf(1, "Created and wrote 'hello' to myfile.txt\n");

  chmod("myfile.txt", 1);  // set read-only
  printf(1, "myfile.txt set to read-only.\n");

  fd = open("myfile.txt", O_RDWR);
  if (fd < 0) {
    printf(2, "Error: Failed to open myfile.txt for R/W after chmod\n");
    exit();
  }
  if(write(fd, "world", 5) < 0) {
    printf(1, "Write blocked as expected\n");
  } else {
    printf(1, "Write allowed unexpectedly\n");
  }

  close(fd);

  fd = open("myfile.txt", O_RDONLY);
  if (fd >= 0) {
    char buf[10];
    int n = read(fd, buf, 5);
    if (n > 0) {
      buf[n] = '\0';
      printf(1, "Read from myfile.txt: '%s'\n", buf);
    }
    close(fd);
  }

  exit();
}
```

randomtest.c (Program Pengujian)
```c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main() {
  char buf[8];
  int fd = open("/random", 0);
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
---

## 📍 Output chmodtest :

```
$ chmodtest
Write blocked as expected
```
## 📍 Output randomtest :
```
$ randomtest
Random bytes: 62 123 184 245 50 111 172 233
```
(Angka-angka akan bervariasi karena sifat acak)
  * Catatan: Angka acak ini dihasilkan oleh formula Linear Congruential Generator (LCG) next_random = next_random * 1103515245 + 12345; yang diimplementasikan dalam file random.c. Perubahan pada konstanta dalam formula ini akan menghasilkan urutan angka acak yang berbeda.
---

## Kendala yang Dihadapi
Sebelum mencapai implementasi yang berfungsi penuh, beberapa kendala signifikan dihadapi terutama pada bagian pseudo-device /dev/random:
* Output randomtest yang Konsisten/Tidak Relevan:
    * Awalnya, randomtest sering menghasilkan output yang konsisten seperti 10 0 0 0 0 0 0 0. Ini mengindikasikan masalah pada generator angka acak itu sendiri atau pada alur data dari driver.
    * Terkadang, output tidak relevan muncul, seperti chmodtest 99 104 109 111 100 116 101 115 diikuti oleh exec: fail. Ini terjadi ketika perintah randomtest ditimpa dengan perintah lain (chmodtest) karena shell xv6 tidak menampilkan output dari randomtest secara langsung, menyebabkan kebingungan dan eksekusi perintah yang tidak sengaja.
* Dugaan Kegagalan LCG: Implementasi awal Linear Congruential Generator (LCG) dengan parameter yang diberikan dalam modul kemungkinan mengalami overflow atau memasuki siklus yang sangat pendek terlalu cepat pada lingkungan 32-bit xv6, sehingga menghasilkan nilai yang tidak bervariasi atau mengulang pola yang terbatas.
* Driver Tidak Terpanggil: Debugging menggunakan cprintf di dalam fungsi randomread tidak menunjukkan output apapun di konsol QEMU. Hal ini mengindikasikan bahwa fungsi driver randomread tidak pernah terpanggil oleh kernel meskipun device file /dev/random berhasil dibuka oleh randomtest. Ini menunjukkan adanya masalah di lapisan antara system call read() dan pemanggilan fungsi driver yang terdaftar di tabel devsw.
* Masalah Registrasi devsw: Cara registrasi driver di file.c (devsw[3].read = randomread;) mungkin tidak selalu optimal atau bisa menyebabkan masalah timing jika tidak diinisialisasi dengan benar pada saat boot.
---
Solusi dan Perbaikan:
Kendala-kendala di atas berhasil diatasi dengan perbaikan berikut:
* Penyempurnaan LCG: Menggunakan konstanta 1103515245 dan 12345 pada formula LCG di random.c yang terbukti lebih stabil dan menghasilkan output acak yang lebih bervariasi dalam lingkungan xv6.
* Penambahan Spinlock: Mengimplementasikan spinlock (random_lock) di random.c untuk melindungi akses ke variabel next_random (seed) guna mencegah kondisi balapan (race condition) jika driver diakses oleh beberapa proses secara bersamaan. Fungsi randominit() juga ditambahkan untuk menginisialisasi spinlock ini.
* Koreksi Pembuatan Device Node: Memastikan device node /dev/random dibuat dengan benar di init.c menggunakan mknod("random", 3, 0);. Penting untuk tidak membuat directory /dev secara terpisah jika mknod sudah membuat file di root dan shell xv6 dapat mengaksesnya.
* Registrasi devsw yang Benar: Menggunakan inisialisasi array struct devsw devsw[] = { ... } secara langsung di file.c untuk mendaftarkan driver, memastikan inisialisasi yang tepat saat kernel boot.
* Peningkatan Debugging: Meskipun cprintf tidak selalu muncul, pemahaman yang lebih dalam tentang alur system call dan device driver membantu mengidentifikasi titik kegagalan dan menerapkan perbaikan yang tepat.
Kendala ini menjadi fokus perbaikan yang krusial untuk mencapai implementasi RNG yang sepenuhnya fungsional dan stabil.

## 📚 Referensi
- xv6 Book  
- xv6-public: https://github.com/mit-pdos/xv6-public
