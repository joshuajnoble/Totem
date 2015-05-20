#include "ThreeSixtyUnwrap.h"

void ThreeSixtyUnwrap::initUnwrapper(ofPtr<ofBaseVideoDraws> videoSource, int outputWidth, int outputHeight)
{
	this->videoSource = videoSource;

	if (XML.loadFile("UnwarperSettings.xml")){
		//printf("UnwarperSettings.xml loaded!\n");
	}
	else{
		//printf("Unable to load UnwarperSettings.xml!\nPlease check 'data' folder.\n");
	}

	//maxR_factor   = XML.getValue("MAXR_FACTOR", 0.96);
	//minR_factor   = XML.getValue("MINR_FACTOR", 0.16);
	maxR_factor = XML.getValue("MAXR_FACTOR", 0.95);
	minR_factor = XML.getValue("MINR_FACTOR", 0.45);
	angularOffset = XML.getValue("ROTATION_DEGREES", 0.0);
	unwarpedW = outputWidth;
	unwarpedH = outputHeight;

	// Interpolation method: 
	// 0 = CV_INTER_NN, 1 = CV_INTER_LINEAR, 2 = CV_INTER_CUBIC.
	interpMethod = (int)XML.getValue("INTERP_METHOD", 1);
	yWarpA = XML.getValue("R_WARP_A", 0.1850);
	yWarpB = XML.getValue("R_WARP_B", 0.8184);
	yWarpC = XML.getValue("R_WARP_C", -0.0028);

	//======================================
	// create data structures for unwarping
	blackOpenCV = cvScalarAll(0);
	warpedW = this->videoSource->getWidth();
	warpedH = this->videoSource->getHeight();

	int nWarpedBytes = warpedW * warpedH * 3;

	warpedImageOpenCV.allocate(warpedW, warpedH);
	warpedPixels = ofPtr<unsigned char>(new unsigned char[nWarpedBytes]);
	warpedIplImage = warpedImageOpenCV.getCvImage();
	cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedW, warpedH));

	int nUnwarpedPixels = unwarpedW * unwarpedH;
	int nUnwarpedBytes = unwarpedW * unwarpedH * 3;
	unwarpedImage.allocate(unwarpedW, unwarpedH, OF_IMAGE_COLOR);
	unwarpedPixels.allocate(unwarpedW, unwarpedH, 3);

	unwarpedImageOpenCV.allocate(unwarpedW, unwarpedH);
	unwarpedImageOpenCV.setROI(0, 0, unwarpedW, unwarpedH);
	unwarpedIplImage = unwarpedImageOpenCV.getCvImage();

	srcxArrayOpenCV.allocate(unwarpedW, unwarpedH);
	srcyArrayOpenCV.allocate(unwarpedW, unwarpedH);
	srcxArrayOpenCV.setROI(0, 0, unwarpedW, unwarpedH);
	srcyArrayOpenCV.setROI(0, 0, unwarpedW, unwarpedH);

	xocvdata = (float*)srcxArrayOpenCV.getCvImage()->imageData;
	yocvdata = (float*)srcyArrayOpenCV.getCvImage()->imageData;

	playerScaleFactor = (float)(ofGetHeight() - unwarpedH) / (float)warpedH;
	savedWarpedCx = warpedCx = XML.getValue("CENTERX", warpedW / 2.0);
	savedWarpedCy = warpedCy = XML.getValue("CENTERY", warpedH / 2.0);
	savedAngularOffset = angularOffset;

	computePanoramaProperties();
	computeInversePolarTransform();
}

void ThreeSixtyUnwrap::update()
{
	this->videoSource->update();
	if (this->videoSource->isFrameNew())
	{
		if (_bCenterChanged || _bAngularOffsetChanged)
		{
			//XML.setValue("CENTERX", warpedCx);
			//XML.setValue("CENTERY", warpedCy);
			//XML.setValue("ROTATION_DEGREES", angularOffset);
			computePanoramaProperties();
			computeInversePolarTransform();

			_bAngularOffsetChanged = false;
			_bCenterChanged = false;
		}

		memcpy(warpedPixels.get(), this->videoSource->getPixels(), warpedW*warpedH * 3);
		warpedIplImage->imageData = (char*)warpedPixels.get();

		cvSetImageROI(warpedIplImage, cvRect(0, 0, warpedIplImage->width, warpedIplImage->height));

		cvRemap(warpedIplImage,
			unwarpedIplImage,
			srcxArrayOpenCV.getCvImage(),
			srcyArrayOpenCV.getCvImage(),
			interpMethod | CV_WARP_FILL_OUTLIERS, blackOpenCV);

		unwarpedPixels.setFromPixels((unsigned char*)unwarpedIplImage->imageData, unwarpedIplImage->width, unwarpedIplImage->height, 3);
		unwarpedPixels.mirror(true, false);

		unwarpedImage.setFromPixels(unwarpedPixels.getPixels(), unwarpedW, unwarpedH, OF_IMAGE_COLOR, true);
		unwarpedImage.update();
	}
}

void ThreeSixtyUnwrap::close()
{
	this->warpedPixels.reset();
	this->unwarpedImageOpenCV.clear();
	this->unwarpedImage.clear();
	this->unwarpedPixels.clear();
	this->srcxArrayOpenCV.clear();
	this->srcyArrayOpenCV.clear();
}

//=============================================
void ThreeSixtyUnwrap::computePanoramaProperties()
{
	maxR = warpedH * maxR_factor / 2;
	minR = warpedH * minR_factor / 2;
}


//Used for the by hand portion and OpenCV parts of the shootout. 
//For the by hand, use the normal unwarpedW width instead of the step
//For the OpenCV, get the widthStep from the CvImage and use that for quarterstep calculation
//=============================================
void ThreeSixtyUnwrap::computeInversePolarTransform()
{

	// we assert that the two arrays have equal dimensions, srcxArray = srcyArray
	float radius, angle;
	float circFactor = 0 - TWO_PI / (float)unwarpedW;
	float difR = maxR - minR;
	int   dstRow, dstIndex;

	xocvdata = (float*)srcxArrayOpenCV.getCvImage()->imageData;
	yocvdata = (float*)srcyArrayOpenCV.getCvImage()->imageData;

	for (int dsty = 0; dsty<unwarpedH; dsty++){
		float y = ((float)dsty / (float)unwarpedH);
		float yfrac = yWarpA*y*y + yWarpB*y + yWarpC;
		yfrac = std::min(1.0f, std::max(0.0f, yfrac));

		radius = (yfrac * difR) + minR;
		dstRow = dsty * unwarpedW;

		for (int dstx = 0; dstx<unwarpedW; dstx++){
			dstIndex = dstRow + dstx;
			angle = ((float)dstx * circFactor) + (DEG_TO_RAD * angularOffset);

			xocvdata[dstRow + dstx] = warpedCx + radius*cosf(angle);
			yocvdata[dstRow + dstx] = warpedCy + radius*sinf(angle);
		}
	}

	srcxArrayOpenCV.setFromPixels(xocvdata, unwarpedW, unwarpedH);
	srcyArrayOpenCV.setFromPixels(yocvdata, unwarpedW, unwarpedH);
}