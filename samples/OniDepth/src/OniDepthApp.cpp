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
#include "cinder/params/Params.h"

#include "CinderOni.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class OniDepthApp : public App
{
 public:
	static void prepareSettings( Settings *settings );
	void setup() override;
	void cleanup() override;

	void update() override;
	void draw() override;

 private:
	mndl::oni::OniCaptureRef mOniCaptureRef;
	gl::TextureRef mDepthTexture;

	mndl::oni::OniCapture::DepthMode mDepthMode = mndl::oni::OniCapture::DepthMode::SCALED;
	bool mInvertDepth = false;

	params::InterfaceGlRef mParams;
};

void OniDepthApp::prepareSettings(Settings *settings)
{
	settings->setWindowSize( 640, 480 );
}

void OniDepthApp::setup()
{
	if ( openni::OpenNI::initialize() != openni::STATUS_OK )
	{
	    console() << openni::OpenNI::getExtendedError() << endl;
		quit();
	}

	mndl::oni::OniCapture::Options options;
	options.mEnableColor = false;

	try
	{
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

	mOniCaptureRef->start();

	mParams = params::InterfaceGl::create( "Parameters", ivec2( 200, 300 ) );
	mParams->addParam( "Depth mode", { "scaled", "histogram", "raw" }, (int *)( &mDepthMode ) ).updateFn(
			[ & ]() { mOniCaptureRef->setDepthMode( mDepthMode ); } );
	mParams->addParam( "Invert depth", &mInvertDepth ).updateFn(
			[ & ]() { mOniCaptureRef->invertDepth( mInvertDepth ); } );
}

void OniDepthApp::cleanup()
{
	mOniCaptureRef->stop();
	openni::OpenNI::shutdown();
}

void OniDepthApp::update()
{
	if ( mOniCaptureRef->checkNewDepthFrame() )
	{
		mDepthTexture = gl::Texture::create( mOniCaptureRef->getDepthImage() );
	}
}

void OniDepthApp::draw()
{
	gl::clear();
	gl::viewport( getWindowSize() );
	gl::setMatricesWindow( getWindowSize() );

	if ( mDepthTexture )
	{
		gl::draw( mDepthTexture );
	}

	mParams->draw();
}

CINDER_APP( OniDepthApp, RendererGl, OniDepthApp::prepareSettings )
