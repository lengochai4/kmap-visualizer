#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <wx/filename.h>

#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iostream>
#include <math.h>
#include <set>
#include <climits>

#include <fstream>
#include <filesystem>
#include <shellapi.h>
#include <Windows.h>
using namespace std;
namespace fs = filesystem;

/*
SETUP PROJECT TRƯỚC KHI CHẠY:
- Cài đặt wxWidgets và chỉnh sửa Properties của project (Path):
    | https://www.youtube.com/watch?v=ONYW3hBbk-8&list=PLJOV-tVIwUCL3NnoNg9xwLmxgFjn4co7y&index=1

- Cần ISO C++ 17 Standard (Properties - C/C++ - Language - C++ Language Standard).
*/


//BIẾN ĐỔI INPUT
vector<string> dsvitri;
int slbien;
vector<char> dsbien; //Giữ thứ tự biến để in ra sau khi rút gọn

struct group {
    int rowsize, colsize;
    int startrow, startcol;
    set<pair<int, int>> cells; //Các nhóm bé hơn nằm trong nhóm này
};

//Chuyển một toán tử sang bit nhị phân
string chuyendoi(const string& toantu)
{
    string bits = "";
    bool phudinhchung = false;
    for (int i = 0; i < toantu.length(); i++) {
        if (toantu[i] == '(' && toantu[i - 1] == '-')
            phudinhchung = true;

        if (toantu[i] == ')')
            phudinhchung = false;

        if (isalpha(toantu[i])) {
            if (phudinhchung) bits += "0";
            else {
                if (i > 0) {
                    if (toantu[i - 1] == '-') bits += "0";
                    else bits += "1";
                }
                //i==0
                else bits += "1";
            }
        }
    }
    return bits;
}
//Tách chuỗi biểu thức thành từng toán tử và chuyển đổi bit, lưu vào dsvitri
void xuly(const string& bieuthuc) {
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
        if (!toantu.empty()) dsvitri.push_back(chuyendoi(toantu));
    }
    dsbien = bien;
}

//BIẾN ĐỔI SANG KMAP
//Chuyển từ bit sang vị trí trong mảng 2 chiều
int vitri(const string& bit) {
    static vector<string> bits2 = { "00","01","11","10" };
    if (bit.size() == 1)
        return (bit == "1") ? 1 : 0;
    if (bit.size() == 2) {
        for (int i = 0; i < bits2.size(); i++)
            if (bit == bits2[i]) return i;
    }
    return -1;
}

//Biến đổi tất cả toán tử sang vị trí trong mảng 2 chiều
vector<vector<bool>> biendoi_kmap(const vector<string>& dsvitri) {
    int chiadoi = slbien / 2;
    int row = pow(2, chiadoi);
    int col = pow(2, slbien - chiadoi);
    vector<vector<bool>> kmap(row, vector<bool>(col, false));

    for (const string& x : dsvitri) {
        int r, c;
        if (chiadoi == 0)  r = 0;
        else r = vitri(x.substr(0, chiadoi));

        if ((slbien - chiadoi) == 0) c = 0;
        else c = vitri(x.substr(chiadoi));

        if (r >= 0 && c >= 0) kmap[r][c] = true;
    }
    return kmap;
}


//RÚT GỌN BIỂU THỨC KMAP
//Kiểm tra 1 nhóm có tồn tại trong mảng không
bool timnhom(const vector<vector<bool>>& kmap, int startrow, int startcol, int rowsizecheck, int colsizecheck) {
    int row = kmap.size(), col = kmap[0].size();

    for (int i = 0; i < rowsizecheck; i++) {
        for (int j = 0; j < colsizecheck; j++) {
            int rowcheck = (startrow + i) % row;
            int colcheck = (startcol + j) % col;
            if (!kmap[rowcheck][colcheck]) return false;
        }
    }
    return true;
}

