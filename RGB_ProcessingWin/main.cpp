#define _USE_MATH_DEFINES
#include <iostream>
#include <vector>
#include <math.h>
#include <cmath>
#include <fstream>
#include <random>
#include <chrono>
//Thread building blocks library
#include <tbb/task_scheduler_init.h>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/blocked_range2d.h>
#include <tbb/parallel_reduce.h>
//Free Image library
#include <FreeImagePlus.h>

using namespace std;
using namespace tbb;
using namespace chrono;

//FOR TESTING PURPOSES
ofstream csvFile;

//PROTOTYPES
int grainModification(int sampleSizeInput = 4, int grain = 8);
float gaussian2D(float initx, float inity, float x, float y, float sigma);
void gaussianBlurFuncBW(int sampleSizeInput, bool runSequential = false, uint64_t testNumber = 1);
void part2Func(float threshold = 50.0f, bool runThreshold = false, bool runRedPixel = false);
void grainTest(int sampleSize = 4, int minGrain = 1, int maxGrain = 100000000000, float step = 2);
void UIFunc();

int main()
{
	srand(time(NULL));
    int nt = task_scheduler_init::default_num_threads();
    task_scheduler_init T(nt);
	UIFunc();
	//grainTest(4, 1, 2000, 100);

	cout << "\n\n*****Program complete*****\n\n";

	return 0;
}

void UIFunc() 
{
	string input = "";
	//Part 1 (Greyscale Gaussian blur): -----------DO NOT REMOVE THIS COMMENT----------------------------//
	cout << "Would you like to run the gaussian blur function (y/n)?\n";
	cin >> input;
	transform(input.begin(), input.end(), input.begin(), ::tolower);
	if (input == "y" || input == "yes") {
		int sampleSize = -1;
		int tests = -1;
		bool seq = false;
		string input2 = "";
		cout << "What sample size would you like?\n";
		while (sampleSize < 1) {
			cin >> sampleSize;
		}
		cout << "Would you like to run the sequential version (y/n)?\n";
		transform(input2.begin(), input2.end(), input2.begin(), ::tolower);
		cin >> input2;
		if (input2 == "y" || input2 == "yes")
		{
			seq = true;
		}
		cout << "How many times would you like to run the test?\n";
		while (tests < 0) {
			cin >> tests;
		}
		gaussianBlurFuncBW(sampleSize, seq, tests);
	}
	//Part 2 (Colour image processing): -----------DO NOT REMOVE THIS COMMENT----------------------------//

	input = "";
	cout << "Would you like to run the Colour Image processing function (y/n)?\n";
	cin >> input;
	transform(input.begin(), input.end(), input.begin(), ::tolower);
	if (input == "y" || input == "yes") {
		int threshold = -1;
		int tests = -1;
		bool thres = false;
		bool redPix = false;
		string input2 = "";

		cout << "Would you like to run the threshold (y/n)?\n";
		transform(input2.begin(), input2.end(), input2.begin(), ::tolower);
		cin >> input2;
		if (input2 == "y" || input2 == "yes")
		{
			thres = true;
			cout << "What threshold  would you like?\n";
			while (threshold < 1) {
				cin >> threshold;
			}
		}
		cout << "Would you like to run the red dot (y/n)?\n";
		transform(input2.begin(), input2.end(), input2.begin(), ::tolower);
		cin >> input2;
		if (input2 == "y" || input2 == "yes")
		{
			redPix = true;
		}
		part2Func(threshold, thres, redPix);
	}
}

void grainTest(int sampleSize, int minGrain, int maxGrain, float step) 
{
	cout << "Running Grain Test\n\n";
	csvFile.open("SampleSize_" + to_string(sampleSize) + "-" 
		+ to_string(minGrain) + "_" + to_string(maxGrain) 
		+ "-grain_test-" + to_string(chrono::system_clock::to_time_t(chrono::system_clock::now()))+ ".csv");
	csvFile << "Grain Size" << "," << "Parallel Duration" << "\n";
	for (int i = minGrain; i <= maxGrain; i += step) 
	{
		cout << "Grain Size: " << i <<endl;
		int val = grainModification(sampleSize, i);
		csvFile << "Grain " << to_string(i) << "," << to_string(val) << "\n";
		//for proper increments starting at 1
		if (i == 1 && step > 1) i = 0;
	}
	csvFile.close();

	cout << "Grain test complete\n\n";

}

