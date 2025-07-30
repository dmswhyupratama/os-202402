# 📝 Laporan Tugas Akhir

**Mata Kuliah**: Sistem Operasi
**Semester**: Genap / Tahun Ajaran 2024–2025
**Nama**: Dimas Wahyu Pratama
**NIM**: 240202858
**Modul yang Dikerjakan**: Modul 3 – Manajemen Memori Tingkat Lanjut

---

## 📌 Deskripsi Singkat Tugas

* **Modul 3 – Manajemen Memori Tingkat Lanjut:**:
Praktikum ini mengeksplorasi dua fitur penting dalam manajemen memori pada OS xv6: Copy-on-Write (CoW) Fork dan Shared Memory ala System V. Tujuannya adalah untuk meningkatkan efisiensi pemakaian memori pada saat fork(), serta memungkinkan komunikasi antar proses melalui memori bersama dengan mekanisme yang aman dan efisien.
---

## 🛠️ Rincian Implementasi

* Implementasi Copy-on-Write (CoW) Fork:
    1. Menambahkan array ref_count[] di vm.c untuk menghitung referensi setiap halaman fisik.
    2. Membuat fungsi incref() dan decref() untuk manajemen reference count, termasuk penanganan pembebasan halaman fisik saat reference count mencapai nol.
    3. Menambahkan flag PTE_COW di mmu.h untuk menandai halaman yang diatur sebagai CoW.
    4. Membuat fungsi baru cowuvm() di vm.c yang menggantikan copyuvm() dalam fork(). Fungsi ini mengatur halaman-halaman proses anak sebagai read-only dan menandainya dengan PTE_COW, serta menginkrementasi reference count halaman fisik yang dibagi.
    5. Memodifikasi fungsi fork() di proc.c untuk memanggil cowuvm() saat menyalin ruang alamat proses.
    6. Menambahkan handler di trap.c untuk menangani page fault yang disebabkan oleh upaya penulisan ke halaman CoW. Handler ini akan mengalokasikan halaman fisik baru, menyalin konten, memperbarui PTE, dan mengurangi reference count halaman lama.
* Implementasi Shared Memory ala System V:
    1. Menambahkan struktur shmtab[] di sysproc.c untuk menyimpan kunci (key), pointer frame fisik, dan reference count untuk setiap segmen shared memory.
    2. Mengimplementasikan system call shmget(int key) di sysproc.c untuk mendapatkan atau membuat segmen shared memory berdasarkan kunci yang diberikan. Fungsi ini juga memetakan halaman shared memory ke ruang alamat proses pengguna.
    3. Mengimplementasikan system call shmrelease(int key) di sysproc.c untuk melepaskan segmen shared memory berdasarkan kunci. Fungsi ini mengurangi reference count dan membebaskan halaman fisik jika count mencapai nol, serta menghapus pemetaan dari page table proses.
    4. Melakukan registrasi system call shmget dan shmrelease di user.h (deklarasi), usys.S (entri stub), syscall.c (deklarasi extern dan registrasi ke array syscalls[]), dan syscall.h (definisi nomor system call).
    5. Menambahkan program uji cowtest.c dan shmtest.c ke Makefile.
---

## ✅ Uji Fungsionalitas
Program uji yang digunakan untuk memverifikasi implementasi manajemen memori tingkat lanjut adalah:

* cowtest: Digunakan untuk menguji fungsionalitas Copy-on-Write (CoW) saat fork(). Program ini memverifikasi bahwa halaman memori dibagikan secara read-only antara parent dan child, dan penyalinan hanya terjadi saat penulisan.
* shmtest: Digunakan untuk menguji system call shmget() dan shmrelease() serta fungsionalitas shared memory. Program ini memverifikasi bahwa proses yang berbeda dapat berbagi dan memodifikasi data pada segmen memori yang sama.
---

## 💻 Implementasi Kode
Berikut adalah ringkasan perubahan kode pada setiap file:

