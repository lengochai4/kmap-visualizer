#include <wx/wx.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <set>
#include <fstream>
#include <filesystem>
#include <shellapi.h>
#include <Windows.h>
using namespace std;
namespace fs = filesystem;


//====================BIẾN ĐỔI INPUT====================
//Danh sách bit của các toán tử và số lượng biến
vector<string> dsvitri;
int slbien;
vector<char> dsbien; //Giữ thứ tự biến để in ra sau khi rút gọn

struct group {
    int rowsize, colsize;
    int startrow, startcol;
    set<pair<int, int>> cells; //Tập ô mà group này chiếm
};

//Chuyển đổi từ một toán tử sang bit nhị phân của nó
string chuyendoi(const string& toantu)
{
    string bits = "";
    for (int i = 0; i < toantu.length(); i++) {
        if (toantu[i] != '-' && toantu[i] != ' ') {
            if (i > 0) {
                if (toantu[i - 1] == '-') bits += "0";
                else if (toantu[i - 1] != '-') bits += "1";
            }
            //i==0
            else bits += "1";
        }
    }
    return bits;
}
//Kiểm tra số lượng biến của một biểu thức, tách biểu thức thành từng toán tử và xử lý nhị phân
void xuly(const string& bieuthuc) {
    slbien = 0;
    dsvitri.clear();
    vector<char> bien;
    for (char c : bieuthuc) {
        if ((find(bien.begin(), bien.end(), c) == bien.end()) && c != ' ' && c != '+' && c != '-') {
            slbien++;
            bien.push_back(c);
        }
    }

    stringstream ss(bieuthuc);
    string toantu;
    while (getline(ss, toantu, '+'))
    {
        if (!toantu.empty()) dsvitri.push_back(chuyendoi(toantu));
    }
    dsbien = bien;
}


//====================BIẾN ĐỔI SANG KMAP====================
int Index(const string& bits) {
    static vector<string> bits2 = { "00","01","11","10" };
    if (bits.size() == 1) return (bits == "1") ? 1 : 0;
    if (bits.size() == 2) {
        for (int i = 0; i < 4; i++) if (bits == bits2[i]) return i;
    }
    return -1;
}

vector<vector<bool>> KMap_Transformation(const vector<string>& dsvitri) {
    int nua = slbien / 2;
    int row = pow(2, nua);
    int col = pow(2, slbien - nua);
    vector<vector<bool>> kmap(row, vector<bool>(col, false));

    for (const string& x : dsvitri) {
        int r, c;
        if (nua == 0)  r = 0;
        else r = Index(x.substr(0, nua));

        if ((slbien - nua) == 0) c = 0;
        else c = Index(x.substr(nua));

        if (r >= 0 && c >= 0) kmap[r][c] = true;
    }
    return kmap;
}


//====================RÚT GỌN BIỂU THỨC KMAP====================
//Kiểm tra 1 nhóm có tồn tại trong mảng không
bool FindGroup(const vector<vector<bool>>& kmap, int startrow, int startcol, int rowsizecheck, int colsizecheck) {
    int row = kmap.size(),
        col = kmap[0].size();

    for (int i = 0; i < rowsizecheck; i++) {
        for (int j = 0; j < colsizecheck; j++) {
            int rowcheck = (startrow + i) % row;
            int colcheck = (startcol + j) % col;
            if (!kmap[rowcheck][colcheck]) return false;
        }
    }
    return true;
}

// Tạo tập các ô mà một group chiếm để so sánh
set<pair<int, int>> ListGroup(int startrow, int startcol, int rowsize, int colsize, int row, int col) {
    set<pair<int, int>> s;
    for (int i = 0; i < rowsize; i++) {
        for (int j = 0; j < colsize; j++) {
            int grouprow = (startrow + i) % row;
            int groupcol = (startcol + j) % col;
            s.insert({ grouprow, groupcol });
        }
    }
    return s;
}

//Kiểm tra xem group con có nằm trong group lớn nào khác trong tập không
bool Inside(const set<pair<int, int>>& big, const set<pair<int, int>>& smaller) {
    for (auto& p : smaller) if (big.find(p) == big.end()) return false;
    return true;
}

