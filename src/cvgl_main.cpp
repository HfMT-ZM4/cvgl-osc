
#include "cvglMainProcess.hpp"
#include "cvglCVCamera.hpp"
#include "cvglDeckLinkCamera.hpp"


int main( void )
{
    
    std::cout << "hardware_concurrency " << std::thread::hardware_concurrency() << std::endl;
    std::string sendIp = "192.168.100.1";

    cvglMainProcess app;
    app.init(8888, 9999, sendIp);

    cvglDeckLinkCamera bm_cam(2);
    cvglDeckLinkCamera bm_cam2(4); // 2 & 4
    
    cout << "blackmagic " << bm_cam.hasCamera() << " checking cv cam" << endl;
    cvglCVCamera cvcam(0);
   
    if( bm_cam.hasCamera() )
    {
        cout << "found blackmagic 2 " << endl;
        bm_cam.setProcessFrameCallback( [&app](cv::UMat& mat) { app.processFrame(mat, 1); } );
        app.context.setupWindow( bm_cam.getWidth(), bm_cam.getHeight() );
        bm_cam.start();


    }

    if( bm_cam2.hasCamera() )
    {
       cout << "found blackmagic 4" << endl;
       bm_cam2.setProcessFrameCallback( [&app](cv::UMat& mat) { app.processFrame(mat, 2); } );
       bm_cam2.start();

       if( !bm_cam.hasCamera() )
       {
           app.context.setupWindow( bm_cam2.getWidth(), bm_cam2.getHeight() );
           app.useCameraID(2);
       }
    }

    if( cvcam.hasCamera() )
    {
        cout << "doing cv camera " << endl;
        cvcam.setProcessFrameCallback( [&app](cv::UMat& mat){ app.processFrame(mat, 3); } );
        cvcam.start();
    }

    if( !bm_cam.hasCamera() && !bm_cam2.hasCamera() )
    {
        if(cvcam.hasCamera())
        {
            app.context.setupWindow( cvcam.getWidth(), cvcam.getHeight() );
            app.useCameraID(3);
        }
        else
        {
            app.close();
            return -1;
        }

    }

    
    
    
  //  app.context.setupWindow( 1920, 1080 );
    
    std::string shader_path = "/home/rama/animism-cvgl/src/";

    if( !app.context.loadShaderFiles( shader_path + "vertex.vs", shader_path + "fragment.fs" ) ){
        cout << "failed to load shaders" << endl;
        return -1;
    }
    
    // start UDP server
    app.start();
    
    app.initObjs();
    
    app.initCues();

    cout << "starting draw loop " << endl;

    // main GL loop`
    while( !app.context.shouldClose() )
    {
        app.draw();
    }
    
  //  cvcam.stop();
//    bm_cam.stop();
    
    app.close();
    
    return 0;
}