vm.c
```c
// vm.c (di bagian global, setelah #include)
// ... (struktur shmregion dan shmtab yang sudah ada) ...

#define PHYSTOP 0xE000000
#define NPAGE (PHYSTOP >> 12)

int ref_count[NPAGE]; // satu counter per physical page

void incref(char *pa) {
  // Pastikan ada mekanisme penguncian jika ref_count diakses oleh multiple CPU
  // Contoh: acquire(&kmem.lock);
  ref_count[V2P(pa) >> 12]++;
  // Contoh: release(&kmem.lock);
}

void decref(char *pa) {
  // Pastikan ada mekanisme penguncian jika ref_count diakses oleh multiple CPU
  // Contoh: acquire(&kmem.lock);
  int idx = V2P(pa) >> 12;
  if (--ref_count[idx] == 0)
    kfree(pa);
  // Contoh: release(&kmem.lock);
}

// ... (fungsi setupkvm, allocuvm, deallocuvm, freevm, clearpteu) ...

pde_t*
cowuvm(pde_t *pgdir, uint sz)
{
  pde_t *d;
  pte_t *pte;
  uint pa;
  uint flags;

  if((d = setupkvm()) == 0)
    return 0;

  for(uint i = 0; i < sz; i += PGSIZE){
    if((pte = walkpgdir(pgdir, (void*)i, 0)) == 0)
      panic("cowuvm: pte should exist");
    if(!(*pte & PTE_P))
      panic("cowuvm: page not present");

    pa = PTE_ADDR(*pte);
    flags = PTE_FLAGS(*pte);

    flags &= ~PTE_W;
    flags |= PTE_COW;

    incref((char*)P2V(pa));

    if(mappages(d, (void*)i, PGSIZE, pa, flags) < 0){
      freevm(d);
      return 0;
    }

    *pte = (*pte & ~PTE_W) | PTE_COW;
  }
  return d;
}

// ... (sisa kode vm.c) ...
```

trap.c
```c
// ... include dan deklarasi lainnya ...

void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_PGFLT){
    uint va = rcr2();
    struct proc *p = myproc();

    pte_t *pte = walkpgdir(p->pgdir, (void*)va, 0);

    if(!pte || !(*pte & PTE_P) || !(*pte & PTE_COW)){
      cprintf("pid %d %s: Invalid page fault at %x (PTE: %p, flags: %x)--kill proc\n",
              p->pid, p->name, va, pte, pte ? *pte & 0xFFF : 0);
      p->killed = 1;
      return;
    }

    uint pa = PTE_ADDR(*pte);
    char *mem = kalloc();
    if(mem == 0){
      cprintf("kalloc failed in page fault handler\n");
      p->killed = 1;
      return;
    }

    memmove(mem, (char*)P2V(pa), PGSIZE);
    decref((char*)P2V(pa));

    *pte = V2P(mem) | PTE_W | PTE_U | PTE_P;
    *pte &= ~PTE_COW;

    lcr3(V2P(p->pgdir));
    return;
  }
      
  // ... sisa fungsi trap ...
}
```

proc.c
```c
// ... fungsi-fungsi proc.c lainnya ...

int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  if((np = allocproc()) == 0){
    return -1;
  }

  // Ganti copyuvm dengan cowuvm
  if((np->pgdir = cowuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;

  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// ... sisa fungsi proc.c ...
```

defs.h
```c
// ... deklarasi fungsi lainnya ...

// vm.c
// ...
pde_t* cowuvm(pde_t *pgdir, uint sz);
// ...
void            incref(char *pa);
void            decref(char *pa);
// ...
```

mmu.h
```c
#define PTE_COW 0x200  // custom flag untuk CoW
// ... flag PTE lainnya ...
```

