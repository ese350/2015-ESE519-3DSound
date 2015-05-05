#include <algorithm>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>

#include "../include/Mixer3D.h"
#include "../include/fft.h"
#include "../include/mit_hrtf_lib.h"
#include "../../CircularBuffer2.h"
#include "../../IMU/Location.h"

using namespace std;


struct wavFileData
{
  long n;
  int sampleRate;
  int bitDepth;
  int channels;
};

Location iLoc;

Mixer3D::Mixer3D(int bufSize, int smpRate, int bitD,  CircularBuffer<double> *circBuff):myCircBuff(circBuff), bufferSize(bufSize), sampleRate(smpRate), bitDepth(bitD)
{
  maxTemp = 0;
  outputLeft = new complex*[1];
  outputRight = new complex*[1];
  overlapLeft = new complex*[1];
  overlapRight = new complex*[1];
  
  //This is the variable used to store the data gotten from the world
  inputTempTemp1 = new complex[2*bufferSize];
  
  for (int i = 0; i < 1; i++)
    {
      overlapLeft[i] = new complex[bufferSize];
      overlapRight[i] = new complex[bufferSize];
      outputLeft[i] = new complex[2*bufferSize];
      outputRight[i] = new complex[2*bufferSize];
    }
  
  //This is for testing
  overlapInput = new complex[2 * bufferSize];
  
  //These are for filter fetch
  lFil = new short[2 * bufferSize];
  rFil = new short[2 * bufferSize];
  
  //These are for storing filters for processing
  clFil = new complex*[1];
  crFil = new complex*[1];
  for(int i = 0; i < 1; i++)
    {
      clFil[i] = new complex[2*bufferSize];
      crFil[i] = new complex[2*bufferSize];
    }
  
  fInput = new complex[2 * bufferSize];
  fFilter = new complex[2 * bufferSize];
  
  signFlag=0;//0 means the azimuth value is positive
  filterFlag=0;//0 means the filters for the current consecutive iterations are the same
  
  elevation = new int[1];
  for(int i =0; i<1;i++)
    {
      Azimuth[i]=iLoc.getAzimuth();
      previousAzimuth[i]=Azimuth[i];
      elevation[i]=90;
    }
  
}

int Mixer3D::HRTFLoading(int* pAzimuth, int* pElevation, unsigned int samplerate, unsigned int diffused, complex *&leftFilter, complex *&rightFilter)
{
  int size = mit_hrtf_get(pAzimuth, pElevation, 44100, diffused, lFil, rFil);
  for (int i = 0; i < size; i++)
    {
      leftFilter[i] = (double)(lFil[i]);
      rightFilter[i] = (double)(rFil[i]);
    }
  
  return size;
}

void Mixer3D::convolution(complex *input, complex *filter,complex *output, long nSig, long nFil, long nFFT)
{
	//Check for invalid inputs.
  if (input == NULL || filter == NULL)
    {
      cout << "Could not perform convolution on empty aaaa arrays!" << endl;
      return;
    }
  
  
  //Perform FFT on both input and filter.
  CFFT::Forward(input,fInput, (unsigned int)nFFT);
	CFFT::Forward(filter,fFilter, (unsigned int)nFFT);
	
	for (int i = 0; i < nFFT; i++)
	  output[i] = fInput[i] * fFilter[i];
	
	CFFT::Inverse(output, (unsigned int)nFFT);
}

void Mixer3D::stereoConvolution(complex *input, complex *leftFilter, complex *rightFilter, complex *leftOutput, complex *rightOutput, long nSIG, long nFIL, long nFFT)
{
  convolution(input, leftFilter, leftOutput, nSIG, nFIL, nFFT);
  convolution(input, rightFilter, rightOutput, nSIG, nFIL, nFFT);
}