int grainModification(int sampleSizeInput, int grain)
{

	int sampleSize = sampleSizeInput;
	int sigma = 6 * sampleSize;
	// Setup and load input image dataset
	fipImage inputImage;
	inputImage.load("../Images/render_1_lowres.png");
	inputImage.convertToFloat(); //Converts image to black and white

	auto width = inputImage.getWidth();
	auto height = inputImage.getHeight();
	const float*  inputBuffer = (float*)inputImage.accessPixels();

	// Setup output image array
	fipImage outputImage;
	outputImage = fipImage(FIT_FLOAT, width, height, 32);
	float *outputBuffer = (float*)outputImage.accessPixels();

	// parallel_for version
	auto pt1 = high_resolution_clock::now();

	parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, grain, 0, width, width >> 2), [&](const blocked_range2d<uint64_t, uint64_t>& r) {

		auto y1 = r.rows().begin();
		auto y2 = r.rows().end();
		auto x1 = r.cols().begin();
		auto x2 = r.cols().end();

		int startingPosX = sampleSize;
		int startingPosY = sampleSize;
		for (uint64_t y = y1; y < y2; ++y)
		{
			for (uint64_t x = x1; x < x2; ++x)
			{
				float pixelValue = 0.0f;

				if (x <= sampleSize) startingPosX = 0;
				else startingPosX = sampleSize;

				if (y <= sampleSize) startingPosY = 0;
				else startingPosY = sampleSize;

				int timesToRun = 1;
				if (startingPosX == 0) timesToRun *= 2;
				if (startingPosY == 0) timesToRun *= 2;

				for (int compensation = 0; compensation < timesToRun; compensation++)
				{
					for (uint64_t nx = x - startingPosX; nx <= (x + sampleSize); nx++)
					{
						for (uint64_t ny = y - startingPosY; ny <= (y + sampleSize); ny++)
						{
							if ((ny > 0 && ny < height) && (nx > 0 && nx < width))
								pixelValue += (gaussian2D(x, y, nx, ny, sigma) * inputBuffer[ny * width + nx]);
							else
								pixelValue += (gaussian2D(x, y, x, y, sigma) * inputBuffer[y * width + x]);
						}
					}
				}
				outputBuffer[y * width + x] = pixelValue;
			}
		}
	});

	auto pt2 = high_resolution_clock::now();
	auto pt_dur = duration_cast<microseconds>(pt2 - pt1);
	return pt_dur.count();
}

float gaussian2D(float initx, float inity, float x, float y, float sigma)
{
	float sig = (sigma * sigma);
	float xvals = (initx - x)*(initx - x);
	float yvals = (inity - y)*(inity - y);
	float value = 1.0f / (2.0f * M_PI * sig * exp(-((xvals + yvals) / (2.0f * sig))));
	return value;
}

