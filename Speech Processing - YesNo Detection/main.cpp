 // Sp_AS1_Yes_No.cpp : Defines the entry point for the console application.
/*
	Speech Processing CS 566: Assignment 01
	input: Yes/No voice data in text format using CoolEdit2000 Software
	output: to classify whether voice data in question is Yes or No.
			file_name_final_values - contain output result of analysis.
			file_name_normalised - contain normalised input
			file_name_features_extract - contain energy, zcr of frames
			file_name_segregated_sound - contain segregated voice data.
	Roll No: 214101058 MTech CSE'23
*/

#include "stdafx.h"
//#pragma warning (disable : 4996) //to disable fopen() warning
#include <stdio.h>
#include <stdlib.h> //atoi
#include <string.h>  //strcat, strcpy
#include <conio.h>	//getch,
//#include <math.h>	// ,
//#include <ctype.h>	// ,


//Common Settings
#define sizeFrame 100
#define scaleAmp 5000
#define countMaxFrame 400

#define initIgnoreHeaderLines 0			//change 
#define initIgnoreSamples 0				//change 
#define initNoiseFrames 5				//change 
#define thresholdNoiseToEnergyFactor 3	//change 
#define lastFourtyPercThreshold 30	//change

#define samplingRate 16000

//variable decalaration
//files stream
FILE *fp_ip, *fp_norm_ft, *fp_norm, *fp_norm_seg, *fp_final_op;							// file pointer for input stream and output stream.
const char filePath[] = "../data_input/";
char fileNameIp[100], completePathIp[300], completePathNorm[300], completePathNormFeature[300], completePathNormSeg[300], completePathFinOp[300];
char charsLineToRead[50];

//Speech
int samples[600000], maxAmp = 0;  //max samples in recording to consider
double normSamples[600000];  //max samples in recording to consider
long cntTotSamples=0, cntTotFrames=0;
//ZCR and ENERGY
float thresholdZCR=0, DCshift =0;
float cntZCR[400], avgZCR[400], avgEnergy[400];  
float totEnergy=0, noiseEnergy=0, initNoiseZCR=0, initNoiseAvgZCR=0;;
double thresholdEnergy=0;
// start and end marker, Frames
long start=0, end=0 ;

