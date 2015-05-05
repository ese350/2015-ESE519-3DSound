#ifndef _CircularBuffer_h
#define _CircularBuffer_h

#define INITIAL 3000
#define MAXSIZE 1024
#define EXTRASIZE 4096

class CircularBuffer{
    /* If wptr == rptr, it is empty */
    /* Remember: capacity-1 is all you can store */
private:
    size_t rptr,wptr,capacity;
    unsigned short data[MAXSIZE+EXTRASIZE];
    
public:
    CircularBuffer(size_t cap){
        rptr = 0;
        wptr = 0;
        //printf("\r\n%u  %u",rptr,wptr);
        //fflush(stdout);
        capacity = MAXSIZE+EXTRASIZE;
        //clear();
    }
    
    ~CircularBuffer(){
        //delete[] data;
    }
    
    size_t writeSizeRemaining();
    void write(unsigned short *dataPtr, size_t samples);
    void writeOneSample(unsigned short dac_data);
    size_t readSizeRemaining();
    void read(unsigned short *dataPtr, size_t samples);
    unsigned short readOneSample();
    void clear();
    
};

size_t CircularBuffer::writeSizeRemaining() {
    return capacity-readSizeRemaining()-1;
}

void CircularBuffer::write(unsigned short *dataPtr, size_t samples) 
{
    /* Assumes enough space is avilable in data */    
    //Condition 1 if wptr+samples<capacity
    //Condition 2 else two for loops
    
    size_t sizeRemaining = capacity - wptr;
    if(sizeRemaining <= samples){
        size_t i=0;
        for(; wptr<capacity; i++) 
            data[wptr++] = dataPtr[i];
        for(wptr = 0; i < samples; i++)
            data[wptr++] = dataPtr[i];
            
    } else {
        for(size_t i=0; i<samples; i++)
            data[wptr++] = dataPtr[i];
    }
 //   printf("\r\nwptr: %u,rptr: %u",wptr,rptr);
//    fflush(stdout);
}

size_t CircularBuffer::readSizeRemaining(){ 
    return (wptr >= rptr) ? wptr - rptr : (capacity-rptr+wptr);
}

void CircularBuffer::read(unsigned short *buf, size_t samples) 
{
    /* Assumes enough space is avilable in data */
    
    //Condition 1 if wptr+samples<capacity
    //Condition 2 else two for loops

    size_t sizeRemaining = capacity - rptr;
    if(sizeRemaining <= samples){
        size_t i=0;
        for(; rptr < capacity; rptr++) {
           buf[i++] = data[rptr];
         }
        for(rptr = 0; i<samples; i++)
           buf[i] = data[rptr++];
    } else {
        for(size_t i=0; i < samples ; i++)
           buf[i] = data[rptr++];
    }
}

unsigned short CircularBuffer::readOneSample() {
    if(rptr>=capacity){
         rptr=0;
    }
    return data[rptr++]; 
}

/*void CircularBuffer::writeOneSample(unsigned short dac_data) {
    if(wptr>=capacity){
         wptr=0;
    }
    data[wptr++] = dac_data;
}*/

void CircularBuffer::clear() {
    wptr = capacity-1;
    rptr = 0;
    for(size_t i = 0; i<capacity; i=i+2){
        data[i] = INITIAL;
        data[i+1] = 0;
    }
}

#endif