// Gaussian BW function
void gaussianBlurFuncBW(int sampleSizeInput, bool runSequential , uint64_t testNumber) {

	//desired number of tests and setup code
	uint64_t numTests = testNumber;
	int sampleSize = sampleSizeInput;
	int sigma = 6 * sampleSize;

	//cout to inform user of their choices
	cout << "\n\n***Running Gaussian Blur Test (BLACK & WHITE)***\n\n"
		<< "Number of tests = " << testNumber << endl
		<< "Desired Kernel Size = " << sampleSize << endl
		<< "Sigma Size = " << sigma << endl << endl;

	//save results if code is being run more than once
	if (numTests > 1)
	{
		csvFile.open(to_string(sampleSizeInput) + "_sampleSize.csv");
		csvFile << "Sequential Duration" << "," << "Parallel Duration" << "," << "Speedup" << "\n";
	}
	// Setup and load input image dataset
	fipImage inputImage;
	inputImage.load("../Images/render_1_lowres.png");
	inputImage.convertToFloat(); //Converts image to black and white

	auto width = inputImage.getWidth();
	auto height = inputImage.getHeight();
	const float*  inputBuffer = (float*)inputImage.accessPixels();

	// Setup output image array
	fipImage outputImage;
	outputImage = fipImage(FIT_FLOAT, width, height, 32);
	float *outputBuffer = (float*)outputImage.accessPixels();

	// Test sequential vs. parallel_for versions - run test multiple times and track speed-up
	double meanSpeedup = 0.0f;

	for (uint64_t testIndex = 0; testIndex < numTests; ++testIndex) {

		// Sequential version of gaussian blur function
		auto sLambda = [&](const blocked_range2d<uint64_t, uint64_t>& r) {
			auto y1 = r.rows().begin();
			auto y2 = r.rows().end();
			auto x1 = r.cols().begin();
			auto x2 = r.cols().end();

			int startingPosX = sampleSize;
			int startingPosY = sampleSize;
			for (uint64_t y = y1; y < y2; ++y)
			{
				for (uint64_t x = x1; x < x2; ++x)
				{
					float pixelValue = 0.0f;

					if (x <= sampleSize) startingPosX = 0;
					else startingPosX = sampleSize;

					if (y <= sampleSize) startingPosY = 0;
					else startingPosY = sampleSize;

					int timesToRun = 1;
					if (startingPosX == 0) timesToRun *= 2;
					if (startingPosY == 0) timesToRun *= 2;

					for (int compensation = 0; compensation < timesToRun; compensation++)
					{
						for (uint64_t nx = x - startingPosX; nx <= (x + sampleSize); nx++)
						{
							for (uint64_t ny = y - startingPosY; ny <= (y + sampleSize); ny++)
							{
								if ((ny > 0 && ny < height) && (nx > 0 && nx < width))
									pixelValue += (gaussian2D(x, y, nx, ny, sigma) * inputBuffer[ny * width + nx]);
								else
									pixelValue += (gaussian2D(x, y, x, y, sigma) * inputBuffer[y * width + x]);
							}
						}
					}
					outputBuffer[y * width + x] = pixelValue;
				}
			}
		};

		cout << "Running sequential gaussian blur BW now...\n";
		auto st1 = high_resolution_clock::now();

		//only runs sequential if desired
		if (runSequential) sLambda(blocked_range2d<uint64_t, uint64_t>(0, height, 0, width));

		else cout << "Sequential version not run...ignore\n";
		auto st2 = high_resolution_clock::now();
		auto st_dur = duration_cast<microseconds>(st2 - st1);
		cout << "sequential gaussian operation took = " << st_dur.count() << "\n\n\n";

		// parallel_for version

		cout << "Parallel Gaussian Function running now...\n";
		auto pt1 = high_resolution_clock::now();

		parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, 8, 0, width, width >> 2), [&](const blocked_range2d<uint64_t, uint64_t>& r) 
		{
			auto y1 = r.rows().begin();
			auto y2 = r.rows().end();
			auto x1 = r.cols().begin();
			auto x2 = r.cols().end();

			int startingPosX = sampleSize;
			int startingPosY = sampleSize;
			//accesses each pixel
			for (uint64_t y = y1; y < y2; ++y)
			{
				for (uint64_t x = x1; x < x2; ++x)
				{
					//pixel value for currently accessed pixel
					float pixelValue = 0.0f;

					//starting position adjustments
					if (x <= sampleSize) startingPosX = 0;
					else startingPosX = sampleSize;

					if (y <= sampleSize) startingPosY = 0;
					else startingPosY = sampleSize;

					int timesToRun = 1;
					if (startingPosX == 0) timesToRun *= 2;
					if (startingPosY == 0) timesToRun *= 2;
					//compensates just incase pixel is too close to edge
					for (int compensation = 0; compensation < timesToRun; compensation++)
					{
						//accesses surrounding pixels in kernel
						for (uint64_t nx = x - startingPosX; nx <= (x + sampleSize); nx++)
						{
							for (uint64_t ny = y - startingPosY; ny <= (y + sampleSize); ny++)
							{
								//checks if the surrounding pixel is within the image bounds
								if ((ny > 0 && ny < height) && (nx > 0 && nx < width))
									pixelValue += (gaussian2D(x, y, nx, ny, sigma) * inputBuffer[ny * width + nx]);
								else
									pixelValue += (gaussian2D(x, y, x, y, sigma) * inputBuffer[y * width + x]);
							}
						}
					}
					//1D array to 2D array
					outputBuffer[y * width + x] = pixelValue;
				}
			}
		});

		auto pt2 = high_resolution_clock::now();
		cout << "Test Complete" << endl;
		auto pt_dur = duration_cast<microseconds>(pt2 - pt1);
		cout << "parallel gaussian operation took = " << pt_dur.count() << "\n\n\n";

		double speedup = double(st_dur.count()) / double(pt_dur.count());
		cout << "Test " << testIndex << " speedup = " << speedup << endl;
		if (numTests > 1) csvFile << to_string(st_dur.count()) << "," << to_string(pt_dur.count()) << "," << to_string(speedup) << "\n";
		meanSpeedup += speedup;
	}
	meanSpeedup /= double(numTests);
	cout << "Mean speedup = " << meanSpeedup << endl;

	//closes CSV
	if (numTests > 1)csvFile.close();

	cout << "Saving parallel version of image...\n";

	outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputImage.convertTo32Bits();
	outputImage.save("grey_blurred.png");

	cout << "...done\n\n";
}

