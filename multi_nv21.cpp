#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
using namespace std; 
const char* InPath = "4160x3120.nv21";
const char* OutPath = "4160x3120_new.nv21";
#define Width 4160
#define Height 3120

void nv21_Thread1(int T1_Width, int T1_Height, char** ThreadBuffer) {
    //线程一：局部置黑
    for(int i=0;i<T1_Height;i++) { //Y变量
        for(int j=0;j<T1_Width;j++) {
            ThreadBuffer[i][j] = 0;
        }
    }
    int T1_uvHeight = Height+T1_Height/2;
    for(int i=Height;i<T1_uvHeight;i++) { //UV变量
        for(int j=0;j<T1_Width;j++) {
            ThreadBuffer[i][j] = 128;
        }
    }
}

void nv21_Thread2(int T1_Width, int T1_Height, int T2_RectWidth, int T2_RectHeight, char** ThreadBuffer) {
    for(int j=T1_Width;j<T2_RectWidth;j++) {
        ThreadBuffer[T1_Height][j] = 0;
    }
    for(int j=T1_Width;j<T2_RectWidth;j++) {
        ThreadBuffer[T2_RectHeight-1][j] = 0;
    }
    for(int i=T1_Height;i<T2_RectHeight;i++) {
        ThreadBuffer[i][T1_Width] = 0;
    }
    for(int i=T1_Height;i<T2_RectHeight;i++) {
        ThreadBuffer[i][T2_RectWidth-1] = 0;
    }
}

void nv21_Thread3(int T3_BegWidth, int T3_BegHeight, int T3_MidWidth, int T3_SwapWidth, int Mutex_Height, char** ThreadBuffer) {
    //线程3：水平镜像
    for(int i=T3_BegHeight;i<Height;i++){ //Y变量
        for(int j=T3_BegWidth;j<T3_MidWidth;j++){
         	char temp=ThreadBuffer[i][j];
            ThreadBuffer[i][j]=ThreadBuffer[i][T3_SwapWidth-j];
            ThreadBuffer[i][T3_SwapWidth-j]=temp;
        }
    }
    for(int i=Height+(T3_BegHeight)/2;i<Mutex_Height;i++){ //UV变量
        for(int j=T3_BegWidth;j<T3_MidWidth;j++){
         	char temp=ThreadBuffer[i][j];
            ThreadBuffer[i][j]=ThreadBuffer[i][T3_SwapWidth-j];
            ThreadBuffer[i][T3_SwapWidth-j]=temp;
        }
    }
}

void nv21_Thread4(int T4_Width, int T4_Height, int T4_MidWidth, int T4_yBegHeight, int T4_uvBegHeight, int Mutex_Height, char** ThreadBuffer) {
    //先左右镜像
    for(int i=T4_yBegHeight;i<Height;i++){ //Y变量
        for(int j=0;j<T4_MidWidth;j++){
         	char temp=ThreadBuffer[i][j];
            ThreadBuffer[i][j]=ThreadBuffer[i][T4_Width-1-j];
            ThreadBuffer[i][T4_Width-1-j]=temp;
        }
    }
    for(int i=T4_uvBegHeight;i<Mutex_Height;i++){ //UV变量
        for(int j=0;j<T4_MidWidth;j++){
         	char temp=ThreadBuffer[i][j];
            ThreadBuffer[i][j]=ThreadBuffer[i][T4_Width-1-j];
            ThreadBuffer[i][T4_Width-1-j]=temp;
        }
    }
    //再上下镜像
    for(int j=0;j<T4_Width;j++){ //Y变量
        for(int i=T4_yBegHeight;i<T4_yBegHeight+T4_Height/2;i++){
         	char temp=ThreadBuffer[i][j];
            ThreadBuffer[i][j]=ThreadBuffer[Height-1-i+T4_yBegHeight][j];
            ThreadBuffer[Height-1-i+T4_yBegHeight][j]=temp;
        }
    }
    for(int j=0;j<T4_Width;j++){ //UV变量
        for(int i=T4_uvBegHeight;i<T4_uvBegHeight+T4_Height/4;i++){
         	char temp=ThreadBuffer[i][j];
            ThreadBuffer[i][j]=ThreadBuffer[Mutex_Height-1-i+T4_uvBegHeight][j];
            ThreadBuffer[Mutex_Height-1-i+T4_uvBegHeight][j]=temp;
        }
    }
}

int main(){ 
    //以二进制形式读取图像
    ifstream in(InPath,ios::binary);
    in.seekg(0,ios::end);
    int Size = in.tellg();
    in.seekg(0,ios::beg);

    //建立数组存储输入图像
    char *InBuffer = new char[Size];
    in.read(InBuffer,Size);
    in.close();
    cout<<"图像像素数为："<<Size<<endl;

    //建立数组存储输出图像
    int Y_size = Width * Height;
    int UV_size = Width * Height / 2;
    char *OutBuffer = new char[Size];

    //建立二维数组来存储YUV变量
    int Mutex_Height = Height + UV_size / Width;//矩阵高度为Y高度加UV高度
    char **ThreadBuffer = new char*[Mutex_Height];
    for(int i=0;i<Mutex_Height;i++) {
        ThreadBuffer[i] = new char[Width];
    }
    for(int i=0,m=0;i<Mutex_Height;i++) {
        for(int j=0;j<Width;j++) {
            ThreadBuffer[i][j] = InBuffer[m];
            m++;
        }
    }

    cout<<"Y_Size: "<<Width<<"*"<<Height<<endl;
    cout<<"UV_Size: "<<Width<<"*"<<Mutex_Height-Height<<endl;
    //线程一：局部置黑
    int T1_Width = 1440, T1_Height = 1080;   

    //线程2：中心画框
    int T2_Width = 1280, T2_Height = 960;
    int T2_RectWidth = T1_Width+T2_Width;
    int T2_RectHeight = T1_Height+T2_Height;

    //线程3：水平镜像
    int T3_Width = 1440, T3_Height = 1080;
    int T3_BegHeight = T1_Height+T2_Height;
    int T3_BegWidth = T1_Width+T2_Width;
    int T3_MidWidth = T3_BegWidth+T3_Width/2;
    int T3_SwapWidth = Width-1+T3_BegWidth;

    //线程4：旋转180度
    int T4_Width = 1440, T4_Height = 1080;
    int T4_yBegHeight = T1_Height+T2_Height;
    int T4_MidWidth = T4_Width/2; 
    int T4_uvBegHeight = Height+(T4_yBegHeight)/2;

    //多线程并行处理图像
    thread nv21_Thread1(nv21_Thread1, T1_Width, T1_Height, std::ref(ThreadBuffer));

    //将处理好的数据存入OutBuffer数组
    int n=0;
    for(int i=0;i<Mutex_Height;i++) {
        for(int j=0;j<Width;j++) {
            OutBuffer[n] = ThreadBuffer[i][j];
            n++;
        }
    }
    
    //输出图像
    ofstream out(OutPath,ios::binary);
    out.write(OutBuffer,Size);
    out.close();

    //释放申请的内存空间
    delete[] InBuffer;
    for(int i=0;i<Mutex_Height;i++) {
        delete[] ThreadBuffer[i];
    } 
    delete[] ThreadBuffer;
    delete[] OutBuffer;
    return 0;
}
