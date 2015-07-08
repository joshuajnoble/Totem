#include "CylinderDisplay.h"
#include "Utils.h"

namespace
{
	const int CYLINDER_PIECE_WIDTH = 44;
	const int CYLINDER_PIECE_HEIGHT = 2;
	const int PIECE_TEXCOORD_WIDTH = 720;
	const int NEO_PIXELS_COUNT = 45;

	class ofRay {
	public:
		ofRay() {}
		ofRay(const ofVec3f &aOrigin, const ofVec3f &aDirection) : mOrigin(aOrigin) { setDirection(aDirection); }

		void            setOrigin(const ofVec3f &aOrigin) { mOrigin = aOrigin; }
		const ofVec3f&    getOrigin() const { return mOrigin; }

		void setDirection(const ofVec3f &aDirection) {
			mDirection = aDirection;
			mInvDirection = ofVec3f(1.0f / mDirection.x, 1.0f / mDirection.y, 1.0f / mDirection.z);
			mSignX = (mDirection.x < 0.0f) ? 1 : 0;
			mSignY = (mDirection.y < 0.0f) ? 1 : 0;
			mSignZ = (mDirection.z < 0.0f) ? 1 : 0;
		}
		const ofVec3f&    getDirection() const { return mDirection; }
		const ofVec3f&    getInverseDirection() const { return mInvDirection; }

		char    getSignX() const { return mSignX; }
		char    getSignY() const { return mSignY; }
		char    getSignZ() const { return mSignZ; }

		ofVec3f calcPosition(float t) const { return mOrigin + mDirection * t; }

		bool calcTriangleIntersection(const ofVec3f &vert0, const ofVec3f &vert1, const ofVec3f &vert2, float *result) const;
		bool calcPlaneIntersection(const ofVec3f &origin, const ofVec3f &normal, float *result) const;

		friend class ofMeshFace;

	protected:
		ofVec3f    mOrigin;
		ofVec3f    mDirection;
		// these are helpful to certain ray intersection algorithms
		char    mSignX, mSignY, mSignZ;
		ofVec3f    mInvDirection;
	};

	// algorithm from "Fast, Minimum Storage Ray-Triangle Intersection"
	bool ofRay::calcTriangleIntersection(const ofVec3f &vert0, const ofVec3f &vert1, const ofVec3f &vert2, float *result) const
	{

		ofVec3f edge1, edge2, tvec, pvec, qvec;
		float det;
		float u, v;
		const float EPSILON = 0.000001f;

		edge1 = vert1 - vert0;
		edge2 = vert2 - vert0;

		pvec = getDirection().getCrossed(edge2);
		det = edge1.dot(pvec);

#if 0 // we don't want to backface cull
		if (det < EPSILON)
			return false;
		tvec = getOrigin() - vert0;

		u = tvec.dot(pvec);
		if ((u < 0.0f) || (u > det))
			return false;

		qvec = tvec.getCrossed(edge1);
		v = getDirection().dot(qvec);
		if (v < 0.0f || u + v > det)
			return false;

		*result = edge2.dot(qvec) / det;
		return true;
#else
		if (det > -EPSILON && det < EPSILON)
			return false;

		float inv_det = 1.0f / det;
		tvec = getOrigin() - vert0;
		u = tvec.dot(pvec) * inv_det;
		if (u < 0.0f || u > 1.0f)
			return false;

		qvec = tvec.getCrossed(edge1);

		v = getDirection().dot(qvec) * inv_det;
		if (v < 0.0f || u + v > 1.0f)
			return 0;

		*result = edge2.dot(qvec) * inv_det;
		return true;
#endif
	}

	bool ofRay::calcPlaneIntersection(const ofVec3f &planeOrigin, const ofVec3f &planeNormal, float *result) const
	{
		float denom = planeNormal.dot(getDirection());

		if (denom != 0.0f){
			*result = planeNormal.dot(planeOrigin - getOrigin()) / denom;
			return true;
		}
		return false;
	}
}

// ********************************************************************************************************************
void CylinderDisplay::initCylinderDisplay(int width, int height)
{
	this->windowWidth = width;
	this->windowHeight = height;
	this->totemVideoSource.reset();
}


