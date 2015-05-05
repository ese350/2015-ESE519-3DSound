# 2015-ESE519-3DSound
 This is the repository for ESE 519 project: 3D Sound. The goal is to produce binaural audio by mixing mono audio wavefile and performing auditory processing to obtain stereo waveform. 
 The binaural audio is integrated with virtual reality by using Oculus rift to visualize a virtual scenario created using Unity software.
 The link for the blog is :
 
 http://sankethshetty87.blogspot.com/

 The actual code for the project is in 3dsound_jack. It contains subfolders: Mixer & IMU.
 The makefile can be used as follows:
 to compile all source code: make all;
 to clean and remove all object files: make clean;

 The git repository also includes our ALSA code, which was used in initial stages but was abandoned due to latency issues. 
 However we have provided it for your review.
 The folder also includes the RTES_Oculus_Unity_files folder which contains the scenario used for virtual reality

 