// Tạo tập hợp các ô mà một nhóm chiếm để so sánh
set<pair<int, int>> o_nhom(int startrow, int startcol, int rowsize, int colsize, int row, int col) {
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

//Kiểm tra xem nhóm hiện tại có nằm trong nhóm lớn nào khác hay không
bool bentrong(const set<pair<int, int>>& big, const set<pair<int, int>>& smaller) {
    for (auto& p : smaller) if (big.find(p) == big.end()) return false;
    return true;
}


bool areagiamdan(const pair<int, int>& a, const pair<int, int>& b) {
    return a.first * a.second > b.first * b.second;
}
//Rút gọn KMap
vector<group> rutgon_kmap(vector<vector<bool>> kmap) {
    vector<group> all;
    int row = kmap.size(), col = kmap[0].size();

    //Tạo kích thước các nhóm theo 2^n
    vector<pair<int, int>> groupsize;
    for (int r = row; r >= 1; r /= 2) {
        for (int c = col; c >= 1; c /= 2) {
            groupsize.push_back({ r, c });
        }
    }
    sort(groupsize.begin(), groupsize.end(), areagiamdan);


    for (pair<int, int> x : groupsize) {
        int rowsizecheck = x.first, colsizecheck = x.second;

        for (int startrow = 0; startrow < row; startrow++) {
            for (int startcol = 0; startcol < col; startcol++) {
                if (timnhom(kmap, startrow, startcol, rowsizecheck, colsizecheck)) {
                    set<pair<int, int>> test = o_nhom(startrow, startcol, rowsizecheck, colsizecheck, row, col);
                    bool next = false;

                    auto it = all.begin();
                    while (it != all.end()) {
                        if (it->cells == test) {
                            next = true;
                            break;
                        }
                        else if (bentrong(it->cells, test)) {
                            next = true;
                            break;
                        }
                        else if (bentrong(test, it->cells)) {
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

//BIỂN ĐỔI MA TRẬN RÚT GỌN SANG BIỂU THỨC
vector<string> Bits2 = { "00","01","11","10" };

string vitrisangbit(int index, int bitlength) {
    if (bitlength == 0) return "";
    if (bitlength == 1) return (index == 0) ? "0" : "1";
    if (bitlength == 2) return Bits2[index];
    return "";
}

string nhomsangbit(int rowindex, int colindex, int numberofvariables) {
    int nua = numberofvariables / 2;
    int right = nua;
    int left = numberofvariables - nua;

    string leftside = vitrisangbit(rowindex, right);
    string rightside = vitrisangbit(colindex, left);
    return leftside + rightside;
}

string GrouptoToanTu(const group& g, int numofvar) {
    auto x = g.cells.begin();
    string change = nhomsangbit(x->first, x->second, numofvar);
    x++;

    for (; x != g.cells.end(); x++) {
        string b = nhomsangbit(x->first, x->second, numofvar);
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

string chuoibieuthuc(const vector<group>& groups, int numofvar) {
    set<string> unique;

    for (const auto& g : groups) unique.insert(GrouptoToanTu(g, numofvar));
    string output;

    for (auto x = unique.begin(); x != unique.end(); x++) {
        if (x != unique.begin()) output += " + ";
        output += *x;
    }
    return output.empty() ? "0" : output;
}

//LIỆT KÊ CÁC BIỂU THỨC RÚT GỌN
struct PIbaoquat {
    vector<group> dachon;
    string bieuthuc;
};

//Liệt kê tất cả ô 1 trong kmap
static vector<pair<int, int>> lietke_o1(const vector<vector<bool>>& kmap) {
    vector<pair<int, int>> cells;
    for (int r = 0; r < (int)kmap.size(); ++r)
        for (int c = 0; c < (int)kmap[0].size(); ++c)
            if (kmap[r][c]) cells.push_back({ r,c });
    return cells;
}

//Tạo map ô theo vị trí các PI
static vector<vector<int>> tablebaophu(const vector<pair<int, int>>& ones, const vector<group>& PIs)
{
    vector<vector<int>> cover(ones.size());
    for (int i = 0; i < (int)ones.size(); ++i) {
        for (int j = 0; j < (int)PIs.size(); ++j) {
            if (PIs[j].cells.count(ones[i])) cover[i].push_back(j);
        }
    }
    return cover;
}

struct PI_Chiacat {
    vector<int> vitri_epi;          //Chỉ số PI là EPI
    vector<int> vitri_pi_conlai;   //Chỉ số PI còn lại
    vector<int> vitri_o1_chuaphu; //Chỉ số ô 1 chưa phủ sau khi lấy EPI
};

static PI_Chiacat Cat_EPI(const vector<group>& PIs, const vector<pair<int, int>>& ones, const vector<vector<int>>& cover)
{
    int m = ones.size();
    vector<int> soluongbaophu(m);
    for (int i = 0; i < m; ++i) soluongbaophu[i] = (int)cover[i].size();

    vector<char> dabaophu(m, false);
    vector<int> essentialpi;

    //EPI: ô có soluongbaophu = 1
    for (int i = 0; i < m; ++i) {
        if (soluongbaophu[i] == 1) {
            int epi = cover[i][0];
            if (find(essentialpi.begin(), essentialpi.end(), epi) == essentialpi.end())
                essentialpi.push_back(epi);
        }
    }

    for (int e : essentialpi) {
        for (int i = 0; i < m; ++i) {
            if (!dabaophu[i] && find(cover[i].begin(), cover[i].end(), e) != cover[i].end())
                dabaophu[i] = true;
        }
    }

    vector<int> conlai;
    for (int i = 0; i < m; ++i) if (!dabaophu[i]) conlai.push_back(i);

    vector<int> piconlai;
    for (int j = 0; j < (int)PIs.size(); ++j) {
        if (find(essentialpi.begin(), essentialpi.end(), j) == essentialpi.end())
            piconlai.push_back(j);
    }

    return { essentialpi, piconlai, conlai };
}

// Tính một nghiệm, lấy ít PI nhất
static pair<int, int> soluongtoantu(const vector<int>& vitridachon, const vector<group>& PIs, int soluongbien)
{
    auto dembien = [&](const group& g)->int {

        string term = GrouptoToanTu(g, soluongbien);
        int dem = 0;
        for (char ch : term) {
            if (isalpha((unsigned char)ch)) dem++;
        }
        return dem;
        };
    int k = vitridachon.size();
    int lits = 0;
    for (int vitri : vitridachon) {
        lits += dembien(PIs[vitri]);
    }
    return { k, lits };
}

// Đếm bit 1
static int dembit1(uint32_t x) {
    int c = 0;
    while (x) {
        x &= (x - 1); ++c;
    }
    return c;
}

static vector<vector<int>> timbaophumin(const vector<int>& vitrio1_chuaphu, const vector<int>& vitripiconlai, const vector<vector<int>>& baophu, const vector<group>& PIs, int soluongbien)
{
    vector<vector<int>> solutions;
    pair<int, int> totnhat = { INT_MAX, INT_MAX };

    int R = vitripiconlai.size();

    uint32_t total = (R >= 31) ? 0u : (1u << R);
    for (uint32_t mask = 0; mask < total; ++mask) {
        int cnt = dembit1(mask);
        if (cnt > totnhat.first) continue;

        bool ok = true;
        for (int oi : vitripiconlai) {
            bool covered = false;
            for (int b = 0; b < R; ++b)
                if (mask & (1u << b)) {
                    int piIdx = vitripiconlai[b];
                    if (find(baophu[oi].begin(), baophu[oi].end(), piIdx) != baophu[oi].end()) {
                        covered = true; break;
                    }
                }
            if (!covered) {
                ok = false; break;
            }
        }
        if (!ok) continue;

        vector<int> choose;
        for (int b = 0; b < R; ++b) if (mask & (1u << b))
            choose.push_back(vitripiconlai[b]);

        auto cst = soluongtoantu(choose, PIs, soluongbien);
        if (cst < totnhat) {
            totnhat = cst;
            solutions.clear();
            solutions.push_back(choose);
        }
        else if (cst == totnhat) {
            solutions.push_back(choose);
        }
    }
    return solutions;
}


static vector<PIbaoquat> buildtatcadapan(const vector<vector<bool>>& kmap, const vector<group>& PIs, int soluongbien)
{
    vector<PIbaoquat> out;

    auto ones = lietke_o1(kmap);
    if (ones.empty()) {
        out.push_back({ {}, "0" });
        return out;
    }

    auto cover = tablebaophu(ones, PIs);
    auto sp = Cat_EPI(PIs, ones, cover);

    //Tất cả được EPI phủ, trả về chính nó
    if (sp.vitri_o1_chuaphu.empty()) {
        vector<group> chosen;
        for (int e : sp.vitri_epi) chosen.push_back(PIs[e]);
        string expr = chuoibieuthuc(chosen, soluongbien);
        out.push_back({ chosen, expr });
        return out;
    }
    auto themnhom = timbaophumin(sp.vitri_o1_chuaphu, sp.vitri_pi_conlai, cover, PIs, soluongbien);

    //Lắp thành nghiệm đầy đủ 
    for (auto& choose : themnhom) {
        vector<group> chosen;
        for (int e : sp.vitri_epi) chosen.push_back(PIs[e]);
        for (int x : choose) chosen.push_back(PIs[x]);

        string expr = chuoibieuthuc(chosen, soluongbien);
        out.push_back({ chosen, expr });
    }
    return out;
}

//RENDER SANG HÌNH ẢNH
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

// Tạo file dot cho graphviz
bool makeDot(string expr, string dotf, string pngf) {
    ofstream f(dotf);
    if (!f.is_open()) return false;

    f << "digraph G {\n";
    f << "rankdir=LR; splines=ortho; nodesep=0.6; ranksep=0.8;\n";
    f << "node[fontsize=10, style=filled, fillcolor=white];\n";

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
            f << aN << "[shape=none,image=\"" << pngf << "/AND.png\","
                " label=\"\", imagescale=true, fixedsize=true, width=0.5, height=0.5, margin=0];\n";
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
                f << nN << "[shape=none,image=\"" << pngf << "/NOT.png\","
                    "label = \"\", imagescale=true, fixedsize=true, width=0.6, height=0.6, margin=0];\n";
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
        f << "OR[shape=none,image=\"" << pngf << "/OR.png\","
            "label = \"\", imagescale=true, fixedsize=true, width=0.5, height=0.5, margin=0];\n";
        for (auto& x : ANDs) f << x << "->OR;\n";
        f << "F[shape=doublecircle,label=\"F\",fillcolor=\"#FFA07A\"];\n";
        f << "OR->F;\n";
    }

    f << "}\n";
    f.close();
    return true;
}

// Gọi graphviz để render PNG
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
        cerr << "Tao tien trinh that bai,loi! " << GetLastError() << endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return kiemtrafiletontai(pngf);
}

// Bảng kmap hiển thị
class KmapPanel : public wxPanel {
public:
    vector<vector<bool>> kmap;
    vector<group> groups;
    int slbien;
    vector<char> variables;

    KmapPanel(wxWindow* parent)
        : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(400, 400))
    {
        Bind(wxEVT_PAINT, &KmapPanel::OnPaint, this);
    }

    void SetData(const vector<vector<bool>>& data, const vector<group>& g, int nVar, const vector<char>& vari) {
        kmap = data;
        groups = g;
        slbien = nVar;
        variables = vari;
        Refresh();
    }

    void OnPaint(wxPaintEvent&) {
        if (kmap.empty()) return;
        wxPaintDC dc(this);
        dc.Clear();

        int rows = kmap.size();
        int cols = kmap[0].size();
        int cellSize = 80;

        int startX = 50;
        int startY = 50;

        wxFont font(12, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_BOLD);
        dc.SetFont(font);

        // Nhãn hàng,cột theo số biến
        vector<string> rowLabels, colLabels;
        if (slbien == 2) {
            rowLabels = { "0", "1" };
            colLabels = { "0", "1" };
        }
        else if (slbien == 3) {
            rowLabels = { "0", "1" };
            colLabels = { "00", "01", "11", "10" };
        }
        else if (slbien == 4) {
            rowLabels = { "00", "01", "11", "10" };
            colLabels = { "00", "01", "11", "10" };
        }

        // Biến xác định kmap
        if (slbien == 2) {
            string var = string(1, variables[0]) + "|" + string(1, variables[1]);
            dc.DrawText(wxString(var), startX - 35, startY - 30);
        }
        else if (slbien == 3) {
            string var = string(1, variables[0]) + "|" + string(1, variables[1]) + string(1, variables[2]);
            dc.DrawText(wxString(var), startX - 40, startY - 30);
        }
        else if (slbien == 4) {
            string var = string(1, variables[0]) + string(1, variables[1]) + "|" + string(1, variables[2]) + string(1, variables[3]);
            dc.DrawText(wxString(var), startX - 50, startY - 30);
        }

        // Nhãn hàng
        for (int i = 0; i < rows && i < (int)rowLabels.size(); i++) {
            dc.DrawText(rowLabels[i],
                startX - 30,
                startY + i * cellSize + cellSize / 2 - 8);
        }

        // Nhãn cột
        for (int j = 0; j < cols && j < (int)colLabels.size(); j++) {
            dc.DrawText(colLabels[j],
                startX + j * cellSize + cellSize / 2 - 10,
                startY - 25);
        }

        // Vẽ các ô kmap
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int x = startX + j * cellSize;
                int y = startY + i * cellSize;

                if (kmap[i][j])
                    dc.SetBrush(wxBrush(wxColour(255, 200, 200)));
                else
                    dc.SetBrush(*wxWHITE_BRUSH);

                dc.SetPen(*wxBLACK_PEN);
                dc.DrawRectangle(x, y, cellSize, cellSize);

                dc.DrawText(kmap[i][j] ? "1" : "0",
                    x + cellSize / 2 - 5,
                    y + cellSize / 2 - 8);
            }
        }

        // Vẽ nhóm
        if (!groups.empty()) {
            int colorIndex = 0;
            vector<wxColour> colors = {
                wxColour(255,0,0,80), wxColour(0,255,0,80),
                wxColour(0,0,255,80), wxColour(255,255,0,80),
                wxColour(255,0,255,80), wxColour(0,255,255,80)
            };

            for (const auto& g : groups) {
                wxColour col = colors[colorIndex % colors.size()];
                dc.SetPen(wxPen(col, 3, wxPENSTYLE_SOLID));
                dc.SetBrush(wxBrush(col, wxBRUSHSTYLE_TRANSPARENT));

                int x = startX + g.startcol * cellSize;
                int y = startY + g.startrow * cellSize;
                int w = g.colsize * cellSize;
                int h = g.rowsize * cellSize;

                int rows = kmap.size();
                int cols = kmap[0].size();

                bool wrapRow = (g.startrow + g.rowsize > rows);
                bool wrapCol = (g.startcol + g.colsize > cols);

                if (!wrapRow && !wrapCol) {
                    dc.DrawRoundedRectangle(x, y, w, h, 15);
                }
                else {
                    if (wrapCol)
                        dc.DrawRoundedRectangle(startX, y, (g.startcol + g.colsize - cols) * cellSize, h, 15);
                    if (wrapRow)
                        dc.DrawRoundedRectangle(x, startY, w, (g.startrow + g.rowsize - rows) * cellSize, 15);
                    dc.DrawRoundedRectangle(x % (cols * cellSize), y % (rows * cellSize),
                        min(w, cols * cellSize), min(h, rows * cellSize), 15);
                }
                colorIndex++;
            }
        }
    }
};
// Giao diện wxWidgets
class MyFrame : public wxFrame {
public:
    KmapPanel* kmap_down;
    KmapPanel* kmap_up;
    wxTextCtrl* o_nhap;
    wxChoice* o_bieuthuc_rutgon;
    vector<string> cacbieuthucrutgon;
    wxStaticBitmap* anh_mach;
    vector<PIbaoquat> allSolutions;

    MyFrame() : wxFrame(NULL, wxID_ANY, "K-map Tool", wxDefaultPosition, wxSize(900, 600)) {
        Maximize(true);
        wxPanel* panel = new wxPanel(this);
        wxStandardPaths& path = wxStandardPaths::Get();
        wxString exedir = wxFileName(path.GetExecutablePath()).GetPath();
        wxString imgdir = exedir + "/Images";

        wxBoxSizer* mainsizer = new wxBoxSizer(wxVERTICAL);
        wxBoxSizer* topsizer = new wxBoxSizer(wxHORIZONTAL);
        wxBoxSizer* leftsizer = new wxBoxSizer(wxVERTICAL);

        //Logo trường
        wxImage logotruong(imgdir + "/LOGO.png", wxBITMAP_TYPE_PNG);
        if (logotruong.IsOk())
            logotruong = logotruong.Scale(100, 100, wxIMAGE_QUALITY_HIGH);
        else {
            logotruong.Create(100, 100);
            logotruong.SetRGB(wxRect(0, 0, 100, 100), 255, 255, 255);
        }
        wxStaticBitmap* o_logo = new wxStaticBitmap(panel, -1, wxBitmap(logotruong));

        //Tên thành viên nhóm
        wxImage dsthanhvien(imgdir + "/danhsachtv.png", wxBITMAP_TYPE_PNG);
        if (dsthanhvien.IsOk())
            dsthanhvien = dsthanhvien.Scale(480, 180, wxIMAGE_QUALITY_HIGH);
        else {
            dsthanhvien.Create(480, 180);
            dsthanhvien.SetRGB(wxRect(0, 0, 480, 180), 255, 255, 255);
        }
        wxStaticBitmap* o_danhsach = new wxStaticBitmap(panel, -1, wxBitmap(dsthanhvien));

        //Tên trường
        wxImage tentruong(imgdir + "/tentruongSPKT.png", wxBITMAP_TYPE_PNG);
        if (tentruong.IsOk())
            tentruong = tentruong.Scale(370, 70, wxIMAGE_QUALITY_HIGH);
        else {
            tentruong.Create(330, 50);
            tentruong.SetRGB(wxRect(0, 0, 330, 50), 255, 255, 255);
        }
        wxStaticBitmap* o_tentruong = new wxStaticBitmap(panel, -1, wxBitmap(tentruong));

        //Gộp tên trường + logo thành 1 khung theo trục ngang 
        wxBoxSizer* logo_tentruong_sizer = new wxBoxSizer(wxHORIZONTAL);
        logo_tentruong_sizer->Add(o_logo, 0, wxLEFT | wxTOP | wxALIGN_LEFT, 5);
        logo_tentruong_sizer->Add(o_tentruong, 0, wxLEFT | wxALIGN_TOP, 5);

        //Gộp tên trường + logo + danh sách vào 1 khung theo trục dọc 
        leftsizer->Add(logo_tentruong_sizer, 0, wxALIGN_LEFT | wxALL, 0);
        leftsizer->Add(o_danhsach, 0, wxTOP | wxALIGN_LEFT, 5);

        wxBoxSizer* inputSizer = new wxBoxSizer(wxVERTICAL);

        //Ô nhập
        wxBoxSizer* line1 = new wxBoxSizer(wxHORIZONTAL);
        line1->Add(new wxStaticText(panel, -1, "Nhap bieu thuc:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_nhap = new wxTextCtrl(panel, -1, "");
        line1->Add(o_nhap, 1, wxALL | wxEXPAND, 5);
        inputSizer->Add(line1, 0, wxEXPAND | wxALL, 5);

        //Nút tính toán
        wxButton* nut = new wxButton(panel, -1, "Tinh toan");
        nut->Bind(wxEVT_BUTTON, &MyFrame::OnCalculate, this);
        inputSizer->Add(nut, 0, wxALIGN_CENTER | wxALL, 5);

        //Ô in ra biểu thức rút gọn
        wxBoxSizer* line2 = new wxBoxSizer(wxHORIZONTAL);
        line2->Add(new wxStaticText(panel, -1, "Bieu thuc rut gon:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 5);
        o_bieuthuc_rutgon = new wxChoice(panel, wxID_ANY);
        line2->Add(o_bieuthuc_rutgon, 1, wxALL | wxEXPAND, 5);
        inputSizer->Add(line2, 0, wxEXPAND | wxALL, 5);

        // Thao tác chọn biểu thức rút gọn
        o_bieuthuc_rutgon->Bind(wxEVT_CHOICE, &MyFrame::OnSelectExpression, this);
        topsizer->Add(leftsizer, 0, wxALIGN_TOP | wxALL, 5);
        topsizer->Add(inputSizer, 1, wxEXPAND | wxALL, 5);

        mainsizer->Add(topsizer, 0, wxEXPAND | wxALL, 5);

        wxBoxSizer* mach_kmap_sizer = new wxBoxSizer(wxHORIZONTAL);

        //Ô mạch logic
        wxBoxSizer* mach_sizer = new wxBoxSizer(wxVERTICAL);
        mach_sizer->Add(new wxStaticText(panel, -1, "Mach logic:"), 0, wxALL, 5);
        wxImage img(400, 300);
        img.SetRGB(wxRect(0, 0, 400, 300), 255, 255, 255);
        anh_mach = new wxStaticBitmap(panel, -1, wxBitmap(img));
        mach_sizer->Add(anh_mach, 1, wxEXPAND | wxALL, 10);
        mach_kmap_sizer->Add(mach_sizer, 1, wxEXPAND | wxALL, 5);
        mainsizer->Add(mach_kmap_sizer, 1, wxEXPAND | wxALL, 0);


        //Bảng kmap
        wxBoxSizer* kmap_sizer = new wxBoxSizer(wxVERTICAL);
        kmap_sizer->Add(new wxStaticText(panel, -1, "K-map (Ban dau):"), 0, wxALIGN_TOP | wxLEFT, 0);
        kmap_up = new KmapPanel(panel);
        kmap_sizer->Add(kmap_up, 1, wxTOP, 0);

        kmap_sizer->Add(new wxStaticText(panel, -1, "K-map (Sau khi rut gon):"), 0, wxALIGN_TOP | wxLEFT, 0);
        kmap_down = new KmapPanel(panel);
        kmap_sizer->Add(kmap_down, 1, wxTOP, 0);

        panel->SetSizerAndFit(mainsizer);
        mach_kmap_sizer->Add(kmap_sizer, 0, wxEXPAND | wxTOP, -150);

        panel->SetSizer(mainsizer);
    }


    //MAIN CHẠY CHÍNH
    void OnCalculate(wxCommandEvent&) {
        string input = o_nhap->GetValue().ToStdString();
        xuly(input);  // -> gán slbien, dsbien, dsvitri

        auto kmap = biendoi_kmap(dsvitri);

        auto PI = rutgon_kmap(kmap);

        allSolutions = buildtatcadapan(kmap, PI, slbien);

        o_bieuthuc_rutgon->Clear();
        cacbieuthucrutgon.clear();
        for (int i = 0; i < (int)allSolutions.size(); ++i) {
            wxString label = wxString::Format("Case %d: %s", i + 1, allSolutions[i].bieuthuc);
            o_bieuthuc_rutgon->Append(label);
            cacbieuthucrutgon.push_back(allSolutions[i].bieuthuc);
        }
        if (!allSolutions.empty()) {
            o_bieuthuc_rutgon->SetSelection(0);
        }

        auto kmapSOP = biendoi_kmap(dsvitri);
        kmap_up->SetData(kmapSOP, {}, slbien, dsbien);

        if (!allSolutions.empty()) {
            const auto& sel = allSolutions[0];

            kmap_down->SetData(kmap, sel.dachon, slbien, dsbien);

            // Render mạch theo biểu thức ban đầu
            wxStandardPaths& path = wxStandardPaths::Get();
            wxString _Exepath = path.GetExecutablePath();
            wxFileName filename(_Exepath);
            wxString exedir = filename.GetPath();

            wxString exe = exedir + "/bin/dot.exe";
            wxString pngf = exedir + "/Images";
            wxString dotF = exedir + "/circuit.dot";
            wxString pngF = exedir + "/circuit.png";

            string exepath = string(exe.mb_str());
            string pngfolder = string(pngf.mb_str());
            string dotpath = string(dotF.mb_str());
            string pngpath = string(pngF.mb_str());

            if (makeDot(sel.bieuthuc, dotpath, pngfolder) && doGraph(exepath, dotpath, pngpath)) {
                wxImage img(pngpath);
                if (img.IsOk()) {
                    img.Rescale(1100, 600, wxIMAGE_QUALITY_HIGH);
                    anh_mach->SetBitmap(wxBitmap(img));
                    anh_mach->Refresh();
                }
            }
        }
    }

    void OnSelectExpression(wxCommandEvent&) {
        int idx = o_bieuthuc_rutgon->GetSelection();
        if (idx == wxNOT_FOUND || idx >= (int)allSolutions.size()) return;

        auto kmap = biendoi_kmap(dsvitri);

        kmap_down->SetData(kmap, allSolutions[idx].dachon, slbien, dsbien);

        wxStandardPaths& path = wxStandardPaths::Get();
        wxString _Exepath = path.GetExecutablePath();
        wxFileName filename(_Exepath);
        wxString exedir = filename.GetPath();

        wxString exe = exedir + "/bin/dot.exe";
        wxString pngf = exedir + "/Images";
        wxString dotF = exedir + "/circuit.dot";
        wxString pngF = exedir + "/circuit.png";

        string exepath = string(exe.mb_str());
        string pngfolder = string(pngf.mb_str());
        string dotpath = string(dotF.mb_str());
        string pngpath = string(pngF.mb_str());

        if (makeDot(allSolutions[idx].bieuthuc, dotpath, pngfolder) && doGraph(exepath, dotpath, pngpath)) {
            wxImage img(pngpath);
            if (img.IsOk()) {
                img.Rescale(1100, 600, wxIMAGE_QUALITY_HIGH);
                anh_mach->SetBitmap(wxBitmap(img));
                anh_mach->Refresh();
            }
        }
    }
};

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