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

//Chuyển một toán tử sang bit nhị phân an toàn
string chuyendoi(const string& toantu)
{
    string bits = "";
    bool phudinhchung = false;
    for (int i = 0; i < (int)toantu.length(); i++) {
        if (i > 0 && toantu[i] == '(' && toantu[i - 1] == '-')
            phudinhchung = true;

        if (toantu[i] == ')')
            phudinhchung = false;

        if (isalpha((unsigned char)toantu[i])) {
            if (phudinhchung) bits += "0";
            else {
                int j = i - 1;
                while (j >= 0 && isspace((unsigned char)toantu[j])) j--;
                if (j >= 0 && (toantu[j] == '-' || toantu[j] == '~' || toantu[j] == '!'))
                    bits += "0";
                else
                    bits += "1";
            }
        }
    }
    return bits;
}

//Tách chuỗi biểu thức thành từng toán tử, tự động mở rộng các số hạng thiếu biến sang minterm
void xuly(const string& bieuthuc) {
    slbien = 0;
    dsvitri.clear();
    vector<char> bien;

    // Trích xuất danh sách biến duy nhất và chuẩn hóa chữ in hoa
    for (char c : bieuthuc) {
        if (isalpha((unsigned char)c)) {
            char u = (char)toupper((unsigned char)c);
            if (find(bien.begin(), bien.end(), u) == bien.end()) {
                bien.push_back(u);
            }
        }
    }
    sort(bien.begin(), bien.end());
    slbien = (int)bien.size();
    dsbien = bien;

    // Đảm bảo tối thiểu 2 biến cho K-map
    if (slbien < 2) {
        if (slbien == 0) {
            dsbien = { 'A', 'B' };
            slbien = 2;
        }
        else if (slbien == 1) {
            char extra = (dsbien[0] == 'A') ? 'B' : 'A';
            dsbien.push_back(extra);
            sort(dsbien.begin(), dsbien.end());
            slbien = 2;
        }
    }

    set<string> mintermSet;
    stringstream ss(bieuthuc);
    string toantu;
    while (getline(ss, toantu, '+'))
    {
        string tClean = toantu;
        tClean.erase(remove_if(tClean.begin(), tClean.end(), ::isspace), tClean.end());
        if (tClean.empty()) continue;

        if (tClean == "1") {
            int totalM = 1 << slbien;
            for (int m = 0; m < totalM; ++m) {
                string bits = "";
                for (int k = 0; k < slbien; ++k) {
                    bits += ((m >> (slbien - 1 - k)) & 1) ? "1" : "0";
                }
                mintermSet.insert(bits);
            }
            continue;
        }
        if (tClean == "0") continue;

        // val[k]: 0 = phủ định, 1 = khẳng định, -1 = vắng mặt (don't care)
        vector<int> val(slbien, -1);
        bool phudinhchung = false;
        for (int i = 0; i < (int)toantu.length(); i++) {
            if (toantu[i] == '(' && i > 0 && toantu[i - 1] == '-')
                phudinhchung = true;
            else if (toantu[i] == ')')
                phudinhchung = false;
            else if (isalpha((unsigned char)toantu[i])) {
                char varChar = (char)toupper((unsigned char)toantu[i]);
                auto it = find(dsbien.begin(), dsbien.end(), varChar);
                if (it != dsbien.end()) {
                    int idx = (int)distance(dsbien.begin(), it);
                    bool isNeg = phudinhchung;
                    if (!isNeg) {
                        int j = i - 1;
                        while (j >= 0 && isspace((unsigned char)toantu[j])) j--;
                        if (j >= 0 && (toantu[j] == '-' || toantu[j] == '~' || toantu[j] == '!'))
                            isNeg = true;
                    }
                    val[idx] = isNeg ? 0 : 1;
                }
            }
        }

        // Mở rộng thành các minterm tương ứng
        int totalM = 1 << slbien;
        for (int m = 0; m < totalM; ++m) {
            bool match = true;
            for (int k = 0; k < slbien; ++k) {
                int bit = (m >> (slbien - 1 - k)) & 1;
                if (val[k] != -1 && val[k] != bit) {
                    match = false;
                    break;
                }
            }
            if (match) {
                string bits = "";
                for (int k = 0; k < slbien; ++k) {
                    bits += ((m >> (slbien - 1 - k)) & 1) ? "1" : "0";
                }
                mintermSet.insert(bits);
            }
        }
    }

    for (const string& mBits : mintermSet) {
        dsvitri.push_back(mBits);
    }
}

