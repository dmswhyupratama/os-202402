# 📝 Laporan Tugas Akhir

**Mata Kuliah**: Sistem Operasi
**Semester**: Genap / Tahun Ajaran 2024–2025
**Nama**: `Dimas Wahyu Pratama`
**NIM**: `240202858`
**Modul yang Dikerjakan**: `Modul 1 – System Call dan Instrumentasi Kernel`

---

## 📌 Deskripsi Singkat Tugas

* **Modul 1 – System Call dan Instrumentasi Kernel**:
  Menambahkan dua system call baru, yaitu `getpinfo()` untuk melihat proses yang aktif dan `getReadCount()` untuk menghitung jumlah pemanggilan `read()` sejak boot.
---

## 🛠️ Rincian Implementasi

* Menambahkan Struktur pinfo: Mendefinisikan struct pinfo di proc.h untuk menyimpan informasi PID, ukuran memori, dan nama proses.
* Menambahkan Counter readcount: Mendeklarasikan variabel global readcount di sysproc.c untuk melacak jumlah panggilan read().
* Menambahkan Nomor System Call Baru: Mendefinisikan nomor system call SYS_getpinfo (22) dan SYS_getreadcount (23) di syscall.h.
* Mendaftarkan System Call: Menambahkan deklarasi extern untuk sys_getpinfo dan sys_getreadcount di syscall.c, serta mendaftarkannya ke dalam array syscalls[].
* Menambahkan Deklarasi User-Level: Mendeklarasikan fungsi getpinfo() dan getreadcount() di user.h agar dapat diakses oleh program pengguna.
* Menambahkan Entri System Call di usys.S: Menambahkan entri SYSCALL(getpinfo) dan SYSCALL(getreadcount) di usys.S untuk membuat stub fungsi user-level yang memanggil system call kernel.
* Mengimplementasikan Fungsi Kernel:
  1. sys_getreadcount(): Mengembalikan nilai readcount saat ini.
  2. sys_getpinfo(): Mengambil informasi proses dari ptable kernel, mengumpulkannya ke dalam struct pinfo sementara di kernel, lalu menyalin seluruh struct tersebut ke ruang pengguna menggunakan copyout() untuk keamanan.
* Memodifikasi sys_read(): Menambahkan readcount++; di awal fungsi sys_read() di sysfile.c untuk menginkrementasi counter setiap kali read() dipanggil.
* Membuat Program Penguji: Membuat dua program user-level (ptest.c dan rtest.c) untuk menguji fungsionalitas system call yang baru diimplementasikan.
* Mendaftarkan Program Penguji ke Makefile: Menambahkan _ptest dan _rtest ke daftar UPROGS di Makefile agar dapat dibangun dan dijalankan di xv6.
---

## ✅ Uji Fungsionalitas

* `ptest`: untuk menguji `getpinfo()`
* `rtest`: untuk menguji `getReadCount()`
* `cowtest`: untuk menguji fork dengan Copy-on-Write
* `shmtest`: untuk menguji `shmget()` dan `shmrelease()`
* `chmodtest`: untuk memastikan file `read-only` tidak bisa ditulis
* `audit`: untuk melihat isi log system call (jika dijalankan oleh PID 1)
---

## 💻 Implementasi Kode
Berikut adalah ringkasan perubahan kode pada setiap file:

proc.h
```c
// ... kode yang ada ...
#define MAX_PROC 64 

struct pinfo {
  int pid[MAX_PROC];
  int mem[MAX_PROC];
  char name[MAX_PROC][16];
};
// ... sisa kode yang ada ...
```

syscall.h
```c
// ... nomor syscall yang ada ...

#define SYS_getpinfo     22  // Tambahkan Nomor System Call Baru
#define SYS_getreadcount 23  // Tambahkan Nomor System Call Baru
```

user.h
```c
// ... deklarasi syscall yang ada ...
struct pinfo; // forward declaration
int getpinfo(struct pinfo *);
int getreadcount(void);
// ... sisa kode yang ada ...
```
usys.S
```c
SYSCALL(getpinfo)         
SYSCALL(getreadcount)
```

syscall.c
```c
// ... include dan fungsi fetch/arg yang ada ...
extern int sys_getpinfo(void);       // tambahkan deklarasi baru
extern int sys_getreadcount(void);   // tambahkan deklarasi baru

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
// ... syscall yang ada ...
[SYS_close]   sys_close,
[SYS_getpinfo]     sys_getpinfo,       // tambahkan deklarasi ini di array
[SYS_getreadcount] sys_getreadcount,   // tambahkan deklarasi ini di array
};
// ... sisa kode yang ada ...
```