void part2Func(float threshold, bool runThreshold, bool runRedPixel)
{
	// Setup Input images
	fipImage inputImage;
	inputImage.load("../Images/render_1_lowres.png");

	fipImage secondImage;
	secondImage.load("../Images/render_2_lowres.png");
	
	//get first image width and height
	unsigned int width = inputImage.getWidth();
	unsigned int height = inputImage.getHeight();

	//total Pixels required for white pixel count
	float totalPixels = width * height;

	// Setup Output image array
	fipImage outputImage;
	outputImage = fipImage(FIT_BITMAP, width, height, 24);

	//2D Vector to hold the RGB colour data of an image
	vector<vector<RGBQUAD>> rgbValues;
	rgbValues.resize(height, vector<RGBQUAD>(width));


	cout << "Runnin parallel RGB subtraction\n";
	parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, 8, 0, width, width >> 2), [&](const blocked_range2d<uint64_t, uint64_t>& r) {
		//RGBQUAD values for currently accessed pixels in images 1 and 2
		RGBQUAD rgb;
		RGBQUAD rgb2;
		auto y1 = r.rows().begin();
		auto y2 = r.rows().end();
		auto x1 = r.cols().begin();
		auto x2 = r.cols().end();
		//Extract colour data from image and store it as individual RGBQUAD elements for every pixel
		for (uint64_t y = y1; y < y2; ++y)
		{
			for (uint64_t x = x1; x < x2; ++x)
			{
				inputImage.getPixelColor(x, y, &rgb); //Extract pixel(x,y) colour data and place it in rgb
				secondImage.getPixelColor(x, y, &rgb2);

				//sets the new values equal to the subtracted RGB values of the first -  second image
				rgbValues[y][x].rgbRed = abs(rgb.rgbRed - rgb2.rgbRed);
				rgbValues[y][x].rgbGreen = abs(rgb.rgbGreen - rgb2.rgbGreen);
				rgbValues[y][x].rgbBlue = abs(rgb.rgbBlue - rgb2.rgbBlue);

				if (runThreshold) //if the user wants to run the threshold to produce a white and black image
				{
					//if the value is above the threshold the pixel must be set to white
					if (rgbValues[y][x].rgbRed + rgbValues[y][x].rgbGreen + rgbValues[y][x].rgbBlue > threshold) 
					{
						rgbValues[y][x].rgbRed = 255;
						rgbValues[y][x].rgbGreen = 255;
						rgbValues[y][x].rgbBlue = 255;
					}
				}
				//must be done outside of threshold incase threshold is not run
				outputImage.setPixelColor(x, y, &rgbValues[y][x]);
			}
		}
	});
	
	//parallel reduce can return int. This is used to count how many white pixels are in the image (perfectly white).
	int whitePixelCounter = parallel_reduce(blocked_range2d<int, int>(0, height, 0, width), 0, [&](blocked_range2d< int, int> & range, int count) ->int 
	{
		for (int x = range.cols().begin(); x < range.cols().end(); x++) 
		{
			for (int y = range.rows().begin(); y < range.rows().end(); y++) 
			{
				if (rgbValues[y][x].rgbRed + rgbValues[y][x].rgbGreen + rgbValues[y][x].rgbBlue == 765) 
				{
					count++;
				}
			}
		}
		return count;
	}, [&](int x, int y) -> int {return x + y; });
	//white pixel calculations
	float whitePixelPercent = (whitePixelCounter / totalPixels) * 100.0f;
	cout << "White Pixels = " << whitePixelCounter << endl;
	cout << "White Pixel % = " << whitePixelPercent << endl;
	cout << "Finished Parallel\n";

	if (runRedPixel) {
		cout << "Now finding random red pixel: \n";
		//determines random red pixel location
		int randomX = rand() % width;
		int randomY = rand() % height;
		randomX = rand() % width;
		randomY = rand() % height;

		//Random red pixel
		rgbValues[randomY][randomX].rgbRed = 255.0f;
		rgbValues[randomY][randomX].rgbGreen = 0.0f;
		rgbValues[randomY][randomX].rgbBlue = 0.0f;
		//for debugging purposes
		cout << "Random Pixel should be at: " << randomX << "," << randomY << endl;
		outputImage.setPixelColor(randomX, randomY, &rgbValues[randomY][randomX]);


		//parallel for to loop through the image again to find the red pixel
		parallel_for(blocked_range2d<uint64_t, uint64_t>(0, height, 8, 0, width, width >> 2), [&](const blocked_range2d<uint64_t, uint64_t>& r) {
			auto y1 = r.rows().begin();
			auto y2 = r.rows().end();
			auto x1 = r.cols().begin();
			auto x2 = r.cols().end();

			//loops through the image in sections to get pixel data
			for (uint64_t y = y1; y < y2; ++y)
			{
				for (uint64_t x = x1; x < x2; ++x)
				{
					//returns true if the pixel is red
					if (rgbValues[y][x].rgbRed == 255.0f && rgbValues[y][x].rgbGreen == 0.0f && rgbValues[y][x].rgbBlue == 0.0f)
					{
						//cancels task once pixel is found
						if (task::self().cancel_group_execution())
						{
							//informs user via console where the Pixel was found
							cout << "Random Pixel found at " << x << "," << y << endl;
						}
					}
				}
			}
		});
	}
	//Save the processed image
	outputImage.convertToType(FREE_IMAGE_TYPE::FIT_BITMAP);
	outputImage.convertTo24Bits();
	outputImage.save("RGB_processed.png");
}