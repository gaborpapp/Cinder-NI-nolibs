/*
 Copyright (c) 2012-2015, Gabor Papp

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cinder/Cinder.h"
#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "CinderOni.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OniBasicApp : public App
{
 public:
	static void prepareSettings( Settings *settings );
	void setup() override;
	void cleanup() override;

	void update() override;
	void draw() override;

	void mouseUp( MouseEvent event ) override;

 private:
	mndl::oni::OniCaptureRef mOniCaptureRef;
	gl::TextureRef mColorTexture, mDepthTexture;
};

void OniBasicApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize( 1280, 480 );
}

void OniBasicApp::setup()
{
	if ( openni::OpenNI::initialize() != openni::STATUS_OK )
	{
	    console() << openni::OpenNI::getExtendedError() << endl;
		quit();
	}

	try
	{
		mOniCaptureRef = mndl::oni::OniCapture::create( openni::ANY_DEVICE );
	}
	catch ( const mndl::oni::OniCapture::ExcFailedCreateColorStream &exc )
	{
		mndl::oni::OniCapture::Options options;
		options.mEnableColor = false;
		mOniCaptureRef = mndl::oni::OniCapture::create( openni::ANY_DEVICE, options );
	}
	catch ( const mndl::oni::ExcOpenNI &exc )
	{
		console() << exc.what() << endl;
		quit();
	}

	openni::VideoMode depthMode;
	depthMode.setResolution( 640, 480 );
	depthMode.setFps( 30 );
	depthMode.setPixelFormat( openni::PIXEL_FORMAT_DEPTH_1_MM );
	mOniCaptureRef->getDepthStreamRef()->setVideoMode( depthMode );

	if ( mOniCaptureRef->getColorStreamRef() )
	{
		openni::VideoMode colorMode;
		colorMode.setResolution( 640, 480 );
		colorMode.setFps( 30 );
		colorMode.setPixelFormat( openni::PIXEL_FORMAT_RGB888 );
		mOniCaptureRef->getColorStreamRef()->setVideoMode( colorMode );
	}

	mOniCaptureRef->start();
}

void OniBasicApp::cleanup()
{
	mOniCaptureRef->stop();
	openni::OpenNI::shutdown();
}

void OniBasicApp::update()
{
	if ( mOniCaptureRef->checkNewColorFrame() )
	{
		mColorTexture = gl::Texture::create( mOniCaptureRef->getColorImage() );
	}
	if ( mOniCaptureRef->checkNewDepthFrame() )
	{
		mDepthTexture = gl::Texture::create( mOniCaptureRef->getDepthImage() );
	}
}

void OniBasicApp::draw()
{
	gl::clear();
	gl::viewport( getWindowSize() );
	gl::setMatricesWindow( getWindowSize() );

	if ( mDepthTexture )
		gl::draw( mDepthTexture );
	if ( !mOniCaptureRef->getColorStreamRef() )
		gl::drawStringCentered( "No color stream available!", ivec2( 960, 240 ) );
	else if ( mColorTexture )
		gl::draw( mColorTexture, ivec2( 640, 0 ) );
}

void OniBasicApp::mouseUp( MouseEvent event )
{
	/*
	// toggle infrared video
	mNI.setVideoInfrared( ! mNI.isVideoInfrared() );
	*/
}

CINDER_APP( OniBasicApp, RendererGl, &OniBasicApp::prepareSettings )
