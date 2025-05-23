# 📘 Tutorial Praktikum 3: Pengenalan xv6 dan Instalasi xv6

---

## 🎯 **Tujuan Pembelajaran**

Setelah mengikuti praktikum ini, mahasiswa diharapkan mampu:

- Menjelaskan tujuan penggunaan xv6 sebagai OS pembelajaran.
- Mengunduh dan membangun sistem operasi xv6 menggunakan QEMU.
- Menjalankan xv6 dan mengenali struktur direktori kode sumber menggunakan Docker.

---
## 🧩 **1. Apa itu xv6?**

xv6 adalah sistem operasi sederhana berbasis Unix Version 6 yang ditulis ulang oleh MIT untuk keperluan pendidikan. Fitur utama xv6 meliputi:

- Proses dan sistem panggilan (`fork`, `exec`, `wait`, `exit`)
- Manajemen memori
- File system
- Driver sederhana dan interrupt

> xv6 digunakan untuk mempelajari **struktur sistem operasi** dan bagaimana komponen-komponennya berinteraksi secara langsung pada level kernel.

---

## 📁 **2. Persiapan Struktur Folder Proyek**

Buat struktur direktori berikut agar file konfigurasi, skrip, dan hasil pengamatan terorganisasi:

```
/Praktikum_3
│
├── Dockerfile
├── README.md 
├── captures/         # Folder untuk screenshot/output
└── xv6-public/       # Folder hasil clone xv6
```

---

## 🐳 **3. Build Docker Image**

Docker image adalah lingkungan virtual berisi tools untuk menjalankan xv6, sehingga Anda tidak perlu mengatur ulang OS atau dependensi manual.

Buat file `Dockerfile` di folder `Praktikum_3` dengan isi:

```dockerfile
FROM debian:bullseye
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential git qemu-system-x86 \
    vim nano neovim python3 curl make gcc gdb \
    nasm libgcc-9-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /xv6
RUN git clone https://github.com/mit-pdos/xv6-public.git .
RUN make

CMD ["/bin/bash"]
```

Build Docker image:
```bash
docker build -t xv6 .
```

---

## 🚀 **4. Jalankan Docker Container**

Jalankan lingkungan praktikum di dalam Docker:

```bash
docker run -it --rm xv6
```
**Penjelasan:**
- `-it`: agar bisa interaktif.
- `--rm`: otomatis hapus container saat keluar.

---

## 🔁 **5. Jalankan xv6 (QEMU)**

Di dalam container, jalankan:

```bash
make qemu-nox
```

Jika berhasil, akan muncul tampilan seperti:
```
Booting from Hard Disk..xv6...
cpu1: starting 1
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ 
```

---

## ⏹️ **6. Keluar dari QEMU**

Tekan `Ctrl + a`, lalu tekan `x` untuk keluar dari tampilan QEMU.

```
$ QEMU: Terminated
```
Setelah keluar dari QEMU, Anda akan kembali ke terminal container.

---

## 🔍 **7. Struktur Direktori xv6-public**

Penjelasan file penting:

| File / Folder | Deskripsi                                      |
| ------------- | ---------------------------------------------- |
| `main.c`      | Titik masuk utama OS                           |
| `proc.c`      | Manajemen proses (`fork`, `exit`, `scheduler`) |
| `sysproc.c`   | Implementasi syscall user-level                |
| `syscall.c`   | Tabel dan dispatcher syscall                   |
| `usys.S`      | Stub syscall untuk user space                  |
| `vm.c`        | Virtual memory dan paging                      |
| `fs.c`        | Sistem file (inode, block, path)               |
| `trap.c`      | Penanganan interrupt dan syscall               |
| `bio.c`       | Buffer I/O dari disk                           |
| `kernel/`     | Folder untuk file header dan sistem utama      |
| `user/`       | Program user-space (seperti `ls`, `cat`, `sh`) |

> **Tugas:** scren shoot struktur ini sebagai bagian dari laporan.

---