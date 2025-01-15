#include <opencv2/opencv.hpp>
#include <windows.h>
#include <iostream>
#include <chrono>

struct ThreadData 
{
    cv::Mat* src;
    cv::Mat* dst;
    int startCol;
    int endCol;
};


DWORD WINAPI BlurThread(LPVOID lpParam)
{
    ThreadData* data = static_cast<ThreadData*>(lpParam);
    cv::Mat region = (*data->src)(cv::Rect(data->startCol, 0, data->endCol - data->startCol, data->src->rows));

    for (int i = 0; i < 2; ++i)
    {
        cv::GaussianBlur(region, region, cv::Size(15, 15), 0);
    }

    region.copyTo((*data->dst)(cv::Rect(data->startCol, 0, data->endCol - data->startCol, data->src->rows)));
    return 0;
}


void processImage(const std::string& inputFile, const std::string& outputFile, int numThreads)
{
    cv::Mat image = cv::imread(inputFile);
    if (image.empty())
    {
        std::cerr << "Error: cannot open the file\n";
        return;
    }

    cv::Mat outputImage = image.clone();

    int width = image.cols;
    int stripeWidth = width / numThreads;

    std::vector<HANDLE> threads(numThreads);
    std::vector<ThreadData> threadData(numThreads);


    for (int i = 0; i < numThreads; ++i)
    {
        threadData[i] = { &image, &outputImage, i * stripeWidth, (i == numThreads - 1) ? width : (i + 1) * stripeWidth };
        threads[i] = CreateThread(NULL, 0, BlurThread, &threadData[i], CREATE_SUSPENDED, NULL);
        
        if (threads[i] == NULL)
        {
            std::cerr << "Error on creating thread\n";
            return;
        }
    }

    for (HANDLE thread : threads)
    {
        ResumeThread(thread);
    }

    WaitForMultipleObjects(numThreads, threads.data(), TRUE, INFINITE);

    cv::imwrite(outputFile, outputImage);

    for (HANDLE thread : threads)
    {
        CloseHandle(thread);
    }
}

int main(int argc, char* argv[])
{
    setlocale(LC_ALL, "");

    if (argc != 5)
    {
        std::cerr << "Using " << argv[0] << "<input.bmp> <output.bmp> <numThreads> <numCores>\n";
        return 1;
    }

    std::string inputFile = argv[1];
    std::string outputFile = argv[2];
    int numThreads = std::stoi(argv[3]);
    int numCores = std::stoi(argv[4]);

    SetProcessAffinityMask(GetCurrentProcess(), numCores);

    auto startTime = std::chrono::high_resolution_clock::now();

    processImage(inputFile, outputFile, numThreads);

    auto endTime = std::chrono::high_resolution_clock::now();

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

    std::cout << "Время выполнения: " << duration << " миллисекунд.\n";

    return 0;
}