bool areagiamdan(const pair<int, int>& a, const pair<int, int>& b) {
    return a.first * a.second > b.first * b.second;
}
//Rút gọn KMap
vector<group> KMap_Minimization(vector<vector<bool>> kmap) {
    vector<group> all;
    int row = kmap.size(), col = kmap[0].size();

    vector<pair<int, int>> groupsize;
    for (int r = row; r >= 1; r /= 2) {
        for (int c = col; c >= 1; c /= 2) {
            groupsize.push_back({ r, c });
        }
    }

    //Sắp xếp theo diện tích giảm dần
    sort(groupsize.begin(), groupsize.end(), areagiamdan);

    for (pair<int, int> x : groupsize) {
        int rowsizecheck = x.first, colsizecheck = x.second;

        for (int startrow = 0; startrow < row; startrow++) {
            for (int startcol = 0; startcol < col; startcol++) {
                if (FindGroup(kmap, startrow, startcol, rowsizecheck, colsizecheck)) {
                    set<pair<int, int>> test = ListGroup(startrow, startcol, rowsizecheck, colsizecheck, row, col);
                    bool next = false;

                    auto it = all.begin();
                    while (it != all.end()) {
                        if (it->cells == test) {
                            next = true;
                            break;
                        }
                        else if (Inside(it->cells, test)) {
                            next = true;
                            break;
                        }
                        else if (Inside(test, it->cells)) {
                            it = all.erase(it);
                        }
                        else ++it;
                    }
                    //Nếu không bị skip thì thêm group mới vào danh sách
                    if (!next) {
                        group g;
                        g.rowsize = rowsizecheck;
                        g.colsize = colsizecheck;
                        g.startrow = startrow;
                        g.startcol = startcol;
                        g.cells = std::move(test);
                        all.push_back(g);

                    }
                }
            }
        }

    }
    return all;
}


//====================BIỂN ĐỔI MA TRẬN RÚT GỌN SANG BIỂU THỨC====================
vector<string> Bits2 = { "00","01","11","10" };

string IndextoBit(int index, int bitlength) {
    if (bitlength == 0) return "";
    if (bitlength == 1) return (index == 0) ? "0" : "1";
    if (bitlength == 2) return Bits2[index];
    return "";
}

string CelltoBit(int rowindex, int colindex, int numberofvariables) {
    int nua = numberofvariables / 2;
    int right = nua;
    int left = numberofvariables - nua;

    string leftside = IndextoBit(rowindex, right);
    string rightside = IndextoBit(colindex, left);
    return leftside + rightside;
}

string GrouptoToanTu(const group& g, int numofvar) {
    auto x = g.cells.begin();
    string change = CelltoBit(x->first, x->second, numofvar);
    x++;

    for (; x != g.cells.end(); x++) {
        string b = CelltoBit(x->first, x->second, numofvar);
        for (int k = 0; k < numofvar; ++k) {
            if (change[k] != 'x' && change[k] != b[k]) change[k] = 'x';
        }
    }

    string toantu;
    for (int k = 0; k < numofvar; ++k) {
        if (change[k] == '0') { toantu += "-"; toantu.push_back(dsbien[k]); }
        else if (change[k] == '1') { toantu.push_back(dsbien[k]); }
    }
    if (toantu.empty()) toantu = "1";
    return toantu;
}

string ChuoiBieuThuc(const vector<group>& groups, int numofvar) {
    set<string> unique;

    for (const auto& g : groups) unique.insert(GrouptoToanTu(g, numofvar));
    string output;

    for (auto x = unique.begin(); x != unique.end(); x++) {
        if (x != unique.begin()) output += " + ";
        output += *x;
    }
    return output.empty() ? "0" : output;
}



//====================RENDER SANG HÌNH ẢNH====================
// kiểm tra coi việc mở file có lỗi không 
bool kiemtrafiletontai(const string& path) {
    ifstream mofile(path);
    return mofile.good();
}

// Tách từ tích các tổng ra thành các cụm tích nhỏ để xử lý 
vector<string> tachchuoi(const string& s, char phancach) {
    vector<string> daycacchuoicon;
    string chuoicon;
    for (char c : s) {
        if (c == phancach) {
            if (!chuoicon.empty()) daycacchuoicon.push_back(chuoicon);
            chuoicon = "";
        }
        else chuoicon += c;
    }
    if (!chuoicon.empty()) daycacchuoicon.push_back(chuoicon);
    return daycacchuoicon;
}

