#include <wx/wx.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <algorithm>
#include <iostream>
#include <math.h>
using namespace std;

vector<int> dsvitri; // danh sách dữ liệu cho thuật toán Kmap (ô kề)
int slbien;

int chuyendoi(const string& toantu)
{
    string toantureal = "";
    for (int i = 0; i < toantu.length(); i++) {
        if (toantu[i] != ' ') toantureal += toantu[i];
    }
    //x-yz
    vector<int> bits;
    for (int i = 0; i < toantureal.length(); i++) {
        if (toantureal[i] != '-') {
            if (i > 0) {
                if (toantureal[i - 1] == '-') bits.push_back(0);
                else if (toantureal[i - 1] != '-') bits.push_back(1);
            }
            //trường hợp i==0
            else bits.push_back(1);
        }
    }

    //chuyển từ bit sang số
    int num = 0;
    int j = 0;
    for (int i = bits.size() - 1; i >= 0; i--) {
        num += bits[j] * pow(2, i);
        j++;
    }
    return num;
}
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
    cout << slbien << endl;
}

int main() {
    string input = "";
    getline(cin, input);
    xuly(input);
    for (int x : dsvitri) {
        cout << x << endl;
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