sysproc.c
```c
// sysproc.c (di bagian global, setelah #include)
// ...
#define MAX_SHM 16 // Jumlah maksimum segmen shared memory yang didukung

struct {
  int key;      // Kunci unik untuk segmen shared memory
  char *frame;  // Alamat fisik halaman memori yang dibagi
  int refcount; // Jumlah proses yang sedang menggunakan segmen ini
} shmtab[MAX_SHM];

// ... deklarasi extern lainnya ...

#define USERTOP 0xA0000  // 640KB typical user top in xv6 (sesuaikan jika perlu)

// Implementasi sys_shmget: Mendapatkan atau membuat segmen shared memory
void* sys_shmget(void) {
  int key;
  if(argint(0, &key) < 0)
    return (void*)-1;

  struct proc *curproc = myproc();

  for(int i = 0; i < MAX_SHM; i++){
    if(shmtab[i].key == key){
      shmtab[i].refcount++;
      if (mappages(curproc->pgdir, (void*)(USERTOP - (i+1)*PGSIZE), PGSIZE,
                   V2P(shmtab[i].frame), PTE_W|PTE_U) < 0) {
        shmtab[i].refcount--;
        return (void*)-1;
      }
      return (void*)(USERTOP - (i+1)*PGSIZE);
    }
  }

  for(int i = 0; i < MAX_SHM; i++){
    if(shmtab[i].key == 0){
      shmtab[i].key = key;
      shmtab[i].frame = kalloc();
      if (shmtab[i].frame == 0) {
        shmtab[i].key = 0;
        return (void*)-1;
      }
      shmtab[i].refcount = 1;
      memset(shmtab[i].frame, 0, PGSIZE);
      if (mappages(curproc->pgdir, (void*)(USERTOP - (i+1)*PGSIZE), PGSIZE,
                   V2P(shmtab[i].frame), PTE_W|PTE_U) < 0) {
        kfree(shmtab[i].frame);
        shmtab[i].key = 0;
        shmtab[i].frame = 0;
        return (void*)-1;
      }
      return (void*)(USERTOP - (i+1)*PGSIZE);
    }
  }

  return (void*)-1;
}

// Implementasi sys_shmrelease: Melepaskan segmen shared memory
int sys_shmrelease(void) {
  int key;
  if(argint(0, &key) < 0)
    return -1;

  struct proc *curproc = myproc();

  for(int i = 0; i < MAX_SHM; i++){
    if(shmtab[i].key == key){
      shmtab[i].refcount--;
      clearpteu(curproc->pgdir, (char*)(USERTOP - (i+1)*PGSIZE)); // Hapus pemetaan

      if(shmtab[i].refcount == 0){
        kfree(shmtab[i].frame);
        shmtab[i].key = 0;
        shmtab[i].frame = 0;
      }
      return 0;
    }
  }
  return -1;
}
```

user.h
```c
// ... deklarasi syscall lainnya ...
void* shmget(int);
int shmrelease(int);
```

usys.S
```c
// ... SYSCALL makro lainnya ...
SYSCALL(shmget)
SYSCALL(shmrelease)
```

syscall.c
```c
// ... deklarasi extern lainnya ...
extern int sys_shmget(void);
extern int sys_shmrelease(void);
...
static int (*syscalls[])(void) = {
// ... syscall lainnya ...
[SYS_close]   sys_close,
[SYS_shmget]     sys_shmget,
[SYS_shmrelease] sys_shmrelease,
};
// ... sisa kode syscall.c ...
```

syscall.h
```c
#define SYS_close  21
// ... jika ada syscall dari modul sebelumnya, sesuaikan nomor ini
#define SYS_shmget     25
#define SYS_shmrelease 26
```

cowtest.c (Program Pengujian)
```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  char *p = sbrk(4096); // Alokasikan satu halaman memori
  p[0] = 'X'; // Tulis ke halaman di proses induk

  int pid = fork(); // Lakukan fork

  if(pid == 0){
    // Proses anak
    p[0] = 'Y'; // Tulis ke halaman di proses anak (ini akan memicu CoW)
    printf(1, "Child sees: %c\n", p[0]); // Anak melihat 'Y'
    exit();
  } else {
    // Proses induk
    wait(); // Induk menunggu anak selesai
    printf(1, "Parent sees: %c\n", p[0]); // Induk masih melihat 'X'
  }
  exit();
}
```

shmtest.c (Program Pengujian)
```c
#include "types.h"
#include "stat.h"
#include "user.h"

int main() {
  char *shm = (char*) shmget(42); // Dapatkan segmen shared memory dengan key 42
  if (shm == (char*)-1) {
    printf(2, "shmget failed in parent\n");
    exit();
  }
  shm[0] = 'A'; // Tulis 'A' ke shared memory dari proses induk

  if(fork() == 0){
    // Proses anak
    char *shm2 = (char*) shmget(42); // Dapatkan segmen shared memory yang sama
    if (shm2 == (char*)-1) {
      printf(2, "shmget failed in child\n");
      exit();
    }
    printf(1, "Child reads: %c\n", shm2[0]); // Anak membaca 'A' dari shared memory
    shm2[1] = 'B'; // Anak menulis 'B' ke shared memory
    shmrelease(42); // Anak melepaskan segmen shared memory
    exit();
  } else {
    // Proses induk
    wait(); // Induk menunggu anak selesai
    printf(1, "Parent reads: %c\n", shm[1]); // Induk membaca 'B' dari shared memory
    shmrelease(42); // Induk melepaskan segmen shared memory
  }
  exit();
}
```

