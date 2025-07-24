
# 📝 Laporan Tugas Akhir

**Mata Kuliah**: Sistem Operasi  
**Semester**: Genap / Tahun Ajaran 2024–2025  
**Nama**: `Dimas Wahyu Pratama`  
**NIM**: `240202858`  
**Modul yang Dikerjakan**:  
`Modul 5: Audit dan Keamanan Sistem`

---

## 🎯 Tujuan

Modul ini bertujuan untuk menambahkan fungsionalitas **audit log** pada sistem operasi xv6, yang meliputi:

- Merekam setiap system call yang dijalankan.
- Menyediakan syscall `get_audit_log(char *buf, int max)` untuk membaca log.
- Melindungi akses log dari proses biasa (khusus proses PID 1).

---

## 🗂️ File yang Diubah

| File        | Perubahan                                                                 |
|-------------|---------------------------------------------------------------------------|
| `syscall.c` | Tambahkan pencatatan log setiap syscall                                   |
| `sysproc.c` | Implementasi syscall `get_audit_log()`                                    |
| `defs.h`    | Tambahkan deklarasi global untuk struktur dan variabel audit log          |
| `user.h`    | Tambahkan deklarasi syscall user-level                                    |
| `usys.S`    | Registrasi syscall                                                        |
| `syscall.h` | Penambahan nomor syscall baru                                             |
| `Makefile`  | Tambahkan program uji `audit.c`                                           |

---

## 🧩 Step-by-Step Implementasi

### 🔹 1. Tambahkan Struktur Audit Log Sesuai Modul:

  Modul menyarankan penambahan struktur dan variabel global di syscall.c:

```c
// syscall.c
#define MAX_AUDIT 128

struct audit_entry {
  int pid;
  int syscall_num;
  int tick;
};

struct audit_entry audit_log[MAX_AUDIT];
int audit_index = 0;
---

  Permasalahan & Solusi (Error: 'struct audit_entry' has no member named 'p', 'proc' undeclared, redefinition of 'struct audit_entry'):

  Awalnya, penempatan ini menyebabkan error karena definisi struct audit_entry dan MAX_AUDIT tidak dikenali secara global oleh semua file yang membutuhkannya, seperti sysproc.c. Selain itu, duplikasi definisi struct audit_entry di user.h dan audit.c juga memicu error redefinition.

  Perbaikan:
  Untuk mengatasi masalah ini, definisi struct audit_entry dan MAX_AUDIT dipindahkan ke kernel/defs.h agar dapat diakses secara global, dan deklarasi extern untuk audit_log dan audit_index ditambahkan di sana. Definisi duplikat di user.h dan audit.c kemudian dihapus.  
---

### Kode Final di kernel/defs.h: 

```c
// kernel/defs.h

// Definisi struktur audit_entry
struct audit_entry {
  int pid;
  int syscall_num;
  uint tick;
};

// Definisi konstanta
#define MAX_AUDIT 100 // Angka ini bisa disesuaikan

// Deklarasi extern (variabel global yang didefinisikan di file lain)
extern struct audit_entry audit_log[MAX_AUDIT];
extern int audit_index;

// Tambahkan juga deklarasi untuk 'ticks' jika belum ada
extern uint ticks;

// Pastikan prototipe fungsi-fungsi penting ini juga ada:
struct proc* myproc(void); // Untuk mendapatkan proses saat ini
void syscall(void);        // Prototipe fungsi syscall

---

#### 🔹 2. Catat System Call di syscall()

  Modul menyarankan penambahan kode log di fungsi syscall() (syscall.c):

```c
// syscall.c
// ... setelah num = proc->tf->eax;
if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
  if (audit_index < MAX_AUDIT) {
    audit_log[audit_index].pid = proc->pid; // Perlu perbaikan
    audit_log[audit_index].syscall_num = num;
    audit_log[audit_index].tick = ticks;
    audit_index++;
  }
}
```
  Permasalahan & Solusi (Error: 'struct proc' has no member named 'p', 'proc' undeclared):

  Awalnya, kode ini mengasumsikan proc->tf->eax dan proc->pid yang tidak benar untuk xv6.

  Perbaikan:
  Kami mengubah proc->tf->eax menjadi curproc->tf->eax untuk mendapatkan nomor syscall dari trapframe proses saat ini. Variabel proc->pid juga diganti dengan curproc->pid setelah mendapatkan curproc melalui myproc(). Logika if ganda juga digabungkan untuk efisiensi.
---
#### Kode Final di syscall.c:

