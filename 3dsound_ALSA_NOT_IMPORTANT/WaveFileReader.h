/* Read the wavefile */
#include "CircularBuffer.h"

typedef struct uFMT_STRUCT {
  short comp_code;
  short num_channels;
  unsigned sample_rate;
  unsigned avg_Bps;
  short block_align;
  short sig_bps;
} FMT_STRUCT;

class WaveFileReader {

private:
    FILE * wavefile;
    bool isRepeat;
    unsigned dataChunkSize;
    long int startofdata;
    void readHeader();
    FMT_STRUCT wav_format;

public:
    WaveFileReader( FILE *, bool);
    float readSamplePeriod();
    size_t read(unsigned short *buf, int size);
};

WaveFileReader::WaveFileReader(FILE *wavefile, bool isRepeat){
    this->wavefile = wavefile;
    this->isRepeat = isRepeat;
    readHeader();
}

float WaveFileReader::readSamplePeriod(){
    return 1000000/(wav_format.sample_rate);
}

void WaveFileReader::readHeader(){
    unsigned chunk_id, data, chunk_size;
    fread(&chunk_id,4,1,wavefile);
    fread(&chunk_size,4,1,wavefile);
    
    while (!feof(wavefile)){
        switch (chunk_id) {
            case 0x46464952:
                fread(&data,4,1,wavefile);
                break;
            case 0x20746d66:
                fread(&wav_format,sizeof(wav_format),1,wavefile); 
                if (chunk_size > sizeof(wav_format))
                    fseek(wavefile,chunk_size-sizeof(wav_format),SEEK_CUR);
                break;
            case 0x5453494c:
                fseek(wavefile,chunk_size,SEEK_CUR);
                break;
            case 0x61746164:
              startofdata = ftell(wavefile);
              dataChunkSize = chunk_size;
              fseek(wavefile,chunk_size,SEEK_CUR);
              break;
          default:
            data=fseek(wavefile,chunk_size,SEEK_CUR);
            break;
        }
        fread(&chunk_id,4,1,wavefile);
        fread(&chunk_size,4,1,wavefile);
    }
    fseek(wavefile,startofdata,SEEK_SET);
}

size_t WaveFileReader::read(unsigned short *circBuf, int size){
    unsigned channel;
    short unsigned dac_data;
    long long slice_value;
    char *slice_buf;
    short *data_sptr;
    unsigned char *data_bptr;
    int *data_wptr;
    
    slice_buf=(char *)malloc(wav_format.block_align);
    if (!slice_buf) {
      printf("Unable to malloc slice buffer");
      exit(1);
    }

    for (size_t i=0; i<size; i++){
    // while(1){
        fread(slice_buf,wav_format.block_align,1,wavefile);
        if (feof(wavefile)) {
            if(isRepeat){
                fseek(wavefile,startofdata,SEEK_SET);
                printf("Reached end of file, reapeat is on");
            }else{  
                printf("Reached end of file, reapeat is off");
                return 0;
            }
        }
        data_sptr=(short *)slice_buf;     // 16 bit samples
        data_bptr=(unsigned char *)slice_buf;     // 8 bit samples
        data_wptr=(int *)slice_buf;     // 32 bit samples
        slice_value=0;
        for (channel=0;channel<wav_format.num_channels;channel++) {
            switch (wav_format.sig_bps) {
                case 16:
                    slice_value+=data_sptr[channel];
                    break;
                case 32:
                    slice_value+=data_wptr[channel];
                    break;
                case 8:
                  slice_value+=data_bptr[channel];
                  break;
            }
        }
        slice_value/=wav_format.num_channels;
        switch (wav_format.sig_bps) {
            case 8:     
                slice_value<<=8;
                break;
            case 16:
                slice_value+=32768;
                break;
            case 32: 
                slice_value>>=16;
                slice_value+=32768;
                break;
        }
        dac_data=(short unsigned)slice_value;
        circBuf[i] = dac_data;
    }
    free(slice_buf);
    return size;
}