// ********************************************************************************************************************
void CylinderDisplay::allocateBuffers()
{
	this->warpedW = this->totemVideoSource->getWidth();
	this->warpedH = this->totemVideoSource->getHeight();

	/// set up the cylinder
	float scale = this->windowHeight / (float)warpedH / 4;
	auto radius = roundf(warpedW * scale);
	auto height = roundf(warpedH * 2 * PI * scale);
	cylinder.set(radius, height, 120, 60, 0, false);
	cylinder.mapTexCoords(0, 0, warpedW, warpedH);

	this->cylinderCircumference = radius * 2 * PI;

	//createCylinderPiece(leftCylinderPiece, warpedW, 1080 * 2, CYLINDER_PIECE_WIDTH);
	//createCylinderPiece(rightCylinderPiece, warpedW, 1080 * 2, CYLINDER_PIECE_WIDTH);

	//finder.setup("haarcascade_mcs_upperbody.xml");
	//finder.setPreset(ofxCv::ObjectFinder::Accurate);
	//finder.setMaxSizeScale(3.0);

	isDrawingLeftCylinder = false;
	isDrawingRightCylinder = false;
}


// ********************************************************************************************************************
void CylinderDisplay::setTotemVideoSource(ofPtr<ofBaseVideoDraws> videoSource)
{
	this->totemVideoSource = videoSource;
	this->allocateBuffers();
}


// ********************************************************************************************************************
ofPtr<ofBaseVideoDraws> CylinderDisplay::getTotemVideoSource()
{
	return this->totemVideoSource;
}


// ********************************************************************************************************************
void CylinderDisplay::DoWelcome(const string& eventName)
{
	introPlaylist.addKeyFrame(Playlist::Action::tween(6000.0f, &this->viewRotationAngle, this->viewRotationAngle + 360.0f));
	if (eventName.length())
	{
		introPlaylist.addKeyFrame(Playlist::Action::event(this, eventName));
	}
}


// ********************************************************************************************************************
void CylinderDisplay::SetViewAngle(float angle, bool animate)
{
	if (this->totemVideoSource && animate)
	{
		// Fix the rotation speed instead of having a fixed duration no matter how far we rotate.
		while (this->viewRotationAngle >= 360)
		{
			this->viewRotationAngle -= 360;
		}

		float duration = 6000.0f / 360.0 * std::abs(this->viewRotationAngle - angle);
		introPlaylist.addKeyFrame(Playlist::Action::tween(duration, &this->viewRotationAngle, angle));
	}
	else
	{
		this->viewRotationAngle = angle;
	}
}


// ********************************************************************************************************************
void CylinderDisplay::update()
{
	introPlaylist.update();

	if (this->totemVideoSource)
	{
		this->totemVideoSource->update();
		if (this->totemVideoSource->isFrameNew())
		{
		}
	}
}


// ********************************************************************************************************************
void CylinderDisplay::draw()
{
	if (this->scale != 1.0)
	{
		ofPushMatrix();
		ofScale(this->scale, this->scale);
	}

	ofSetColor(255);
	//this->fboOutput.draw(0, 0);
	this->drawTexturedCylinder();

	//if (isDrawingLeftCylinder)
	//{
	//	ofPushMatrix();
	//	ofTranslate(430, (ofGetHeight() / 2), 300);
	//	ofRotateY(135);
	//	drawLeftCylinder();
	//	ofPopMatrix();
	//}

	//if (isDrawingRightCylinder)
	//{
	//	ofPushMatrix();
	//	ofTranslate(1200, (ofGetHeight() / 2), 100);
	//	ofRotateY(80);
	//	drawRightCylinder();
	//}

	if (this->scale != 1.0)
	{
		ofPopMatrix();
	}
}


// ********************************************************************************************************************
void CylinderDisplay::drawTexturedCylinder()
{
	if (this->totemVideoSource)
	{
		ofEnableDepthTest();
		ofPushMatrix();

		ofTranslate(ofGetWidth() / 2, (ofGetHeight() / 2), 100);
		ofRotateY(this->viewRotationAngle);
		this->totemVideoSource->getTextureReference().bind();
		cylinder.draw();
		this->totemVideoSource->getTextureReference().unbind();

		ofPopMatrix();
		ofDisableDepthTest();
	}
}


