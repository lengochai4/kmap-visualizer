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


//Danh sách bit của các toán tử và số lượng biến
vector<string> dsvitri;
int slbien;
struct group {
    int rowsize, colsize;
    int startrow, startcol;
    set<pair<int, int>> cells;  //Các nhóm bé hơn trùng với nhóm này
};
vector<char> dsbien;            //Giữ thứ tự biến để in ra sau khi rút gọn


//====================BIẾN ĐỔI INPUT====================
//Chuyển đổi toán tử sang bit nhị phân
string chuyendoitbit(const string& toantu)
{
    string bits = "";
    for (int i = 0; i < toantu.length(); i++) {
        if (isalpha(toantu[i])) {
            if (i > 0) {
                if (toantu[i - 1] == '-') bits += "0";
                else bits += "1";
            }
            else bits += "1";
        }
    }
    return bits;
}
//Tách biểu thức thành các toán tử & chuyển đổi bit
void xulybieuthuc(const string& bieuthuc) {
    slbien = 0;
    dsvitri.clear();
    vector<char> bien;
    for (char c : bieuthuc) {
        if (isalpha(c) && find(bien.begin(), bien.end(), c) == bien.end()) {
            slbien++;
            bien.push_back(c);
        }
    }

    stringstream ss(bieuthuc);
    string toantu;
    while (getline(ss, toantu, '+'))
    {
        if (!toantu.empty()) dsvitri.push_back(chuyendoitbit(toantu));
    }
    dsbien = bien;
}


//====================BIẾN ĐỔI SANG KMAP====================
//Đưa bit thành vị trí trong vector 2 chiều
int bitsangvector(const string& bits) {
    vector<string> bien2 = { "00","01","11","10" };

    if (bits.size() == 1) 
        return (bits == "1") ? 1 : 0;

    if (bits.size() == 2)
        for (int i = 0; i < bien2.size(); i++)
            if (bits == bien2[i]) return i;

    return -1;
}

//Đưa tất cả bit toán tử vào K-Map
vector<vector<bool>> biendoiKmap(const vector<string>& dsvitri) {
    int chiadoi = slbien / 2;
    int vectorrow = pow(2, chiadoi);
    int vectorcol = pow(2, slbien - chiadoi);
    vector<vector<bool>> kmap(vectorrow, vector<bool>(vectorcol, false));

    for (const string& x : dsvitri) {
        int lefttorow, righttocol;
        if (chiadoi == 0)  lefttorow = 0;
        else lefttorow = bitsangvector(x.substr(0, chiadoi));

        if ((slbien - chiadoi) == 0) righttocol = 0;
        else righttocol = bitsangvector(x.substr(chiadoi));

        if (lefttorow >= 0 && righttocol >= 0) kmap[lefttorow][righttocol] = true;
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
        cerr << "Tao tien trình that bai, loi: " << GetLastError() << endl;
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
        line1->Add(new wxStaticText(panel, -1, "Nhap bieu thuc:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_nhap = new wxTextCtrl(panel, -1, "");
        line1->Add(o_nhap, 1, wxALL | wxEXPAND, 5);
        mainSizer->Add(line1, 0, wxEXPAND | wxALL, 10);

        wxButton* nut = new wxButton(panel, -1, "Tinh toan");
        nut->Bind(wxEVT_BUTTON, &MyFrame::OnCalculate, this);
        mainSizer->Add(nut, 0, wxALIGN_CENTER | wxALL, 5);

        wxBoxSizer* line2 = new wxBoxSizer(wxHORIZONTAL);
        line2->Add(new wxStaticText(panel, -1, "Bieu thuc rut gon:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_bieuthuc_rutgon = new wxTextCtrl(panel, -1, "", wxDefaultPosition, wxDefaultSize, wxTE_READONLY);
        line2->Add(o_bieuthuc_rutgon, 1, wxALL | wxEXPAND, 5);
        mainSizer->Add(line2, 0, wxEXPAND | wxALL, 10);

        mainSizer->Add(new wxStaticText(panel, -1, "Mach logic:"), 0, wxALL, 5);
        wxImage img(400, 300);
        img.SetRGB(wxRect(0, 0, 400, 300), 255, 255, 255);
        anh_mach = new wxStaticBitmap(panel, -1, wxBitmap(img));
        mainSizer->Add(anh_mach, 1, wxEXPAND | wxALL, 10);

        panel->SetSizer(mainSizer);
    }

    void OnCalculate(wxCommandEvent&) {
        string input = o_nhap->GetValue().ToStdString();
        xulybieuthuc(input);
        auto kmap = biendoiKmap(dsvitri);
        auto groups = KMap_Minimization(kmap);
        string ans = ChuoiBieuThuc(groups, slbien);
        o_bieuthuc_rutgon->SetValue(ans);

        string exepath = R"(C:\Users\LeFat\OneDrive\Desktop\DM-GT_PROJECT_GROUP_08\bin\dot.exe)";
        string dotpath = R"(C:\Users\LeFat\OneDrive\Desktop\DM-GT_PROJECT_GROUP_08\circuit.dot)";
        string pngpath = R"(C:\Users\LeFat\OneDrive\Desktop\DM-GT_PROJECT_GROUP_08\circuit.png)";
        string pngfolder = R"(C:\Users\LeFat\OneDrive\Desktop\DM-GT_PROJECT_GROUP_08\Images)";

        if (!makeDot(ans, dotpath, pngfolder)) {
            wxMessageBox("Khong the tao .dot!");
            return;
        }
        if (!doGraph(exepath, dotpath, pngpath)) {
            wxMessageBox("Khong render duoc PNG!");
            return;
        }

        Sleep(200);
        if (!fs::exists(pngpath)) {
            wxMessageBox("Khong tim thay file PNG!");
            return;
        }

        wxImage img(pngpath);
        if (!img.IsOk()) {
            wxMessageBox("Khong doc duoc anh!");
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