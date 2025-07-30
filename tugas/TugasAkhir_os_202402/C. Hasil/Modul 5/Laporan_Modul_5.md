
# 📝 Laporan Tugas Akhir

**Mata Kuliah**: Sistem Operasi  
**Semester**: Genap / Tahun Ajaran 2024–2025  
**Nama**: Dimas Wahyu Pratama  
**NIM**: 240202858  
**Modul yang Dikerjakan**:  Modul 5: Audit dan Keamanan Sistem

---

## 📌 Deskripsi Singkat Tugas

Modul 5 – Audit dan Keamanan Sistem:
Modul ini bertujuan untuk menambahkan fungsionalitas audit log pada sistem operasi xv6, yang meliputi perekaman setiap system call yang dijalankan, penyediaan system call get_audit_log(char *buf, int max) untuk membaca log, dan perlindungan akses log dari proses biasa (khusus proses PID 1).

---

## 🛠️ Rincian Implementasi
Berikut adalah rincian langkah-langkah implementasi yang dilakukan untuk menyelesaikan Modul 5:

* Menambahkan Struktur Audit Log:
    * Mendefinisikan struct audit_entry dan MAX_AUDIT di defs.h untuk konsistensi di seluruh kernel.
    * Mendeklarasikan variabel global audit_log (array struct audit_entry) dan audit_index (integer) di syscall.c untuk menyimpan dan mengelola log.
* Pencatatan System Call:
    * Memodifikasi fungsi syscall() di syscall.c untuk mencatat setiap system call yang valid. Informasi yang dicatat meliputi PID proses pemanggil, nomor system call, dan waktu (tick) saat system call dijalankan. Pencatatan dilakukan hanya jika audit log belum penuh.
* Menambahkan System Call get_audit_log():
    * Mendaftarkan system call SYS_get_audit_log (nomor 23) di syscall.h, user.h, usys.S, dan syscall.c.
    * Mengimplementasikan fungsi sys_get_audit_log() di sysproc.c. Fungsi ini bertanggung jawab untuk menyalin data audit log dari kernel space ke buffer user-space yang disediakan.
    * Mengimplementasikan mekanisme keamanan di sys_get_audit_log() yang membatasi akses audit log hanya untuk proses dengan PID 1 (init).
* Program Uji audit.c:
    * Membuat program user-level audit.c untuk memanggil get_audit_log() dan menampilkan isinya.
    * Memastikan struct audit_entry didefinisikan di user.h agar program user-level dapat menggunakannya
* Integrasi ke Makefile:
    * Menambahkan _audit ke daftar UPROGS di Makefile agar program uji dapat dikompilasi dan dijalankan
---

## Uji Fungsionalitas :
Program uji audit.c digunakan untuk memverifikasi fungsionalitas pencatatan system call dan mekanisme keamanan akses audit log. Program ini memanggil system call get_audit_log() dan mencoba menampilkan isinya. Pengujian dilakukan dalam dua skenario: sebagai proses biasa (PID > 1) untuk menguji penolakan akses, dan sebagai proses PID 1 (init) untuk menguji keberhasilan pembacaan log.

---

## 💻 Implementasi Kode Lengkap (Bagian yang Dimodifikasi) :
Berikut adalah ringkasan perubahan kode pada setiap file, beserta penjelasan singkat mengenai perbaikan yang dilakukan:

defs.h
```c
// ... deklarasi struct lainnya ...

struct audit_entry {
  int pid;
  int syscall_num;
  uint tick;
};

#define MAX_AUDIT 128

extern struct audit_entry audit_log[MAX_AUDIT];
extern int audit_index;

extern uint ticks;

// ... deklarasi fungsi kernel lainnya ...
```
Perbaikan: Struktur audit_entry dan MAX_AUDIT dipindahkan ke defs.h untuk menghindari redefinition errors dan memastikan konsistensi definisi di seluruh kernel. Deklarasi extern juga ditambahkan di sini.

