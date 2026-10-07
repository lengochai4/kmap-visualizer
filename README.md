# Boolean Minimization Visualizer & Logic Circuit Drawer (K-Map Tool)

> **Final Term Project — Discrete Mathematics and Graph Theory**  
> **Sophomore Year Project (Year 2)**  
> **Ho Chi Minh City University of Technology and Education (HCMUTE)**  
> *Faculty of International Education (FIE)*

---

## 👥 Team Members — Group 8

| No. | Full Name | Student ID | Role |
| :---: | :--- | :---: | :--- |
| 1 | **Lê Ngọc Hải** | `24110089` | Team Leader / Core Developer |
| 2 | **Lê Huy Phát** | `24110118` | Developer |
| 3 | **Huỳnh Lê Anh Tuấn** | `24110143` | Developer |

---

## 📌 Project Overview

This project provides an interactive desktop GUI application to visualize the process of **Boolean function minimization using Karnaugh Maps (K-Maps)** and to automatically generate and display the corresponding **Boolean logic circuit diagram**.

### Key Features
- **Expression Input**: Supports Sum-of-Products (SOP) Boolean expressions with negation (`-`) and logical OR (`+`).
- **Dynamic Variable Detection**: Automatically identifies variables (supports 2, 3, and 4 variables).
- **Dual K-Map Visualization**:
  - **Initial K-Map**: Displays the raw truth-value distribution (`0` and `1`).
  - **Minimization K-Map**: Highlights optimal cell groupings (Prime Implicants) with distinct semi-transparent colors and boundary wrapping (cylindrical and spherical wrap-around).
- **Multiple Minimal Cover Enumeration**: Identifies Essential Prime Implicants (EPIs) and solves for all alternate minimal cover solutions (Case 1, Case 2, etc.).
- **Automatic Circuit Drawing**: Uses **Graphviz** (`dot.exe`) under the hood to generate clean, orthographic logic circuit schematics (with AND, OR, NOT gate symbols) and renders them in real time within the application.

---

## 🛠️ Technology Stack & Dependencies

- **Language**: C++ (Compiled under **ISO C++17 Standard**).
- **GUI Framework**: [wxWidgets](https://www.wxwidgets.org/) (Version 3.2+ recommended).
- **Circuit Graph Renderer**: [Graphviz](https://graphviz.org/) (`dot.exe`).
- **IDE & Toolset**: Microsoft Visual Studio 2022 (`v143` toolset, `x64`).

---

## 🚀 Getting Started & Build Instructions

### 1. Prerequisites
- Windows 10/11 (64-bit).
- Visual Studio 2022 with the **Desktop development with C++** workload.
- Pre-built wxWidgets library for MSVC (recommended directory: `vc_x64_lib`).

### 2. Environment Setup
1. Download and extract wxWidgets (e.g., `C:\wxWidgets-3.2.x`).
2. Add a system environment variable:
   - **Variable Name**: `WXWIN`
   - **Variable Value**: Path to your wxWidgets root folder (e.g., `C:\wxWidgets-3.2.x`).

### 3. Visual Studio Project Configuration
1. Open the solution file: `Group8_FinalProject/Group8_FinalProject.sln`.
2. Set configuration to **Debug** or **Release** on platform **x64**.
3. Verify project properties:
   - **C/C++ -> General -> Additional Include Directories**:  
     `$(WXWIN)\include\msvc;$(WXWIN)\include;`
   - **C/C++ -> Language -> C++ Language Standard**:  
     `ISO C++17 Standard (/std:c++17)`
   - **Linker -> General -> Additional Library Directories**:  
     `$(WXWIN)\lib\vc_x64_lib`
   - **Linker -> System -> SubSystem**:  
     `Windows (/SUBSYSTEM:WINDOWS)`
4. Build the project (`Ctrl + Shift + B`).

### 4. Runtime Assets
When running the built executable (`Group8_FinalProject.exe`), ensure the following assets reside in the executable's directory:
- `bin/dot.exe`: Graphviz executable used to compile `.dot` scripts into PNG images.
- `Images/`: Contains logic gate icons (`AND.png`, `OR.png`, `NOT.png`), university logos (`LOGO.png`, `tentruongSPKT.png`), and member credits (`danhsachtv.png`).

---

## 📖 User Guide

1. **Input Boolean Expression**: Enter your expression into the *Nhap bieu thuc* text box.
   - Use `-` preceding a variable for negation, e.g., `-A B + A -B` or `A B C + -A -B C`.
   - Separate product terms using `+`.
2. **Calculate**: Click the **Tinh toan** button.
   - **K-map (Ban dau)**: Shows the un-grouped K-map.
   - **K-map (Sau khi rut gon)**: Shows color-coded rectangular cell loops for Prime Implicants.
   - **Bieu thuc rut gon**: Dropdown list containing minimized expressions.
   - **Mach logic**: Graphically displays the generated logic gate circuit diagram.
3. **Switch Cases**: If multiple minimal SOP expressions exist, select any case from the dropdown to view its corresponding groupings and circuit schematic.

---

## 📂 Project Directory Structure

```text
DM-GT_PROJECT/
│
├── README.md                           # Project documentation (English)
├── .gitignore                          # Visual Studio and build output ignore rules
├── input project Kmap.cpp               # Main C++ application source file
│
├── Group8_FinalProject/                # Visual Studio Project directory
│   ├── Group8_FinalProject.sln         # VS Solution
│   ├── Group8_FinalProject.vcxproj     # VCXProj configuration
│   ├── Group8_FinalProject.vcxproj.filters
│   └── x64/Debug/                      # Output build & runtime assets
│       ├── Group8_FinalProject.exe     # Executable binary
│       ├── bin/                        # Graphviz binaries (dot.exe)
│       └── Images/                     # Gate symbols, logos, team banner
│
└── [Pending Cleanup]:
    ├── circuit.png                     # Temporary runtime output image
    ├── có bảng Kmap                    # Legacy source draft (missing extension)
    └── Group8_FinalProject/testlogic.cpp # Empty placeholder file (0 bytes)
```

---

## 📄 License & Acknowledgments

Developed by **Group 8** for the *Discrete Mathematics and Graph Theory* course at **Ho Chi Minh City University of Technology and Education (HCMUTE)**.
