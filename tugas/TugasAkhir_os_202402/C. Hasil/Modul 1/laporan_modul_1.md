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

* Menambahkan dua system call baru di file `sysproc.c` dan `syscall.c`
* Mengedit `user.h`, `usys.S`, dan `syscall.h` untuk mendaftarkan syscall
* Menambahkan struktur `struct pinfo` di `proc.h`
* Menambahkan counter `readcount` di kernel
* Membuat dua program uji: `ptest.c` dan `rtest.c`
---

## ✅ Uji Fungsionalitas

* `ptest`: untuk menguji `getpinfo()`
* `rtest`: untuk menguji `getReadCount()`
* `cowtest`: untuk menguji fork dengan Copy-on-Write
* `shmtest`: untuk menguji `shmget()` dan `shmrelease()`
* `chmodtest`: untuk memastikan file `read-only` tidak bisa ditulis
* `audit`: untuk melihat isi log system call (jika dijalankan oleh PID 1)

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
Read Count Sebelum: 13
```

Jika ada screenshot:

```
=======
![Output Modul 2](../Screenshots/Modul2.png)

```

---

## ⚠️ Kendala yang Dihadapi

---

## 📚 Referensi

Tuliskan sumber referensi yang Anda gunakan, misalnya:

* Buku xv6 MIT: [https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf](https://pdos.csail.mit.edu/6.828/2018/xv6/book-rev11.pdf)
* Repositori xv6-public: [https://github.com/mit-pdos/xv6-public](https://github.com/mit-pdos/xv6-public)
* Stack Overflow, GitHub Issues, diskusi praktikum

---

