Modul ini akan membantu Anda menjalankan sistem operasi xv6 di dalam container Docker untuk memahami proses dan thread dalam OS.

## Langkah Singkat

1. *Build Docker Image*
'''
docker build -t xv6-praktikum .
'''

2. *Jalankan Container*
'''
docker run -it --rm xv6-praktikum
'''

3. *Masuk ke xv6*
'''
make qemu-nox
'''
4. *Perintah dalam xv6*
'''
ps
fork
exit
kill <pid>
'''

## Tujuan Praktikum
Memahami struktur proses dalam kernel xv6 -
Mengamati proses dan percabangannya
-Latihan dengan syscall dasar