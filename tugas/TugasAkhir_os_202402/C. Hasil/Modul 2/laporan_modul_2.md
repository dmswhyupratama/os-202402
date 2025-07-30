# 📝 Laporan Tugas Akhir

Mata Kuliah: Sistem Operasi
Semester: Genap / Tahun Ajaran 2024–2025
Nama: Dimas Wahyu Pratama
NIM: 240202858
Modul yang Dikerjakan: Modul 2 – Penjadwalan CPU Lanjutan (Priority Scheduling Non-Preemptive)

---

## 📌 Deskripsi Singkat Tugas

* **Modul 2 – Penjadwalan CPU Lanjutan (Priority Scheduling Non-Preemptive):**:
Praktikum ini berfokus pada modifikasi algoritma penjadwalan proses di xv6 dari Round Robin menjadi Non-Preemptive Priority Scheduling. Tugas utamanya meliputi penambahan field prioritas pada setiap proses, implementasi system call set_priority() untuk mengatur prioritas, dan modifikasi scheduler agar selalu menjalankan proses RUNNABLE dengan prioritas tertinggi (nilai numerik terkecil) hingga proses tersebut selesai atau menyerahkan CPU secara sukarela.
---

## 🛠️ Rincian Implementasi

* Menambahkan Field priority: Menambahkan field int priority; ke dalam struct proc di file proc.h. Nilai prioritas yang lebih kecil menunjukkan prioritas yang lebih tinggi (misalnya, 0 adalah prioritas tertinggi, 100 adalah terendah).
* Inisialisasi priority: Mengatur nilai default p->priority = 60; untuk setiap proses baru yang dialokasikan di fungsi allocproc() dalam proc.c.
* Membuat System Call set_priority(int):
   1. Menambahkan nomor system call SYS_set_priority (misalnya 22 atau nomor berurutan yang sesuai) di syscall.h.
   2. Menambahkan deklarasi fungsi int set_priority(int priority); di user.h.
   3. Menambahkan entri SYSCALL(set_priority) di usys.S untuk stub user-level.
   4. Mendaftarkan system call sys_set_priority di syscall.c dengan menambahkan deklarasi extern dan entri ke array syscalls[].
   5. Mengimplementasikan fungsi sys_set_priority() di sysproc.c yang mengambil argumen prioritas dari user-space dan mengatur field priority proses saat ini.
* Memodifikasi Fungsi scheduler(): Mengubah logika scheduler di proc.c untuk mencari proses RUNNABLE dengan nilai priority terkecil (prioritas tertinggi). Proses yang terpilih akan dijalankan hingga selesai atau menyerahkan CPU secara sukarela (non-preemptive).
* Membuat Program Pengujian ptest.c: Membuat program user-level ptest.c yang melakukan forking dua proses anak. Setiap anak diatur prioritasnya secara berbeda, dan kemudian melakukan pekerjaan sibuk untuk menguji urutan eksekusi berdasarkan prioritas.
* Menambahkan Program Pengujian ke Makefile: Mendaftarkan _ptest ke dalam daftar UPROGS di Makefile agar dapat dikompilasi dan dijalankan di xv6.
---

## ✅ Uji Fungsionalitas

* Program ptest digunakan untuk menguji keberhasilan implementasi penjadwalan prioritas non-preemptive.
* Output dari program menunjukkan bahwa proses anak dengan prioritas lebih tinggi (Child 2) selesai terlebih dahulu, diikuti oleh proses anak dengan prioritas lebih rendah (Child 1), dan terakhir proses induk (Parent). Ini memvalidasi bahwa scheduler memprioritaskan proses berdasarkan nilai prioritas yang telah diatur
---

## 💻 Implementasi Kode
Berikut adalah ringkasan perubahan kode pada setiap file:

proc.h
```c
struct proc {
  // ...
  char name[16];               // Process name (debugging)
  int priority;                // Tambahkan Ini
};
```