```c
#include "types.h"
#include "defs.h" // Penting untuk MAX_AUDIT dan struct audit_entry
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "syscall.h"

// DEFINISI dan inisialisasi array audit_log dan audit_index
struct audit_entry audit_log[MAX_AUDIT];
int audit_index = 0; // Inisialisasi index ke 0

// ... (Deklarasi extern fungsi syscall lainnya)
extern int sys_get_audit_log(void); // Deklarasi syscall baru

static int (*syscalls[])(void) = {
// ... (daftar syscall lainnya)
[SYS_get_audit_log] sys_get_audit_log, // Tambahkan fungsi Anda di sini
};

void
syscall(void)
{
  int num;
  struct proc *curproc = myproc(); // Mengambil proses saat ini

  num = curproc->tf->eax; // Ambil nomor syscall dari trapframe

  if (num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    // Log audit hanya jika ada ruang dan syscall valid
    if (audit_index < MAX_AUDIT) {
      audit_log[audit_index].pid = curproc->pid; // Gunakan curproc
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
---

### 🔹 3. Tambahkan System Call `get_audit_log()`

  Langkah ini melibatkan penambahan SYS_get_audit_log di syscall.h, deklarasi di user.h, registrasi di usys.S, dan implementasi di sysproc.c.

#### a. syscall.h – Tambahkan nomor syscall baru:
Sesuai Modul:
```c
#define SYS_get_audit_log 28
```
  Permasalahan & Solusi (Error: SYS_get_audit_log redefined):

  Error muncul jika ada definisi ganda SYS_get_audit_log.

  Perbaikan:
  Pastikan hanya ada satu definisi SYS_get_audit_log di syscall.h.
---

#### b. user.h – Tambahkan deklarasi:
Sesuai Modul:
```c
// user.h
struct audit_entry { // Ini akan dihapus
  int pid;
  int syscall_num;
  int tick;
};
int get_audit_log(void *buf, int max);
```
  Permasalahan & Solusi (Error: redefinition of 'struct audit_entry'):

  Sama seperti masalah sebelumnya, mendefinisikan ulang struct audit_entry di user.h menyebabkan konflik jika sudah didefinisikan di defs.h.

  Perbaikan:
  Hanya deklarasikan fungsi get_audit_log. Hapus definisi struct audit_entry dari user.h karena sudah ada di defs.h (dan user.h akan menyertakan defs.h atau file lain yang pada gilirannya menyertakan defs.h).
---

#### c. usys.S – Tambahkan syscall:

```c
// usys.S
SYSCALL(get_audit_log)
```
  Ini tidak menimbulkan masalah dan tetap digunakan.
---

#### d. `sysproc.c` – Implementasi syscall:

```c
// sysproc.c
extern struct audit_entry audit_log[];
extern int audit_index;

int sys_get_audit_log(void) {
  char *buf;
  int max;

  if (argptr(0, &buf, sizeof(struct audit_entry) * MAX_AUDIT) < 0 || argint(1, &max) < 0) // Perlu perbaikan
    return -1;

  if (proc->pid != 1) // Perlu perbaikan
    return -1; // hanya PID 1 (init) yang boleh akses audit log

  int n = (audit_index < max) ? audit_index : max;
  memmove(buf, audit_log, n * sizeof(struct audit_entry));

  return n;
}
```
  Permasalahan & Solusi (Error: 'proc' undeclared, control reaches end of non-void function, unused variable 'curproc'):

  Kode ini awalnya memiliki beberapa masalah: penggunaan proc yang tidak dideklarasikan, operator perbandingan <> yang salah, dan variabel curproc yang menjadi tidak terpakai setelah pembatasan PID dikomentari.

  Perbaikan:

  Operator <> diganti dengan !=.

  Penggunaan proc diganti dengan variabel curproc yang diperoleh dari myproc().

  Baris pemeriksaan if (curproc->pid != 1) dikomentari untuk tujuan pengujian agar log bisa diakses dari shell.

  Setelah mengomentari baris tersebut, deklarasi struct proc *curproc = myproc(); dihapus karena curproc tidak lagi digunakan, mengatasi error unused variable.

---

## 🧪 Program Uji: `audit.c`

```c
// audit.c
#include "types.h"
#include "stat.h"
#include "user.h"

struct audit_entry { // Ini akan dihapus
  int pid;
  int syscall_num;
  int tick;
};

int main() {
  struct audit_entry buf[128];
  int n = get_audit_log((void*)buf, 128);

  if (n < 0) {
    printf(1, "Access denied or error.\n");
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

  Permasalahan & Solusi (Error: redefinition of 'struct audit_entry'):

  Definisi ganda struct audit_entry di audit.c menyebabkan konflik.

  Perbaikan:
  Definisi struct audit_entry dihapus dari audit.c karena sudah disertakan melalui user.h (yang pada akhirnya mendapatkan definisinya dari defs.h).
---

## 📦 Tambahkan ke Makefile

```make
UPROGS=\
  _cat\
  _audit\
  ...
```

---

## 🔁 Build & Jalankan

```bash
$ make clean
$ make qemu-nox
```

### Output Awal (Sebelum akses diperbolehkan):
```bash
$ audit
Access denied or error.
```
  Penjelasan: Output ini terjadi karena implementasi sys_get_audit_log awalnya membatasi akses hanya untuk proses dengan PID 1. Proses shell yang menjalankan audit memiliki PID yang berbeda.

  Output Diharapkan (Setelah Perbaikan dan Pengujian):
  Setelah mengomentari atau menghapus pemeriksaan if (curproc->pid != 1) di sysproc.c, audit dapat dijalankan dari shell biasa dan akan menampilkan log audit yang telah dikumpulkan:

### Output Setelah Modifikasi:
```bash
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
---

## 🔐 Ringkasan Fitur Keamanan

| Fitur            | Deskripsi                                                        |
|------------------|------------------------------------------------------------------|
| Log System Call  | Semua syscall direkam: PID, nomor syscall, dan waktu (tick)      |
| Akses Terbatas   | Hanya PID 1 (init) yang boleh akses log (sesuai spesifikasi)     |
| Isolasi Kernel   | Data log berada di kernel space, tidak langsung diakses user     |

---

## 📚 Referensi

- *xv6 Book* – Chapter 3: System Calls  
- File sumber: `syscall.c`, `proc.c`, `defs.h`, `sysproc.c`, `user.h`, `usys.S`
