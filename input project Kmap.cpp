#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
using namespace std;

vector<char> loaibien = { 'x','y','z','w' };
int slbien;
vector<int> dsvitri; // danh sách dữ liệu cho thuật toán Kmap (ô kề)

//Hàm chuyển đổi toán tử sang vị trí trong K-map
int chuyendoi(const string& toantu)
{
    vector<int> bits(slbien, 0);

    for (int i = 0; i < (int)toantu.size(); i++)
    {
        char c = toantu[i];
        if (c == ' ' || c == '-') continue;
        int val = 1;

        for (int v = 0; v < slbien; v++)
        {
            if (c == loaibien[v])
            {
                if (i > 0 && toantu[i - 1] == '-') val = 0; // phủ định dạng -x
                bits[v] = val;
            }
        }
    }
    // chuyển bit thành vị trí
    int vitri = 0;
    for (int i = 0; i < slbien; i++) 
    {
        vitri = (vitri << 1) | bits[i];
    }
    return vitri;
}

//Hàm xử lý biểu thức
void xuly(const string& bieuthuc) 
{
    slbien = 0;
    dsvitri.clear();

    map<char, bool> ktxuathien = { {'x',false}, {'y',false}, {'z',false}, {'w',false} };
    for (char c : bieuthuc) 
    {
        for (int v = 0; v < 4; v++) 
        {
            if (c == loaibien[v] && !ktxuathien[c]) 
            {
                slbien++;
                ktxuathien[c] = true;
            }
        }
    }

    stringstream ss(bieuthuc);
    string toantu;
    while (getline(ss, toantu, '+')) 
    {
        if (!toantu.empty()) dsvitri.push_back(chuyendoi(toantu));
    }
}

// Lớp giao diện chính
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
        sapdoc->Add(new wxStaticText(bang, -1, "Mach logic:"), 0, wxALL , 15);
        wxImage img(200, 150);
        img.SetRGB(wxRect(0, 0, 200, 150), 255, 255, 255);
        anh_mach = new wxStaticBitmap(bang, -1, wxBitmap(img));
        sapdoc->Add(anh_mach, 1, wxEXPAND | wxALL,  10);

        bang->SetSizer(sapdoc);
    }

    void OnCalculate(wxCommandEvent& event) 
    {
        wxString bieuthuc = o_nhap->GetValue();

        // Chưa có kết quả => để trống output
        o_bieuthuc_rutgon->Clear();

        // Chưa có dữ liệu mạch => reset ảnh trắng
        wxImage img(200, 150);
        img.SetRGB(wxRect(0, 0, 200, 150), 255, 255, 255); // nền trắng
        anh_mach->SetBitmap(wxBitmap(img));
    }
};


// App chính
class MyApp : public wxApp 
{
public : virtual bool OnInit() {

        MyFrame* khung = new MyFrame();
        khung->Show(true);
        return true;
    }
};

wxIMPLEMENT_APP(MyApp); //chứa hàm main => ko cần viết hàm main

