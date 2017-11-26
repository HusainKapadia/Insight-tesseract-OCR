
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <fstream>  

using namespace cv;
using namespace std;

int n;
int x_start = 20  ; //20
int y_start = 160  ; //30
int x_end =  620 ; //560
int y_end =  250 ; //350

int main(int argc, char* argv[]) {

  fstream opt;
  ofstream edit;
  opt.open("data.txt", std::fstream::trunc | ios::out);
 
 VideoCapture cap(0);
 if(!cap.isOpened())
 {
  cout<<"\nError opening camera";
  return -1;
 }

  bool start = false;
  //Rect text1ROI(20,30,560,350);
  Rect text1ROI(x_start,y_start,x_end,y_end); 
Mat image_prev,temp;
cap>>image_prev;

tesseract::TessBaseAPI *myOCR;

while(1)
{
   myOCR = new tesseract::TessBaseAPI(); // initilize tesseract OCR engine

  if (myOCR->Init(NULL, "eng")) {
    fprintf(stderr, "Could not initialize tesseract.\n");
    exit(1);
  }

 // tesseract::PageSegMode pagesegmode = static_cast<tesseract::PageSegMode>(7); // treat the image as a single text line
 // myOCR->SetPageSegMode(pagesegmode);


  Mat image ;
  cap>>image;


   vector< vector<Point> > contours;
   vector<Vec4i> hierarchy;
   
   Mat gray,gray_prev;
   
   cvtColor(image,gray,CV_BGR2GRAY);
   cvtColor(image_prev,gray_prev,CV_BGR2GRAY);
   absdiff(gray,gray_prev,temp);
   
   threshold(temp,temp, 50, 255, CV_THRESH_BINARY);
   GaussianBlur(gray,gray,Size(1,1),0,0);
    
   Mat image2;
   threshold(gray, image2, 0, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
   rectangle(image2,Point(x_start,y_start),Point(x_end,y_end),Scalar(255,255,255));
   
   //dilate(image2,image2,getStructuringElement(MORPH_ELLIPSE,Size(1,1)));

   findContours(temp,contours,hierarchy,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);

    cout<<"\n"<<contours.size();
   if(contours.size()>3)
     start = false;
   else
     start = true;
  
  if(start)
  {
  // recognize text
   myOCR->TesseractRect( image2.data, 1, image2.step1(), text1ROI.x, text1ROI.y, text1ROI.width, text1ROI.height);
   char *text1 = myOCR->GetUTF8Text();

  
  // remove "newline"
   string t1(text1);
   t1.erase(std::remove(t1.begin(), t1.end(), '\n'), t1.end());
 
  cout<<"\nfound text1: \n";
  printf("%s",t1.c_str());
  printf("\n");


  n = atoi(t1.c_str());
  cout<<"\n n = "<<n;
  opt<<t1.c_str()<<endl;

  delete(text1);
  }
  else
    cout<<"\nmoving";

  waitKey(100);
  imshow("Thresh",image2);
 

  if(waitKey(30)>=27)
    break;

  myOCR->Clear();
  myOCR->End();
  image_prev=image;
}
  opt.close();
  opt.open("data.txt");

 edit.open("filtered.txt",std::ofstream::trunc);
  char c;

  if(edit.is_open())
  {
   while(opt.get(c))
   {
    if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<= '9'))
     edit.put(c);
    else
     edit.put(' ');
   }
  }
  
  opt.close();
  edit.close();
  destroyAllWindows();
  
  system("espeak -s90 -f filtered.txt --stdout > play");//system() runs the given command in the terminal
  system("aplay play");   
     return 0;
}
