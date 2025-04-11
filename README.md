# Tucil2_13523127_13523129
## Deskripsi
Program kompresi gambar yang memanfaatkan algoritma divide and conquer pada struktur data quadtree.

## Cara Menggunakan

### Prasyarat
Kompiler: G++ (atau kompiler C++ lain yang mendukung C++11 atau lebih baru).
### Instalasi
#### Clone repositori ini ke komputer Anda:
```bash
git clone https://github.com/vant8k/Tucil2_13523127_13523129.git
```
#### Masuk ke direktori proyek
```bash
cd Tucil2_13523127_13523129
```
### Menjalankan Program
#### Buka terminal atau command prompt.
```bash
cd bin
```
#### Arahkan direktori ke folder bin, yang berisi file hasil kompilasi:
```bash
./quadtree
```
#### Format Masukan
Format masukan adalah sebagai berikut :
- Masukkan path untuk gambar yang ingin dikompres
- Masukkan nilai threshold serta besar minimal blok yang diinginkan.
- Masukkan nilai target kompresi (0-1) untuk mengaktifkan adaptive threshold. Masukkan 0 jika ingin menonaktifkan fitur ini.
- Masukkan path untuk hasil keluaran gambar yang sudah dikompres.
## Fitur
- Kompresi gambar berbasis quadtree dengan metrik error: Variance, MAD, Max Pixel Difference, dan Entropy.
- Konfigurasi ambang batas (threshold), ukuran blok minimum, dan target kompresi.
- Uji coba pada gambar umum dan kontras tinggi untuk evaluasi efisiensi dan kualitas.
- Laporan hasil: waktu eksekusi, ukuran file terkompresi, persentase kompresi, kedalaman pohon, dan jumlah node.
## Struktur Direktori
```bash
Tucil2_13523127_13523129/
│-- bin/          # Folder berisi file hasil kompilasi
|-- doc/          # Folder berisi dokumen laporan
│-- src/          # Folder berisi source code C++
│-- test/         # Folder berisi file test berupa .txt sebagai keterangan serta hasil output gambar
|-- LICENSE       # File keterrangan lisensi
│-- README.md     # File dokumentasi
```
# Catatan
- Jika ada error saat menjalankan program, pastikan Anda berada di dalam direktori proyek yang benar.
- Pastikan semua dependensi sudah terinstal dengan benar.
- Jika terjadi masalah dengan input, periksa kembali format file input sesuai spesifikasi.
# Lisensi
Program ini dibuat untuk keperluan akademik dan bebas digunakan serta dimodifikasi sesuai kebutuhan.