## 📷 Hasil Uji

### 📍 Output `cowtest`:

```
$ cowtest
Child sees: Y
Parent sees: X
```
* Output ini menunjukkan bahwa cowtest berhasil dijalankan. Proses anak (Child) berhasil memodifikasi salinan halamannya sendiri (melihat 'Y'), sementara proses induk (Parent) masih melihat nilai aslinya ('X'). Ini mengkonfirmasi bahwa Copy-on-Write berfungsi dengan benar, di mana halaman disalin hanya saat terjadi penulisan.


### 📍 Output `shmtest`:

```
$ shmtest
Child reads: A
Parent reads: B
```
* Output ini menunjukkan bahwa shmtest berhasil dijalankan. Proses anak (Child) berhasil membaca data ('A') yang ditulis oleh proses induk (Parent) ke shared memory. Kemudian, proses anak menulis 'B', dan proses induk berhasil membaca 'B' dari shared memory yang sama. Ini memvalidasi bahwa shared memory berfungsi dengan benar, memungkinkan komunikasi antar proses melalui memori bersama.
---

## ⚠️ Kendala yang Dihadapi

Sebelum melakukan debugging dan perbaikan, beberapa kendala signifikan yang dihadapi adalah:

* Kegagalan Eksekusi Program Uji: Program shmtest gagal dieksekusi dengan pesan exec: fail dan exec make failed. Ini mengindikasikan bahwa file program mungkin belum berhasil dikompilasi ke dalam filesystem xv6 atau ada masalah pada path eksekusi.
* Kernel Panic dan Invalid Page Fault: Terjadi serangkaian Invalid page fault dan trap 13 (general protection fault) yang menyebabkan kernel panic dan sistem crash. Hal ini menunjukkan bahwa handler page fault belum menangani akses memori ilegal dengan benar, seperti akses ke alamat 0x0 atau 0x1000 yang tidak valid atau tidak dipetakan.
* Masalah Pemetaan Shared Memory: Pemetaan shared memory bermasalah, kemungkinan alamat USERTOP - PGSIZE belum dipetakan dengan benar atau berbenturan dengan halaman lain.
* Kurangnya Validasi Error pada System Call: Implementasi awal system call shmget() dan shmrelease() kurang memiliki validasi error yang memadai, sehingga sulit untuk melakukan debugging dan tidak ada output yang jelas mengenai keberhasilan atau kegagalan saat system call dipanggil.

Solusi dan Perbaikan:
Kendala-kendala ini berhasil diatasi dengan beberapa perbaikan kunci:
* alidasi dan Penanganan Error yang Lebih Baik: Menambahkan pemeriksaan if (shm == (char*)-1) pada shmget di shmtest.c untuk menangkap kegagalan alokasi atau pemetaan shared memory.
* Perbaikan Page Fault Handler: Memastikan page fault handler di trap.c secara tepat mengidentifikasi dan menangani page fault CoW, termasuk alokasi halaman baru, penyalinan data, pembaruan PTE dengan flag PTE_W dan tanpa PTE_COW, serta pembebasan reference halaman lama.
* Pembersihan PTE pada shmrelease: Menambahkan pemanggilan clearpteu() di shmrelease() untuk memastikan pemetaan halaman shared memory dihapus dari page table proses yang melepaskannya. Ini mencegah dangling references dan konflik pemetaan.
* Inisialisasi shmtab: Memastikan shmtab diinisialisasi dengan benar dan frame fisik dialokasikan dengan kalloc() hanya sekali per kunci shared memory dan dibebaskan dengan kfree() hanya ketika reference count mencapai nol.
* Konsistensi Reference Counting: Memastikan incref() dan decref() dipanggil secara konsisten di seluruh kode manajemen memori (termasuk di allocuvm, deallocuvm, freevm, walkpgdir, dan setupkvm) untuk menjaga integritas reference count.

---

## 📚 Referensi

Tuliskan sumber referensi yang Anda gunakan, misalnya:

* Buku xv6 MIT: [https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)
* Repositori xv6-public: [https://github.com/mit-pdos/xv6-public](https://github.com/mit-pdos/xv6-public)
* Stack Overflow, GitHub Issues, diskusi praktikum

---

