#include <iostream>
#include <stdio.h>
#include<string.h>
#include<string>
using namespace std;


int main(int argc, char* argv[]) {
    size_t n = 10;
    char* arr = new char[n];

    cout << sizeof(arr) << endl;    // 输出指针大小，64位的操作系统，指针大小为 8 字节

    memset(arr, 0, n);              // 初始化分配的内存

    cout << strlen(arr) << endl;    // 从 arr 指针指向的内存单元开始，遇到 '\0' 字符结束

    delete[] arr;   // 释放分配的堆区内存
    return 0;
}