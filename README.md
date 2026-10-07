# Boolean Minimization Visualizer & Logic Circuit Drawer (K-Map Tool)
> **Đồ án cuối kỳ môn Toán rời rạc và Lý thuyết đồ thị (Discrete Mathematics and Graph Theory)**  
> **Dự án môn học - Năm 2**  
> **Trường Đại học Sư phạm Kỹ thuật TP.HCM (HCMUTE)** — *Viện Đào tạo Quốc tế (Faculty of International Education)*

---

## 👥 Thành viên nhóm thực hiện (Nhóm 8)

| STT | Họ và Tên | Mã số sinh viên (MSSV) | Vai trò |
| :---: | :--- | :---: | :--- |
| 1 | **Lê Ngọc Hải** | `24110089` | Trưởng nhóm / Lập trình chính |
| 2 | **Lê Huy Phát** | `24110118` | Thành viên |
| 3 | **Huỳnh Lê Anh Tuấn** | `24110143` | Thành viên |

---

## 📌 Giới thiệu dự án

Chương trình được phát triển nhằm mục đích trực quan hóa quá trình tối tiểu hóa hàm Boole bằng **bản đồ Karnaugh (Karnaugh Map - K-Map)** và tự động vẽ sơ đồ mạch logic tương ứng từ biểu thức tối tiểu.

Phần mềm hỗ trợ:
- Nhập biểu thức Boole dạng tổng các tích (SOP - Sum of Products).
- Tự động phân tích biểu thức và trích xuất các biến logic.
- Hiển thị bản đồ K-map gốc trước khi rút gọn (2, 3 và 4 biến).
- Tìm kiếm các **Tế bào lớn (Prime Implicants - PI)** và **Tế bào lớn thiết yếu (Essential Prime Implicants - EPI)**.
- Rút gọn biểu thức và khoanh nhóm trực quan trên bản đồ K-map với các màu sắc phân biệt (hỗ trợ hiển thị nhóm cuộn vòng biên - wrapping).
- Liệt kê các trường hợp nghiệm rút gọn tối tiểu (Multiple minimal covers).
- Tự động sinh mã Graphviz `.dot` và gọi `dot.exe` để vẽ sơ đồ mạch logic gồm các cổng AND, OR, NOT dưới dạng ảnh PNG hiển thị trực tiếp trên giao diện.

---

## 🛠️ Công nghệ & Thư viện sử dụng

