# Praktikum Sistem Operasi menggunakan xv6

## 🔧 Topik: System Call, Manajemen Proses, dan Penjadwalan Proses

---

## 🎯 Tujuan Umum

* Mahasiswa mampu menelusuri dan memahami implementasi *fork()*, *exec()*, dan *exit()* pada xv6.
* Mahasiswa mampu menambahkan system call baru.
* Mahasiswa memahami dan memodifikasi algoritma penjadwalan proses pada kernel xv6.

---

## 🗂️ Persiapan Awal

Build Docker image:
```bash
docker build -t xv6_nim .
```

---

## 🚀 **4. Jalankan Docker Container**

Jalankan lingkungan praktikum di dalam Docker:

```bash
docker run -it --rm xv6_nim
```

---

# ** 🧪 Bagian 1: System Call `helloworld`**

## A. Tambahkan Nomor System Call

**File:** `syscall.h`

Tambahkan di bagian akhir:

```c
#define SYS_helloworld 24  
```

---

## B. Tambahkan Entri ke `syscall.c`

**File:** `syscall.c`

Tambahkan deklarasi eksternal fungsi:

```c
extern int sys_helloworld(void);
```

Tambahkan ke array `syscalls[]`:

```c
[SYS_helloworld] sys_helloworld,
```

---

## C. Definisikan Fungsi `sys_helloworld`

**File:** `sysproc.c`

Tambahkan:

```c
int sys_helloworld(void) {
  cprintf("Hello from xv6 kernel [NIM_NAMA]!\n");
  return 0;
}
```

---

## D. Tambahkan ke `usys.S`

**File:** `usys.S`

Tambahkan entri:

```asm
SYSCALL(helloworld)
```

⚠️ **Pastikan urutan penulisan sama dan tidak typo.**

---

## E. Tambahkan Deklarasi ke `user.h`

**File:** `user.h`

Tambahkan:

```c
int helloworld(void);
```

---

## F. (Opsional) Tambahkan Program Penguji

**Buat file baru misal `hello.c`:**

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
  helloworld();
  exit();
}
```

Tambahkan ke Makefile (`UPROGS`):

```make
UPROGS=\
  _hello\
  ...
```

---

### 🔁 Terakhir: Jalankan `make clean && make qemu-nox`

```bash
make clean
make qemu-nox
```

---

# 🧪 Bagian 2: Pengamatan Manajemen Proses di xv6

## ✅ Langkah 1 – Tambahkan Log untuk Observasi Proses

### A. Tambahkan Log ke `fork()`

**File:** `proc.c`, fungsi `fork()`

Cari:

```c
c->state = RUNNABLE;
```

Tambahkan setelahnya:

```c
cprintf("[kernel] fork: Parent PID %d → Child PID %d\n", p->pid, c->pid);
```

---

### B. Tambahkan Log ke `exec()`

**File:** `exec.c`, fungsi `exec()`

Setelah baris:

```c
  ...
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  switchuvm(curproc);
  freevm(oldpgdir);

    // === Tambahkan log observasi di sini ===
```

Tambahkan:

```c
cprintf("[kernel] exec: PID %d menjalankan program \"%s\"\n", curproc->pid, curproc->name);
```
Hasil Potongan akhir menjadi :
```c
  curproc->pgdir = pgdir;
  curproc->sz = sz;
  curproc->tf->eip = elf.entry;  // main
  curproc->tf->esp = sp;
  switchuvm(curproc);
  freevm(oldpgdir);

  cprintf("[kernel] exec: PID %d menjalankan program \"%s\"\n", curproc->pid, curproc->name);

  return 0;

```

---

### C. Tambahkan Log ke `exit()`

**File:** `proc.c`, fungsi `exit()`

Sebelum baris:

```c
  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
```

Tambahkan:

```c
 // Tambahkan log observasi di sini
  cprintf("[kernel] exit: PID %d (\"%s\") exited\n", curproc->pid, curproc->name);