// ********************************************************************************************************************
void CylinderDisplay::drawLeftCylinder()
{
	if (this->totemVideoSource)
	{
		float A = 0.90;
		float B = 1.0 - A;
		currentLeftCylinder = A * currentLeftCylinder + B * targetLeftCylinder;

		this->totemVideoSource->getTextureReference().bind();
		leftCylinderPiece.draw();
		this->totemVideoSource->getTextureReference().unbind();
	}
}


// ********************************************************************************************************************
void CylinderDisplay::drawRightCylinder()
{
	if (this->totemVideoSource)
	{
		float A = 0.90;
		float B = 1.0 - A;
		currentRightCylinder = A * currentRightCylinder + B * targetRightCylinder;

		this->totemVideoSource->getTextureReference().bind();
		rightCylinderPiece.draw();
		this->totemVideoSource->getTextureReference().unbind();
	}
}


// ********************************************************************************************************************
void CylinderDisplay::createCylinderPiece(ofMesh &m, float radius, float height, float degrees)
{

	m.setupIndicesAuto();

	ofVec2f lastCoord;
	int f = 0;
	for (float deg = 0; deg < degrees; deg += 1.0, f += 4)
	{
		float ca = deg * 0.01745329252;//cache current angle
		float na = (deg + 1) * 0.01745329252;//cache next angle (could do with a conditional and storing previous angle)
		float ccos = cos(ca);//current cos
		float csin = sin(ca);//current sin
		float ncos = cos(na);//next cos
		float nsin = sin(na);//next sin

		ofVec3f tl(radius * ccos, height * .5, radius * csin);//top left = current angle, positive y
		ofVec3f bl(radius * ccos, -height * .5, radius * csin);//bottom left = current angle, negative y
		ofVec3f tr(radius * ncos, height * .5, radius * nsin);//top right = next angle, positive y
		ofVec3f br(radius * ncos, -height * .5, radius * nsin);//bottom right = next angle, negative y

		/*
		tl--tr
		|  /|
		| / |
		|/  |
		bl--br
		*/

		// 6 verts
		m.addVertex(tl);
		m.addTexCoord(ofVec2f(radius * ccos, 380));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f(radius * ncos, 380));
		m.addVertex(bl);
		m.addTexCoord(ofVec2f(radius * ccos, 700));

		m.addVertex(bl);
		m.addTexCoord(ofVec2f(radius * ccos, 700));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f(radius * ncos, 380));
		m.addVertex(br);
		m.addTexCoord(ofVec2f(radius * ncos, 700));

		lastCoord.set(ofVec2f(radius * ncos, 700));
	}

	prevTcoordLCP.set(0, 0, lastCoord.x, lastCoord.y);
	prevTcoordRCP.set(0, 0, lastCoord.x, lastCoord.y);

}


// ********************************************************************************************************************
void CylinderDisplay::mapTexCoords(ofMesh &m, float u1, float v1, float u2, float v2)
{
	//for (int j = 0; j < m.getNumTexCoords(); j++)
	//{
	//	ofVec2f tcoord = m.getTexCoord(j);
	//	tcoord.x = ofMap(tcoord.x, prevTcoordRCP.x, prevTcoordRCP.z, u1, u2, true);
	//	m.setTexCoord(j, tcoord);
	//}

	m.clearTexCoords();
	m.clearVertices();

	int f = 0;
	for (float deg = 0; deg < 44.0; deg += 1.0, f += 4)
	{
		float ca = deg * 0.01745329252;//cache current angle
		float na = (deg + 1) * 0.01745329252;//cache next angle (could do with a conditional and storing previous angle)
		float ccos = cos(ca);//current cos
		float csin = sin(ca);//current sin
		float ncos = cos(na);//next cos
		float nsin = sin(na);//next sin

		ofVec3f tl(warpedW * ccos, 1080, warpedW * csin);//top left = current angle, positive y
		ofVec3f bl(warpedW * ccos, -1080, warpedW * csin);//bottom left = current angle, negative y
		ofVec3f tr(warpedW * ncos, 1080, warpedW * nsin);//top right = next angle, positive y
		ofVec3f br(warpedW * ncos, -1080, warpedW * nsin);//bottom right = next angle, negative y

		/*
		tl--tr
		|  /|
		| / |
		|/  |
		bl--br
		*/

		// 6 verts
		m.addVertex(tl);
		m.addTexCoord(ofVec2f((warpedW * ccos) - (warpedW - u1), 700));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f((warpedW * ncos) - (warpedW - u1), 700));
		m.addVertex(bl);
		m.addTexCoord(ofVec2f((warpedW * ccos) - (warpedW - u1), 380));

		m.addVertex(bl);
		m.addTexCoord(ofVec2f((warpedW * ccos) - (warpedW - u1), 380));
		m.addVertex(tr);
		m.addTexCoord(ofVec2f((warpedW * ncos) - (warpedW - u1), 700));
		m.addVertex(br);
		m.addTexCoord(ofVec2f((warpedW * ncos) - (warpedW - u1), 380));

	}
}


