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
// Kiểm tra file tồn tại
bool kiemtrafiletontai(const string& path) {
    ifstream op(path);
    return op.good();
}

// Tách chuỗi theo ký tự
vector<string> tachchuoi(const string& s, char phancach) {
    vector<string> daycactich;
    string tich;
    for (char c : s) {
        if (c == phancach) {
            if (!tich.empty()) daycactich.push_back(tich);
            tich = "";
        }
        else tich += c;
    }
    if (!tich.empty()) daycactich.push_back(tich);
    return daycactich;
}

// Tạo file .dot cho Graphviz
bool makeDot(string expr, string dotf, string pngf) {
    ofstream f(dotf);
    if (!f.is_open()) return false;

    f << "digraph G {\n";
    f << "rankdir=LR; splines=ortho; nodesep=0.6; ranksep=0.8;\n";
    f << "node[fontsize=12, style=filled, fillcolor=lightblue];\n";

    for (char var : dsbien)
        f << var << "[shape=circle, label=\"" << var << "\", fillcolor=\"#DDA0DD\"];\n";

    int cNOT = 0, cAND = 0;
    auto daychuoi = tachchuoi(expr, '+');
    vector<string> ANDs;

    for (auto& t : daychuoi) {
        t.erase(remove_if(t.begin(), t.end(), ::isspace), t.end());
        if (t.empty()) continue;

        // thêm đếm biến 
        int sobien = 0;
        for (char c : t)
            if (isalpha(c)) sobien++;

        string target;

        if (sobien > 1) {
            cAND++;
            string aN = "A" + to_string(cAND);
            f << aN << "[shape=none,image=\"" << pngf << "\\AND.png\"];\n";
            target = aN;
        }
        else {
            target = "OR";
        }

        for (int i = 0; i < t.size(); i++) {
            char c = t[i];
            if (c == '-') continue;
            string var(1, c);

            if (i > 0 && t[i - 1] == '-') {
                cNOT++;
                string nN = "N" + to_string(cNOT);
                f << nN << "[shape=none,image=\"" << pngf << "\\NOT.png\"];\n";
                f << var << "->" << nN << ";\n";
                f << nN << "->" << target << ";\n";
            }
            else {
                f << var << "->" << target << ";\n";
            }
        }

        if (sobien > 1)
            ANDs.push_back(target);
    }

    if (ANDs.size() == 1) {
        f << "F[shape=doublecircle,label=\"F\",fillcolor=\"#FFA07A\"];\n";
        f << ANDs[0] << "->F;\n";
    }
    else {
        f << "OR[shape=none,image=\"" << pngf << "\\OR.png\"];\n";
        for (auto& x : ANDs) f << x << "->OR;\n";
        f << "F[shape=doublecircle,label=\"F\",fillcolor=\"#FFA07A\"];\n";
        f << "OR->F;\n";
    }

    f << "}\n";
    f.close();
    return true;
}

// Gọi Graphviz để render PNG
bool doGraph(const string& exe, const string& dotf, const string& pngf) {
    if (!kiemtrafiletontai(exe) || !kiemtrafiletontai(dotf)) return false;

    string cmd = "\"" + exe + "\" -Tpng \"" + dotf + "\" -o \"" + pngf + "\"";
    vector<char> buf(cmd.begin(), cmd.end());
    buf.push_back('\0');

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    BOOL ok = CreateProcessA(NULL, buf.data(), NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi);

    if (!ok) {
        cerr << "Tạo tiến trình thất bại, lỗi: " << GetLastError() << endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return kiemtrafiletontai(pngf);
}

// ===========================================================================
// Giao diện wxWidgets
class MyFrame : public wxFrame {
public:
    wxTextCtrl* o_nhap;
    wxTextCtrl* o_bieuthuc_rutgon;
    wxStaticBitmap* anh_mach;

    MyFrame() : wxFrame(NULL, wxID_ANY, "K-map Tool", wxDefaultPosition, wxSize(900, 600)) {
        Maximize(true);
        wxPanel* panel = new wxPanel(this);

        wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* line1 = new wxBoxSizer(wxHORIZONTAL);
        line1->Add(new wxStaticText(panel, -1, "Nhập biểu thức:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_nhap = new wxTextCtrl(panel, -1, "");
        line1->Add(o_nhap, 1, wxALL | wxEXPAND, 5);
        mainSizer->Add(line1, 0, wxEXPAND | wxALL, 10);

        wxButton* nut = new wxButton(panel, -1, "Tính toán");
        nut->Bind(wxEVT_BUTTON, &MyFrame::OnCalculate, this);
        mainSizer->Add(nut, 0, wxALIGN_CENTER | wxALL, 5);

        wxBoxSizer* line2 = new wxBoxSizer(wxHORIZONTAL);
        line2->Add(new wxStaticText(panel, -1, "Biểu thức rút gọn:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_bieuthuc_rutgon = new wxTextCtrl(panel, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        line2->Add(o_bieuthuc_rutgon, 1, wxALL | wxEXPAND, 5);
        mainSizer->Add(line2, 0, wxEXPAND | wxALL, 10);

        mainSizer->Add(new wxStaticText(panel, -1, "Mạch logic:"), 0, wxALL, 5);
        wxImage img(400, 300);
        img.SetRGB(wxRect(0, 0, 400, 300), 255, 255, 255);
        anh_mach = new wxStaticBitmap(panel, -1, wxBitmap(img));
        mainSizer->Add(anh_mach, 1, wxEXPAND | wxALL, 10);

        panel->SetSizer(mainSizer);
    }

    void OnCalculate(wxCommandEvent&) {
        string input = o_nhap->GetValue().ToStdString();
        xuly(input);
        auto kmap = KMap_Transformation(dsvitri);
        auto groups = KMap_Minimization(kmap);
        string ans = ChuoiBieuThuc(groups, slbien);
        o_bieuthuc_rutgon->SetValue(ans);

        string exepath = R"(E:\hk1_25-26\OOP_with_Cpp\FirstGUI\FirstGUI\bin\dot.exe)";
        string dotpath = R"(E:\hk1_25-26\OOP_with_Cpp\FirstGUI\FirstGUI\circuit.dot)";
        string pngpath = R"(E:\hk1_25-26\OOP_with_Cpp\FirstGUI\FirstGUI\circuit.png)";
        string pngfolder = R"(E:\hk1_25-26\OOP_with_Cpp\FirstGUI\FirstGUI\Images)";

        if (!makeDot(ans, dotpath, pngfolder)) {
            wxMessageBox("Không thể tạo .dot!");
            return;
        }
        if (!doGraph(exepath, dotpath, pngpath)) {
            wxMessageBox("Không render được PNG!");
            return;
        }

        Sleep(200);
        if (!fs::exists(pngpath)) {
            wxMessageBox("Không tìm thấy file PNG!");
            return;
        }

        wxImage img(pngpath);
        if (!img.IsOk()) {
            wxMessageBox("Không đọc được ảnh!");
            return;
        }

        img.Rescale(800, 500, wxIMAGE_QUALITY_HIGH);
        anh_mach->SetBitmap(wxBitmap(img));
        anh_mach->Refresh();
    }
};

// ===========================================================================
// App chính
class MyApp : public wxApp {
public:
    bool OnInit() override {
#ifdef __WXMSW__
        SetProcessDPIAware();
#endif
        wxInitAllImageHandlers();
        auto* frame = new MyFrame();
        frame->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp);