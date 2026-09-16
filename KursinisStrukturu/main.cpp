#include <iostream>
#include <fstream>
#include <chrono>
#include <algorithm>
using namespace std;
long long uzkeista;
int k5R[5000];
int k10R[10000];
int k50R[50000];
int k100R[100000];
int k5D[5000];
int k10D[10000];
int k50D[50000];
int k100D[100000];
int k5M[5000];
int k10M[10000];
int k50M[50000];
int k100M[100000];
void loadFromFile(const char* filename, int arr[], int size) {
    ifstream in(filename);
    if (!in) {
        cout << "Nepavyko atidaryti failo: " << filename << endl;
        return;
    }
    for (int i = 0; i < size; i++) {
        in >> arr[i];
    }
    in.close();
}
void getfileshere() {
    loadFromFile("5kSkaiciu.txt",   k5R,   5000);
    loadFromFile("10kSkaiciu.txt",  k10R,  10000);
    loadFromFile("50kSkaiciu.txt",  k50R,  50000);
    loadFromFile("100kSkaiciu.txt", k100R, 100000);
    loadFromFile("5kSkaiciuDid.txt",   k5D,   5000);
    loadFromFile("10kSkaiciuDid.txt",  k10D,  10000);
    loadFromFile("50kSkaiciuDid.txt",  k50D,  50000);
    loadFromFile("100kSkaiciuDid.txt", k100D, 100000);
    loadFromFile("5kSkaiciuMaz.txt",   k5M,   5000);
    loadFromFile("10kSkaiciuMaz.txt",  k10M,  10000);
    loadFromFile("50kSkaiciuMaz.txt",  k50M,  50000);
    loadFromFile("100kSkaiciuMaz.txt", k100M, 100000);
    cout << "Visi duomenys nuskaityti sekmingai.\n";
}
void insertionSort(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            uzkeista++;
            j--;
        }
        arr[j + 1] = key;
    }
}
int partition(int arr[], int low, int high) {
    int pivot = arr[(low + high) / 2];
    int i = low, j = high;
    while (i <= j) {
        while (arr[i] < pivot) i++;
        while (arr[j] > pivot) j--;
        if (i <= j) {
            swap(arr[i], arr[j]);
            i++; j--;
        }
        else uzkeista++;
    }
    return i;
}
void quickSort(int arr[], int low, int high) {
    if (low < high) {
        int index = partition(arr, low, high);
        quickSort(arr, low, index - 1);
        quickSort(arr, index, high);
    }
}
void laikas(int *masyvas, int dydis, string duomenys) {
    int *tempInsertion = new int[dydis];
    int *tempQuick = new int[dydis];
    copy(masyvas, masyvas + dydis, tempInsertion);
    copy(masyvas, masyvas + dydis, tempQuick);
    uzkeista = 0;
    auto start = chrono::high_resolution_clock::now();
    insertionSort(tempInsertion, dydis);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> duration = end - start;
    cout << "-----------------------------------------------------------------------------\n";
    cout << "Rusuojama is " << dydis << " duomenu lenteles kuri yra " << duomenys << "\n";
    cout << "Insertion sort rikiavimo trukme: " << duration.count() << " ms\n";
    cout << "Insertion sort rikiavimo sukeitimai: " << uzkeista << "\n";
    uzkeista = 0;
    auto startQS = chrono::high_resolution_clock::now();
    quickSort(tempQuick, 0, dydis - 1);
    auto endQS = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> durationQS = endQS - startQS;
    cout << "Quick sort rikiavimo trukme:     " << durationQS.count() << " ms\n";
    cout << "Quick sort rikiavimo sukeitimai:     " << uzkeista << "\n";
    delete[] tempInsertion;
    delete[] tempQuick;
}
int main() {
    getfileshere();
    laikas(k5R, 5000, "NESURUSIUOTA");
    laikas(k10R, 10000, "NESURUSIUOTA");
    laikas(k50R, 50000, "NESURUSIUOTA");
    laikas(k100R, 100000, "NESURUSIUOTA");
    laikas(k5D, 5000, "SURUSIUOTA DIDEJIMO TVARKA");
    laikas(k10D, 10000, "SURUSIUOTA DIDEJIMO TVARKA");
    laikas(k50D, 50000, "SURUSIUOTA DIDEJIMO TVARKA");
    laikas(k100D, 100000, "SURUSIUOTA DIDEJIMO TVARKA");
    laikas(k5M, 5000, "SURUSIUOTA MAZEJIMO TVARKA");
    laikas(k10M, 10000, "SURUSIUOTA MAZEJIMO TVARKA");
    laikas(k50M, 50000, "SURUSIUOTA MAZEJIMO TVARKA");
    laikas(k100M, 100000, "SURUSIUOTA MAZEJIMO TVARKA");
    return 0;
}