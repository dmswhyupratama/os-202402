# 📝 Laporan Tugas Akhir

Mata Kuliah: Sistem Operasi
Semester: Genap / Tahun Ajaran 2024–2025
Nama: Dimas Wahyu Pratama
NIM: 240202858
Modul yang Dikerjakan:
(Modul 2 – Proses dan Manajemen Proses)

---

## 📌 Deskripsi Singkat Tugas

* **Modul 2 – Proses dan Manajemen Proses:**:
  Praktikum ini berfokus pada mekanisme pembuatan dan manajemen proses di `xv6`. Salah satu tugas utamanya adalah membuat program yang melakukan forking dan menampilkan urutan eksekusi proses `parent` dan `child`.
---

## 🛠️ Rincian Implementasi

* Membuat program `ptest.c` yang melakukan forking dua proses anak
* Mengatur urutan eksekusi agar child selesai terlebih dahulu, diikuti parent
* Menggunakan `fork()` dan `wait()` untuk mengatur sinkronisasi antar proses
* Menambahkan program ke `Makefile` agar dapat di-compile dalam xv6
---

## ✅ Uji Fungsionalitas

* Program ptest digunakan untuk menguji keberhasilan proses fork() dan wait()
* Output dari program menunjukkan bahwa proses child selesai terlebih dahulu, baru kemudian parent
* Tidak ada error atau panic selama proses eksekusi berlangsung

---

## 📷 Hasil Uji

### 📍Output `ptest`:

```
Child 2 selesai  
Child 1 selesai  
Parent selesai
```
Jika ada screenshot:

```
![hasil Modul2](.Modul2.png)
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