- **Ngôn ngữ**: C++ (tiêu chuẩn **ISO C++17** trở lên).
- **GUI Framework**: [wxWidgets](https://www.wxwidgets.org/) (phiên bản 3.x).
- **Vẽ mạch logic**: [Graphviz](https://graphviz.org/) (`dot.exe` để render đồ thị logic sang PNG).
- **IDE phát triển**: Microsoft Visual Studio 2022 (v143).

---

## 🚀 Hướng dẫn cài đặt & Biên dịch

### 1. Yêu cầu hệ thống
- Hệ điều hành: Windows 10/11 (x64).
- Visual Studio 2022 (đã cài đặt workload *Desktop development with C++*).
- Thư viện wxWidgets đã được build sẵn (khuyên dùng cấu hình `vc_x64_lib`).

### 2. Thiết lập biến môi trường wxWidgets
1. Tải và giải nén wxWidgets (ví dụ: `C:\wxWidgets-3.2.x`).
2. Thiết lập biến môi trường hệ thống:
   - Tên biến: `WXWIN`
   - Giá trị: Đường dẫn tới thư mục wxWidgets (ví dụ: `C:\wxWidgets-3.2.x`).

### 3. Cấu hình Project trong Visual Studio
1. Mở file giải pháp: `Group8_FinalProject/Group8_FinalProject.sln`.
2. Đảm bảo cấu hình là **Debug** hoặc **Release**, nền tảng **x64**.
3. Kiểm tra các thuộc tính project:
   - **C/C++ -> General -> Additional Include Directories**: `$(WXWIN)\include\msvc;$(WXWIN)\include;`
   - **C/C++ -> Language -> C++ Language Standard**: `ISO C++17 Standard (/std:c++17)`.
   - **Linker -> General -> Additional Library Directories**: `$(WXWIN)\lib\vc_x64_lib`.
   - **Linker -> System -> SubSystem**: `Windows (/SUBSYSTEM:WINDOWS)`.
4. Nhấn **Build Solution** (`Ctrl + Shift + B`).

### 4. Tài nguyên cần thiết khi chạy chương trình
Thư mục chứa file thực thi (`.exe`) cần có các thư mục con sau:
- `bin/`: Chứa file thực thi `dot.exe` của Graphviz.
- `Images/`: Chứa hình ảnh cổng logic (`AND.png`, `OR.png`, `NOT.png`), logo trường (`LOGO.png`, `tentruongSPKT.png`) và danh sách nhóm (`danhsachtv.png`).

---

## 📖 Hướng dẫn sử dụng

1. **Nhập biểu thức**: Nhập biểu thức dạng SOP vào ô *Nhập biểu thức*.
   - Ký tự phủ định sử dụng dấu trừ `-`, ví dụ: `-A B + A -B` hoặc `-(AB)`.
   - Phép cộng logic sử dụng dấu `+`.
2. **Tính toán**: Nhấn nút **Tính toán**.
   - Bảng **K-map (Ban đầu)** hiển thị các giá trị `1` và `0` tương ứng trên bản đồ.
   - Bảng **K-map (Sau khi rút gọn)** hiển thị các nhóm khoanh tế bào lớn với viền màu nổi bật.
   - Hộp chọn **Biểu thức rút gọn** hiển thị các trường hợp tối tiểu (Case 1, Case 2,...).
   - Ô **Mạch logic** hiển thị sơ đồ cổng logic tương ứng được render tự động.
3. **Chuyển đổi nghiệm**: Nếu có nhiều trường hợp rút gọn tương đương, chọn từng Case trong danh sách để xem sơ đồ mạch và khoanh nhóm tương ứng.

---

## 📂 Cấu trúc thư mục dự án

```text
DM-GT_PROJECT/
│
├── README.md                           # Tài liệu hướng dẫn dự án
├── input project Kmap.cpp               # Mã nguồn C++ chính của chương trình
│
├── Group8_FinalProject/                # Thư mục Project Visual Studio
│   ├── Group8_FinalProject.sln         # Solution file
│   ├── Group8_FinalProject.vcxproj     # Project file C++
│   ├── Group8_FinalProject.vcxproj.filters
│   └── x64/Debug/                      # Thư mục thực thi & Debug
│       ├── Group8_FinalProject.exe     # File chạy
│       ├── bin/                        # Chứa Graphviz dot.exe
│       │   └── dot.exe
│       └── Images/                     # Tài nguyên hình ảnh, cổng logic, logo
│           ├── AND.png
│           ├── OR.png
│           ├── NOT.png
│           ├── LOGO.png
│           ├── tentruongSPKT.png
│           └── danhsachtv.png
│
└── [Khuyến nghị dọn dẹp]:
    ├── circuit.png                     # (File ảnh tạm sinh ra trong lúc chạy)
    ├── có bảng Kmap                    # (Bản thảo mã nguồn cũ không có phần mở rộng)
    └── Group8_FinalProject/testlogic.cpp # (File trống 0 bytes)
```

---

## 📝 Đánh giá & Định hướng cải tiến

- [ ] Chuẩn hóa cấu trúc thư mục (chuyển mã nguồn vào `src/`, tách tài nguyên dùng chung vào `assets/`).
- [ ] Bổ sung bộ phân tích cú pháp biểu thức Boole mạnh mẽ hơn (hỗ trợ biến nhiều chữ số, kiểm tra dấu ngoặc, xử lý khoảng trắng).
- [ ] Khắc phục các điểm bất cập logic trong thuật toán bao phủ và render đồ thị mạch.
- [ ] Bổ sung tính năng tự động tìm kiếm Graphviz trên máy người dùng nếu không có sẵn file nội bộ.
