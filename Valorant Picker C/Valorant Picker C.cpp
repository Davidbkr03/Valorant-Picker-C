#include <opencv2/opencv.hpp> 
#include <iostream> 
#include <windows.h>
#include <filesystem>
#include <direct.h>

using namespace cv;
using namespace std;

Mat hwnd2mat(HWND hwnd) {

    HDC hwindowDC, hwindowCompatibleDC;

    int height, width, srcheight, srcwidth;
    HBITMAP hbwindow;
    Mat src;
    BITMAPINFOHEADER  bi;

    hwindowDC = GetDC(hwnd);
    hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    RECT windowsize;    // get the height and width of the screen
    GetClientRect(hwnd, &windowsize);

    srcheight = windowsize.bottom;
    srcwidth = windowsize.right;
    height = windowsize.bottom;  //change this to whatever size you want to resize to
    width = windowsize.right;

    src.create(height, width, CV_8UC4);

    // create a bitmap
    hbwindow = CreateCompatibleBitmap(hwindowDC, width, height);
    bi.biSize = sizeof(BITMAPINFOHEADER);    //http://msdn.microsoft.com/en-us/library/windows/window/dd183402%28v=vs.85%29.aspx
    bi.biWidth = width;
    bi.biHeight = -height;  //this is the line that makes it draw upside down or not
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // use the previously created device context with the bitmap
    SelectObject(hwindowCompatibleDC, hbwindow);
    // copy from the window device context to the bitmap device context
    StretchBlt(hwindowCompatibleDC, 0, 0, width, height, hwindowDC, 0, 0, srcwidth, srcheight, SRCCOPY); //change SRCCOPY to NOTSRCCOPY for wacky colors !
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, height, src.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);  //copy from hwindowCompatibleDC to hbwindow

    // avoid memory leak
    DeleteObject(hbwindow); DeleteDC(hwindowCompatibleDC); ReleaseDC(hwnd, hwindowDC);

    return src;
}

bool NMultipleTemplateMatching(Mat mInput, Mat mTemplate, float Threshold, float Closeness, vector<Point2f>& List_Matches, bool click)
{
    bool found = false;
    Mat mResult;
    Size szTemplate = mTemplate.size();
    Size szTemplateCloseRadius((szTemplate.width / 2) * Closeness, (szTemplate.height / 2) * Closeness);

    matchTemplate(mInput, mTemplate, mResult, TM_CCOEFF_NORMED);
    threshold(mResult, mResult, Threshold, 1.0, THRESH_TOZERO);
    
    
        double minval, maxval;
        Point minloc, maxloc;
        minMaxLoc(mResult, &minval, &maxval, &minloc, &maxloc);

        if (maxval >= Threshold)
        {
            List_Matches.push_back(maxloc);
            if (click == true)
            {
                //move mouse to the center of the template
                SetCursorPos(maxloc.x + szTemplate.width / 2, maxloc.y + szTemplate.height / 2);
                //click mouse
                mouse_event(2, 0, 0, 0, 0);
                Sleep(5);
                mouse_event(4, 0, 0, 0, 0);
            }

            found = true;
        }           
    
    //imshow("reference", mDebug_Bgr);
    return found;
}

int main(int argc, char** argv)
{
    const size_t size = 1024;
    char buffer[size];

    if (_getcwd(buffer, size) != nullptr) {
        cout << "Current working directory: " << buffer << endl;
    }
    else {
        cerr << "Error getting current working directory" << endl;
    }
    
    Mat AgentRGB, AgentGray;
    AgentRGB = imread("Agents/ASTRA.png", 1);
    //imshow("AgentRGB", AgentRGB);

    Mat LockingRGB, LockingGray;
    LockingRGB = imread("Agents/LOCKIN.PNG", 1);

    Mat initial;
    initial = imread("Agents/INITIAL.PNG", 1);

    bool search = false;
    bool running = true;

    while (running)
    {
        HWND hDesktopWnd;
        hDesktopWnd = GetDesktopWindow();
        Mat mScreenShot = hwnd2mat(hDesktopWnd);
        Mat mSource_Gray, mResult_Bgr = mScreenShot.clone();

        float Threshold = 0.9;
        float Closeness = 0.9;
        vector<Point2f> List_Matches;

        cvtColor(mScreenShot, mSource_Gray, COLOR_BGR2GRAY);
        cvtColor(initial, AgentGray, COLOR_BGR2GRAY);

        search = NMultipleTemplateMatching(mSource_Gray, AgentGray, Threshold, Closeness, List_Matches, false);

        if (search == true)
		{
			cout << "Found Initial" << endl;
		}

        while (search == true)
        {

            hDesktopWnd = GetDesktopWindow();
            mScreenShot = hwnd2mat(hDesktopWnd);

            cvtColor(mScreenShot, mSource_Gray, COLOR_BGR2GRAY);
            cvtColor(AgentRGB, AgentGray, COLOR_BGR2GRAY);
            cvtColor(LockingRGB, LockingGray, COLOR_BGR2GRAY);

            //namedWindow("Screen Shot", WINDOW_AUTOSIZE);
            //imshow("Screen Shot", mSource_Gray);


            if (NMultipleTemplateMatching(mSource_Gray, AgentGray, Threshold, Closeness, List_Matches, true))
			{
				search = false;
			}

            NMultipleTemplateMatching(mSource_Gray, LockingGray, Threshold, Closeness, List_Matches, true);
        }

    }

    

    

    //imshow("Final Results", mResult_Bgr);

    // Wait for any keystroke in the window 
    // waitKey(0);
    return 0;
}