// CHỈNH SỬA: Đã cập nhật hàm makeDot để cấu trúc sơ đồ mạch rõ ràng hơn (AND-OR 2 tầng)
bool makeDot(string expr, string dotF, string imgF) {
    ofstream f(dotF);
    if (!f.is_open()) { cerr << "không thể mở\n"; return 0; }    //kiểm tra xem file có mở được không 

    // Đặt hướng đồ thị (trái sang phải), cỡ chữ và định nghĩa nút
    f << "digraph G{\nrankdir=LR;\nnode[fontsize=12, style=filled, fillcolor=lightblue];\n";

    // 1. ĐỊNH NGHĨA TẤT CẢ CÁC BIẾN ĐẦU VÀO MỘT LẦN (để Graphviz sắp xếp gọn gàng hơn)
    for (char var : dsbien) {
        f << " " << var << "[shape=circle, label=\"" << var << "\", fillcolor=\"#DDA0DD\"];\n"; // Input style
    }

    int cNOT = 0, cAND = 0;         //Đếm để đặt tên các biến nút khác nhau 
    auto chuoi = tachchuoi(expr, '+');
    vector<string> ANDs;

    // 2. XỬ LÝ TỪNG TOÁN TỬ TÍCH (Product Term)
    for (auto& t : chuoi) {
        t.erase(remove_if(t.begin(), t.end(), ::isspace), t.end());
        if (t.empty()) continue;

        cAND++;
        string aN = "A" + to_string(cAND);
        // Định nghĩa cổng AND
        f << " " << aN << "[shape=none,image=\"" << imgF << "\\AND.png\"];\n";

        // 2b. Kết nối các literal (biến hoặc biến đảo) vào cổng AND
        for (size_t i = 0; i < t.size(); i++) {
            char c = t[i];
            if (c == '-') continue;
            string var(1, c); // Tên biến gốc (ví dụ: "A")

            if (i > 0 && t[i - 1] == '-') {
                // Biến đảo (Complemented, ví dụ: -A)
                cNOT++;
                string nN = "N" + to_string(cNOT);

                // Định nghĩa cổng NOT (nút trung gian)
                f << " " << nN << "[shape=none,image=\"" << imgF << "\\NOT.png\"];\n";

                // Kết nối Biến -> NOT
                f << var << "->" << nN << " [label=\"\"];\n";

                // Kết nối NOT -> AND
                f << nN << "->" << aN << " [label=\"\"];\n";
            }
            else {
                // Biến không đảo (ví dụ: A)
                // Kết nối Biến -> AND trực tiếp
                f << var << "->" << aN << " [label=\"\"];\n";
            }
        }
        ANDs.push_back(aN);
    }

    // 3. ĐỊNH NGHĨA VÀ KẾT NỐI CỔNG OR CUỐI CÙNG
    // Sử dụng màu sắc khác cho cổng OR và Output
    f << " OR[shape=none,image=\"" << imgF << "\\OR.png\"];\n";
    for (auto& x : ANDs) {
        f << x << "->OR [label=\"\"];\n"; // Kết nối đầu ra của tất cả các cổng AND vào cổng OR
    }

    // 4. ĐỊNH NGHĨA VÀ KẾT NỐI ĐẦU RA F
    f << " F[shape=doublecircle,label=\"F\", fillcolor=\"#FFA07A\"];\nOR->F [label=\"\"];\n}\n";
    f.close();

    cout << "hoàn thành nút: " << dotF << "\n";
    return 1;
}



// ========================= RENDER RA MẠCH (PNG) ===================================
bool doGraph(const string& exe, const string& dotF, const string& pngF)
{
    if (!kiemtrafiletontai(exe) || !kiemtrafiletontai(dotF)) return false;

    string cmd = "\"" + exe + "\" -Tpng \"" + dotF + "\" -o \"" + pngF + "\"";

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE; // ẩn cửa sổ

    BOOL ok = CreateProcessA(
        NULL,        //chạy file exe ở đây luôn khỏi tìm 
        cmd.data(),  // nơi chứa địa chỉ file exe 
        NULL,        // thuộc tính bảo mật
        NULL,        // thuộc tính bảo mật
        FALSE,       // khởi chạy các file hay tài nguyên khác trong main vì chỉ cần chạy tool trong dot.exe nên không cần tích hợp để false (tắt)
        0,           // không cần đặt cờ để đưa ra cửa sổ 
        NULL,        // không cần gọi biến môi trường 
        NULL,
        &si, &pi
    );

    if (!ok) return false;

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return kiemtrafiletontai(pngF);
}


//====================CODE GIAO DIỆN WINDOW (SỬ DỤNG WXWIDGETS)====================
class MyFrame : public wxFrame
{
public:
    wxTextCtrl* o_nhap;                // ô nhập biểu thức
    wxTextCtrl* o_bieuthuc_rutgon;     // ô hiển thị biểu thức rút gọn
    wxStaticBitmap* anh_mach;          // vùng hiển thị mạch logic (ảnh)