// ********************************************************************************************************************
void CylinderDisplay::findLeftFace()
{
	this->totemVideoSource->getPixelsRef().cropTo(cropped, 0, 380, 1920, 700);
	finder.update(cropped);

	ofRectangle leftMost(1920, 0, 1920, 0);

	for (int i = 0; i < finder.size(); i++) {
		ofRectangle object = finder.getObjectSmoothed(i);
		if (object.getTopLeft().x < leftMost.getTopLeft().x && object.x > 0) {
			leftMost = object;

			cout << leftMost << endl;
		}
	}

	if (leftMost.x > 1920) {
		leftMost.x = 1920;
	}

	//mapTexCoords(leftCylinderPiece, leftMost.x + 100, 380, leftMost.x + PIECE_TEXCOORD_WIDTH - 100, 700);
	mapTexCoords(leftCylinderPiece, max(400, min(1920, (int)leftMost.x + 200)), 380, 0, 700);
}


// ********************************************************************************************************************
void CylinderDisplay::findRightFace()
{
	this->totemVideoSource->getPixelsRef().cropTo(cropped, 0, 380, 1920, 700);
	finder.update(cropped);

	ofRectangle rightMost;

	for (int i = 0; i < finder.size(); i++) {
		ofRectangle object = finder.getObject(i);
		if (object.x > rightMost.x && object.x < 1920) {
			rightMost = object;
			rightMost.width = object.width;
			cout << rightMost << endl;

		}
	}

	if (rightMost.x < 700) {
		rightMost.x = 700;
	}

	mapTexCoords(rightCylinderPiece, min(1920, max(500, (int)rightMost.x + 200)), 380, 0, 700);
}


// ********************************************************************************************************************
void CylinderDisplay::DragStart(ofPoint screenPosition)
{
	if (this->isDragging)
	{
		return;
	}

	this->isDragging = true;
	this->dragStart = screenPosition;
	this->startDragAngle = this->viewRotationAngle;
}


// ********************************************************************************************************************
void CylinderDisplay::DragMove(ofPoint screenPosition)
{
	if (!this->isDragging)
	{
		return;
	}

	auto xDelta = screenPosition.x - this->dragStart.x;
	auto adj = -0.125f;

	//auto testAdj1 = -(float)this->windowWidth / this->warpedW;
	//auto testAdj2 = -(float)this->windowWidth / this->cylinderCircumference / 2.5;
	
	if (windowWidth == 1920)
	{
		adj = -(float)this->windowWidth / this->cylinderCircumference / 2.5;
	}

	auto angleDelta = (xDelta * adj);

	this->viewRotationAngle = this->startDragAngle + angleDelta;
	while (this->viewRotationAngle >= 360)
	{
		this->viewRotationAngle -= 360;
	}
	while (this->viewRotationAngle < 0)
	{
		this->viewRotationAngle += 360;
	}
}


// ********************************************************************************************************************
void CylinderDisplay::DragEnd(ofPoint screenPosition)
{
	if (!this->isDragging)
	{
		return;
	}

	this->isDragging = false;
}