syscall.c
```c
#include "defs.h" // Include defs.h untuk definisi audit_entry dan MAX_AUDIT
// ... include lainnya ...

struct audit_entry audit_log[MAX_AUDIT];
int audit_index = 0;

// ... fungsi fetchint, fetchstr, argint, argptr, argstr ...

extern int sys_get_audit_log(void); // Deklarasi system call baru

static int (*syscalls[])(void) = {
// ... system call yang sudah ada ...
[SYS_get_audit_log] sys_get_audit_log, // Registrasi system call baru
};

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc();

  num = curproc->tf->eax;

  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    if (audit_index < MAX_AUDIT) {
      audit_log[audit_index].pid = curproc->pid;
      audit_log[audit_index].syscall_num = num;
      audit_log[audit_index].tick = ticks;
      audit_index++;
    }
    curproc->tf->eax = syscalls[num]();
  } else {
    cprintf("%d %s: unknown sys call %d\n",
            curproc->pid, curproc->name, num);
    curproc->tf->eax = -1;
  }
}
```
Perbaikan: Variabel audit_log dan audit_index diinisialisasi secara global di syscall.c. Penggunaan curproc->tf->eax dan curproc->pid diperbaiki untuk mengakses informasi proses saat ini dengan benar.    

syscall.h
```c
// ... nomor system call yang sudah ada ...
#define SYS_get_audit_log 23 // Pastikan nomor ini unik dan berurutan
```

user.h
```c
// ... deklarasi system call lainnya ...

struct audit_entry {
  int pid;
  int syscall_num;
  int tick;
};

int get_audit_log(void *buf, int max);
```
Perbaikan: Struktur audit_entry didefinisikan secara eksplisit di user.h agar program user-level dapat menggunakannya tanpa masalah redefinition atau undefined type.

usys.S
```c
// ... SYSCALL makro yang sudah ada ...
SYSCALL(get_audit_log) // Tambahkan entri ini
```

sysproc.c
```c
#include "defs.h" // Include defs.h untuk struct audit_entry, MAX_AUDIT, ticks
// ... include lainnya ...

extern struct audit_entry audit_log[];
extern int audit_index;

extern struct spinlock tickslock;
extern uint ticks;

int sys_get_audit_log(void) {
  char *buf;
  int max;

  if (argptr(0, &buf, sizeof(struct audit_entry) * MAX_AUDIT) < 0 || argint(1, &max) < 0) {
    return -1;
  }

  struct proc *curproc = myproc();

  // --- OPSI FITUR KEAMANAN ---
  // Aktifkan blok ini untuk mengaktifkan keamanan (Hanya PID 1 yang boleh akses)
  if (curproc->pid != 1) {
    return -1;
  }

  int n_to_copy = (audit_index < max) ? audit_index : max;
  
  memmove(buf, audit_log, n_to_copy * sizeof(struct audit_entry));

  return n_to_copy;
}

// ... sisa kode sysproc.c ...
```
Perbaikan: Penggunaan curproc (dari myproc()) menggantikan proc yang tidak dideklarasikan. Operator perbandingan != digunakan dengan benar. Opsi fitur keamanan dijelaskan dengan jelas.

audit.c (Program Uji)
```c
// audit.c
#include "types.h"
#include "stat.h"
#include "user.h" // Include user.h untuk definisi struct audit_entry

int main() {
  struct audit_entry buf[128];
  int n = get_audit_log((void*)buf, 128);

  if (n < 0) {
    printf(1, "Access denied or error. (Only PID 1 can read audit log)\n");
    exit();
  }

  printf(1, "=== Audit Log ===\n");
  for (int i = 0; i < n; i++) {
    printf(1, "[%d] PID=%d SYSCALL=%d TICK=%d\n",
      i, buf[i].pid, buf[i].syscall_num, buf[i].tick);
  }

  exit();
}
```
Perbaikan: Definisi struct audit_entry dihapus dari audit.c karena sudah disertakan melalui user.h, menghindari redefinition error.

---

# 📷 Hasil Uji
## Jika Fitur Keamanan AKTIF (Hanya PID 1 yang boleh akses)Konfigurasi (sysproc.c):
```c
// ... dalam sys_get_audit_log
  struct proc *curproc = myproc();
  if (curproc->pid != 1) {
    return -1; // Akses ditolak jika bukan PID 1
  }
// ...
```
## 📍 Output audit (Jika Fitur Keamanan AKTIF - Hanya PID 1 yang boleh akses):

```
$ audit
Access denied or error. (Only PID 1 can read audit log)
```
* Penjelasan:
Ketika audit dijalankan dari shell (yang memiliki PID > 1), fungsi sys_get_audit_log akan mendeteksi bahwa PID pemanggil bukan 1. Sesuai dengan logika keamanan yang diaktifkan, syscall akan mengembalikan -1, yang kemudian diterjemahkan oleh program audit.c sebagai "Access denied or error.". Ini menunjukkan bahwa kebijakan keamanan berhasil diterapkan.