sysfile.c
```c
// ... include dan fungsi file-system lainnya ...

extern int readcount; // Deklarasi extern untuk readcount dari sysproc.c

int
sys_read(void)
{  
  readcount++; // Tambahkan baris ini
  struct file *f;
  int n;
  char *p;
  if(argfd(0, 0, &f) < 0 || argint(2, &n) < 0 || argptr(1, &p, n) < 0)
    return -1;
  return fileread(f, p, n);
}

// ... sisa kode yang ada ...
```

sysproc.c
```c
// ... include dan deklarasi ptable extern ...

int readcount = 0; //tambahkan di global sysproc.c

// ... fungsi sys_fork, sys_exit, sys_wait, sys_kill, sys_getpid, sys_sbrk, sys_sleep, sys_uptime ...

int sys_getreadcount(void) {
  return readcount;
}

int sys_getpinfo(void) {
  struct pinfo *user_ptable_ptr; // Pointer ke struct pinfo di user-space
  struct pinfo kernel_ptable;    // Struct pinfo sementara di kernel-space
  struct proc *p;
  int i = 0;

  if (argptr(0, (void*)&user_ptable_ptr, sizeof(struct pinfo)) < 0) {
    return -1;
  }

  memset(&kernel_ptable, 0, sizeof(kernel_ptable));

  acquire(&ptable.lock);
  for (p = ptable.proc; p < &ptable.proc[NPROC] && i < MAX_PROC; p++) {
    if (p->state != UNUSED) {
      kernel_ptable.pid[i] = p->pid;
      kernel_ptable.mem[i] = p->sz;
      safestrcpy(kernel_ptable.name[i], p->name, sizeof(kernel_ptable.name[i]));
      i++;
    }
  }
  release(&ptable.lock);

  if (copyout(myproc()->pgdir, (uint)user_ptable_ptr, (char*)&kernel_ptable, sizeof(kernel_ptable)) < 0) {
    return -1;
  }

  return 0;
}
```
---


## 📷 Hasil Uji

### 📍 Output `ptest `:

```
PID    MEM     NAME
1      12288   init
2      16384   sh
3      12288   ptest
```

### 📍 Output `rtest `:

```
Read Count Sebelum: 12

Read Count Setelah: 13
```
---

### 📍 Program Uji `ptest `:

```c
#include "types.h"
#include "stat.h"
#include "user.h"
#include "proc.h" // Sertakan proc.h untuk definisi struct pinfo

int main() {
  struct pinfo p;
  if (getpinfo(&p) < 0) {
    printf(2, "Error: getpinfo failed\n");
    exit();
  }

  printf(1, "PID\tMEM\tNAME\n");
  for(int i = 0; i < MAX_PROC && p.pid[i] != 0; i++) {
    printf(1, "%d\t%d\t%s\n", p.pid[i], p.mem[i], p.name[i]);
  }
  exit();
}

```
---

### 📍 Program Uji `rtest `:

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  char buf[10];
  printf(1, "Read Count Sebelum: %d\n", getreadcount());
  printf(1, "Masukkan 5 karakter (lalu Enter): ");
  read(0, buf, 5); // Baca 5 karakter dari stdin
  printf(1, "Read Count Setelah: %d\n", getreadcount());
  exit();
}
```
---

## ⚠️ Kendala yang Dihadapi
Kendala utama yang dihadapi selama implementasi adalah pemahaman dan penerapan mekanisme transfer data yang aman antara ruang kernel dan ruang pengguna, khususnya pada system call sys_getpinfo().

Awalnya, terdapat kecenderungan untuk mencoba menulis data langsung ke alamat memori yang disediakan oleh program pengguna. Meskipun dalam beberapa skenario terbatas ini mungkin tampak berfungsi, praktik ini sangat berbahaya dan tidak aman di lingkungan kernel. Kernel tidak boleh langsung mengakses atau menulis ke memori pengguna tanpa validasi yang tepat, karena pointer pengguna bisa saja tidak valid, menunjuk ke area yang tidak diizinkan, atau bahkan mencoba menyerang kernel.

Solusi untuk kendala ini adalah dengan menggunakan fungsi copyout() yang disediakan oleh xv6. copyout() dirancang khusus untuk menyalin data dari ruang kernel ke ruang pengguna dengan aman. Fungsi ini melakukan validasi alamat memori pengguna untuk memastikan bahwa penulisan dilakukan ke area yang valid dan diizinkan dalam ruang alamat proses pengguna. Dengan demikian, implementasi sys_getpinfo yang aman melibatkan pengumpulan semua data proses ke dalam struct pinfo sementara di memori kernel, lalu menggunakan copyout() untuk menyalin seluruh struct tersebut ke alamat memori yang disediakan oleh pengguna.
## 📚 Referensi

Tuliskan sumber referensi yang Anda gunakan, misalnya:

* Buku xv6 MIT: [https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)
* Repositori xv6-public: [https://github.com/mit-pdos/xv6-public](https://github.com/mit-pdos/xv6-public)
* Stack Overflow, GitHub Issues, diskusi praktikum

---

