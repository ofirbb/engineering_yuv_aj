/**
int LightFieldClass::findImageFromPose(Point2f uvCoord) {

	lightfieldStructUnit neighbors[4];

	findClosestNeighbors(this, uvCoord, neighbors);

	//interpolation1
	float diffXLeftTop = uvCoord.x - neighbors[0].position.x;
	float diffXRightTop = neighbors[1].position.x - uvCoord.x;
	float diffXLeftBottom = uvCoord.x - neighbors[2].position.x;
	float diffXRightBottom = neighbors[3].position.x - uvCoord.x;


	float alpha1 = diffXLeftTop / (diffXLeftTop + diffXRightTop);
	float alpha2 = diffXRightTop / (diffXLeftTop + diffXRightTop);
	float alpha3 = diffXLeftBottom / (diffXLeftBottom + diffXRightBottom);
	float alpha4 = diffXRightBottom / (diffXLeftBottom + diffXRightBottom);

	Point2f interp1, interp2;
	interp1.y = interp2.y = uvCoord.y;

	float slope1 = (closestCoord1.x - closestCoord2.x) / (closestCoord1.y - closestCoord2.y);
	interp1.x = interp1.y / slope1;

	float slope2 = (closestCoord3.x - closestCoord4.x) / (closestCoord3.y - closestCoord4.y);
	interp2.x = interp2.y / slope2;


	return SUCCESS;


}

*/