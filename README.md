# bin2c
A simple utility for converting a binary file to a header source file which can be included within an application like wxWidgets. Work based on gwilymk's bin2c

# Usage
bin2c ok.png

Now you have a header file called ok.h which can be use like this:
```
#include "ok.h"

MyFrame::MyFrame(...)
{
  auto button1 = new wxButton(panel, wxID_OK);
  button1->SetToolTip(_("Show a dialog with information"));
  button1->SetBitmap(wxBITMAP_PNG_FROM_DATA(ok)); // same name as header
}
```