    MyFrame()
        : wxFrame(NULL, wxID_ANY, "K-map Tool", wxDefaultPosition, wxSize(800, 600))
    {
        Maximize(true);
        wxPanel* bang = new wxPanel(this, -1);

        // Tạo sizer chính theo chiều dọc
        wxBoxSizer* sapdoc = new wxBoxSizer(wxVERTICAL);

        // Hàng nhập biểu thức
        wxBoxSizer* sapngang1 = new wxBoxSizer(wxHORIZONTAL);
        sapngang1->Add(new wxStaticText(bang, -1, "Nhap bieu thuc:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_nhap = new wxTextCtrl(bang, -1, "");
        sapngang1->Add(o_nhap, 1, wxALL | wxEXPAND, 5);
        sapdoc->AddSpacer(20);
        sapdoc->Add(sapngang1, 0, wxEXPAND | wxALL, 10);

        // Nút tính toán
        wxBoxSizer* sapngang2 = new wxBoxSizer(wxHORIZONTAL);
        wxButton* nut_tinh = new wxButton(bang, -1, "Tinh toan");
        nut_tinh->Bind(wxEVT_BUTTON, &MyFrame::OnCalculate, this);
        sapngang2->AddStretchSpacer(1);
        sapngang2->Add(nut_tinh, 0, wxALL, 5);
        sapngang2->AddStretchSpacer(1);
        sapdoc->Add(sapngang2, 0, wxEXPAND | wxALL, 10);

        // Biểu thức rút gọn
        wxBoxSizer* sapngang3 = new wxBoxSizer(wxHORIZONTAL);
        sapngang3->Add(new wxStaticText(bang, -1, "Bieu thuc rut gon:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_bieuthuc_rutgon = new wxTextCtrl(bang, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        sapngang3->Add(o_bieuthuc_rutgon, 1, wxALL | wxEXPAND, 5);
        sapdoc->Add(sapngang3, 0, wxEXPAND | wxALL, 10);

        // Mạch logic
        sapdoc->Add(new wxStaticText(bang, -1, "Mach logic:"), 0, wxALL, 15);
        wxImage img(200, 150);
        img.SetRGB(wxRect(0, 0, 200, 150), 255, 255, 255);
        anh_mach = new wxStaticBitmap(bang, -1, wxBitmap(img));
        sapdoc->Add(anh_mach, 1, wxEXPAND | wxALL, 10);

        bang->SetSizer(sapdoc);
    }

    void OnCalculate(wxCommandEvent& event)
    {
        string input = o_nhap->GetValue().ToStdString();
        xuly(input);
        auto kmap = KMap_Transformation(dsvitri);
        auto groups = KMap_Minimization(kmap);
        string ans = ChuoiBieuThuc(groups, slbien);
        o_bieuthuc_rutgon->SetValue(ans);

        // ===================== RENDER TRỰC TIẾP VỚI PNG =====================
        wxImage imgAND(R"(E:\hk1_25-26\OOP_with_Cpp\FirstGUI\FirstGUI\Images\AND.png)");
        wxImage imgOR(R"(E:\hk1_25-26\OOP_with_Cpp\FirstGUI\FirstGUI\Images\OR.png)");
        wxImage imgNOT(R"(E:\hk1_25-26\OOP_with_Cpp\FirstGUI\FirstGUI\Images\NOT.png)");

        if (!imgAND.IsOk() || !imgOR.IsOk() || !imgNOT.IsOk()) {
            wxMessageBox("Không load được file PNG cổng!");
            return;
        }

        int w = 1000, h = 600; // Tăng kích thước canvas
        wxBitmap bmp(w, h);
        wxMemoryDC dc(bmp);
        dc.SetBackground(*wxWHITE_BRUSH);
        dc.Clear();

        auto terms = tachchuoi(ans, '+');

        // Xóa các phần tử rỗng
        terms.erase(std::remove_if(terms.begin(), terms.end(),
            [](const string& s) { return s.empty(); }),
            terms.end());

        if (terms.empty()) {
            dc.DrawText("No terms to display", 50, 50);
            anh_mach->SetBitmap(bmp);
            anh_mach->Refresh();
            return;
        }

        vector<wxPoint> andOutputs;
        int startX = 50;
        int startY = 50;
        int maxTermHeight = 0;

        // Tính toán layout trước
        vector<int> termHeights;
        for (auto& term : terms) {
            int termInputs = 0;
            for (size_t i = 0; i < term.size(); i++) {
                if (term[i] != '-') termInputs++;
            }
            int termHeight = std::max(imgAND.GetHeight(), termInputs * 20 + 10);
            termHeights.push_back(termHeight);
            maxTermHeight = std::max(maxTermHeight, termHeight);
        }

        // Vẽ các cổng AND
        for (size_t idx = 0; idx < terms.size(); idx++) {
            auto& term = terms[idx];
            term.erase(remove_if(term.begin(), term.end(), ::isspace), term.end());
            if (term.empty()) continue;

            int andX = startX;
            int andY = startY + (idx * (maxTermHeight + 40)); // Khoảng cách đều giữa các AND

            wxBitmap bmpAND(imgAND);
            dc.DrawBitmap(bmpAND, andX, andY, true);

            // Vẽ input lines và labels
            vector<int> inputYs;
            int termInputs = 0;

            // Đếm số input thực tế (bỏ qua dấu '-')
            for (size_t i = 0; i < term.size(); i++) {
                if (term[i] != '-') {
                    termInputs++;
                    inputYs.push_back(andY + 10 + (termInputs - 1) * 20);
                }
            }

            // Vẽ đường input và labels
            int inputIndex = 0;
            for (size_t i = 0; i < term.size(); i++) {
                char c = term[i];
                if (c == '-') continue;

                int inputY = inputYs[inputIndex];

                // Vẽ đường input
                dc.DrawLine(andX - 40, inputY, andX, inputY);

                // Vẽ label biến
                string label = "";
                if (i > 0 && term[i - 1] == '-') {
                    label = "-" + string(1, c);
                }
                else {
                    label = string(1, c);
                }
                dc.DrawText(label, andX - 60, inputY - 8);

                // Vẽ cổng NOT nếu cần
                if (i > 0 && term[i - 1] == '-') {
                    int notX = andX - 35;
                    int notY = inputY - imgNOT.GetHeight() / 2;
                    wxBitmap bmpNOT(imgNOT);
                    dc.DrawBitmap(bmpNOT, notX, notY, true);

                    // Điều chỉnh đường kết nối NOT->AND
                    dc.DrawLine(notX + imgNOT.GetWidth(), inputY, andX, inputY);
                    dc.DrawLine(notX, inputY, notX + imgNOT.GetWidth() / 2, inputY);
                }

                inputIndex++;
            }

            // Lưu vị trí output của AND
            andOutputs.push_back(wxPoint(andX + imgAND.GetWidth(), andY + imgAND.GetHeight() / 2));
        }

        // Vẽ cổng OR - đặt ở giữa chiều dọc
        int orX = startX + 300;
        int orY = startY + (terms.size() * (maxTermHeight + 40)) / 2 - imgOR.GetHeight() / 2;
        wxBitmap bmpOR(imgOR);
        dc.DrawBitmap(bmpOR, orX, orY, true);

        // Nối các AND -> OR
        int orInputY = orY + imgOR.GetHeight() / 2;
        for (auto& p : andOutputs) {
            // Vẽ đường cong hoặc đường thẳng có góc
            int midX1 = p.x + 50;
            int midX2 = orX - 50;

            dc.DrawLine(p.x, p.y, midX1, p.y); // Ngang từ AND
            dc.DrawLine(midX1, p.y, midX2, orInputY); // Chéo đến OR
            dc.DrawLine(midX2, orInputY, orX, orInputY); // Ngang vào OR
        }

        // Vẽ output F
        int outX = orX + imgOR.GetWidth() + 30;
        int outY = orInputY;

        // Vẽ đường output
        dc.DrawLine(orX + imgOR.GetWidth(), outY, outX, outY);

        // Vẽ vòng tròn output
        dc.SetBrush(*wxRED_BRUSH);
        dc.SetPen(wxPen(*wxBLACK, 2));
        dc.DrawCircle(outX + 10, outY, 10);

        // Vẽ label F
        dc.SetTextForeground(*wxBLACK);
        dc.DrawText("F", outX + 5, outY - 7);

        dc.SelectObject(wxNullBitmap);
        anh_mach->SetBitmap(bmp);
        anh_mach->Refresh();
    }
};


// App chính
class MyApp : public wxApp
{
public:
    virtual bool OnInit() {
#ifdef __WXMSW__
        SetProcessDPIAware();
#endif
        wxInitAllImageHandlers(); // BẮT BUỘC nếu dùng PNG
        MyFrame* khung = new MyFrame();
        khung->Show(true);
        return true;
    }
};


wxIMPLEMENT_APP(MyApp); //chứa hàm main => ko cần viết hàm main
