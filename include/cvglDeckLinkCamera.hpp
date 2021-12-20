
#pragma once

#include <iostream>
#include <thread>
#include <memory>
#include "DeckLinkAPI.h"
#include "platform.h"

#include "opencv2/imgproc.hpp"
#include "cvglCameraBase.hpp"
#include "cvglTheadPool.hpp"

class cvglDeckLinkCamera : public cvglCameraBase, public IDeckLinkInputCallback
{
public:
    cvglDeckLinkCamera(int index = 1);
    
    virtual ~cvglDeckLinkCamera(void) override
    {
        stop();
        Release();
    }
    
    int blackmagicScan( int index );
    int blackmagicScanAll();
    
    void start();
    void pause();
    void stop();
    
    
    HRESULT STDMETHODCALLTYPE QueryInterface (REFIID iid, LPVOID *ppv) override;
    ULONG   STDMETHODCALLTYPE AddRef () override;
    ULONG   STDMETHODCALLTYPE Release () override;
    
    HRESULT STDMETHODCALLTYPE VideoInputFormatChanged ( BMDVideoInputFormatChangedEvents notificationEvents,
                                                       IDeckLinkDisplayMode *newDisplayMode,
                                                       BMDDetectedVideoInputFormatFlags detectedSignalFlags ) override;
    
    HRESULT STDMETHODCALLTYPE VideoInputFrameArrived ( IDeckLinkVideoInputFrame* videoFrame,
                                                      IDeckLinkAudioInputPacket* audioPacket ) override;
    
    
private:
    
    bool blackmagic = false;
    
    BMDPixelFormat m_pixelFormat = bmdFormat8BitYUV;

    int32_t m_refCount;
    
    std::timed_mutex m_mutex;
    
    bool init_softlock = true;
    bool m_isplaying = false;
    
    IDeckLinkProfileAttributes*    m_deckLinkAttributes = NULL;
    IDeckLink*              m_deckLink = NULL;
    IDeckLinkInput*         m_deckLinkInput = NULL;
    
    size_t m_framecount = 0, m_next_framecount = 0;
    
    void checkFrameOrderStart(){ m_next_framecount++; }
    void checkFrameOrderEnde(){ m_framecount++;
        if( m_framecount != m_next_framecount)
            printf("******* camera backup *******\n");
    }
    
    std::unique_ptr<ThreadPool>  m_thread_pool;

    
};