//BIẾN ĐỔI SANG KMAP
//Chuyển từ bit sang vị trí trong mảng 2 chiều (Gray code)
int vitri(const string& bit) {
    static vector<string> bits2 = { "00","01","11","10" };
    if (bit.size() == 1)
        return (bit == "1") ? 1 : 0;
    if (bit.size() == 2) {
        for (int i = 0; i < (int)bits2.size(); i++)
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
        int r = (chiadoi == 0) ? 0 : vitri(x.substr(0, chiadoi));
        int c = ((slbien - chiadoi) == 0) ? 0 : vitri(x.substr(chiadoi));

        if (r >= 0 && r < row && c >= 0 && c < col)
            kmap[r][c] = true;
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
    if (bitlength == 2 && index >= 0 && index < (int)Bits2.size()) return Bits2[index];
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

static PI_Chiacat CatEPI(const vector<group>& PIs, const vector<pair<int, int>>& ones, const vector<vector<int>>& cover)
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

    int R = (int)vitripiconlai.size();

    uint32_t total = (R >= 31) ? 0u : (1u << R);
    for (uint32_t mask = 0; mask < total; ++mask) {
        int cnt = dembit1(mask);
        if (cnt > totnhat.first) continue;

        bool ok = true;
        // Duyệt qua tất cả các ô 1 chưa được phủ để kiểm tra xem đã được phủ bởi mask chưa
        for (int oi : vitrio1_chuaphu) {
            bool covered = false;
            for (int b = 0; b < R; ++b) {
                if (mask & (1u << b)) {
                    int piIdx = vitripiconlai[b];
                    if (find(baophu[oi].begin(), baophu[oi].end(), piIdx) != baophu[oi].end()) {
                        covered = true;
                        break;
                    }
                }
            }
            if (!covered) {
                ok = false;
                break;
            }
        }
        if (!ok) continue;

        vector<int> choose;
        for (int b = 0; b < R; ++b) {
            if (mask & (1u << b))
                choose.push_back(vitripiconlai[b]);
        }

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
    auto sp = CatEPI(PIs, ones, cover);

    // Tất cả ô 1 đã được EPI phủ, trả về nghiệm tối tiểu duy nhất
    if (sp.vitri_o1_chuaphu.empty()) {
        vector<group> chosen;
        for (int e : sp.vitri_epi) chosen.push_back(PIs[e]);
        string expr = chuoibieuthuc(chosen, soluongbien);
        out.push_back({ chosen, expr });
        return out;
    }

    auto themnhom = timbaophumin(sp.vitri_o1_chuaphu, sp.vitri_pi_conlai, cover, PIs, soluongbien);

    // Lắp các tổ hợp tối tiểu đầy đủ
    for (auto& nhomchon : themnhom) {
        vector<group> nhomdachon;
        for (int e : sp.vitri_epi) nhomdachon.push_back(PIs[e]);
        for (int x : nhomchon) nhomdachon.push_back(PIs[x]);

        string expr = chuoibieuthuc(nhomdachon, soluongbien);
        out.push_back({ nhomdachon, expr });
    }

    // Loại bỏ các trường hợp biểu thức trùng lặp
    vector<PIbaoquat> uniqueOut;
    set<string> seenExpr;
    for (const auto& sol : out) {
        if (seenExpr.find(sol.bieuthuc) == seenExpr.end()) {
            seenExpr.insert(sol.bieuthuc);
            uniqueOut.push_back(sol);
        }
    }
    return uniqueOut;
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

    // Xử lý trường hợp biểu thức là hằng số 0 hoặc 1
    string exprClean = expr;
    exprClean.erase(remove_if(exprClean.begin(), exprClean.end(), ::isspace), exprClean.end());
    if (exprClean == "0") {
        f << "ZERO[shape=circle, label=\"0\", fillcolor=\"#D3D3D3\"];\n";
        f << "F[shape=doublecircle, label=\"F\", fillcolor=\"#FFA07A\"];\n";
        f << "ZERO -> F;\n";
        f << "}\n";
        f.close();
        return true;
    }
    if (exprClean == "1") {
        f << "ONE[shape=circle, label=\"1\", fillcolor=\"#D3D3D3\"];\n";
        f << "F[shape=doublecircle, label=\"F\", fillcolor=\"#FFA07A\"];\n";
        f << "ONE -> F;\n";
        f << "}\n";
        f.close();
        return true;
    }

    for (char var : dsbien)
        f << var << "[shape=circle, label=\"" << var << "\", fillcolor=\"#DDA0DD\"];\n";

    int cNOT = 0, cAND = 0;
    auto daychuoi = tachchuoi(expr, '+');
    vector<string> termOutputs;

    for (auto& t : daychuoi) {
        t.erase(remove_if(t.begin(), t.end(), ::isspace), t.end());
        if (t.empty()) continue;

        int sobien = 0;
        for (char c : t)
            if (isalpha((unsigned char)c)) sobien++;

        if (sobien == 0) continue;

        string target;
        if (sobien > 1) {
            cAND++;
            string aN = "A" + to_string(cAND);
            f << aN << "[shape=none, image=\"" << pngf << "/AND.png\","
                " label=\"\", imagescale=true, fixedsize=true, width=0.5, height=0.5, margin=0];\n";
            target = aN;
        }

        for (int i = 0; i < (int)t.size(); i++) {
            char c = t[i];
            if (c == '-') continue;
            string var(1, c);

            if (i > 0 && t[i - 1] == '-') {
                cNOT++;
                string nN = "N" + to_string(cNOT);
                f << nN << "[shape=none, image=\"" << pngf << "/NOT.png\","
                    " label=\"\", imagescale=true, fixedsize=true, width=0.6, height=0.6, margin=0];\n";
                f << var << " -> " << nN << ";\n";
                if (sobien > 1) {
                    f << nN << " -> " << target << ";\n";
                }
                else {
                    target = nN;
                }
            }
            else {
                if (sobien > 1) {
                    f << var << " -> " << target << ";\n";
                }
                else {
                    target = var;
                }
            }
        }

        termOutputs.push_back(target);
    }

    if (termOutputs.empty()) {
        f << "ZERO[shape=circle, label=\"0\", fillcolor=\"#D3D3D3\"];\n";
        f << "F[shape=doublecircle, label=\"F\", fillcolor=\"#FFA07A\"];\n";
        f << "ZERO -> F;\n";
    }
    else if (termOutputs.size() == 1) {
        f << "F[shape=doublecircle, label=\"F\", fillcolor=\"#FFA07A\"];\n";
        f << termOutputs[0] << " -> F;\n";
    }
    else {
        f << "OR[shape=none, image=\"" << pngf << "/OR.png\","
            " label=\"\", imagescale=true, fixedsize=true, width=0.5, height=0.5, margin=0];\n";
        for (const auto& tNode : termOutputs) {
            f << tNode << " -> OR;\n";
        }
        f << "F[shape=doublecircle, label=\"F\", fillcolor=\"#FFA07A\"];\n";
        f << "OR -> F;\n";
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
        cerr << "Tao tien trinh that bai, loi! " << GetLastError() << endl;
        return false;
    }

    WaitForSingleObject(pi.hProcess, 5000); // 5s timeout chống treo ứng dụng
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

                int rows = kmap.size();
                int cols = kmap[0].size();

                vector<pair<int, int>> rowIntervals;
                if (g.startrow + g.rowsize <= rows) {
                    rowIntervals.push_back({ g.startrow, g.rowsize });
                }
                else {
                    rowIntervals.push_back({ g.startrow, rows - g.startrow });
                    rowIntervals.push_back({ 0, g.startrow + g.rowsize - rows });
                }

                vector<pair<int, int>> colIntervals;
                if (g.startcol + g.colsize <= cols) {
                    colIntervals.push_back({ g.startcol, g.colsize });
                }
                else {
                    colIntervals.push_back({ g.startcol, cols - g.startcol });
                    colIntervals.push_back({ 0, g.startcol + g.colsize - cols });
                }

                for (const auto& rInt : rowIntervals) {
                    for (const auto& cInt : colIntervals) {
                        int rx = startX + cInt.first * cellSize;
                        int ry = startY + rInt.first * cellSize;
                        int rw = cInt.second * cellSize;
                        int rh = rInt.second * cellSize;
                        dc.DrawRoundedRectangle(rx, ry, rw, rh, 15);
                    }
                }
                colorIndex++;
            }
        }
    }
};
static wxString TimAsset(const wxString& exedir, const wxString& relativePath) {
    if (wxFileExists(exedir + "/" + relativePath) || wxDirExists(exedir + "/" + relativePath))
        return exedir + "/" + relativePath;
    if (wxFileExists(exedir + "/../x64/Debug/" + relativePath) || wxDirExists(exedir + "/../x64/Debug/" + relativePath))
        return exedir + "/../x64/Debug/" + relativePath;
    if (wxFileExists(exedir + "/x64/Debug/" + relativePath) || wxDirExists(exedir + "/x64/Debug/" + relativePath))
        return exedir + "/x64/Debug/" + relativePath;
    return exedir + "/" + relativePath;
}

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
        wxString imgdir = TimAsset(exedir, "Images");

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
        else {
            o_bieuthuc_rutgon->Append("Case 1: 0");
            o_bieuthuc_rutgon->SetSelection(0);
            cacbieuthucrutgon.push_back("0");
        }

        auto kmapSOP = biendoi_kmap(dsvitri);
        kmap_up->SetData(kmapSOP, {}, slbien, dsbien);

        wxStandardPaths& path = wxStandardPaths::Get();
        wxString _Exepath = path.GetExecutablePath();
        wxFileName filename(_Exepath);
        wxString exedir = filename.GetPath();

        wxString exe = TimAsset(exedir, "bin/dot.exe");
        wxString pngf = TimAsset(exedir, "Images");
        wxString dotF = exedir + "/circuit.dot";
        wxString pngF = exedir + "/circuit.png";

        string exepath = string(exe.mb_str());
        string pngfolder = string(pngf.mb_str());
        string dotpath = string(dotF.mb_str());
        string pngpath = string(pngF.mb_str());

        string bthucVe = allSolutions.empty() ? "0" : allSolutions[0].bieuthuc;
        vector<group> nhomVe = allSolutions.empty() ? vector<group>{} : allSolutions[0].dachon;
        kmap_down->SetData(kmap, nhomVe, slbien, dsbien);

        if (makeDot(bthucVe, dotpath, pngfolder) && doGraph(exepath, dotpath, pngpath)) {
            wxImage img(pngpath);
            if (img.IsOk()) {
                img.Rescale(1100, 600, wxIMAGE_QUALITY_HIGH);
                anh_mach->SetBitmap(wxBitmap(img));
                anh_mach->Refresh();
            }
        }
    }

    void OnSelectExpression(wxCommandEvent&) {
        int idx = o_bieuthuc_rutgon->GetSelection();
        if (idx == wxNOT_FOUND) return;

        auto kmap = biendoi_kmap(dsvitri);
        string bthucVe = "0";
        vector<group> nhomVe;

        if (idx < (int)allSolutions.size()) {
            bthucVe = allSolutions[idx].bieuthuc;
            nhomVe = allSolutions[idx].dachon;
        }

        kmap_down->SetData(kmap, nhomVe, slbien, dsbien);

        wxStandardPaths& path = wxStandardPaths::Get();
        wxString _Exepath = path.GetExecutablePath();
        wxFileName filename(_Exepath);
        wxString exedir = filename.GetPath();

        wxString exe = TimAsset(exedir, "bin/dot.exe");
        wxString pngf = TimAsset(exedir, "Images");
        wxString dotF = exedir + "/circuit.dot";
        wxString pngF = exedir + "/circuit.png";

        string exepath = string(exe.mb_str());
        string pngfolder = string(pngf.mb_str());
        string dotpath = string(dotF.mb_str());
        string pngpath = string(pngF.mb_str());

        if (makeDot(bthucVe, dotpath, pngfolder) && doGraph(exepath, dotpath, pngpath)) {
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