```
Menjadi:

```c
...
  // Pass abandoned children to init.
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->parent == curproc){
      p->parent = initproc;
      if(p->state == ZOMBIE)
        wakeup1(initproc);
    }
  }

  // Tambahkan log observasi di sini
  cprintf("[kernel] exit: PID %d (\"%s\") exited\n", curproc->pid, curproc->name);

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
  ...
```
---

### D. Tambahkan Log ke `wait()`

**File:** `proc.c`, fungsi `wait()`

Sebelum:

```c
if(p->state == ZOMBIE){
```

Tambahkan:

```c
cprintf("[kernel] wait: PID %d waiting for child PID %d\n", curproc->pid, p->pid);
```

Menjadi:
```c
    ...
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      havekids = 1;

      cprintf("[kernel] wait: PID %d waiting for child PID %d\n", curproc->pid, p->pid);

      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
    ...
```
---

### E. Tambahkan Log ke `scheduler()`

**File:** `proc.c`, fungsi `scheduler()`

Setelah:

```c
if(p->state != RUNNABLE)
  continue;
```

Tambahkan:

```c
cprintf("[kernel] scheduler: Running PID %d (%s)\n", p->pid, p->name);
```

---

## ✅ Langkah 2 – Tambahkan File Program Uji

### A. Buat File Baru: `proclife.c`

**Tempatkan di folder utama (root)** karena di xv6-public tidak ada folder `user/`.

**Buat file baru:**

```bash
vim proclife.c
```

**Isi file `proclife.c`:**

```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  int pid = fork();
  if(pid < 0){
    printf(1, "[user] fork failed\n");
    exit();
  }
  if(pid == 0){
    // Child process
    printf(1, "[user] Child: my pid is %d\n", getpid());
    char *argv[] = { "echo", "hello [NIM_NAMA] from child", 0 };
    exec("echo", argv);
    printf(1, "[user] exec failed\n");
  } else {
    // Parent process
    printf(1, "[user] Parent: my pid is %d, waiting for child...\n", getpid());
    wait();
    printf(1, "[user] Parent: child finished\n");
  }
  exit();
}
```

---

### B. Tambahkan ke Makefile

**Edit `Makefile`**
Cari bagian:

```makefile
UPROGS=\
  _cat\
  _echo\
  ...
```

Tambahkan baris:

```makefile
  _proclife\
```

---

## ✅ Langkah 4 – Build dan Jalankan

```bash
make clean
make qemu-nox
```

Setelah xv6 boot:

```bash
$ proclife
```

---

## 🧾 Contoh Output

```
$ proclife
[kernel] scheduler: Running PID 2 (sh)
[kernel] scheduler: Running PID 2 (sh)
[kernel] fork: Parent PID 2 → Child PID 3
[kernel] scheduler: Running PID 3 (sh)
[kernel] wait: PID 2 waiting for child PID 3
[kernel] scheduler: Running PID 3 (sh)
[kernel] scheduler: Running PID 3 (sh)
[kernel] scheduler: Running PID 3 (sh)
[kernel] scheduler: Running PID 3 (sh)
[kernel] scheduler: Running PID 3 (sh)
[kernel] scheduler: Running PID 3 (sh)
[kernel] scheduler: Running PID 3 (sh)
[kernel] scheduler: Running PID 3 (sh)
[kernel] exec: PID 3 menjalankan program "proclife"
...

```

---

## 📌 Ringkasan Proses yang Terjadi

| Fungsi        | Deskripsi                                                   |
| ------------- | ----------------------------------------------------------- |
| `fork()`      | Membuat proses anak, yang identik dengan proses induk       |
| `exec()`      | Menimpa proses anak dengan program baru (misalnya `echo`)   |
| `exit()`      | Menandakan proses telah selesai                             |
| `wait()`      | Proses induk menunggu sampai anak selesai                   |
| `scheduler()` | Kernel memilih proses `RUNNABLE` dan menjadwalkannya di CPU |

---
