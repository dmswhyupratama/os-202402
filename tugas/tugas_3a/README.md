# 📘 Tutorial Praktikum 3: Eksplorasi Penjadwalan Proses

---

## 🧭 **Pendahuluan**

Tutorial ini akan memandu Anda untuk memodifikasi algoritma penjadwalan pada sistem operasi xv6 menggunakan Docker. Anda akan belajar menambahkan fitur penjadwalan berbasis prioritas, membuat syscall baru, dan menjalankan sistem menggunakan container Docker.

---

## 📁 **1. Persiapan Struktur Folder Proyek**

Kita akan membuat struktur direktori proyek di folder mana saja pada PC/laptop lokal Anda, untuk memisahkan komponen praktikum agar file konfigurasi, skrip, dan hasil pengamatan terorganisasi.
**How:** Gunakan perintah berikut di terminal:

```
/Praktikum_3
│
├── Dockerfile
└── README.md 
```

---

## 🐳 **2. Build Docker Image**

Docker image adalah lingkungan virtual berisi tools untuk menjalankan xv6, agar tidak perlu mengatur ulang OS atau dependensi manual dan hanya dilakukan satu kali di awal praktikum.

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

Lalu buka terminal , untuk build docker jalankan:
```bash
docker build -t xv6 .
```

## 🚀 **3. Jalankan Docker Container**

Agar semua eksperimen dilakukan dalam sistem terisolasi, jalankan lingkungan praktikum di dalam Docker.

caranya:
```bash
docker run -it --rm xv6
```
**Penjelasan:**

* `-it`: agar bisa interaktif.
* `--rm`: otomatis hapus container saat keluar.

---

## 🔁 **4. Jalankan xv6 (QEMU)**

Selanjutnya kita akan menjalankan OS xv6 yang telah dibuild sebelumnya, untuk mengamati dan menguji cara kerja kernel dan proses nya.

caranya di dalam container, jalankan:

```bash
make qemu-nox
```
Jika tertampil  seperti berikut ini maka sukses masuk ke QEMU xv6
```

Booting from Hard Disk..xv6...
cpu1: starting 1
cpu0: starting 0
sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
init: starting sh
$ 

```

---

## ⏹️ **5. Keluar dari QEMU**

Tekan `Ctrl + a`, lalu tekan `x` untuk keluar dari tampilan QEMU.

```
$ QEMU: Terminated
```
saat keluar dari Qemu akan masuk ke terminal container lagi.
---