int main()
{
	long i, j;
// General Information to Display
	printf("---- WELCOME TO YES/NO Speech Detector ----\n");		
	printf("-Common Settings are : -\n");							
	printf(" Frame Size : %d\n", sizeFrame);						
	printf(" Amplitutde Value to Scale : %d\n", scaleAmp);			
	printf(" Max Frames to Consider : %d\n", countMaxFrame);		
	printf(" Intital Header Lines Ignore Count : %d\n", initIgnoreHeaderLines); 
	printf(" Intital Samples to Ignore : %d\n",initIgnoreSamples);	
	printf(" Intital Noise Frames Count : %d\n",initNoiseFrames);	
	printf(" Threshold Energy Factor : %d\n",thresholdNoiseToEnergyFactor); 
	printf(" last Fourty Perc Threshold : %d\n",lastFourtyPercThreshold); 
	printf("----------------\n\n");									
// Input File Name
	printf("=> Enter filename(100 char) (present in data_input folder) : ");
		scanf("%[^\n]", fileNameIp);
		//strcpy(fileNameIp, "yes_sample_1.txt");
	printf("\n-----------------------------------\n");

// Files Renaming and Path adjustment
	strcpy(completePathIp, filePath); strcpy(completePathNorm, filePath); strcpy(completePathNormFeature, filePath); strcpy(completePathNormSeg, filePath); strcpy(completePathFinOp, filePath);
	
	strcat(completePathIp, fileNameIp); 
	strcat(completePathNorm, fileNameIp);			strcat(completePathNorm, "_normalized.txt"); 
	strcat(completePathNormFeature, fileNameIp);	strcat(completePathNormFeature, "_norm_features_extract.txt");
	strcat(completePathNormSeg, fileNameIp);		strcat(completePathNormSeg, "_norm_segregated_sound.txt");
	strcat(completePathFinOp, fileNameIp);			strcat(completePathFinOp, "_final_values.txt");
	

// opening respective input and output file.
	fp_ip = fopen(completePathIp, "r");		
	fp_norm = fopen(completePathNorm, "w+");	
	fp_norm_ft = fopen(completePathNormFeature, "w");	
	fp_norm_seg = fopen(completePathNormSeg, "w");	
	fp_final_op = fopen(completePathFinOp, "w");
	
		if(fp_ip == NULL || fp_norm == NULL || fp_norm_ft == NULL || fp_norm_seg == NULL || fp_final_op==NULL ){
			perror("\nError: ");
			printf("\n File Names are Input, NormIp, NormFeature, FinalOp : \n%s, \n%s, \n%s \n%s \n", completePathIp, completePathNorm, fp_norm_ft, fp_final_op );
				fprintf(fp_final_op, "\nError: ");
				fprintf(fp_final_op, "\n File Names are Input, NormIp, NormFeature, FinalOp : \n%s, \n%s, \n%s \n%s \n", completePathIp, completePathNorm, fp_norm_ft, fp_final_op );
			getch();
			return EXIT_FAILURE;
		}
		fprintf(fp_final_op, "---- WELCOME TO YES/NO Speech Detector ----\n");
		fprintf(fp_final_op, "-Common Settings are : \n");
		fprintf(fp_final_op, " Frame Size : %d\n", sizeFrame);
		fprintf(fp_final_op, " Amplitutde Value to Scale : %d\n", scaleAmp);
		fprintf(fp_final_op, " Max Frames to Consider : %d\n", countMaxFrame);
		fprintf(fp_final_op, " Intital Header Lines Ignore Count : %d\n", initIgnoreHeaderLines);
		fprintf(fp_final_op, " Intital Noise Frames Count : %d\n",initNoiseFrames);
		fprintf(fp_final_op, " Intital Samples to Ignore : %d\n",initIgnoreSamples);
		fprintf(fp_final_op, " Threshold Energy Factor : %d\n",thresholdNoiseToEnergyFactor);
		fprintf(fp_final_op, " last Fourty Perc Threshold : %d\n",lastFourtyPercThreshold);
		fprintf(fp_final_op, "----------------\n\n");
	
//DC Shift and Normalizing 
	long totIgnore;
		 totIgnore=initIgnoreHeaderLines + initIgnoreSamples; //5 + 2 = 7
    // totIgnore=0;
	long sample_index = totIgnore+1; // till 7 samples ignored, sample count is 8, so to make array index 0 there is +1 
	long sample_index_norm = 0;
	double normFactor = 0;
	double normOutput = 0;
		while(!feof(fp_ip)){
			fgets(charsLineToRead, 50, fp_ip);
			cntTotSamples += 1 ;  

			if(cntTotSamples > totIgnore){
				sample_index_norm = cntTotSamples - sample_index;
				samples[sample_index_norm] = (int)atoi(charsLineToRead);
				DCshift += samples[sample_index_norm];
						if(abs(samples[sample_index_norm]) > maxAmp)
							maxAmp = abs(samples[sample_index_norm]);
			}
		}

	cntTotSamples = cntTotSamples - totIgnore;
	DCshift = DCshift/cntTotSamples;
	cntTotFrames = cntTotSamples/sizeFrame;

	normFactor = (double)scaleAmp/maxAmp;
	// saving in normalised file
	for(i=0; i<cntTotSamples; i++){
		normOutput = (double)(samples[i] - DCshift)*normFactor;
		normSamples[i]=normOutput;
		fprintf(fp_norm, "%lf\n", normSamples[i]);
	}

	printf("\n---- FileName : %s ----\n\n",fileNameIp);
	printf(" TOTAL SAMPLES : %d\n TOTAL FRAMES : %d\n DC SHIFT needed : %lf\n Maximum Amplitude : %d\n Normalization Factor : %lf\n ", cntTotSamples, cntTotFrames, DCshift, maxAmp, normFactor);
		fprintf(fp_final_op, "\n---- FileName : %s ----\n\n",fileNameIp );
		fprintf(fp_final_op, " TOTAL SAMPLES : %d\n TOTAL FRAMES : %d\n DC SHIFT needed : %lf\n Maximum Amplitude : %d\n Normalization Factor : %lf\n ", cntTotSamples, cntTotFrames, DCshift, maxAmp, normFactor);
			
//Frames ZCR and Energy.
	rewind(fp_norm);
	float s_i, s_i_1=1;
	for(i=0;i < cntTotFrames;i++)
		{
			cntZCR[i]=0;
			avgEnergy[i]=0;
			for(j=0;j < sizeFrame ;j++)
			{
				fgets(charsLineToRead, 50, fp_norm); // reading from normalised input
				s_i = (float)atof(charsLineToRead);
				avgEnergy[i] += (s_i*s_i);
				cntZCR[i] +=  (s_i_1*s_i < 0);
				s_i_1 = s_i;
			}	
			avgEnergy[i]/=sizeFrame;
			avgZCR[i] = cntZCR[i]/sizeFrame;
			totEnergy+=avgEnergy[i];
			fprintf(fp_norm_ft, "%f %0.1f \n", avgEnergy[i], cntZCR[i]);
		}
// calculating noise energy and threshold.
	for(i=0;i < initNoiseFrames;i++){
			initNoiseZCR+=cntZCR[i];
			initNoiseAvgZCR+=avgZCR[i];
			noiseEnergy+=avgEnergy[i];
	}
		thresholdZCR=initNoiseZCR/initNoiseFrames;
		//thresholdZCR=initNoiseAvgZCR/initNoiseFrames;    //Change
		//thresholdZCR*=0.9;								//Change
		noiseEnergy/=initNoiseFrames;
		thresholdEnergy=noiseEnergy*thresholdNoiseToEnergyFactor;

	printf("\n--------\n\n");
	printf("\n---- Number of Initial Noise Frames : %d ----\n\n", initNoiseFrames);
	printf(" Avg Noise Energy : %lf\n Total Noise ZCR : %0.1f\n Threshold ZCR : %0.1f\n Threshold Energy(avg noise*%d) : %0.5lf\n ", noiseEnergy, initNoiseZCR, thresholdZCR, thresholdNoiseToEnergyFactor, thresholdEnergy);
		fprintf(fp_final_op, "\n--------\n\n");
		fprintf(fp_final_op, "\n---- Number of Initial Noise Frames : %d ----\n\n", initNoiseFrames);
		fprintf(fp_final_op, " Avg Noise Energy : %lf\n Total Noise ZCR : %0.1f\n Threshold ZCR : %0.1f\n Threshold Energy(avg noise*%d) : %0.5lf\n ", noiseEnergy, initNoiseZCR, thresholdZCR, thresholdNoiseToEnergyFactor, thresholdEnergy);


//start and end marker of speech
	bool flag=false;		//to detect start mark
	// -3 to ignore last 3 frames.
	/*
	for(i=0;i < cntTotFrames-3;i++){		
			if(avgZCR[i+1] < thresholdZCR || avgEnergy[i+1] > thresholdEnergy){
				if(flag == false && avgEnergy[i+2] > thresholdEnergy && avgEnergy[i+3] > thresholdEnergy){
					start = i  ;
					flag = true;
				}
			}
			else if(flag == true && avgZCR[i] > thresholdZCR && avgEnergy[i] < thresholdEnergy && avgEnergy[i-1] < thresholdEnergy && avgEnergy[i-2] < thresholdEnergy){
				    end = i ;
					flag = false;
			}
	}
 */
//	/*
	for(int i=0; i<cntTotFrames-3; ++i){
			if(!flag && avgEnergy[i+1] > thresholdEnergy && avgEnergy[i+2] > thresholdEnergy && avgEnergy[i+3] > thresholdEnergy){
					start = i;
					flag = 1;
			}
			else if(flag && avgEnergy[i+1] <= thresholdEnergy && avgEnergy[i+2] <= thresholdEnergy&& avgEnergy[i+3] <= thresholdEnergy){
				end = i;
				flag = 0;
				break;
			}
		}
//	*/
	if(flag == 1) end = cntTotFrames - 5; //if end is not found then making the last frame - 3 as the end marker for the word
	long startSample= (start+1)*sizeFrame;
	long endSample= (end+1)*sizeFrame;
	long totFramesVoice = end-start+1;
	
	// saving segregated voice data in different file
	for(i=startSample-1; i<endSample; i++){
		fprintf(fp_norm_seg, "%lf\n", normSamples[i]);
	}
// calculating avg engery or zcr of voice data.
	float voiceAvgEnergy=0, voicecntZCR=0, voiceavgZCR=0;
	for(i=start;i <= end;i++){
			voiceAvgEnergy+=avgEnergy[i];
			voicecntZCR+=cntZCR[i];
			voiceavgZCR+=avgZCR[i];
	}
	voiceAvgEnergy/=totFramesVoice;
	voicecntZCR/=totFramesVoice;
	voiceavgZCR/=totFramesVoice;

		printf("\n--------\n\n");
		printf("\n---- Segregated Data Saved in File: %s ----\n\n", completePathNormSeg);
		printf(" START Frame : %ld\t END Frame : %ld\n Total Frames : %ld\n", start, end, totFramesVoice);
		printf(" Starting Sample : %ld\t Ending Sample : %ld\n", startSample, endSample);
		//printf("Starting time : %lf Ending time (seconds) : %lf\n", 1.0*(start+1)*sizeFrame/samplingRate, 1.0*(end+1)*sizeFrame/samplingRate);
		printf("\n Average Energy of the segregated voice data : %f", voiceAvgEnergy);
		printf("\n Average ZCR of the segregated voice data : %0.1f\n", voicecntZCR);

		printf("\n--------\n\n");
		printf("\n---- Final OutPut ----\n\n");

				fprintf(fp_final_op, "\n--------\n\n");
				fprintf(fp_final_op, "\n---- Segregated Data Saved in File: %s ----\n\n", completePathNormSeg);
				fprintf(fp_final_op, " START Frame : %ld\t END Frame : %ld\n Total Frames : %ld\n", start, end, totFramesVoice);
				fprintf(fp_final_op, " Starting Sample : %ld\t Ending Sample : %ld\n", startSample, endSample);
				fprintf(fp_final_op, "\n Average Energy of the segregated voice data : %f", voiceAvgEnergy);
				fprintf(fp_final_op, "\n Average ZCR of the segregated voice data : %0.1f\n", voicecntZCR);
				fprintf(fp_final_op, "\n--------\n\n");
				fprintf(fp_final_op, "\n---- Final OutPut ----\n\n");

	long lLowZCRcount=0, rHighZCRcount=0;
		for(i=end; i >= start;i--)
		{
			if(avgZCR[i] > voiceavgZCR && avgEnergy[i] < voiceAvgEnergy)
				rHighZCRcount++;
			if(avgZCR[start+end-i] < voiceavgZCR && avgEnergy[i] < voiceAvgEnergy)
				lLowZCRcount++;
		}
		printf("  lLowZCRcount = %ld\n", lLowZCRcount);
		printf("  rHighZCRcount = %d\n", rHighZCRcount);
			fprintf(fp_final_op, "  lLowZCRcount = %ld\n", lLowZCRcount);
			fprintf(fp_final_op, "  rHighZCRcount = %d\n", rHighZCRcount);
	long midframe;
		midframe=(end+start)/2;
		if(midframe == 0){
			printf("1===> Non speech signal\n");		fprintf(fp_final_op, "1===> Non speech signal\n");
		}
		else if(rHighZCRcount > 10*300/sizeFrame){
			printf("1===> Voice Data is YES signal\n");	fprintf(fp_final_op, "1===> Voice Data is YES signal\n");
		}
		else{
			printf("1===> Voice Data is NO signal\n"); 	fprintf(fp_final_op, "1===> Voice Data is NO signal\n");
		}
		printf("  is rHighZCRcount(%ld) > %d\n", rHighZCRcount,10*300/sizeFrame );
	
//	/*
	int cntFricatives = 0;
	//long LastFourtyPercStart = totFramesVoice - totFramesVoice*0.4;
	for ( i=start; i<=end-1; ++i){
		if(cntZCR[i] > 15 && cntZCR[i+1] > 15)
			cntFricatives++;
	}
	
	printf("\n Total Frames : %ld\t count fricatives : %d\n", totFramesVoice, cntFricatives);
		fprintf(fp_final_op, "\n Total Frames : %ld\t count fricatives : %d\n", totFramesVoice, cntFricatives);
	if(cntFricatives >= totFramesVoice * 0.3){	
		printf("2==> YES signal\n"); fprintf(fp_final_op, "2==> YES signal\n");}
	else{	
		printf("2==>  NO signal\n"); fprintf(fp_final_op, "2==>  NO signal\n");}
//	*/

//	/*
	float cntLastFourtyPercZCR = 0, lastFourtyPercTotFrames = totFramesVoice*0.4 ;
	long LastFourtyPercStart = totFramesVoice - lastFourtyPercTotFrames;
	for ( i=LastFourtyPercStart; i<=end; ++i){
			cntLastFourtyPercZCR += cntZCR[i];
	}
	float avgLastFourtyPercZCR = cntLastFourtyPercZCR/lastFourtyPercTotFrames;
	printf("\nLast 40perc frames Total : %f\n Total ZCR : %f\n Avg ZCR : %f\n", lastFourtyPercTotFrames, cntLastFourtyPercZCR, avgLastFourtyPercZCR);
			fprintf(fp_final_op, "\nLast 40perc frames Total : %f\n Total ZCR : %f\n Avg ZCR : %f\n", (float)totFramesVoice*0.4, cntLastFourtyPercZCR, avgLastFourtyPercZCR);
	if(avgLastFourtyPercZCR >= lastFourtyPercThreshold){	
		printf("3==>  YES signal\n"); fprintf(fp_final_op, "3==> YES signal\n");}
	else{	
		printf("3==>  NO signal\n");  fprintf(fp_final_op, "3==>  NO signal\n");}
//	*/

	printf("\n---------------- END -------------------\n"); fprintf(fp_final_op, "\n---------------- END -------------------\n");
	printf("\nPress Enter To Close\n");
	// closing file stream
	fclose(fp_ip);
	fclose(fp_norm);
	fclose(fp_norm_ft);
	fclose(fp_norm_seg);
	fclose(fp_final_op);
	getch();
	return 0;
}