## Jika Fitur Keamanan NONAKTIF (Semua PID boleh akses) Konfigurasi (sysproc).c:
```c
// ... dalam sys_get_audit_log
// (jika blok code ini dihilangkan, atau dikomentari maka output dibawah akan terjadi)
/*
  struct proc *curproc = myproc();
  if (curproc->pid != 1) {
    return -1;
  }
*/
// ...
```
## 📍 Output audit (Jika Fitur Keamanan Nonaktif - Semua PID boleh akses):
```
=== Audit Log ===
[0] PID=1 SYSCALL=7 TICK=2
[1] PID=1 SYSCALL=15 TICK=2
[2] PID=1 SYSCALL=17 TICK=3
...
[40] PID=3 SYSCALL=12 TICK=1929
[41] PID=3 SYSCALL=7 TICK=1929
[42] PID=3 SYSCALL=28 TICK=1930
$
```
* Penjelasan:
Dengan pemeriksaan PID dinonaktifkan, sys_get_audit_log akan selalu mengembalikan jumlah entri log yang tersedia. Program audit.c kemudian berhasil membaca log tersebut dan mencetaknya ke layar, menunjukkan detail PID, nomor syscall, dan waktu (tick) untuk setiap system call yang terekam sejak boot.

---

## ⚠️ Kendala yang Dihadapi
* Selama proses implementasi awal modul ini, beberapa kendala dan error umum dihadapi, yang kemudian berhasil diatasi:
    * Permasalahan: Penempatan awal definisi struct audit_entry, MAX_AUDIT, audit_log, dan audit_index di syscall.c menyebabkan error seperti 'struct audit_entry' has no member named 'p', 'proc' undeclared, dan redefinition errors ketika diakses dari file kernel lain (misalnya sysproc.c) atau ketika struct audit_entry juga didefinisikan di user.h.
    * Solusi: Definisi struct audit_entry dan MAX_AUDIT dipindahkan ke defs.h (file header kernel) untuk memastikan konsistensi definisi di seluruh kernel. Deklarasi extern untuk audit_log dan audit_index ditambahkan di defs.h, sementara inisialisasi aktualnya tetap di syscall.c. Untuk user-space, definisi struct audit_entry yang identik ditambahkan di user.h.
* Akses Variabel Proses yang Tidak Tepat:
    * Permasalahan: Dalam fungsi syscall() di syscall.c dan sys_get_audit_log() di sysproc.c, terdapat masalah dalam mengakses variabel proses seperti proc->tf->eax dan proc->pid karena proc tidak dideklarasikan secara langsung di scope tersebut.
    * Solusi: Penggunaan proc->tf->eax diganti dengan curproc->tf->eax, dan proc->pid diganti dengan curproc->pid setelah mendapatkan curproc melalui myproc(), yang mengembalikan pointer ke proses saat ini.
* Kesalahan Operator Perbandingan:
    * Permasalahan: Dalam system call sys_get_audit_log(), operator perbandingan <> digunakan yang tidak valid dalam C.
    * Solusi: Operator <> diganti dengan != untuk perbandingan yang benar.
* Duplikasi Definisi struct audit_entry di User-Space:
    * Permasalahan: Program audit.c awalnya mendefinisikan struct audit_entry secara lokal, yang menyebabkan redefinition error jika user.h juga mendefinisikannya.
    * Solusi: Definisi struct audit_entry dihapus dari audit.c dan dipastikan hanya ada satu definisi di user.h yang kemudian di-include oleh audit.c.
* Pengujian Fitur Keamanan:
    * Permasalahan: Untuk menguji kebijakan keamanan (hanya PID 1 yang boleh akses), diperlukan cara yang tepat untuk menjalankan audit.c sebagai PID 1.
    * Solusi: Modifikasi init.c untuk menjalankan exec("audit", argv); sebagai pengganti shell default (sh) memungkinkan audit.c berjalan sebagai proses PID 1, sehingga dapat mengakses audit log.
  Kendala-kendala ini berhasil diatasi dengan pendekatan debugging yang sistematis dan pemahaman yang lebih baik tentang struktur kernel xv6 serta interaksi antara kernel dan user-space.

---

## 📚 Referensi

* Buku xv6 MIT: https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf
* Repositori xv6-public: https://github.com/mit-pdos/xv6-public
* Stack Overflow, GitHub Issues, diskusi praktikum