proc.c
```c
static struct proc*
allocproc(void)
{
  // ...
found:
  p->state = EMBRYO;
  p->priority = 60; // Inisialisasi nilai prioritas default
  p->pid = nextpid++;
  // ...
}

void
scheduler(void)
{
  struct proc *p;
  struct proc *highest = 0;
  struct cpu *c = mycpu(); 

  for(;;){
    sti();
    acquire(&ptable.lock);
    highest = 0; // Reset highest setiap kali mencari proses baru

    // Cari proses RUNNABLE dengan prioritas tertinggi (nilai numerik terkecil)
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;
      if(highest == 0 || p->priority < highest->priority)
        highest = p;
    }

    if(highest != 0){
      p = highest;
      c->proc = p; 
      switchuvm(p);
      p->state = RUNNING;
      swtch(&c->scheduler, p->context);
      switchkvm();
      c->proc = 0;
    }
    release(&ptable.lock);
  }
}
```

syscall.h
```c
#define SYS_close  21
#define SYS_set_priority 22 // Tambahkan Nomor System Call Baru
```

user.h
```c
int uptime(void);

// ulib.c
// ...
int set_priority(int priority);  // Deklarasi fungsi set_priority
```

usys.S
```c
SYSCALL(set_priority) // Tambahkan entri baru
```

syscall.c
```c
extern int sys_uptime(void);
extern int sys_set_priority(void);  // Tambahkan deklarasi baru

static int (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
// ...
[SYS_close]   sys_close,
[SYS_set_priority] sys_set_priority,  // Registrasi syscall baru
};
```

sysproc.c
```c
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// Implementasi sys_set_priority
int sys_set_priority(void) {
  int prio;
  if (argint(0, &prio) < 0 || prio < 0 || prio > 100)
    return -1;

  myproc()->priority = prio;
  return 0;
}
```

ptest.c (Program Pengujian)
```c
#include "types.h"
#include "stat.h"
#include "user.h"

// Fungsi untuk mensimulasikan pekerjaan yang sibuk
void busy() {
  for (volatile int i = 0; i < 100000000; i++); // Loop sibuk
}

int main() {
  int pid1 = fork();
  if (pid1 == 0) {
    // Proses anak pertama
    set_priority(90); // Prioritas rendah (angka tinggi = prioritas rendah)
    busy(); // Lakukan pekerjaan sibuk
    printf(1, "Child 1 selesai\n");
    exit(); // Keluar dari proses anak
  }

  int pid2 = fork();
  if (pid2 == 0) {
    // Proses anak kedua
    set_priority(10); // Prioritas tinggi (angka rendah = prioritas tinggi)
    busy(); // Lakukan pekerjaan sibuk
    printf(1, "Child 2 selesai\n");
    exit(); // Keluar dari proses anak
  }

  // Proses induk menunggu kedua anak selesai
  sleep(10); // Beri waktu agar anak-anak bisa dijadwalkan dan berjalan
  wait(); // Tunggu anak pertama
  wait(); // Tunggu anak kedua
  printf(1, "Parent selesai\n");
  exit(); // Keluar dari proses induk
}
```


## 📷 Hasil Uji

### 📍Output `ptest`:

```
Child 2 selesai  
Child 1 selesai  
Parent selesai
```
---

## ⚠️ Kendala yang Dihadapi
Tidak ada kendala signifikan yang dihadapi selama implementasi modul ini(walaupun terdapat beberapa eror yang terjadi karena kesalahan penempatan code atau variabel yang tidak terdeklarasi dengan benar). Fokus utama saat praktikum adalah pada modifikasi logika scheduler untuk mencari proses dengan prioritas tertinggi dan memastikan perilaku non-preemptive yang diharapkan.

## 📚 Referensi

Tuliskan sumber referensi yang Anda gunakan, misalnya:

* Buku xv6 MIT: [https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)
* Repositori xv6-public: [https://github.com/mit-pdos/xv6-public](https://github.com/mit-pdos/xv6-public)
* Stack Overflow, GitHub Issues, diskusi praktikum

---