void Mixer3D::overlapConvolution(float *ioDataLeft,float *ioDataRight)
{
  //Clearing the left and right channels
  for(int i = 0; i < bufferSize; i++)
    {
        ioDataLeft[i] = 0;
        ioDataRight[i] = 0;
    }
  
  int j =0;
  //looping through the audioObjetives and do the mixing one sequentially
  for(j = 0; j < 1; j++)
    {
      //dynamically calculating the Azimuth between every object and the player
     
      Azimuth[j] = iLoc.getAzimuth();
      if (Azimuth[j] >= 180){
	Azimuth[j] = Azimuth[j] - 360;
      }
      else if (Azimuth[j] <= -180){
	Azimuth[j] = Azimuth[j] + 360;

      }
      maxTemp = Azimuth[j];
      
      //indicating whether the Azimuth value is negative or not because the HRTFLoading function will change the sign of Azimuth if it is negative
      signFlag = 0;
      
      //indicating whether the filter has been changed for this coming iteration from the previous one
      filterFlag = 0;
      
      if (Azimuth[j] < 0)
	signFlag = 1;
      else
	signFlag = 0;
      
      //loading in input data for the iteration accordingly
      myCircBuff->read(inputTempTemp1,bufferSize);
      
      
      
      //zero padding the input for this iteration for the proper fft size
      for (int i = bufferSize; i < 2 * bufferSize; i++)
        {
	  inputTempTemp1[i] = 0;
        }
      
      //judging whether the filter has changed
      
      if(Azimuth[j] != previousAzimuth[j])
	{
	  filterFlag=1;
        }
      
      ///////////
      
      //if the filter has changed
      if(filterFlag)
        {
	  
	  nTaps = HRTFLoading(&Azimuth[j], &elevation[j], sampleRate, 1, clFil[j], crFil[j]);
	  
	  Azimuth[j] = maxTemp;
	  //zeroing padding it
	  for (int i = nTaps; i < 2 * bufferSize; i++)
            {
	      clFil[j][i] = 0;
	      crFil[j][i] = 0;
            }
	  
	  //judging whether the Azimuth value is negative
	  if (signFlag)
	    Azimuth[j] = -Azimuth[j];
	  
	  //recalculating the overlap part since the filter has been changed
	  stereoConvolution(overlapInput, clFil[j], crFil[j], outputLeft[j], outputRight[j], bufferSize, nTaps, 2 * bufferSize);
	  
	  //updating the overlap part for the next iteration
	  for (int i = 0; i < bufferSize; i++)
            {
	      overlapLeft[j][i] = outputLeft[j][i + bufferSize];
	      overlapRight[j][i] = outputRight[j][i + bufferSize];
            }
        }
      
      //////////
      
      //things needed to be done no matter whether the filter has been changed or not
      //doing convolution and get the main data for this iteration
        stereoConvolution(inputTempTemp1, clFil[j], crFil[j], outputLeft[j], outputRight[j], bufferSize, nTaps, 2 * bufferSize);
	
        //adding the overlap part with the main data
        for (int i = 0; i < bufferSize; i++)
	  {
            ioDataLeft[i] +=  (float((outputLeft[j][i].re())/2+ (overlapLeft[j][i].re())/2)/32768.0);
            ioDataRight[i] += (float((outputRight[j][i].re())/2 + (overlapRight[j][i].re())/2)/32768.0);
	    
	  }
	
        //updating the overlapInput for the next iteration for the correpsonding obejct
        for (int i = 0; i < bufferSize; i++)
	  {
            overlapInput[i] = inputTempTemp1[i];
	  }
        for (int i = bufferSize; i < 2 * bufferSize; i++)
	  {
            overlapInput[i] = 0;
	  }
	
        //updating the default overlap information for the next iteration if the filter won't be changed
        for(int i = 0 ; i < bufferSize; i++)
	  {
            overlapLeft[j][i]=outputLeft[j][i+bufferSize];
            overlapRight[j][i]=outputRight[j][i+bufferSize];
	  }
	
        //storing the Azimuth value in this iteration for the comparison for the next iteration so that
        //we can know that whether the filter needs to be changed in the next iteration.
	
    Azimuth[j] = maxTemp;
	previousAzimuth[j]=Azimuth[j];
	printf("\nAzimuth: %d ",Azimuth[j]);
    }
}
